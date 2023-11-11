import type { InsoundAudioModule, AudioEngine as Engine } from "./InsoundAudioModule";
import Module from "../../build/insound-audio";

const Audio = {} as InsoundAudioModule;

try {
    await Module(Audio);
} catch(err) {
    console.error(err);
    throw err;
}

const registry = new FinalizationRegistry((heldValue) => {
    if (heldValue instanceof Audio.AudioEngine) {
        heldValue.delete();
    } else {
        console.error("Failed to finalize AudioEngine object: Wrong " +
            "type was passed to the finalization registry.");
    }
});

export
class AudioEngine {
    engine: Engine;
    constructor() {
        this.engine = new Audio.AudioEngine();
        if (!this.engine.init())
        {
            this.engine.delete();
            throw new Error("Failed to initialize AudioEngine");
        }

        registry.register(this, this.engine, this);
    }

    suspend() {
        this.engine.suspend();
    }

    resume() {
        this.engine.resume();
    }

    /**
     * Call this manually when no longer using the AudioEngine to clean
     * up underlying AudioEngine object. A finalization registry is used on
     * this object, but the standard does not guarantee that the finalization
     * callback gets called, so it's recommended to call this manually.
     */
    release() {
        this.engine.delete();
    }
}
