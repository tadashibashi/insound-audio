import { Callback } from "./Callback";
import { MultiTrackControl } from "./MultiTrackControl";

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
    /** Audio position in seconds from last call to update */
    private lastPosition: number;
    private markers: AudioMarker[];
    private cursor: number;
    private isDirty: boolean;

    /**
     * Callback when marker is passed by playhead
     *
     * param 1: marker object
     * param 2: index in markers array
     */
    readonly onmarker: Callback<[AudioMarker]>;

    /** Read-only, mutating results in unwanted and undefined behavior */
    get array() { return this.markers; }

    get length() { return this.markers.length; }
    [Symbol.iterator]() { return this.markers[Symbol.iterator](); }

    constructor(track: MultiTrackControl)
    {
        this.lastPosition = 0;
        this.cursor = 0;
        this.isDirty = false;

        this.markers = [];
        this.onmarker = new Callback;

        // Hook up callbacks to track
        this.handleSeek = this.handleSeek.bind(this);
        this.handleUpdate = this.handleUpdate.bind(this);

        track.onseek.addListener(this.handleSeek);
        track.onupdate.addListener(this.handleUpdate);
    }

    push(marker: AudioMarker)
    {
        // find the index to insert to
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

    findByName(name: string): AudioMarker | undefined
    {
        return this.markers.find(m => m.name === name);
    }

    findIndexByName(name: string)
    {
        return this.markers.findIndex(m => m.name === name);
    }

    updatePositionByIndex(index: number, newPosition: number)
    {
        const marker = this.markers[index];

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
            this.markers.splice(index, 1);
            this.isDirty = true;
        }
    }

    eraseByIndex(index: number)
    {
        this.markers.splice(index, 1);
        this.isDirty = true;
    }

    clear()
    {
        this.markers.length = 0;
        this.cursor = 0;
        this.lastPosition = 0;
        this.isDirty = true;
    }

    private handleSeek(time: number)
    {
        this.lastPosition = time * 1000;
        this.isDirty = true;
    }

    private handleUpdate(delta: number, position: number)
    {
        position *= 1000;

        if (position !== this.lastPosition) // make sure non-seek, player not paused
        {
            const length = this.markers.length;
            if (this.isDirty) // if dirty, we need to recalibrate cursor
            {

                let newCursor = length - 1;
                for (let i = 0; i < length; ++i)
                {
                    if (this.lastPosition <= this.markers[i].position)
                    {
                        newCursor = i;
                        break;
                    }
                }

                this.cursor = newCursor;
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

            // Invoke markers up until cursor reaches position
            while (this.markers[this.cursor].position <= position)
            {
                this.onmarker.invoke(this.markers[this.cursor++]);

                if (this.cursor >= length)
                {
                    this.cursor = 0;
                    break;
                }
            }
        }

        this.lastPosition = position;
    }

    load(markers: AudioMarker[])
    {
        this.clear();
        markers.forEach(m => this.push(m));
    }
}
