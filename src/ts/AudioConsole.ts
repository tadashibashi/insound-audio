import { AudioChannel, type AudioChannelSettings } from "./AudioChannel";
import { MultiTrackControl } from "./MultiTrackControl";

/**
 * Represents a mix-console with various channel settings.
 */
export class AudioConsole
{
    private track: MultiTrackControl | undefined;

    /**
     * Audio sub-channels. Main bus is found at `AudioConsole#main`.
     */
    readonly channels: AudioChannel[];

    readonly main: AudioChannel;

    constructor(track?: MultiTrackControl)
    {
        if (track)
            this.track = track;
        this.channels = [];
        this.main = new AudioChannel(track, "Main", 0);
    }

    provideTrack(track: MultiTrackControl)
    {
        this.track = track
    }

    hasTrack(): boolean
    {
        return !!this.track;
    }

    /**
     * Applying the following settings to each channel.
     * Channel 0 is the main channel, while the other channels are bussed to it
     * as separate channels.
     *
     * @param settings Settings to set on each channel. Remember that index 0
     *                 is the main channel bus that the others are bussed to.
     * @param transitionTime Time to transition to the new setting in seconds.
     */
    applySettings(settings: {main: AudioChannelSettings, channels: AudioChannelSettings[]},
        transitionTime: number = 0)
    {
        if (!this.track || !this.track.isLoaded) return;

        for (let i = 0; i < settings.channels.length; ++i)
        {
            settings.channels[i].channel.applySettings(settings.channels[i], transitionTime);
        }

        settings.main.channel.applySettings(settings.main, transitionTime);
    }


    /**
     * Add a channel to the console.
     *
     * @param name Name to apply to the new channel
     */
    addChannel(name: string)
    {
        if (!this.track)
        {
            throw Error("AudioConsole failed to load: AudioEngine was not " +
                "emplaced into AudioConsole.");
        }

        this.channels.push(
            new AudioChannel(this.track, name, this.channels.length + 1));
    }

    emplaceChannels(channels: AudioChannel[])
    {
        this.channels.length = 0;
        this.channels.push(...channels);
    }

    /**
     * Add multiple channels to the console
     *
     * @param names Names to apply to each respective newly created channel
     */
    addChannels(names: string[])
    {
        for (const name of names)
        {
            this.addChannel(name);
        }
    }

    getCurrentSettings(): {main: AudioChannelSettings, channels: AudioChannelSettings[]}
    {
        return {
            main: this.main.getCurrentSettings(),
            channels: this.channels.map(chan => chan.getCurrentSettings()),
        };
    }

    getDefaultSettings(): {main: AudioChannelSettings, channels: AudioChannelSettings[]}
    {
        return {
            main: this.main.getDefaultSettings(),
            channels: this.channels.map(chan => chan.getDefaultSettings()),
        };
    }

    /**
     * Clear the channels
     */
    clear()
    {
        this.channels.forEach(chan => chan.clear());
        this.channels.length = 0;
        this.main.clear();
    }

    /**
     * Reset all channel parameters to their default values
     */
    reset(seconds: number = 0)
    {
        for (const chan of this.channels)
        {
            chan.reset(seconds);
        }
    }
}
