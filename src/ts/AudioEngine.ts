import type { InsoundAudioModule, AudioEngine as Engine } from "./InsoundAudioModule";
import Module from "../../build/insound-audio";
import { EmHelper } from "./emscripten";

const Audio = {} as InsoundAudioModule;
let registry: FinalizationRegistry<Engine>;

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
        console.log(Audio);

    } catch(err) {
        console.error(err);
        throw err;
    }

    registry = new FinalizationRegistry((heldValue) => {
        if (heldValue instanceof Audio.AudioEngine) {
            heldValue.delete();
        } else {
            console.error("Failed to finalize AudioEngine object: Wrong " +
                "type was passed to the finalization registry.");
        }
    });
}


export
class AudioEngine {
    engine: Engine;

    // pointer to current bank memory
    currentBank: number;

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

        registry.register(this, this.engine, this);
    }

    /**
     * Load fsbank data
     *
     * @param {ArrayBuffer} buffer - data buffer
     *
     */
    loadBank(buffer: ArrayBuffer)
    {
        this.unloadBank();

        const ptr = EmHelper.allocBuffer(Audio, buffer);

        try {
            this.engine.loadBank(ptr, buffer.byteLength);
            this.currentBank = ptr;
        }
        catch (err)
        {
            console.error(err);
            EmHelper.free(Audio, ptr);
        }
    }

    /**
     * Unload the currently loaded bank data. Safe to call if already unloaded.
     */
    unloadBank()
    {
        // Check if already unloaded
        if (this.currentBank === NULL) return;

        // Free data
        EmHelper.free(Audio, this.currentBank);
        this.currentBank = NULL;
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

    /**
     * Call this manually when no longer using the AudioEngine to clean
     * up underlying AudioEngine object. A finalization registry is used on
     * this object, but the standard does not guarantee that the finalization
     * callback gets called, so it's recommended to call this manually.
     */
    release()
    {
        this.unloadBank();
        this.engine.delete();
    }
}
