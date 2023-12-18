import { AudioEngine } from "./AudioEngine";

export interface SyncPoint {
    text: string;
    offset: number;
}

export class SyncPointMgr
{
    private m_points: SyncPoint[];

    // Do not modify from the outside - readonly!
    get points() { return this.m_points; }

    constructor()
    {
        this.m_points = [];
    }

    /**
     * Update sync points from AudioEngine.
     * Exception unsafe for efficiency...
     */
    update(audio: AudioEngine)
    {
        const points = this.m_points;
        points.length = 0;

        const count = audio.engine.getSyncPointCount();
        for (let i = 0; i < count; ++i)
        {
            const offset = audio.engine.getSyncPointOffsetSeconds(i);
            points.push({
                text: audio.engine.getSyncPointLabel(i),
                offset: offset
            });
        }
    }

    get(index: number)
    {
        return this.m_points.at(index);
    }

    clear()
    {
        this.m_points.length = 0;
    }
}
