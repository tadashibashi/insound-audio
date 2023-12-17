import { AudioEngine } from "./AudioEngine";
import { LinearHashContainer } from "./LinearHashContainer";


export interface SyncPoint {
    text: string;
    offset: number;
}

export class SyncPointMgr
{
    private m_points: LinearHashContainer<SyncPoint>;


    // Do not modify from the outside - readonly!
    get points() { return this.m_points.items; }

    constructor()
    {
        this.m_points = new LinearHashContainer(8);
    }

    /**
     * Update sync points from AudioEngine.
     * Exception unsafe for efficiency...
     */
    update(audio: AudioEngine)
    {
        const audioLength = audio.length;
        const count = audio.engine.getSyncPointCount();

        this.m_points.clear();
        for (let i = 0; i < count; ++i)
        {
            const offset = audio.engine.getSyncPointOffsetSeconds(i);
            this.m_points.push({
                text: audio.engine.getSyncPointLabel(i),
                offset: offset
            }, audioLength ? offset / audioLength : 0);
        }
    }

    get(index: number) {
        return this.m_points.items.at(index);
    }

    /**
     * Get sync points that have occurred between now and last frame.
     *
     * Uses out val pattern for efficiency (no creating a bunch of arrays for
     * the garbage collector to have to recycle).
     *
     * @param  currentTime   current track time in seconds
     * @param  lastFrameTime last frame's track time in seconds
     * @param  trackLength   track's total length in seconds
     * @param  syncPointOut  Array to populate
     * @return Number of sync points retrieved
     */
    findByTime(currentTime: number, lastFrameTime: number, trackLength: number,
        syncPointOut: SyncPoint[]): number
    {
        const bucket = this.m_points.getBucket(currentTime/trackLength);
        const bucketLength = bucket.length;
        if (bucket.length === 0) return 0;

        let count = 0;
        for (let i = 0; i < bucketLength; ++i)
        {
            const point = bucket[i];
            if (point.offset <= currentTime && point.offset > lastFrameTime)
            {
                syncPointOut[count];
                ++count;
            }
        }

        syncPointOut.length = count;
        return count;
    }
}
