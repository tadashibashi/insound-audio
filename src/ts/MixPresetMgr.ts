import { AudioEngine } from "./AudioEngine";

export interface MixPreset {
    name: string;
    volumes: number[];
}

export class MixPresetMgr
{
    private m_presets: MixPreset[];

    // Do not modify from the outside - readonly!
    get presets() { return this.m_presets; }

    constructor()
    {
        this.m_presets = [];
    }

    /**
     * Update sync points from AudioEngine.
     * Exception unsafe for efficiency...
     */
    update(audio: AudioEngine)
    {
        const presets = this.m_presets;
        presets.length = 0;

        const count = audio.engine.getPresetCount();
        for (let i = 0; i < count; ++i)
        {
            presets.push(audio.engine.getPresetByIndex(i));
        }
    }

    get(index: number)
    {
        return this.m_presets.at(index);
    }

    clear()
    {
        this.m_presets.length = 0;
    }
}
