import { MultiTrackControl } from "./MultiTrackControl";

export interface SyncPoint {
    name: string;
    offset: number;
}

export class SyncPointMgr
{
    private m_points: SyncPoint[];

    // Do not modify from the outside - readonly!
    get points() { return this.m_points; }

    get length() { return this.m_points.length; }

    constructor()
    {
        this.m_points = [];
    }

    /**
     * Update sync points from AudioEngine.
     * Exception unsafe for efficiency...
     */
    update(ctrl: MultiTrackControl)
    {
        const points = this.m_points;
        points.length = 0;

        const count = ctrl.track.getSyncPointCount();
        for (let i = 0; i < count; ++i)
        {
            points.push(ctrl.track.getSyncPoint(i));
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
