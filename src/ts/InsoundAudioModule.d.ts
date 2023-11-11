export interface AudioEngine {
    /**
     * Initialize the Audio Engine. Must be called before any other function of
     * the audio engine is called.
     *
     * @returns whether initialization was successful.
     */
    init(): boolean;

    /**
     * Close the Audio Engine.
     *
     * @throws FMODError if there was an error.
     */
    close(): void;

    /**
     * Explicitly delete the underlying Audio Engine object.
     * Please call this when done with the object. The browser will attempt to
     * use FinalizationRegistry, but this call is not guaranteed by the
     * standard, so it is recommended to also call this manually.
     */
    delete(): void;

    /**
     * Resume the audio engine.
     *
     * @throws FMODError if there was an error.
     */
    resume(): void;

    /**
     * Pause the audio engine.
     */
    suspend(): void;
}

export interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (): AudioEngine;
    }
}
