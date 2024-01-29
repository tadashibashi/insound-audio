import { getAudioModule } from "./emaudio/AudioModule";
import { AudioConsole } from "./AudioConsole";
import { Callback } from "./Callback";
import { MixPreset } from "./MixPresetMgr";
import { SpectrumAnalyzer } from "./SpectrumAnalyzer";
import { EmBufferGroup } from "./emaudio/EmBuffer";
import { SoundLoadError } from "./SoundLoadError";
import { AudioEngine } from "./AudioEngine";
import { AudioMarker, MarkerMgr } from "./MarkerMgr";
import { AudioChannel } from "./AudioChannel";

// Get this info from a database to populate a new track with
export interface LoadOptions
{
    script?: string;
    loopPoints?: {start: number, end: number};
    channelNames?: string[];
    markers?: AudioMarker[];

    /** Provide newly ordered channel array, on reloads */
    channels?: AudioChannel[];
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
    private m_markers: MarkerMgr;
    private m_spectrum: SpectrumAnalyzer;
    private m_trackData: EmBufferGroup;
    private m_console: AudioConsole;
    private m_mixPresets: MixPreset[];
    private m_looping: boolean;

    private m_lastPosition: number;

    get track() { return this.m_track; }
    get mixPresets() { return this.m_mixPresets; }
    get console() { return this.m_console; }
    get spectrum() { return this.m_spectrum; }
    get markers() { return this.m_markers; }

    readonly onpause: Callback<[boolean]>;

    /**
     * Fires on every update tick.
     * Param 1 is delta time in seconds
     * Param 2 is current track position in seconds
     */
    readonly onupdate: Callback<[number, number]>;
    readonly onmarker: Callback<[AudioMarker]>;
    readonly onload: Callback<[MultiTrackControl]>;

    /** Called when position is set from lua */
    readonly onseek: Callback<[number]>;

    /** Perform print on console where:
     * @param level - 0 comment, 1 normal, 2 warn, 3 error
     * @param name - logger name, displayed [name]
     *   */
    readonly doprint: Callback<[number, string, string, any?]>;

    constructor(engine: AudioEngine, ptr: number)
    {
        this.m_engine = engine;
        this.m_ptr = ptr;

        this.m_spectrum = new SpectrumAnalyzer();
        this.m_trackData = new EmBufferGroup();

        this.onpause = new Callback;
        this.onupdate = new Callback;
        this.onmarker = new Callback;
        this.onseek = new Callback;
        this.onload = new Callback;
        this.doprint = new Callback;

        this.m_console = new AudioConsole(this);
        this.m_mixPresets = [];
        this.m_looping = true;
        this.m_lastPosition = 0;

        this.m_markers = new MarkerMgr(this);

        const MultiTrackControl = getAudioModule().MultiTrackControl;
        this.m_track = new MultiTrackControl(this.m_ptr, {
            addMarker: (name: string, ms: number) => {
                this.m_markers.push({
                    name: name, position: ms
                });
            },
            editMarker: (index, name, ms) => {
                if (typeof index === "number")
                {
                    --index; // lua indices are 1-based
                    const marker = this.m_markers.array[index];
                    marker.name = name;
                    this.m_markers.updatePositionByIndex(index, ms);
                }
                else
                {
                    const markerIndex = this.m_markers.findIndexByName(index);

                    if (markerIndex !== -1)
                    {
                        this.m_markers.array[markerIndex].name = name;
                        this.m_markers.updatePositionByIndex(markerIndex, ms);
                    }
                }
            },
            getMarker: (index) => {
                return (typeof index === "number") ?
                    this.m_markers.array[index-1] :
                    this.m_markers.findByName(index);
            },
            getMarkerCount: () => {
                return this.m_markers.length;
            },
            setPause: (pause: boolean, seconds: number) => {
                this.m_track.setPause(pause, seconds);
                this.onpause.invoke(pause);
            },
            setPosition: (seconds: number) => {
                this.m_track.setPosition(seconds);
                this.onseek.invoke(seconds);
            },
            setVolume: (ch: number, level: number, seconds: number = 0) => {
                this.m_console.channels.at(ch).params.volume.transitionTo(level, seconds);
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
            },
            print: (level: number, name: string, message: string) => {
                this.doprint.invoke(level, name, message);
            },
        });

        this.m_markers.onmarker.addListener((marker) => {
            try {
                // Notify Lua scripting
                this.m_track.doMarker(marker.name, marker.position);
            }
            catch(err)
            {
                this.printSolError(err);
            }
            // Local callbacks
            this.onmarker.invoke(marker);
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
        // Block loop if looping is turned off
        if (!this.looping)
        {
            if (this.position < this.m_lastPosition)
            {
                const loop = this.loopPoints;
                this.position = (loop.end - .001) * .001;
                this.setPause(true, 0);
                this.m_engine.suspend();
            }
        }

        this.onupdate.invoke(deltaTime, this.position);

        try {
            this.m_track.update(deltaTime);
        }
        catch(err)
        {
            this.printSolError(err);
        }

        this.m_spectrum.update();

        this.m_lastPosition = this.position;
    }

    // ----- Loading / Unloading ----------------------------------------------

    /** Load audio internals after the main file buffer loading */
    private postLoadAudio(opts: LoadOptions)
    {
        // Load script

        this.print(`Track loaded with ${this.m_track.getChannelCount()} channel(s), at ${this.m_track.getLength()} seconds long`);
        try {
            this.m_track.loadScript(opts.script || "");
        }
        catch(err)
        {
            this.printSolError(err);
        }

        // Load markers
        this.m_markers.clear();
        if (opts.markers) // if markers provided use these
        {
            opts.markers.forEach(marker => this.m_markers.push(marker));
        }
        else // otherwise get them from the track
        {
            this.m_markers.loadFromTrack();
        }

        // Set custom loop points
        if (opts.loopPoints)
        {
            // loop points will be set to track automatically via markers
            this.m_markers.loopStart = {
                name: "LoopStart",
                position: opts.loopPoints.start,
            };
            this.m_markers.loopEnd = {
                name: "Loopend",
                position: opts.loopPoints.end,
            };
        }


        // Populate channels from list
        if (opts.channels)
        {
            this.m_console.emplaceChannels(opts.channels);

            // update presets if any chans deleted, remove, if any created, add default preset
            this.m_mixPresets.update(opts.channels);
        }
        else // populate default channels for each track
        {
            this.m_console.clear();

            // Create audio console channels
            const channelCount = this.m_track.getChannelCount();
            const nameCount = opts.channelNames?.length || 0;
            for (let i = 0; i < channelCount; ++i)
            {
                this.m_console.addChannel(i < nameCount ? opts.channelNames[i] : "");
            }
        }


        this.m_track.setPause(true, 0);
        this.m_track.setPosition(0);
        this.m_lastPosition = 0;
        this.onload.invoke(this);
    }

    private print(...args: any[])
    {
        let str = "";
        for (let i = 0; i < args.length; ++i)
        {
            str += args[i].toString() + "\t";
        }

        this.doprint.invoke(0, "INFO", str);
    }

    /**
     * Checks caught object for sol::error object, and fires print error
     * callback.
     *
     * @param error - error object to check
     */
    private printSolError(error: any)
    {
        if (error instanceof WebAssembly["Exception"] &&
            error.message[0] === "sol::error")
        {
            const fullMessage: string = error.message[1];
            const linebreakIndex = fullMessage.indexOf("\n");
            const message = fullMessage.substring(0, linebreakIndex === -1 ? fullMessage.length : linebreakIndex);

            const beforeLineNumberIndex = message.lastIndexOf("]:");
            if (beforeLineNumberIndex === -1) return;

            const afterLineNumberIndex = message.indexOf(":", beforeLineNumberIndex + 2);
            if (afterLineNumberIndex === -1) return;

            const lineNumber = message.substring(beforeLineNumberIndex + 2, afterLineNumberIndex);
            const errorMessage = message.substring(afterLineNumberIndex + 1, message.length);


            this.doprint.invoke(4, "ERROR", errorMessage, parseInt(lineNumber));
        }
    }

    loadFSBank(buffer: ArrayBuffer, opts: LoadOptions = defaultLoadOps)
    {
        if (this.m_track.isLoaded())
            this.m_track.unload();

        this.m_trackData.alloc(buffer, getAudioModule());

        try {
            this.m_track.loadBank(
                this.m_trackData.data[0].ptr, buffer.byteLength);

            this.postLoadAudio(opts);
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
            const failedSounds: {index: number, reason: string}[] = [];

            for (let i = 0, length = buffers.length; i < length; ++i)
            {
                try {
                    this.m_track.loadSound(this.m_trackData.data[i].ptr,
                        this.m_trackData.data[i].size);
                }
                catch(err)
                {
                    failedSounds.push({
                        index: i,
                        reason: processLoadingErrorMessage(err),
                    });
                }
            }

            if (failedSounds.length > 0)
            {
                throw new SoundLoadError(failedSounds);
            }

            this.postLoadAudio(opts);
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

    /**
     * To be called when editing a track and wanting to reset state, and
     * and update the script.
     */
    updateScript(updatedScript: string): boolean
    {
        this.m_track.setPosition(0);
        this.m_track.setPause(true, 0);

        try {
            this.m_track.loadScript(updatedScript);
            return true;
        }
        catch(err)
        {
            this.printSolError(err);
            return false;
        }
    }

    executeScript(script: string): string
    {
        try {
           return this.m_track.executeScript(script);
        }
        catch (err)
        {
            this.printSolError(err);
        }
    }

    unload()
    {
        this.m_track.unload();
        this.m_trackData.free();
    }

    get isLoaded() { return this.m_track.isLoaded(); }

    /** Number of sub-track channels (doesn't include the main bus channel) */
    get channelCount() { return this.m_track.getChannelCount(); }

    get length() { return this.m_track.getLength(); }

    get loopPoints() { return this.m_track.getLoopPoint(); }

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
            if (this.position > (this.loopPoints.end * .001) - .01)
                this.position = 0;
        }

        this.m_track.setPause(pause, seconds);
        this.onpause.invoke(pause);
    }

    get isPaused() { return this.m_track.getPause(); }

    set position(seconds: number) {

        this.m_track.setPosition(seconds);
        this.m_lastPosition = this.m_track.getPosition(); // seeking should cause last position to align with current
        this.onseek.invoke(this.m_track.getPosition());
    }

    get position() { return this.m_track.isLoaded ? this.m_track.getPosition() : 0; }

    set looping(val: boolean) { this.m_looping = val; }
    get looping() { return this.m_looping; }

    setLoopPoint(startMS: number, endMS: number)
    {
        this.m_track.setLoopPoint(startMS, endMS);
    }

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

/**
 * Processes error message from audio loading error object
 * @param  err - error object retrieved from catch statement from loadFSBank or
 *               loadSound functions
 * @return error message string
 */
function processLoadingErrorMessage(err: any): string
{
    let reason: string = "";

     // WebAssembly errors should be emitted from Emscripten, make sure flag is set
    if (err instanceof WebAssembly["Exception"])
    {
        const type: string = err.message[0];
        const message: string = err.message[1];

        switch(type)
        {
        case "Insound::SoundLengthMismatch":
            reason = "track length does not match";
            break;
        case "Insound::FMODError":
            reason = "this file format is not supported";
            break;
        case "std::runtime_error":
            reason = message || "unknown error";
            break;
        default:
            reason = "unknown error";
            break;
        }
    }
    else
    {
        reason = "unknown error";
    }

    return reason;
}
