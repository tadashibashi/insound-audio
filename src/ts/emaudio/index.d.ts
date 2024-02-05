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

declare interface SampleDataInfo {
    ptr: number;
    byteLength: number;
}

declare interface Vector<T> {
    get(index: number): T;
    resize(size: number): void;
    push_back(item: T): void;
    size(): number;

    delete();
}

declare interface InsoundAudioEngine {

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

    /**
     * Create track and get pointer to the new instance. Use the returned
     * pointer as the argument for a new MultiTrackControl object.
     */
    createTrack(): number;

    /**
     * Delete track pointer. Make sure to delete the pointer retrieved from
     * createTrack when done with it to free memory.
     *
     * @param {number} ptr - pointer to the track object
     */
    deleteTrack(ptr: number): void;

    /**
     * Get volume level of the master bus.
     * @return volume level where 0: off, 1: 100%
     */
    getMasterVolume(): number;

    /**
     * Set volume level of the master bus.
     *
     * @param level - volume level where 0: off, 1: 100%
     */
    setMasterVolume(level: number): void;

    /**
     * Get the current estimated volume output level, taking multiple engine
     * parameters into account.
     */
    getAudibility(): number;


    /**
     * Get the total amount of CPU usage used by the audio engine.
     *
     * @return floating point number in percent from 0 to 100
     */
    getCPUUsageTotal(): number;

    /**
     * Get the total amount of CPU usage used by the audio engine DSPs.
     * This is a part of the whole cpu usage as read from `getCPUUsageTotal`.
     *
     * @return floating point number in percent from 0 to 100
     */
    getCPUUsageDSP(): number;
}

declare interface InsoundMultiTrackControl
{
    loadSound(data: number, bytelength: number): void;
    loadBank(data: number, bytelength: number): void;
    loadScript(scriptText: string): string;
    executeScript(script: string): string;
    update(deltaTime: number): void;
    unload(): void;
    isLoaded(): boolean;

    delete(): void;

    update(delta: number): void;

    setPause(pause: boolean, seconds: number): void;
    getPause(): boolean;
    setVolume(ch: number, level: number): void;
    getVolume(ch: number): void;
    setReverbLevel(ch: number, level: number): void;
    getReverbLevel(ch: number): number;
    setPanLeft(ch: number, level: number): void;
    getPanLeft(ch: number): number;
    setPanRight(ch: number, level: number): void;
    getPanRight(ch: number): number;

    setPosition(seconds: number): void;
    getPosition(): number;

    transitionTo(position: number, inTime: number, fadeIn: boolean,
        outTime: number, fadeOut: boolean, clock: number): void;

    getLength(): number;
    getChannelCount(): number;
    getAudibility(ch: number): number;
    setLoopPoint(startMs: number, endMs: number): void; // in ms
    getLoopPoint(): {start: number, end: number}; // in ms

    addSyncPoint(label: string, ms: number): boolean;
    deleteSyncPoint(index: number): boolean;
    editSyncPoint(index: number, label: string, ms: number): boolean;
    getSyncPointCount(): number;
    getSyncPoint(index: number): {name: string, offset: number}; //offset in ms
    getSampleData(index: number): {ptr: number, byteLength: number};

    onSyncPoint(
        callback: (name: string, offset: number, index: number) => void): void;
    doMarker(name: string, ms: number): void;

    samplerate(): number;
    dspClock(): number;

    setParameter(name: string, value: number): void;
}

declare interface InsoundAudioModule extends EmscriptenModule {
    AudioEngine: {
        new (): InsoundAudioEngine;
    }

    MultiTrackControl: {
        new (ptr: number, callbacks: LuaCallbacks): InsoundMultiTrackControl;
    }

    ParamType: ParamType;

    mContext?: AudioContext;
    mWorkletNode?: AudioWorkletNode;
    audioContext: AudioContext;
}

declare const AudioModule: (module: any) => Promise<void>;
