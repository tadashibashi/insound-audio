import { AudioChannelSettings } from "./AudioChannel";

export interface MixPreset {
    name: string;
    mix: AudioChannelSettings[];
}

export class MixPresetMgr
{
    private m_presets: MixPreset[];

    /** Do not modify from the outside - readonly! */
    presets() { return this.m_presets; }

    /** Makes a copy of each preset's name in order */
    getNames() { return this.m_presets.map(mp => mp.name)}

    dragAndDrop(from: number, to: number)
    {
        const temp = [...this.m_presets];
        const splicedPreset = temp[from];
        temp.splice(from, 1);
        temp.splice(to, 0, splicedPreset);

        // for svelte to indicate updated status
        this.m_presets = this.m_presets;
    }

    constructor()
    {
        this.m_presets = [];
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

    push(name: string, volumes: number[])
    {
        this.m_presets.push({
            name, volumes
        });
    }

    clear()
    {
        this.m_presets.length = 0;
    }
}
