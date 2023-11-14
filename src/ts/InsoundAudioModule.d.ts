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

    /**
     * Play track if any loaded.
     */
    play(): void;

    /**
     * Stop track
     */
    stop(): void;

    /**
     * Set the paused status of track
     *
     * @param pause - whether to pause
     *
     */
    setPause(pause: boolean): void;

    /**
     * Get paused status of track
     *
     * @return whether track is paused.
     *
     */
    getPause(): boolean;

    /**
     * Seek to a position in the track
     *
     * @param seconds - number of seconds in the track to seek to
     *
     */
    seek(seconds: number): void;

    /**
     * Get the track's current position in seconds or -1 if track is not loaded
     */
    getPosition(): number;

    /**
     * Get the currently loaded track's length in seconds
     * @return track length in seconds.
     */
    getLength(): number;

    /**
     * Set the main volume
     * @param volume - level where 0 is off and 1 is 100%
     */
    setMainVolume(volume: number): void;

    /**
     * Get the main volume level
     *
     * @return level of the main volume, where 0 is off and 1 is 100%
     *
     */
    getMainVolume(): number;

    /**
     * Set a specific channel's volume. For now, it gets reset every time
     * play is called, so it should be called during every call to start if
     * playback should begin at a differeing level.
     *
     * @param ch       - target channel to set
     * @param volume   - volume level to set, where 0 is off and 1 is
     *                   100%
     */
    setChannelVolume(ch: number, volume: number): void;

    /**
     * Get a channel's current volume. If channel is currently unavailable such
     * as when the track is not loaded or not playing, or if it's an invalid
     * index, -1 will be returned.
     *
     * @param  ch   - target channel to get the volume of
     * @return      volume level of target channel where 0 is off and 1 is 100%
     *
     */
    getChannelVolume(ch: number): number;


    /**
     * Get the number of tracks loaded in the currently loaded fsb.
     */
    trackCount(): number;
}

export interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (): AudioEngine;
    }
}
