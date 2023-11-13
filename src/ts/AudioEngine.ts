import type { InsoundAudioModule, AudioEngine as Engine } from "./InsoundAudioModule";
import Module from "../../build/insound-audio";
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
            Module(Audio);
        });

    } catch(err) {
        console.error(err);
        throw err;
    }
}

interface PointerWrapper {
    ptr: pointer;
}

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
    static registry: FinalizationRegistry<Engine>;

    engine: Engine;

    // container managing the current track data
    track: FSBank;

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

        AudioEngine.registry.register(this, this.engine, this);
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
     * Unload the currently loaded track data. Safe to call if already unloaded.
     */
    unloadTrack()
    {
        this.track.unload();
    }

    /**
     * Check if bank is loaded
     */
    isBankLoaded()
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
        this.engine.play();
    }

    setPause(pause: boolean)
    {
        this.engine.setPause(pause);
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
