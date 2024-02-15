import { SignatureTrack } from "./SignatureTrack";
import { TempoMarker } from "./SyncCondition";

interface MusicMarker {
    /** musical beat number */
    beat: number; // starts at 1
    /** musical measure number */
    measure: number; // starts at 1
    /** current position in track in seconds */
    position: number;
}

/**
 * Uses tempo markers to calculate beats for each horizontal section
 * If no horizontal section, simply start from the start of the track, and
 * whatever BPM marker is available. By default, if there are none, there is
 * one at the beginning that should be 100 bpm.
 */
export class MusicCallbackMgr
{
    markers: MusicMarker[];
    tempos: TempoMarker[];
    signatures: SignatureTrack | null;

    constructor()
    {
        this.tempos = [];
        this.signatures = null;
    }

    positionToBeat(position: number): {measure: number, beat: number}
    {
        if (this.tempos.length === 0 || !this.signatures)
        {
            throw Error("Please call calculateMarkers before calling positionToBeat");
        }

        for (let i = 0; i < this.markers.length; ++i)
        {
            if (this.markers[i].position <= position)
            {
                
            }
        }
    }

    /**
     * Calculate / update marker positions based on tempo / time signature.
     * If a change was made to the tempo markers, this function must be called
     * to adjust to the changes.
     *
     * @param tempoMarkers - tempo markers in order by position
     * @param length       [description]
     */
    calculateMarkers(tempoMarkers: TempoMarker[], signatures: SignatureTrack, length: number)
    {
        let markers: MusicMarker[] = [];
        let nextTempoMarkerIndex = 0; // index in tempoMarkers
        let cursor = 0;               // position in track
        let curTempo = 100            // in musical quarter notes per minute, (default: 100 bpm)
        let curSig = {top: 4, bottom: 4}; // musical time signature, (default: 4/4)
        let curBeat = 0; // 0-indexed here, but musical value will be projected as +1
        let curMeasure = 0;

        // Establish beat markers
        while (cursor < length)
        {

            // apply any tempo updates on beat
            while (nextTempoMarkerIndex < tempoMarkers.length)
            {
                const tempoMarker = tempoMarkers[nextTempoMarkerIndex];
                if (floorDecimal(tempoMarker.position, 2) !== floorDecimal(cursor, 2)) break; // widens precision to 100th of a second for possible discrepancies in other DAWs or human error

                if (tempoMarker.tempo <= 0)
                {
                    console.warn("Cannot apply 0 or negative tempo");
                }
                else
                {
                    curTempo = tempoMarker.tempo;
                }

                ++nextTempoMarkerIndex;
            }

            // apply time signature on downbeat
            if (curBeat === 0)
            {
                const sig = signatures.get(curMeasure + 1);
                if (sig && sig.time)
                {
                    curSig = sig.time;
                }
            }

            // create beat marker
            markers.push({
                beat: curBeat + 1,
                measure: curMeasure + 1,
                position: cursor,
            });

            // progress to next beat
            for (let beatPercent = 0; beatPercent < 1; )
            {
                let secondsPerBeat = 60 / curTempo * 4 / curSig.bottom;
                let projectedNextBeatPos = cursor + (1 - beatPercent) * secondsPerBeat;

                // check if next marker within beat range, apply tempo if so
                if (nextTempoMarkerIndex < tempoMarkers.length)
                {
                    const nextMarker = tempoMarkers[nextTempoMarkerIndex];

                    if (nextMarker.position < projectedNextBeatPos)
                    {
                        if (nextMarker.tempo !== undefined)
                        {
                            if (nextMarker.tempo <= 0)
                            {
                                console.warn("Cannot apply 0 or negative tempo");
                            }
                            else
                            {
                                curTempo = nextMarker.tempo;
                                beatPercent += (nextMarker.position - cursor) / secondsPerBeat;
                                cursor = nextMarker.position;
                            }
                        }

                        ++nextTempoMarkerIndex;
                    }
                    else
                    {
                        cursor = projectedNextBeatPos;
                        break;
                    }
                }
                else
                {
                    cursor = projectedNextBeatPos;
                    break;
                }
            }

            // Update current measure & beat info
            if (curBeat + 1 >= curSig.top)
            {
                ++curMeasure;
            }

            curBeat = (curBeat + 1) % curSig.bottom;
        }

        this.markers = markers;
        this.tempos = tempoMarkers;
        this.signatures = signatures;
    }
}

/**
 * Return a decimal number with precision up to the number of decimal places
 * @param num   - number to floor
 * @param place - number of decimal places of precision
 *                (e.g. 2 decimal places -> 10.1234 = 10.12)
 */
function floorDecimal(num, place)
{
    const flooringConst = Math.pow(10, place);
    return Math.floor(num * flooringConst) / flooringConst;
}
