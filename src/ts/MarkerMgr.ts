import { Callback } from "./Callback";
import { MultiTrackControl } from "./MultiTrackControl";

const LOOPSTART_NAME = "LoopStart";
const LOOPEND_NAME = "LoopEnd";

export interface AudioMarker
{
    /**
     * Name or text label of the marker
     */
    name: string;

    /**
     * Offset from start in milliseconds
     */
    position: number;
}

/**
 * Manager for an audio track's markers. Emits callbacks when markers are
 * triggered from playback.
 *
 */
export class MarkerMgr
{
    private track: MultiTrackControl;
    /** Audio position in seconds from last call to update */
    private lastPosition: number;
    /** Internal list of markers */
    private markers: AudioMarker[];
    /** Current index of marker to check */
    private cursor: number;
    /**
     * Flag indicating if `markers` array was modified, thus indicating that
     * the cursor position has been invalidated. During the update loop, the
     * cursor position will be readjusted when `isDirty` was set to `true`.
     */
    private isDirty: boolean;

    /**
     * Callback when marker is passed by playhead
     *
     * param 1: marker object
     * param 2: index in markers array
     */
    readonly onmarker: Callback<[AudioMarker]>;

    /**
     * Notifies when markers container has been modified (new markers pushed or
     * deleted)
     */
    readonly onmarkersupdated: Callback<[]>;

    /**
     * Notifies when cursor index changes. This happens when the marker has
     * been crossed, or when container is modified.
     *
     * Param1: new cursor position
     * Param2: old cursor position
     */
    readonly oncursorchanged: Callback<[number, number]>;

    /** Read-only, mutating results in unwanted and undefined behavior */
    get array() { return this.markers; }

    /** Get the number of markers contained by the manager */
    get length() { return this.markers.length; }

    get current() { return this.cursor; }

    /** The markers are directly iterable */
    [Symbol.iterator]() { return this.markers[Symbol.iterator](); }

    constructor(track: MultiTrackControl)
    {
        this.lastPosition = 0;
        this.cursor = 0;
        this.isDirty = false;

        this.markers = [];
        this.onmarker = new Callback;
        this.onmarkersupdated = new Callback;
        this.oncursorchanged = new Callback;

        this.m_loopStart = null;
        this.m_loopEnd = null;

        // Hook up callbacks to track
        this.handleSeek = this.handleSeek.bind(this);
        this.handleUpdate = this.handleUpdate.bind(this);

        track.onseek.addListener(this.handleSeek);
        track.onupdate.addListener(this.handleUpdate);

        this.track = track;
    }

    private m_loopStart: AudioMarker | null;
    private m_loopEnd: AudioMarker | null;

    get loopStart() { return this.m_loopStart; }
    get loopEnd() { return this.m_loopEnd; }

    set loopStart(marker: AudioMarker)
    {
        if (marker.name !== LOOPSTART_NAME) return;

        const minPosition = 0;
        const maxPosition = Math.min(this.track.length * 1000 - .001,
            this.m_loopEnd ? this.m_loopEnd.position - .001 : Infinity);
        marker.position = Math.max(Math.min(marker.position, maxPosition), minPosition);

        this.m_loopStart = marker;

        this.track.setLoopPoint(marker.position,
            this.m_loopEnd ? this.m_loopEnd.position : this.track.length * .001);
    }

    set loopEnd(marker: AudioMarker)
    {
        if (marker.name !== LOOPEND_NAME) return;

        const minPosition = Math.max(.001, this.m_loopStart ? this.m_loopStart.position + .001 : -Infinity);
        const maxPosition = this.track.length * 1000 - .001;
        marker.position = Math.max(Math.min(marker.position, maxPosition), minPosition);

        this.m_loopEnd = marker;

        this.track.setLoopPoint(this.m_loopStart ? this.m_loopStart.position : 0,
            marker.position);
    }

    loadFromTrack()
    {
        const pointCount = this.track.track.getSyncPointCount();
        for (let i = 0; i < pointCount; ++i)
        {
            const point = this.track.track.getSyncPoint(i);

            // Ignore special markers, use loop points instead
            if (point.name === LOOPSTART_NAME || point.name === LOOPEND_NAME)
            {
                continue;
            }

            this.push({
                name: point.name,
                position: point.offset,
            });
        }

        // load loop points
        const loop = this.track.track.getLoopPoint();

        this.m_loopStart = {
            name: LOOPSTART_NAME,
            position: loop.start
        };

        this.m_loopEnd = {
            name: LOOPEND_NAME,
            position: loop.end
        };
    }

    /**
     * Add a marker to the container/manager. Automatically inserts it in order
     * of position.
     *
     * Special markers "LoopStart" or "LoopEnd" get added as loop points
     * instead of to the main container.
     *
     * @param marker - marker to insert
     */
    push(marker: AudioMarker)
    {


        // clamp marker position within valid range
        marker.position = this.clampTimePosition(marker.position);

        // find the insertion index (by order of position)
        const length = this.markers.length;
        let index = -1;
        for (let i = 0; i < length; ++i)
        {
            if (marker.position < this.markers[i].position)
            {
                index = i;
                break;
            }
        }

        // insert the marker
        if (index === -1)
        {
            this.markers.push(marker);
        }
        else
        {
            this.markers.splice(index, 0, marker);
        }



        this.isDirty = true;
        return marker;
    }

    /** Find marker by name (only the first occurrence) */
    findByName(name: string): AudioMarker | undefined
    {
        return this.markers.find(m => m.name === name);
    }

    /** Find a marker's index with name (only gets the first occurrence) */
    findIndexByName(name: string)
    {
        return this.markers.findIndex(m => m.name === name);
    }

    findIndex(marker: AudioMarker)
    {
        return this.markers.findIndex(m => m === marker);
    }

    /**
     * Update the position of a provided marker.
     * Marker position should not be set directly - only through this function.
     *
     * @param marker      - marker to update position of
     * @param newPosition - position in milliseconds
     */
    updatePosition(marker: AudioMarker, newPosition: number)
    {
        const index = this.markers.findIndex(m => m === marker);
        if (index === -1) return;

        this.updatePositionByIndex(index, newPosition);
    }

    /**
     * Update the position of a marker at the specified index.
     * Marker position should not be set directly - only through this function.
     *
     * @param index       - index of marker to modify
     * @param newPosition - position in milliseconds
     */
    updatePositionByIndex(index: number, newPosition: number)
    {
        const marker = this.markers[index];

        // clamp new position within range
        newPosition = this.clampTimePosition(newPosition);

        // Find new index
        let newIndex = 0;
        const length = this.markers.length;
        for (let i = 0; i < length; ++i)
        {
            // Don't count the marker to be updated
            if (i === index)
            {
                continue;
            }

            // Once the new position is higher, this is the index to splice to
            if (newPosition < this.markers[i].position)
            {
                break;
            }

            ++newIndex;
        }

        // perform splice if index changed
        if (newIndex !== index)
        {
            const temp = [...this.markers];
            temp.splice(index, 1);
            temp.splice(newIndex, 0, marker);
            this.markers = temp;
        }

        marker.position = newPosition;
        this.isDirty = true;
    }

    /**
     * Erase a marker from the container
     * @param {AudioMarker} marker - marker to delete from container. Must be
     *                             a reference of a marker inside, not a copy
     */
    erase(marker: AudioMarker)
    {
        const index = this.markers.findIndex(m => m === marker);
        if (index !== -1)
        {
            this.eraseByIndex(index);
        }
    }

    /** Erase marker at the specified index */
    eraseByIndex(index: number)
    {
        this.markers.splice(index, 1);
        this.isDirty = true;
    }

    /**
     * Bring marker manager to a clean slate, removing all markers, resetting
     * position to 0
     */
    clear()
    {
        this.markers.length = 0;
        this.cursor = 0;
        this.lastPosition = 0;
        this.isDirty = true;

        this.m_loopStart = null;
        this.m_loopEnd = null;
    }

    private handleSeek(time: number)
    {
        this.lastPosition = time * 1000;

        const oldCursor = this.cursor;
        this.calibrateCursor(this.lastPosition);

        if (this.cursor !== oldCursor)
        {
            this.oncursorchanged.invoke(this.cursor, oldCursor);
        }
    }

    private clampTimePosition(position: number)
    {
        return Math.max(Math.min(position, this.track.length * 1000), 0);
    }

    /** Calibrate cursor to a position in the track (in ms) */
    private calibrateCursor(position: number)
    {
        const length = this.markers.length;
        let newCursor = length;
        for (let i = 0; i < length; ++i)
        {
            if (position <= this.markers[i].position)
            {
                newCursor = i;
                break;
            }
        }

        this.cursor = newCursor;
    }

    private handleUpdate(delta: number, position: number)
    {
        position *= 1000;

        const oldCursor = this.cursor;

        if (position !== this.lastPosition) // make sure non-seek, player not paused
        {
            const length = this.markers.length;
            if (this.isDirty) // if dirty, we need to recalibrate cursor
            {
                this.calibrateCursor(this.lastPosition);

                this.onmarkersupdated.invoke();
                this.isDirty = false;
            }

            // Track looped, invoke marker callback on every marker at the end of the track
            if (position < this.lastPosition)
            {
                while (this.cursor < length)
                {
                    this.onmarker.invoke(this.markers[this.cursor++]);
                }
                this.cursor = 0;
            }

            if (this.cursor < length)
            {
                // Invoke markers up until cursor reaches position
                while (this.markers[this.cursor].position <= position)
                {
                    this.onmarker.invoke(this.markers[this.cursor++]);
                    console.log(position, this.lastPosition);

                    if (this.cursor >= length)
                    {
                        break;
                    }
                }
            }


        }

        this.lastPosition = position;

        if (oldCursor !== this.cursor)
        {
            this.oncursorchanged.invoke(this.cursor, oldCursor);
        }
    }

    load(markers: AudioMarker[])
    {
        this.clear();
        markers.forEach(m => this.push(m));
    }
}
