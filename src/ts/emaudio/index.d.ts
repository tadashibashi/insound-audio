type LuaCallbacks = import("./LuaCallbacks").LuaCallbacks;
type ParamType = import("../params/ParamType").ParamType;

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
     * Load an individual sound. Adds a layer. If a bank was previously loaded
     * with `AudioEngine#loadBank` it will be unloaded here.
     * Please make sure to keep data alive for the lifetime of its usage.
     *
     * @param dataPtr  - const pointer to data
     * @param bytelength - size of data
     */
    loadSound(dataPtr: pointer, bytelength: number): void;

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
     * Set the main bus reverb send level
     *
     * @param level - the send level to set
     */
    setMainReverbLevel(level: number): void;

    /**
     * Get the main bus reverb send level
     *
     * @return main bus reverb send level.
     */
    getMainReverbLevel(): number;

    setMainPanLeft(level: number): void;
    setMainPanRight(level: number): void;
    setMainPan(left: number, right: number): void;
    getMainPanLeft(): number;
    getMainPanRight(): number;

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


    // ----- Individual channel controls --------------------------------------

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
     * Set channel reverb send level
     *
     * @param {number} ch    - the channel index to target (0-indexed)
     * @param {number} level - the channel's reverb level to set
     */
    setChannelReverbLevel(ch: number, level: number): void;

    /**
     * Get a channel's reverb send level
     *
     * @param  ch - channel index to target (0-indexed)
     * @return the channel's current reverb send level
     */
    getChannelReverbLevel(ch: number): number;

    setChannelPanLeft(ch: number, level: number): void;
    setChannelPanRight(ch: number, level: number): void;
    setChannelPan(ch: number, left: number, right: number): void;
    getChannelPanLeft(ch: number): number;
    getChannelPanRight(ch: number): number;

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


    // ----- Looping ----------------------------------------------------------

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
     * Set loop start and end points (in number of seconds).
     */
    setLoopSeconds(loopstart: number, loopend: number): void;

    /**
     * Set loop start and end points (in number of pcm audio samples).
     */
    setLoopSamples(loopstart: number, loopend: number): void;

    /**
     * Get loop start and end points in number of seconds.
     */
    getLoopSeconds(): {loopstart: number, loopend: number};

    /**
     * Get loop start and end points in number of pcm audio samples.
     */
    getLoopSamples(): {loopstart: number, loopend: number};


    /**
     * Set the callback that fires when track has reached the end
     * @param callback - the callback to set
     *
     */
    setEndCallback(callback: () => void): void;

    // ----- Parameters -------------------------------------------------------
    // Parameters are global for the whole track and not tied to specific
    // channels. However, a parameter may influence specific channel instances
    // if so defined in the lua script implementation by the user.

    /**
     * Get the number of parameters in the current track.
     */
    param_count(): number;

    /**
     * Get a parameter name by index
     *
     * @param index - zero-based index of the parameter to query
     */
    param_getName(index: number): string;

    /**
     * Get the type of parameter of a parameter by index
     * @param index - zero-based index of the parameter to query
     *
     * @returns enumeration of the parameter type. @see ParamType
     */
    param_getType(index: number): ParamType;

    /**
     * Get parameter and cast it to a number parameter.
     *
     * @throws error if the type is wrong.
     *
     * @param  index -zero-based index of the parameter to query
     */
    param_getAsNumber(index: number): NumberParam;

    /**
     * Get parameter and cast it to a number parameter.
     *
     * @param  index -zero-based index of the parameter to query
     */
    param_getAsStrings(index: number): StringsParam;

    /**
     * Send a parameter set to the lua script
     *
     * @param index - zero-based index of the parameter to query
     * @param value - value to set
     */
    param_send(index: number, value: number): void;


    // ----- Sync points ------------------------------------------------------
    // Note: sync points are called "markers" on the frontend

    /**
     * Get the number of SyncPoints currently available in the loaded sound
     */
    getSyncPointCount(): number;

    /**
     * Get the number of offset seconds for the syncpoint with the specified
     * index.
     *
     * @param  {number} index - 0-based index, see `getSyncPointCount` to get
     *                          max number of sync points available.
     */
    getSyncPointOffsetSeconds(index: number): number;

    /**
     * Get the label for the syncpoint with the specified index.
     * @param  {numer}  index - 0-based index; see `getSyncPointCount` to get
     *                          the max number of sync points available.
     */
    getSyncPointLabel(index: number): string;

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
        callback: (label: string, offsetSeconds: number, index: number) =>
            void): void;
}

declare interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (callbacks: LuaCallbacks): InsoundAudioEngine;
    }

    ParamType: ParamType;
}

declare const AudioModule: (module: any) => Promise<void>;
