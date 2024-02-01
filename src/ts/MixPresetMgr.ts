import { AudioChannel, AudioChannelSettings } from "./AudioChannel";

/**
 * Mix preset data
 */
export interface MixPreset {
    name: string;
    mix: {
        main: AudioChannelSettings;
        channels: AudioChannelSettings[],
    };
}

/**
 * Manages caching and handling of mix presets
 */
export class MixPresetMgr
{
    private m_presets: MixPreset[];

    get presets() { return this.m_presets; }

    set presets(val: MixPreset[]) { this.m_presets = val; }

    /**
     * Makes a copy of each preset's name in order.
     * Okay to modify since a copy is made.
     */
    get names() { return this.m_presets.map(mp => mp.name)}

    get length() { return this.m_presets.length; }

    constructor()
    {
        this.m_presets = [];
    }

    /**
     * Update presets to account for channel removals and additions enacted
     * by the user.
     *
     * @param channels - array of updated channels minus the main bus, which
     *                   should come from the current `AudioConsole#channels`
     *                   array, or equivalent.
     */
    update(channels: AudioChannel[]): void
    {
        this.m_presets.forEach(preset => {

            // splice channel settings that have been removed
            for (let i = 0; i < preset.mix.channels.length; )
            {
                const settingCh = preset.mix.channels[i].channel;

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

    /** Find the index of a preset, or -1 if it was not found. */
    findIndex(preset: MixPreset): number
    {
        return this.m_presets.findIndex(p => p === preset);
    }

    /**
     * Get preset at an index, negative indices begin with length-1 and count
     * backward from there. Anything exceeding index range results in undefined
     */
    at(index: number): MixPreset
    {
        return this.m_presets.at(index);
    }

    /** Find a preset based on a predicate, or undefined if it doesn't exist */
    find(predicate: (preset: MixPreset, index: number, obj: MixPreset[]) => boolean)
    {
        return this.m_presets.find(predicate);
    }

    /** Find a preset by its name, its first occurrance */
    findByName(name: string): MixPreset | undefined
    {
        return this.m_presets.find(p => p.name === name);
    }

    /** Clear the container of all presets */
    clear()
    {
        this.m_presets.length = 0;
    }

    /** Transforms the object to JSON format for storage in a database */
    toJSON(): string
    {
        throw Error("Not implemented!");
    }

    /** Create a MixPresetMgr from JSON string retrieved from the database */
    static fromJSON(json: string): MixPresetMgr
    {
        throw Error("Not implemented!");
    }
}
