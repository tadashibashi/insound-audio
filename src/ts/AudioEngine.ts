import { audioModuleWasInit, initAudioModule } from "./emaudio/AudioModule";
import { EmBufferGroup } from "./emaudio/EmBuffer";
import { ParameterMgr } from "./params/ParameterMgr";
import { getAudioModule } from "./emaudio/AudioModule";
import { SyncPointMgr } from "./SyncPointMgr";
import { MixPresetMgr } from "./MixPresetMgr";
import { SpectrumAnalyzer } from "./SpectrumAnalyzer";

const registry = new FinalizationRegistry((heldValue: any) => {
    // `heldValue` should be an Emscripten Embind C++ object with its own
    // delete function, which explicitly frees memory of this object.
    if (typeof heldValue.delete === "function") {
        heldValue.delete();
    }
});

// Get this info from a database to populate a new track with
export interface LoadOptions
{
    script?: string;
    loopstart?: number;
    loopend?: number;
}

export class SoundLoadError extends Error
{

    soundIndices: number[];

    getErrorMessage(names: string[]): string[] {
        return this.soundIndices.map(index => names[index] || "Unknown File");
    }

    constructor(indices: number[])
    {
        super();
        this.soundIndices = indices;
    }
}

export class AudioEngine
{
    private module: InsoundAudioModule;
    engine: InsoundAudioEngine;

    // container managing the current track data
    trackData: EmBufferGroup;
    updateHandler: (() => void) | null;
    params: ParameterMgr;
    points: SyncPointMgr;
    presets: MixPresetMgr;
    spectrum: SpectrumAnalyzer;

    private m_lastPosition: number;
    private m_position: number;
    private m_isResetting: boolean;

    get lastPosition() {
        return this.m_lastPosition;
    }

    get position()
    {
        return this.m_position;
    }

    /**
     * Restart the underlying Emscripten audio module and replace the
     * underlying AudioEngine with a new one. To be used when encountering
     * an unrecoverable critical error pertaining to the module.
     */
    async reset()
    {
        if (this.m_isResetting) return;

        this.m_isResetting = true;
        try {
            registry.unregister(this);
            this.params.clear();
            this.points.clear();
            this.presets.clear();
            this.trackData.free();
            this.spectrum.close();
            this.engine.delete();
        }
        finally
        {
            try {
                this.module = await initAudioModule();
                this.spectrum.setModule(this.module);
                this.setAudioEngine(this.module);
                registry.register(this, this.engine, this);
                this.engine.init();
            }
            finally
            {
                this.m_isResetting = false;
            }
        }
    }

    private setAudioEngine(module: InsoundAudioModule)
    {
        this.m_isResetting = false;
        this.engine = new module.AudioEngine({
            setParam: this.params.handleParamReceive,
            getParam: (nameOrIndex: string | number) => {
                return this.params.get(nameOrIndex).value;
            },
            syncpointUpdated: () => {
                this.points.update(this);
            },
            addPreset: (name, volumes) => {
                this.presets.add(name, volumes);
            },
            applyPreset: (indexOrName, seconds) => {
                let preset: {name: string, volumes: number[]};

                if (typeof indexOrName === "string")
                {
                    preset = this.presets.getByName(indexOrName);
                }
                else
                {
                    preset = this.presets.getByIndex(indexOrName);
                }

                this.setMixPreset(preset.volumes, seconds);
            },
            getPreset: (indexOrName) => {
                return (typeof indexOrName === "string") ?
                    this.presets.getByName(indexOrName) :
                    this.presets.getByIndex(indexOrName);
            },
            getPresetCount: () => {
                return this.presets.presets.length;
            }
        });
    }

    constructor()
    {
        // Ensure WebAssembly module was initialized
        if (!audioModuleWasInit())
        {
            throw Error("Cannot instantiate AudioEngine without first " +
                "initializing AudioModule");
        }

        this.module = getAudioModule();
        this.spectrum = new SpectrumAnalyzer(this.module);
        this.params = new ParameterMgr;
        this.points = new SyncPointMgr;
        this.presets = new MixPresetMgr;
        this.setAudioEngine(this.module);

        if (!this.engine.init())
        {
            this.engine.delete();
            throw new Error("Failed to initialize AudioEngine");
        }

        this.trackData = new EmBufferGroup();
        this.updateHandler = null;

        registry.register(this, this.engine, this);

        this.m_lastPosition = 0;
        this.m_position = 0;
    }

    /** Get the current WebAudio context */
    get context()
    {
        return this.module.mContext;
    }

    /**
     * Set the on update callback, gets called 60fps+ when updating sound engine.
     *
     * @param callback - callback to set.
     */
    onUpdate(callback: () => void): void {
        this.updateHandler = callback;
    }

    /**
     * Load fsbank data
     *
     * @param {ArrayBuffer} buffer - data buffer
     *
     */
    loadTrack(buffer: ArrayBuffer, opts: LoadOptions = {
        script: ""
    })
    {
        this.trackData.free();
        this.trackData.alloc(buffer, this.module);

        try {
            this.engine.loadBank(this.trackData.data[0].ptr, buffer.byteLength);
            const result = this.engine.loadScript(opts.script || "");
            if (result)
                console.error("Lua Script error:", result);
            if (opts.loopend !== undefined && opts.loopstart !== undefined)
                this.engine.setLoopSeconds(opts.loopstart, opts.loopend);
            this.params.load(this);
            this.points.update(this);
            this.m_lastPosition = 0;
            this.m_position = 0;
        }
        catch (err)
        {
            this.trackData.free();

            throw err;
        }
    }

    loadSounds(buffers: ArrayBuffer[], opts: LoadOptions = {
        script: ""
    })
    {
        if (this.isTrackLoaded())
            this.unloadTrack();

        this.trackData.free();
        for (let i = 0; i < buffers.length; ++i)
        {
            this.trackData.alloc(buffers[i], this.module);
        }

        try {
            const problematicSounds: number[] = [];
            const length = this.trackData.data.length;
            for (let i = 0; i < length; ++i)
            {
                try {
                    this.engine.loadSound(this.trackData.data[i].ptr,
                        this.trackData.data[i].size);
                }
                catch(err)
                {
                    problematicSounds.push(i);
                    console.log(err);
                }
            }

            if (problematicSounds.length > 0)
            {
                throw new SoundLoadError(problematicSounds);
            }

            const result = this.engine.loadScript(opts.script || "");
            if (result)
                console.error("Lua Script error:", result);
            if (opts.loopend !== undefined && opts.loopstart !== undefined)
                this.engine.setLoopSeconds(opts.loopstart, opts.loopend);
            this.params.load(this);
            this.points.update(this);
            this.m_lastPosition = 0;
            this.m_position = 0;
        }
        catch(err)
        {
            this.reset();
            throw err;
        }
    }

    /**
     * Set a mix preset on the current track
     *
     * @param volumes - list of volumes to set each channel with
     * @param seconds - transition time in seconds
     */
    setMixPreset(volumes: number[], seconds: number)
    {
        const length = Math.min(this.channelCount, volumes.length);

        for (let i = 0; i < length; ++i)
        {

            this.engine.fadeChannelTo(i, volumes[i], seconds);
        }
    }

    /**
     * Unload the currently loaded track data. Safe to call if already unloaded.
     */
    unloadTrack()
    {
        this.engine.unloadBank();
        this.trackData.free();

        this.params.clear();
        this.points.points.length = 0;
        this.presets.presets.length = 0;
    }

    /**
     * Check if bank is loaded
     */
    isTrackLoaded()
    {
        return this.engine.isBankLoaded();
    }

    suspend()
    {
        this.engine.suspend();
    }

    isSuspended(): boolean
    {
        if (!this.module.mContext) return false;

        return this.module.mContext.state === "suspended";
    }

    resume()
    {
        this.engine.resume();
    }

    update()
    {
        if (this.m_isResetting) return;
        if (this.updateHandler)
            this.updateHandler();
        this.engine.update();

        this.m_lastPosition = this.m_position;
        this.m_position = this.engine.getPosition();
    }

    play()
    {
        this.engine.seek(0);
        this.engine.setPause(false, 0);
    }

    stop()
    {
        this.engine.seek(0);
        this.engine.setPause(true, .1);
    }

    setPause(pause: boolean, seconds: number=.1)
    {
        this.engine.setPause(pause, seconds);
    }

    seek(seconds: number)
    {
        // Wrap seconds about the track length
        const length = this.length;

        while (seconds < 0)
            seconds += length;
        while (seconds > length)
            seconds -= length;

        this.engine.seek(seconds);
    }

    get paused()
    {
        return this.engine.getPause();
    }

    get length(): number
    {
        return this.engine.getLength();
    }

    getMainVolume()
    {
        return this.engine.getMainVolume();
    }

    setMainVolume(volume: number)
    {
        this.engine.setMainVolume(volume);
        return this;
    }

    setMainReverbLevel(level: number)
    {
        this.engine.setMainReverbLevel(level);
        return this;
    }

    getMainReverbLevel(): number
    {
        return this.engine.getMainReverbLevel();
    }

    getChannelVolume(ch: number): number
    {
        return this.engine.getChannelVolume(ch);
    }

    setChannelVolume(ch: number, volume: number)
    {
        this.engine.setChannelVolume(ch, volume);
        return this;
    }

    getChannelReverbLevel(ch: number)
    {
        return this.engine.getChannelReverbLevel(ch);
    }

    setChannelReverbLevel(ch: number, level: number)
    {
        this.engine.setChannelReverbLevel(ch, level);
        return this;
    }

    fadeChannelTo(ch: number, volume: number, seconds: number)
    {
        this.engine.fadeChannelTo(ch, volume, seconds);
    }

    getChannelFadeLevel(ch: number, final: boolean = true)
    {
        this.engine.getChannelFadeLevel(ch, final);
    }

    get channelCount(): number
    {
        return this.engine.getChannelCount();
    }

    get looping(): boolean
    {
        return this.engine.isLooping();
    }

    setLooping(looping: boolean)
    {
        this.engine.setLooping(looping);
        return this;
    }

    fadeTo(to: number, seconds: number): AudioEngine
    {
        this.engine.fadeTo(to, seconds);
        return this;
    }

    getFadeLevel(final: boolean = true): number
    {
        return this.engine.getFadeLevel(final);
    }

    setSyncPointCallback(
        callback: (label: string, seconds: number, index: number) => void)
        : void
    {
        this.engine.setSyncPointCallback(callback);
    }

    setEndCallback(callback: () => void): void
    {
        this.engine.setEndCallback(callback);
    }

    getSampleData(index: number)
    {
        const data = this.engine.getSampleData(index);
        const begin = data.ptr/4;
        const end = begin + data.byteLength;
        return this.module.HEAPF32.subarray(begin, end);
    }

    /**
     * Call this manually when no longer using the AudioEngine to clean
     * up underlying AudioEngine object. A finalization registry is used on
     * this object, but the standard does not guarantee that the finalization
     * callback gets called, so it's recommended to call this manually.
     */
    release()
    {
        this.engine.delete();
    }
}
