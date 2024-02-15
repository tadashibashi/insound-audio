/**
 * Music letter enumeration, with underlying number of half-steps from C
 */
export enum Letter
{
    C=0, D=2, E=4, F=5, G=7, A=9, B=11,
}

export enum Accidental
{
    Natural,
    Sharp,
    Flat,
    DoubleSharp,
    DoubleFlat
}

/**
 * Quality of a musical key, includes scales and Messaien's modes of limited
 * transposition. This is based on well-tempered tuned scales with
 * 12 half-steps.
 */
export enum KeyQuality
{
    Minor,
    Major,
    Hexatonic,      // 0 1 4 5 8 9
    Augmented,      // 0 3 4 7 8 11
    Chromatic,      // half-...
    WholeTone,      // Messaien mode 1: whole-...
    Octatonic,      // Messaien mode 2 / diminished scale: half-whole-...
    MessaienMode3,  // whole-half-half-...
    MessaienMode4,  // half-half-m3-half-half-half-m3-half
    MessaienMode5,  // half-M3-half-half-M3-half
    MessaienMode6,  // whole-whole-half-half-whole-whole-half-half
    MessaienMode7,  // half-half-half-whole-half-half-half-half-whole-half
}

interface TimeSignature {
    top: number;
    bottom: number;
}

interface MusicKey {
    /** Root */
    letter: Letter,
    accidental: Accidental,

    /** Describes what kind of key or scale */
    quality: KeyQuality,
    /**
     * Degree of the scale from which the mode starts
     * e.g. Major, with mode 2 is dorian
     * Also, a mode of 1 is redundant
     */
    mode?: number,
}

export interface MusicSignatureInfo {
    time: TimeSignature;
    key: MusicKey
}

export interface MusicSignature
{
    /** Music measure (starts count at 1) */
    measure: number;
    time?: TimeSignature;
    key?: MusicKey
}


/**
 * Stores a list of time signature
 * (extendable to other metadata that pertains to a music measure),
 * in measure order.
 */
export class SignatureTrack
{
    m_sigs: MusicSignature[];

    /** Read-only, array of signatures */
    get array() { return this.m_sigs; }

    constructor(sigs?: MusicSignature[])
    {
        this.m_sigs = [];

        if (sigs)
        {
            sigs.forEach(sig => this.m_sigs.push(sig));
        }
    }

    /**
     * Get signature info object at the specified measure, or null if there is
     * none there.
     * @param measure - music measure number to query - starts count at 1
     */
    get(measure: number): MusicSignature | null
    {
        const length = this.m_sigs.length;
        for (let i = 0; i < length; ++i)
        {
            if (this.m_sigs[i].measure === measure)
            {
                return this.m_sigs[i];
            }
        }

        return null;
    }

    /**
     * Get key and time signature at a particular measure; info carries over
     * from previous measures as the last signature that was passed over.
     * If there were none the defaults are, time signature 4/4, key C Major
     */
    signatureAt(measure: number): MusicSignatureInfo
    {
        if (measure <= 0)
        {
            throw Error("Invalid measure number");
        }

        const res: MusicSignatureInfo = {
            key: {
                letter: Letter.C,
                accidental: Accidental.Natural,
                quality: KeyQuality.Major,
            },
            time: {
                top: 4,
                bottom: 4,
            },
        };

        const sigs = this.m_sigs;
        const length = sigs.length;
        for (let i = 0; i < length && sigs[i].measure <= measure; ++i)
        {
            const curSig = sigs[i];
            if (curSig.key)
            {
                res.key = curSig.key;
            }

            if (curSig.time)
            {
                res.time = curSig.time;
            }
        }

        return res;
    }

    pushArray(sigs: MusicSignature[])
    {
        sigs.forEach(sig => this.push(sig));
    }

    /**
     * Pushes signature object into the container in order by measure number.
     * If there is already a signature object at the indicated measure number,
     * any values present will overwrite the values in that object.
     * @param {MusicSignature} signature - object to add
     */
    push(signature: MusicSignature)
    {
        if (signature.measure <= 0)
        {
            throw Error("Cannot push signature: invalid measure number `" +
                signature.measure + "`");
        }

        // visit each signature to find insertion index
        const length = this.m_sigs.length;
        for (let i = 0; i < length; ++i)
        {
            const curSig = this.m_sigs[i];

            // override any signature with the same measure number
            if (curSig.measure === signature.measure)
            {
                if (signature.key)
                    curSig.key = signature.key;
                if (signature.time)
                    curSig.time = signature.time;
                return;
            }

            // signature has smaller measure number than current, insert it
            if (signature.measure < curSig.measure)
            {
                this.m_sigs.splice(i, 0, signature);
                return;
            }
        }

        // measure number is greater than all - push it to end
        this.m_sigs.push(signature);

        // Update for reactive components
        this.m_sigs = this.m_sigs;
    }

    /**
     * Get number of signature objects inside this container
     */
    get length(): number
    {
        return this.m_sigs.length;
    }

    clear()
    {
        this.m_sigs = [];
    }
}
