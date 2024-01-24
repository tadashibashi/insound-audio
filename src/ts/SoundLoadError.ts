export class SoundLoadError extends Error
{
    info: {index: number, reason: string}[];

    constructor(info: {index: number, reason: string}[])
    {
        super();
        this.info = info;
    }
}
