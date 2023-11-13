export type pointer = number;

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

    /**
     * Update audio engine state, should be called at least 20 times per second
     */
    update(): void;

    /**
     * Load bank. Data should be available for the lifetime of the
     * bank's usage, and should be managed externally, as this class does not
     * take ownership of it.
     *
     * @param  dataPtr    const pointer to data
     * @param  bytelength size of data
     *
     */
    loadBank(dataPtr: pointer, bytelength: number): void;

    /**
     * Unload bank from storage. Data should be freed externally as this class
     * does not take ownership of it.
     */
    unloadBank(): void;

    /**
     * Check if bank is currently loaded
     *
     * @return whether bank data is loaded.
     */
    isBankLoaded(): boolean;
}

export interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (): AudioEngine;
    }
}
