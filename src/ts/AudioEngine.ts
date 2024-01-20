import { audioModuleWasInit, initAudioModule } from "./emaudio/AudioModule";
import { getAudioModule } from "./emaudio/AudioModule";
import { SpectrumAnalyzer } from "./SpectrumAnalyzer";
import { MultiTrackControl } from "./MultiTrackControl";

/** Max time in seconds before AudioEngine should suspend itself. */
const MAX_DOWNTIME = 5;

const registry = new FinalizationRegistry((heldValue: any) => {
    // `heldValue` should be an Emscripten Embind C++ object with its own
    // delete function, which explicitly frees memory of this object.
    if (typeof heldValue.delete === "function")
    {
        heldValue.delete();
    }
});

// Get this info from a database to populate a new track with
export interface LoadOptions
{
    script?: string;
    loopstart?: number;
    loopend?: number;
}

/**
 * Main interface abstraction between audio engine and client-side code
 */
export class AudioEngine
{

    get engine() { return this.m_engine; }

    /** Flag used when resetting to prevent updates */
    private m_isResetting: boolean;

    private m_module: InsoundAudioModule;
    private m_engine: InsoundAudioEngine;
    private m_tracks: MultiTrackControl[];

    private m_lastFrameTime: number;
    private m_downTime: number;

    constructor()
    {
        // Ensure WebAssembly module was initialized
        if (!audioModuleWasInit())
        {
            throw Error("Cannot instantiate AudioEngine without first " +
                "initializing AudioModule");
        }

        this.m_module = getAudioModule();
        SpectrumAnalyzer.setModule(this.m_module);

        this.m_engine = new (this.m_module.AudioEngine)();
        this.m_tracks = [];

        if (!this.m_engine.init())
        {
            this.m_engine.delete();
            throw new Error("Failed to initialize AudioEngine");
        }

        registry.register(this, this.engine, this);

        this.m_lastFrameTime = performance.now();
    }

    /**
     * Restart the underlying Emscripten audio module and replace the
     * underlying AudioEngine with a new one. To be used when encountering
     * an unrecoverable critical error pertaining to the module.
     */
    async reset()
    {
        if (this.m_isResetting) return;

        this.m_isResetting = true;

        // Delete old engine object
        try {
            this.engine.delete();
        }
        catch(err)
        {
            console.error("Error during InsoundAudioEngine#delete:", err);
        }

        // Reset InsoundAudioModule
        try {
            this.m_module = await initAudioModule();
            SpectrumAnalyzer.setModule(this.m_module);
        }
        catch(err)
        {
            // Cannot proceed if audio module failed to restart...
            console.error("Critical error while resetting InsoundAudioModule:",
                err);
            this.m_isResetting = false;
            return;
        }

        // Reset AudioEngine
        try {
            this.m_engine = new (this.m_module.AudioEngine)();

            // Re-register new engine as `heldValue` to finalize
            registry.unregister(this);
            registry.register(this, this.m_engine, this);

            this.engine.init();
        }
        catch(err)
        {
            console.error("Critical error while resetting InsoundAudioEngine:",
                err);
        }
        finally
        {
            this.m_downTime = 0;
            this.m_lastFrameTime = performance.now();
            this.m_isResetting = false;
        }

    }

    createTrack(): MultiTrackControl
    {
        const trackPtr = this.engine.createTrack();
        try {
            const track = new MultiTrackControl(this, trackPtr);
            this.m_tracks.push(track);
            return track;
        }
        catch(err)
        {
            this.engine.deleteTrack(trackPtr);
            throw err;
        }
    }

    /**
     * Delete a track that was created with `MultiTrackControl#createTrack`.
     * @param track - track to delete
     * @returns true if track was deleted, and false otherwise. This means
     * that the track is not owned by this `AudioEngine`.
     */
    deleteTrack(track: MultiTrackControl)
    {
        const length = this.m_tracks.length;
        for (let i = 0; i < length; ++i)
        {
            if (this.m_tracks[i] === track)
            {
                track.delete();

                this.m_tracks.splice(i, 1);
                return true;
            }
        }

        return false;
    }

    get masterVolume() { return this.m_engine.getMasterVolume(); }

    set masterVolume(level: number) { this.m_engine.setMasterVolume(level); }


    /** Get the current WebAudio context */
    get context()
    {
        return this.m_module.mContext;
    }

    suspend()
    {
        this.engine.suspend();
    }

    isSuspended(): boolean
    {
        if (!this.m_module.mContext) return false;

        return this.m_module.mContext.state === "suspended";
    }

    resume()
    {
        this.engine.resume();
    }

    update()
    {
        // Do not update while resetting since tracks and engine are invalid
        if (this.m_isResetting) return;

        // Calculate time
        const now = performance.now();
        const deltaSeconds = (now - this.m_lastFrameTime) * .001;

        // Update audio engine and tracks
        let allPaused = true;
        for (const track of this.m_tracks)
        {
            track.update(deltaSeconds);
            if (allPaused && !track.isPaused || track.track.getAudibility(0))
            {
                allPaused = false;
            }
        }

        // Check downtime to auto-suspend
        if (!this.isSuspended() && allPaused)
        {
            this.m_downTime += deltaSeconds;
            if (this.m_downTime >= MAX_DOWNTIME)
            {
                this.suspend();
                this.m_downTime = 0;
            }
        }
        else
        {
            this.m_downTime = 0;
        }

        this.engine.update();

        // Update last frame time
        this.m_lastFrameTime = now;
    }


    /**
     * Call this manually when no longer using the AudioEngine to clean
     * up underlying AudioEngine object. A finalization registry is used on
     * this object, but the standard does not guarantee that the finalization
     * callback gets called, so it's recommended to call this manually.
     *
     * (All tracks allocated via `createTrack` are freed here)
     */
    release()
    {
        for (const track of this.m_tracks)
        {
            track.delete();
        }

        this.m_tracks.length = 0;
        this.engine.delete();
    }
}
