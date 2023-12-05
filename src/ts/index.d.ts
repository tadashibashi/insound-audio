type ParamType = import("./params/ParamType").ParamType;

declare type pointer = number;

declare interface StringsParam {
    values: string[];
    defaultValue: number;
}

declare interface NumberParam {
    min: number,
    max: number,
    step: number,
    value: number;
    defaultValue: number;
}

declare interface InsoundAudioEngine {
    // ===== System Controls / Lifetime =======================================

    /**
     * Initialize the Audio Engine. Must be called before any other function of
     * the audio engine is called.
     *
     * @return whether initialization was successful.
     */
    init(): boolean;

    /**
     * Close the Audio Engine without deleting the underlying object. May call
     * `init` again to reinitialize after a successful call to `close`.
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


    // ===== Bank Loading =====================================================

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
     * Load a lua script, call after bank is loaded to receive params.
     *
     * @param   text - lua script string
     *
     * @return  error message - empty if no error.
     */
    loadScript(text: string): boolean;


    // ===== Channel Controls =================================================

    /**
     * Set the paused status of track
     *
     * @param pause   - whether to pause
     * @param seconds - number of seconds to fade in-out of pause state
     */
    setPause(pause: boolean, seconds: number): void;

    /**
     * Get paused status of track
     *
     * @return whether track is paused.
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
     *
     * @return      volume level of target channel where 0 is off and 1 is 100%
     *
     */
    getChannelVolume(ch: number): number;

    /**
     * Fade main bus from current fade volume to another in number of `seconds`
     *
     * Note that fade level is different from volume level - they are scaled
     * against each other. Use `getFadeLevel` to get the current fade level.
     */
    fadeTo(to: number, seconds: number): void;

    /**
     * Get the fade level of the main bus
     */
    getFadeLevel(final: boolean): number;

    /**
     * Fade channel to a level - this value is different from the chennel
     * volume, by which it is scaled against. Use `getChannelFadeLevel` to
     * get the fade level of a channel.
     *
     * @param ch      - channel index (0-indexed)
     * @param level   - volume level to fade to (0 = off, 1 = 100%)
     * @param seconds - time in seconds to fade (fade slope is linear)
     */
    fadeChannelTo(ch: number, level: number, seconds: number): void;

    /**
     * Get the current fade level of a specific channel
     *
     * @param   ch    - channel index (0-indexed)
     * @param   final - whether to get current fade level (true), or target
     *                  fade level (false)
     *
     * @return  fade level of the channel.
     */
    getChannelFadeLevel(ch: number, final: boolean): number;

    /**
     * Get the number of tracks loaded in the currently loaded fsb.
     */
    getChannelCount(): number;

    /**
     * Get whether track is set to loop. True by default, when loaded.
     */
    isLooping(): boolean;

    /**
     * Set track looping behavior
     *
     * @param looping - whether track should loop.
     */
    setLooping(looping: boolean): void;

    /**
     * Set the callback that fires when a syncpoint has been reached
     * @param callback - the callback to set with the following parameters:
     *
     * label         - syncpoint name / label text
     *
     * offsetSeconds - offset position of sync point in number of seconds
     *
     * Callback return value is discarded.
     */
    setSyncPointCallback(
        callback: (label: string, offsetSeconds: number) => void): void;

    /**
     * Set the callback that fires when track has reached the end
     * @param callback - the callback to set
     *
     */
    setEndCallback(callback: () => void): void;

    param_count(): number;
    param_getName(index: number): string;
    param_getType(index: number): ParamType;
    param_getAsNumber(index: number): NumberParam;
    param_getAsStrings(index: number): StringsParam;
    param_send(index: number, value: number): void;
}

declare interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (
            paramSet: (index: number, value: number) => void,
            paramGet: (nameOrIndex: string | number) => number
        ): InsoundAudioEngine;
    }

    ParamType: ParamType;
}

declare const AudioModule: (module: any) => Promise<void>;
