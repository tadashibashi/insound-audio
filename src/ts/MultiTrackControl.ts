import { SyncPointMgr } from "./SyncPointMgr";
import { getAudioModule } from "./emaudio/AudioModule";
import { AudioConsole } from "./AudioConsole";
import { Callback } from "./Callback";
import { MixPreset } from "./MixPresetMgr";
import { SpectrumAnalyzer } from "./SpectrumAnalyzer";
import { EmBufferGroup } from "./emaudio/EmBuffer";
import { SoundLoadError } from "./SoundLoadError";
import { AudioEngine } from "./AudioEngine";

// Get this info from a database to populate a new track with
export interface LoadOptions
{
    script?: string;
    loopPoints?: {start: number, end: number};
    channelNames?: string[];
}

const defaultLoadOps = {
    script: "",
    channelNames: [],
};

export class MultiTrackControl
{
    private m_engine: AudioEngine;
    private m_track: InsoundMultiTrackControl;
    private m_ptr: number;
    private m_points: SyncPointMgr;
    private m_spectrum: SpectrumAnalyzer;
    private m_trackData: EmBufferGroup;
    private m_console: AudioConsole;
    private m_mixPresets: MixPreset[];
    private m_looping: boolean;

    get syncpoints() { return this.m_points; }
    get track() { return this.m_track; }
    get mixPresets() { return this.m_mixPresets; }
    get console() { return this.m_console; }
    get spectrum() { return this.m_spectrum; }

    readonly onpause: Callback<[boolean]>;
    readonly onupdate: Callback<[number]>;
    readonly onsyncpoint: Callback<[string, number, number]>;

    constructor(engine: AudioEngine, ptr: number)
    {
        this.m_engine = engine;
        this.m_points = new SyncPointMgr;
        this.m_ptr = ptr;

        this.m_spectrum = new SpectrumAnalyzer();
        this.m_trackData = new EmBufferGroup();

        this.onpause = new Callback;
        this.onupdate = new Callback;
        this.onsyncpoint = new Callback;

        this.m_console = new AudioConsole(this);
        this.m_mixPresets = [];
        this.m_looping = true;

        const MultiTrackControl = getAudioModule().MultiTrackControl;
        this.m_track = new MultiTrackControl(this.m_ptr, {
            syncPointsUpdated: () => {
                this.m_points.update(this);
            },
            setPause: (pause: boolean, seconds: number) => {
                this.m_track.setPause(pause, seconds);
            },
            setVolume: (ch: number, level: number, seconds: number = 0) => {
                this.m_console.channels.at(ch).volume.transitionTo(level, seconds);
            },
            setPanLeft: (ch: number, level: number) => {
                this.m_track.setPanLeft(ch, level);
            },
            setPanRight: (ch: number, level: number) => {
                this.m_track.setPanRight(ch, level);
            },
            setReverbLevel: (ch: number, level: number) => {
                this.m_track.setReverbLevel(ch, level);
            },
            getPresetName: (index: number) => {
                return this.m_mixPresets.at(index).name;
            },
            getPresetCount: () => {
                return this.m_mixPresets.length;
            },
            applyPreset: (indexOrName: number | number, seconds: number = 0) => {
                let preset = (typeof indexOrName === "number") ?
                    this.m_mixPresets.at(indexOrName) :
                    this.m_mixPresets.find(mp => mp.name === indexOrName);
                if (!preset)
                    throw Error("Mix preset could not be found with id: " +
                        indexOrName);

                this.m_console.applySettings(preset.mix, seconds);
            }
        });

        this.m_track.onSyncPoint((name, offset, index) => {
            if (name === "LoopEnd" && !this.m_looping)
            {
                this.setPause(true);
                this.m_engine.suspend();
            }
            console.log(name);

            this.onsyncpoint.invoke(name, offset, index);
        });
    }

    /**
     * Internals become invalidated after this function. Used to clean up
     * when no longer using this object. Please use `AudioEngine#deleteTrack`
     * instead of this function directly.
     */
    delete()
    {
        this.m_track.delete();
    }

    update(deltaTime: number)
    {
        this.onupdate.invoke(deltaTime);
        this.m_track.update(deltaTime);
        this.m_spectrum.update();
    }

    // ----- Loading / Unloading ----------------------------------------------

    loadFSBank(buffer: ArrayBuffer, opts: LoadOptions = defaultLoadOps)
    {
        if (this.m_track.isLoaded())
            this.m_track.unload();

        this.m_trackData.alloc(buffer, getAudioModule());

        try {
            this.m_track.loadBank(
                this.m_trackData.data[0].ptr, buffer.byteLength);
            const scriptErr = this.m_track.loadScript(opts.script || "");
            if (scriptErr)
            {
                console.error("Lua script load error:", scriptErr);
            }

            if (opts.loopPoints)
            {
                this.m_track.setLoopPoint(opts.loopPoints.start,
                    opts.loopPoints.end);
            }

            this.m_points.update(this);
            this.m_console.channels.length = 0;

            const channelCount = this.m_track.getChannelCount();
            const nameCount = opts.channelNames?.length || 0;
            for (let i = 0; i < channelCount; ++i)
            {
                this.m_console.addChannel(i < nameCount ? opts.channelNames[i] : "");
            }
            this.m_track.setPause(true, 0);
        }
        finally
        {
            this.m_trackData.free();
        }
    }

    /**
     * Load track with plain sound files
     *
     * @param buffers - audio binary data buffers
     * @param opts    - loading options
     *
     * @throws SoundLoadError if there was a problem with any of the audio
     *         files. It is a good idea to reset the audio module in this
     *         case, as sometimes there is a bad access that causes the
     *         Emscripten module to crash.
     */
    loadSounds(buffers: ArrayBuffer[], opts: LoadOptions = defaultLoadOps)
    {
        if (this.m_track.isLoaded())
            this.m_track.unload();

        const audioModule = getAudioModule();
        for (const buffer of buffers)
        {
            this.m_trackData.alloc(buffer, audioModule);
        }

        try {
            const badSounds: number[] = [];

            for (let i = 0, length = buffers.length; i < length; ++i)
            {
                try {
                    this.m_track.loadSound(this.m_trackData.data[i].ptr,
                        this.m_trackData.data[i].size);
                }
                catch(err)
                {
                    badSounds.push(i);
                    console.warn("Sound at index " + i + " failed to load:",
                        err);
                }
            }

            if (badSounds.length > 0)
            {
                throw new SoundLoadError(badSounds);
            }

            const scriptErr = this.m_track.loadScript(opts.script || "");
            if (scriptErr)
            {
                console.error("Lua script load error:", scriptErr);
            }

            if (opts.loopPoints)
            {
                this.m_track.setLoopPoint(opts.loopPoints.start, opts.
                    loopPoints.end);
            }

            this.m_points.update(this);

            const channelCount = this.m_track.getChannelCount();
            const nameCount = opts.channelNames?.length || 0;
            for (let i = 0; i < channelCount; ++i)
            {
                this.m_console.addChannel(i < nameCount ? opts.channelNames[i] : "");
            }
            this.m_track.setPause(true, 0);
        }
        catch(err)
        {
            this.unload();
            throw err;
        }
        finally
        {
            this.m_trackData.free();
        }
    }

    unload()
    {
        this.m_track.unload();
        this.m_trackData.free();
        this.m_points.clear();
        this.m_console.clear();
    }

    get isLoaded() { return this.m_track.isLoaded(); }

    /** Number of sub-track channels (doesn't include the main bus channel) */
    get channelCount() { return this.m_track.getChannelCount(); }

    get length() { return this.m_track.getLength(); }


    // ----- Track controls ---------------------------------------------------

    play()
    {
        this.setPause(false, 0);
    }

    stop()
    {
        this.m_track.setPosition(0);
        this.m_track.setPause(true, .1);
    }

    setPause(pause: boolean, seconds: number = 0)
    {
        if (!pause && this.m_engine.isSuspended())
        {
            this.m_engine.resume();
        }

        this.m_track.setPause(pause, seconds);
        this.onpause.invoke(pause);
    }

    get isPaused() { return this.m_track.getPause(); }

    set position(seconds: number) { this.m_track.setPosition(seconds); }
    get position() { return this.m_track.getPosition(); }

    set looping(val: boolean) { this.m_looping = val; }
    get looping() { return this.m_looping; }

    applyMixPreset(indexOrName: number | string, seconds: number = 0)
    {
        let preset: MixPreset | undefined = (typeof indexOrName === "number") ?
            this.m_mixPresets[indexOrName] :
            this.m_mixPresets.find(mp => mp.name === indexOrName);

        if (!preset)
        {
            console.warn("Could not find preset with index:", indexOrName);
            return;
        }

        this.m_console.applySettings(preset.mix, seconds);
    }

    getSampleData(index: number): Float32Array
    {
        const data = this.m_track.getSampleData(index);
        const begin = data.ptr/4;
        const end = begin + data.byteLength;
        return getAudioModule().HEAPF32.subarray(begin, end);
    }
}
