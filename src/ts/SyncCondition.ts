/**
 * Conditions to provide:
 *
 * Every x beat
 * Every x measure
 * Every x beat in every y measure
 * Specific measure + beat
 *
 * Each should return whether range meets condition
 * The bpm, and the seconds of the range should be provided
 * [low exclusive - high inclusive)
 *
 * It should implement Condtitional contract
 * ConditionalGroup
 * Each individual `Conditional` has logical `or` operator
 *
 * It must be indicated the type of sync condition so that the UI can provide
 * the correct inputs. E.g. Every x beat in every y measure and Specific
 * measure + beat both require two integers.
 */

import { IConditional, LogicalOp } from "./Conditional";


export interface TempoMarker
{
    /** Time in seconds */
    position: number;
    tempo: number;
}

export enum SyncType
{
    /** Every `x`th beat */
    EveryBeat,
    /** Every `x`th measure */
    EveryMeasure,
    /** Every `x`th beat in every `y`th measure */
    EveryBeatInMeasure,
    /** Specific beat `x` in measure `y` */
    BeatInMeasure,
}

export interface ISyncCondition
{
    type: SyncType;
    x: number;
    y?: number;
}

/**
 * Container to handle sync conditions
 */
export class SyncConditionGroup implements IConditional
{
    private m_conditions: ISyncCondition[];

    get conditions() { return this.m_conditions; }

    constructor()
    {
        this.m_conditions = [];
        this.not = false;
    }

    logicalOp?: LogicalOp;

    not: boolean;

    evaluate() {
        return true;
    }
}

/**
 * Algorithm:
 * let beat, measure
 * let timeSigTop, timeSigBottom
 * let tempo
 * let curPosition
 * let nextTempoMarker
 *
 * while (curPosition < sectionEnd) each marker, progress 1 timeSigBottom division
 *
 */

/** Get the length of one beat for BPM in seconds */
function beatLength(bpm: number)
{
    return bpm / 60;
}
