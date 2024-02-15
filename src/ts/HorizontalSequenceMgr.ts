import { ConditionalGroup } from "./Conditional";
import { MarkerTransition } from "./MarkerMgr";

enum NoteType {
    Beat,
    Measure,
    BeatInMeasure,
}

interface ExitCondition
{
    condition: ConditionalGroup;
    transition: MarkerTransition;
    sync?: {
        noteType: NoteType;

    };
}

export class HorizontalSection
{
    start: number;
    end: number;
    exitConditions: ExitCondition[];

    constructor()
    {
        this.exitConditions = [];
        this.start = 0;
        this.end = 0;
    }
}




export class HorizontalSequenceMgr
{
    private m_current: HorizontalSection | null = null;
    private m_sections: HorizontalSection[];


}
