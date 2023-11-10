interface AudioEngine {
    /**
     * Initialize the Audio Engine. Must be called before any other function of
     * the audio engine is called.
     *
     * @returns whether initialization was successful.
     */
    init(): boolean;

    /**
     * Close the Audio Engine.
     */
    close(): void;

    /**
     * Explicitly delete the underlying Audio Engine object.
     * Please call this when destroying the object.
     */
    delete(): void;
    resume(): void;
    suspend(): void;
}

declare interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (): AudioEngine;
    }
}
