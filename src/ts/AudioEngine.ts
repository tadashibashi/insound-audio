import { audioModuleWasInit } from "./emaudio/AudioModule";
import { EmBuffer } from "./emaudio/EmBuffer";
import { ParameterMgr } from "./params/ParameterMgr";
import { Audio } from "./emaudio/AudioModule";
import { SyncPointMgr } from "./SyncPointMgr";
import { MixPresetMgr } from "./MixPresetMgr";

const registry = new FinalizationRegistry((heldValue) => {
    if (heldValue instanceof Audio.AudioEngine) {
        heldValue.delete();
    } else {
        console.error("Failed to finalize AudioEngine object: Wrong " +
            "type was passed to the finalization registry.");
    }
});

export interface LoadOptions
{
    script?: string;
    loopstart?: number;
    loopend?: number;
}

export
class AudioEngine
{
    readonly engine: InsoundAudioEngine;

    // container managing the current track data
    trackData: EmBuffer;
    soundsData: EmBuffer[];
    updateHandler: (() => void) | null;
    params: ParameterMgr;
    points: SyncPointMgr;
    presets: MixPresetMgr;

    private m_lastPosition: number;
    private m_position: number;

    get lastPosition() {
        return this.m_lastPosition;
    }

    get position()
    {
        return this.m_position;
    }

    constructor()
    {
        // Ensure WebAssembly module was initialized
        if (!audioModuleWasInit())
        {
            throw Error("Cannot instantiate AudioEngine without first " +
                "initializing AudioModule");
        }

        this.params = new ParameterMgr;
        this.points = new SyncPointMgr;
        this.presets = new MixPresetMgr;
        this.engine = new Audio.AudioEngine({
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

        if (!this.engine.init())
        {
            this.engine.delete();
            throw new Error("Failed to initialize AudioEngine");
        }

        this.trackData = new EmBuffer();
        this.updateHandler = null;

        registry.register(this, this.engine, this);

        this.m_lastPosition = 0;
        this.m_position = 0;
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
        this.trackData.alloc(buffer, Audio);

        try {
            this.engine.loadBank(this.trackData.ptr, buffer.byteLength);
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
            console.error(err);
            this.trackData.free();
        }
    }

    loadSounds(buffers: ArrayBuffer[], opts: LoadOptions = {
        script: ""
    })
    {
        const size = buffers.reduce(
            (accum, buffer) => buffer.byteLength + accum, 0);
        const buf = new Uint8Array(size);

        let ptrOffset = 0;
        for (let i = 0; i < buffers.length; ++i)
        {
            const curBuf = new Uint8Array(buffers[i]);
            buf.set(curBuf, ptrOffset);
            ptrOffset += curBuf.length;
        }

        this.trackData.alloc(buf, Audio);

        try {
            const ptr = this.trackData.ptr;
            ptrOffset = 0;
            for (let i = 0; i < size; ++i)
            {
                this.engine.loadSound(ptr + ptrOffset, buffers[i].byteLength);
                ptrOffset += buffers[i].byteLength;
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
            console.error(err);
            this.trackData.free();
        }

    }

    /**
     * Reload the bank data that is already loaded.
     */
    reload()
    {
        const data = this.trackData;
        if (data.isNull) return false;

        this.engine.loadBank(data.ptr, data.size);
        this.params.load(this);
        this.points.update(this);
        this.m_lastPosition = 0;
        this.m_position = 0;

        return true;
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

    resume()
    {
        this.engine.resume();
    }

    update()
    {
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

    /**
     * Call this manually when no longer using the AudioEngine to clean
     * up underlying AudioEngine object. A finalization registry is used on
     * this object, but the standard does not guarantee that the finalization
     * callback gets called, so it's recommended to call this manually.
     */
    release()
    {
        this.unloadTrack();
        this.engine.delete();
    }
}
