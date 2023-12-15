import { AudioEngine } from "./AudioEngine";


export interface SyncPoint {
    text: string;
    offset: number;
}

export class SyncPointMgr
{
    points: SyncPoint[];

    constructor()
    {
        this.points = [];
    }

    /**
     * Update sync points from AudioEngine
     */
    update(audio: AudioEngine)
    {
        const tempPoints: SyncPoint[] = [];

        const count = audio.engine.getSyncPointCount();
        for (let i = 0; i < count; ++i)
        {
            tempPoints.push({
                text: audio.engine.getSyncPointLabel(i),
                offset: audio.engine.getSyncPointOffsetSeconds(i)
            });
        }

        this.points = tempPoints;
    }

    get(index: number) {
        return this.points.at(index);
    }
}
