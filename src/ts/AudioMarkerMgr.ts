import { MarkerMgr } from "./MarkerMgr";
import { type MultiTrackControl } from "./MultiTrackControl";

export interface MarkerTransition
{
    destination: AudioMarker;
    inTime: number;
    fadeIn: boolean;
    outTime: number;
    fadeOut: boolean;
}

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

    transition?: MarkerTransition;
}


/**
 * Manager for an audio track's markers. Emits callbacks when markers are
 * triggered from playback.
 */
export class AudioMarkerMgr extends MarkerMgr<AudioMarker>
{
    /** The markers are directly iterable */
    [Symbol.iterator]() { return this.markers[Symbol.iterator](); }

    constructor(track: MultiTrackControl)
    {
        super(track);
    }

    /**
     * Populate the marker manager from the current sync points in the track
     * set in the constructor
     */
    loadFromTrack(): void
    {
        const track = this.track;
        this.clear();

        const pointCount = track.track.getSyncPointCount();
        for (let i = 0; i < pointCount; ++i)
        {
            this.push(track.track.getSyncPoint(i));
        }
    }

    protected override cleanMarkers(): boolean
    {
        let altered = false;

        // Check that transitions are pointing to alive transition
        for (const marker of this.markers)
        {
            if (marker.transition)
            {
                // remove it otherwise
                if (!this.markers.find(m => m === marker.transition.destination))
                {
                    marker.transition = undefined;
                    altered = true;
                }
            }
        }

        return altered;
    }

    override push(marker: AudioMarker)
    {
        // clamp marker position within valid range
        marker.position = Math.max(Math.min(marker.position, this.track.length), 0);

        return super.push(marker);
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
}
