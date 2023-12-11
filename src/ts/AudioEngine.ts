import { audioModuleWasInit, initAudioModule } from "./emaudio/AudioModule";
import { EmBuffer } from "./emaudio/EmBuffer";
import { ParameterMgr } from "./params/ParameterMgr";
import { Audio } from "./emaudio/AudioModule";

const registry = new FinalizationRegistry((heldValue) => {
    if (heldValue instanceof Audio.AudioEngine) {
        heldValue.delete();
    } else {
        console.error("Failed to finalize AudioEngine object: Wrong " +
            "type was passed to the finalization registry.");
    }
});

export
class AudioEngine
{
    engine: InsoundAudioEngine;

    // container managing the current track data
    trackData: EmBuffer;
    updateHandler: (() => void) | null;
    params: ParameterMgr;

    constructor()
    {
        // Ensure WebAssembly module was initialized
        if (!audioModuleWasInit())
        {
            throw Error("Cannot instantiate AudioEngine without first " +
                "initializing AudioModule");
        }

        this.params = new ParameterMgr;

        this.engine = new Audio.AudioEngine({
            setParam: this.params.handleParamReceive,
            getParam: (nameOrIndex: string | number) => {
                return this.params.get(nameOrIndex).value;
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
    loadTrack(buffer: ArrayBuffer, script: string)
    {
        this.trackData.alloc(buffer, Audio);

        try {
            this.engine.loadBank(this.trackData.ptr, buffer.byteLength);
            const result = this.engine.loadScript(script);
            if (result)
                console.error("Lua Script error:", result);
            this.params.load(this);
        }
        catch (err)
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
        if (!data.isLoaded) return false;

        this.engine.loadBank(data.ptr, data.size);

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
        if (this.engine.getChannelCount() !== volumes.length)
            throw Error(`setMixPreset error: volumes array length [${volumes.length}] does not match track count [${this.engine.getChannelCount()}]`);

        for (let i = 0; i < volumes.length; ++i)
            this.engine.fadeChannelTo(i, volumes[i], seconds);
    }

    /**
     * Unload the currently loaded track data. Safe to call if already unloaded.
     */
    unloadTrack()
    {
        this.engine.unloadBank();
        this.trackData.free();
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

    get position(): number
    {
        return this.engine.getPosition();
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
        callback: (label: string, seconds: number) => void): void
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
