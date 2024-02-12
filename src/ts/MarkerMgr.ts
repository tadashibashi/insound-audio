import { Callback } from "./Callback";
import { type MultiTrackControl } from "./MultiTrackControl";

export interface IMarker
{
    position: number;
}

/**
 * Manages a list of markers in order by position.
 * Fires callbacks right before marker with clock time for sample accurate
 * synchronization.
 */
export class MarkerMgr<T extends IMarker>
{
    /** list of markers */
    protected markers: T[];

    protected track: MultiTrackControl;

    /** **read-only** list of markers sorted in order of position */
    get array(): T[] { return this.markers; }

    /** Get the number of markers contained by the manager */
    get length() { return this.markers.length; }

    /** index of next marker */
    protected cursor: number;

    /** set when an alteration to markers occurs */
    protected isDirty: boolean;

    /**
     * Callback when marker is just about to pass playhead (~.1 second before)
     * Sample-accurate timings can be planned using the second parameter, which
     * contains the dsp clock of the target marker.
     * You can use a timeout to approximate exact timing via marker position.
     *
     * param 1: marker object
     * param 2: dsp clock of marker
     */
    readonly onmarker: Callback<[T, number]>;

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

    constructor(track: MultiTrackControl)
    {
        this.markers = [];
        this.cursor = 0;
        this.isDirty = false;

        this.onmarker = new Callback;
        this.onmarkersupdated = new Callback;
        this.oncursorchanged = new Callback;

        this.track = track;

        this.update = this.update.bind(this);
        this.seek = this.seek.bind(this);

        track.onupdate.addListener(this.update);
        track.onseek.addListener(this.seek);
    }

    /**
     * Add a marker to the container/manager. Automatically inserts it in order
     * of position.
     *
     * @param marker - marker to insert
     */
    push(marker: T)
    {
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

    /**
     * Called when markers are dirty and are requested to be cleaned
     * Should be overriden with desired behavior for each marker subclass
     *
     * @return whether alteration was made to the markers array
     */
    protected cleanMarkers(): boolean { return false; }

    /**
     * Update the state of the container
     */
    private update(): void
    {
        const track = this.track;
        if (track.isPaused) return;

        let position = track.position;
        const oldCursor = this.cursor;

        if (this.isDirty) // if dirty, we need to recalibrate cursor
        {
            this.calibrateCursor(position);
            if (this.cleanMarkers())
            {
                this.onmarkersupdated.invoke();
            }

            this.isDirty = false;
        }

        for (let marker = this.markers[this.cursor]; this.cursor < this.markers.length; )
        {
            position = track.position; // (update in case seek occurs in onmark callback)

            let checkSec = (marker.position < position) ? // loop probably occured, check position by added length
                marker.position + track.length:
                marker.position;

            if (checkSec - position < .1)
            {
                // get target clock for marker
                const clock = track.track.dspClock() + track.track.samplerate() * (checkSec - position);

                // increment cursor/marker for next check (this happens before callback invocation, since transition may update cursor during callback)
                this.cursor = (this.cursor + 1) % this.markers.length;

                // fire callback
                this.onmarker.invoke(marker, clock);

                marker = this.markers[this.cursor];
            }
            else
            {
                break;
            }
        }

        if (oldCursor !== this.cursor)
        {
            this.oncursorchanged.invoke(this.cursor, oldCursor);
        }
    }

    /**
     * Simulate a seek-to-position, updating this container's cursor.
     * This should be hooked up to a callback when a track that owns this
     * container seeks.
     */
    private seek(position: number): void
    {
        const oldCursor = this.cursor;
        this.calibrateCursor(position);

        if (this.cursor !== oldCursor)
        {
            this.oncursorchanged.invoke(this.cursor, oldCursor);
        }
    }

    /**
     * Erase a marker
     * @param  marker - marker to erase from the container
     * @return        whether marker was erased
     */
    erase(marker: T): boolean
    {
        const index = this.markers.findIndex(m => m === marker);
        if (index !== -1)
        {
            this.eraseByIndex(index);
            return true;
        }

        return false;
    }

    /**
     * Erase marker at the specified index
     * @param index - index of marker to delete (0-based)
     */
    eraseByIndex(index: number): boolean
    {
        if (index < this.markers.length && index >= 0)
        {
            this.markers.splice(index, 1);
            this.isDirty = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    editPosition(marker: T, position: number): boolean
    {
        return this.editPositionByIndex(this.findIndexOf(marker), position);
    }

    /**
     * Edit a pre-existing marker's position
     * @param marker   - marker to edit position of
     * @param position - position to set
     * @return  whether marker position was edited - it will not if position is
     *          already the same as marker's position, or if marker does not
     *          belong to this container
     */
    editPositionByIndex(index: number, position: number): boolean
    {
        const marker = this.markers[index];
        if (marker.position === position) return false;
        if (index === -1) return false;

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
            if (position < this.markers[i].position)
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

            this.isDirty = true; // set dirty flag only if index changed
        }

        marker.position = position;

        // signal update for frontend library
        this.markers = this.markers;

        return true;
    }

    /**
     * Remove all markers, reset state
     */
    clear(): void
    {
        this.markers.length = 0;
        this.cursor = 0;
        this.isDirty = true;

        // signal update for frontend library
        this.markers = this.markers;
    }

    /**
     * Find the index of a marker stored in the container, or -1 if it doesn't
     * belong to the container.
     * @param   marker - marker to find index of
     * @return         index of marker, or -1 if it was not found
     */
    findIndexOf(marker: T): number
    {
        return this.markers.findIndex(m => m === marker);
    }

    // ===== Helpers ==========================================================

    /** Calibrate cursor to a position in the track (in seconds) */
    private calibrateCursor(position: number): void
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
}

