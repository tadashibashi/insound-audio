import { SyncPointMgr } from "./SyncPointMgr";
import { getAudioModule } from "./emaudio/AudioModule";
import { AudioEngine } from "./AudioEngine";
import { AudioConsole } from "./AudioConsole";
import { Callback } from "./Callback";
import { MixPreset } from "./MixPresetMgr";
import { SpectrumAnalyzer } from "./SpectrumAnalyzer";
import { EmBufferGroup } from "./emaudio/EmBuffer";

export class MultiTrackControl
{
    private m_track: InsoundMultiTrackControl;
    private m_ptr: number;
    private m_points: SyncPointMgr;
    private m_spectrum: SpectrumAnalyzer;
    private m_audio: AudioEngine;
    private m_trackData: EmBufferGroup;

    get syncpoints() { return this.m_points; }
    get track() { return this.m_track; }

    readonly onpause: Callback<[boolean]>;

    constructor(audio: AudioEngine, audioConsole: AudioConsole, mixPresets: MixPreset[])
    {
        this.m_points = new SyncPointMgr;

        this.m_ptr = audio.engine.createTrack();
        this.m_audio = audio;
        this.onpause = new Callback;
        this.m_spectrum = new SpectrumAnalyzer();
        this.m_trackData = new EmBufferGroup();

        const MultiTrackControl = getAudioModule().MultiTrackControl;
        this.m_track = new MultiTrackControl(this.m_ptr, {
            syncPointsUpdated: () => {
                this.m_points.update(this);
            },
            setPause: (pause: boolean, seconds: number) => {
                this.m_track.setPause(pause, seconds);
            },
            setVolume: (ch: number, level: number, seconds: number = 0) => {
                audioConsole.channels.at(ch).volume.transitionTo(level, seconds);
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
                return mixPresets.at(index).name;
            },
            getPresetCount: () => {
                return mixPresets.length;
            },
            applyPreset: (indexOrName: number | number, seconds: number = 0) => {
                let preset = (typeof indexOrName === "number") ?
                    mixPresets.at(indexOrName) :
                    mixPresets.find(mp => mp.name === indexOrName);
                if (!preset)
                    throw Error("Mix preset could not be found with id: " +
                        indexOrName);

                audioConsole.applySettings(preset.mix, seconds);
            }
        });
    }

    setPause(pause: boolean, seconds: number = 0)
    {
        this.m_track.setPause(pause, seconds);
        this.onpause.invoke(pause);
    }

    /**
     * Internals become invalidated after this function. Used to clean up
     * when no longer using this object
     */
    delete() {
        this.m_audio.engine.deleteTrack(this.m_ptr);
        this.m_track.delete();
    }
}
