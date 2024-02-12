import { MultiTrackControl } from "./MultiTrackControl";

export interface SyncPoint
{
    name: string;
    position: number;
}

export class SyncPointMgr
{
    private m_points: SyncPoint[];
    private m_track: MultiTrackControl;

    // Do not modify from the outside - readonly!
    get points() { return this.m_points; }

    get length() { return this.m_points.length; }

    constructor(ctrl: MultiTrackControl)
    {
        this.m_points = [];
        this.m_track = ctrl;
    }

    /**
     * Fully update sync points from AudioEngine.
     * Exception unsafe for efficiency...
     */
    update()
    {
        this.m_points = [];

        const count = this.m_track.track.getSyncPointCount();
        for (let i = 0; i < count; ++i)
        {
            this.m_points.push(this.m_track.track.getSyncPoint(i));
        }
    }

    addSyncPoint(name: string, offsetSec: number)
    {
        if (this.m_track.track.addSyncPoint(name, offsetSec))
        {
            //this.update();
            return true;
        }

        return false;
    }

    deleteSyncPoint(index: number)
    {
        if (this.m_track.track.deleteSyncPoint(index))
        {
            //this.update();
            return true;
        }

        return false;
    }

    editSyncPoint(index: number, newName: string, newOffsetSec: number)
    {
        if (this.m_track.track.editSyncPoint(index, newName, newOffsetSec))
        {
            //this.update();
            return true;
        }

        return false;
    }

    get(index: number): SyncPoint | undefined
    {
        return this.m_points.at(index);
    }

    getByName(name: string): SyncPoint | undefined
    {
        const length = this.m_points.length;
        for (let i = 0; i < length; ++i)
        {
            if (this.m_points[i].name === name)
                return this.m_points[i];
        }

        return undefined; // for clarity of intention
    }

    clear()
    {
        this.m_points.length = 0;
    }
}
