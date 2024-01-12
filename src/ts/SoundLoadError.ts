export class SoundLoadError extends Error
{
    soundIndices: number[];

    getErrorMessage(names: string[]): string[] {
        return this.soundIndices.map(index => names[index] || "Unknown File");
    }

    constructor(indices: number[])
    {
        super();
        this.soundIndices = indices;
    }
}
