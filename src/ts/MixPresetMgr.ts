import { AudioChannel, AudioChannelSettings } from "./AudioChannel";

export interface MixPreset {
    name: string;
    mix: {
        main: AudioChannelSettings;
        channels: AudioChannelSettings[],
    };
}

// create function to translate mix preset into db-savable
// put mix in order of channels provided by function
function toDatabase(channels: AudioChannel)
{
    throw Error("Not implemented!");
}


export class MixPresetMgr
{
    private m_presets: MixPreset[];

    /**
     * Do not modify from the outside - readonly!
     */
    get presets() { return this.m_presets; }

    /**
     * Makes a copy of each preset's name in order.
     * Okay to modify since a copy is made.
     */
    get names() { return this.m_presets.map(mp => mp.name)}

    constructor()
    {
        this.m_presets = [];
    }

    /**
     * Update presets to account for channel removals and additions
     * @param channels - array of updated channels minus the main bus
     */
    update(channels: AudioChannel[])
    {
        this.m_presets.forEach(preset => {
            // splice channel settings that have been removed
            for (let i = 0; i < preset.mix.channels.length; )
            {
                const settingCh = preset.mix[i].channel;

                if (!channels.find(ch => ch === settingCh))
                {
                    preset.mix.channels.splice(i, 1);

                    // no increment since current index got spliced out
                }
                else
                {
                    ++i;
                }
            }

            // add default settings for new channels
            for (let i = 0; i < channels.length; ++i)
            {
                if (!preset.mix.channels.find(setting => setting.channel === channels[i]))
                {
                    preset.mix.channels.push(channels[i].getDefaultSettings());
                }
            }
        });
    }

    getByIndex(index: number)
    {
        return this.m_presets.at(index);
    }

    getByName(name: string)
    {
        const presets = this.m_presets;
        const length = presets.length;
        for (let i = 0; i < length; ++i)
        {
            if (presets[i].name === name)
                return presets[i];
        }

        throw Error("Could not find a preset with name \"" + name + "\"");
    }

    clear()
    {
        this.m_presets.length = 0;
    }
}
