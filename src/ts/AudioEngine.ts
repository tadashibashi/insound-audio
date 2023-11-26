import { EmHelper } from "./emscripten";

const Audio = {} as InsoundAudioModule;

type pointer = number;
const NULL: pointer = 0;

export
async function initAudio()
{
    if (Audio._free)
    {
        console.warn("initAudio: Audio module was already initialized.");
        return;
    }

    try {
        await new Promise<void>( (resolve, reject) => {
            const timeout = setTimeout(reject, 7500); // timeout if it doesn't load in 7.5 seconds.
            Audio.onRuntimeInitialized = () => {
                clearTimeout(timeout);
                resolve();
            };

            AudioModule(Audio);
        });

    } catch(err) {
        console.error(err);
        throw err;
    }
}

interface PointerWrapper {
    ptr: pointer;
}

/**
 * Wrapper for WebAssembly memory pointer (in our use for an FSBank)
 */
export
class FSBank
{
    static registry: FinalizationRegistry<PointerWrapper>;
    track: PointerWrapper;

    constructor() {
        this.track = {ptr: NULL};

        FSBank.registry.register(this, this.track, this);
    }

    load(buffer: ArrayBuffer)
    {
        const ptr = EmHelper.allocBuffer(Audio, buffer);
        this.unload();
        this.track.ptr = ptr;
    }

    unload()
    {
        if (this.track.ptr === NULL) return;

        EmHelper.free(Audio, this.track.ptr);

        this.track.ptr = NULL;
    }
}

FSBank.registry = new FinalizationRegistry<PointerWrapper>((heldValue) => {
    if (heldValue && heldValue.ptr !== undefined)
    {
        EmHelper.free(Audio, heldValue.ptr);
    }
    else
    {
        console.error("Failed to finalize MultiLayeredTrack: Wrong type was " +
            "passed to the FinalizationRegistry.");
    }
});


export
class AudioEngine
{
    static registry: FinalizationRegistry<InsoundAudioEngine>;

    engine: InsoundAudioEngine;

    // container managing the current track data
    track: FSBank;
    updateHandler: (() => void) | null;

    constructor()
    {
        // Ensure WebAssembly module was initialized
        if (Audio._free === undefined)
        {
            throw Error("initAudio must be successfully called before " +
                "using instantiating AudioEngine");
        }

        this.engine = new Audio.AudioEngine();
        if (!this.engine.init())
        {
            this.engine.delete();
            throw new Error("Failed to initialize AudioEngine");
        }

        this.track = new FSBank();
        this.updateHandler = null;

        AudioEngine.registry.register(this, this.engine, this);
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
    loadTrack(buffer: ArrayBuffer)
    {
        this.track.load(buffer);

        try {
            this.engine.loadBank(this.track.track.ptr, buffer.byteLength);
        }
        catch (err)
        {
            console.error(err);
            this.track.unload();
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
        if (this.engine.trackCount() !== volumes.length)
            throw Error(`setMixPreset error: volumes array length [${volumes.length}] does not match track count [${this.engine.trackCount()}]`);

        for (let i = 0; i < volumes.length; ++i)
            this.engine.fadeChannelTo(i, volumes[i], seconds);
    }

    /**
     * Unload the currently loaded track data. Safe to call if already unloaded.
     */
    unloadTrack()
    {
        this.engine.unloadBank();
        this.track.unload();
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

    getChannelVolume(ch: number): number
    {
        return this.engine.getChannelVolume(ch);
    }

    setChannelVolume(ch: number, volume: number)
    {
        this.engine.setChannelVolume(ch, volume);
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

    get trackCount(): number
    {
        return this.engine.trackCount();
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

AudioEngine.registry = new FinalizationRegistry((heldValue) => {
    if (heldValue instanceof Audio.AudioEngine) {
        heldValue.delete();
    } else {
        console.error("Failed to finalize AudioEngine object: Wrong " +
            "type was passed to the finalization registry.");
    }
});
