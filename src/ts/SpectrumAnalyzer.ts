
export class SpectrumAnalyzer
{
    private static audioModule: InsoundAudioModule;
    private static instances: SpectrumAnalyzer[] = [];

    private input?: AudioNode;
    private context?: AudioContext;
    private spectrum?: AnalyserNode;
    private mData: Uint8Array;
    private mLastState: string;

    get data(): Uint8Array { return this.mData; }
    set data(val: Uint8Array) { if (val === this.mData) this.mData = val; }

    constructor()
    {
        this.spectrum = undefined;
        this.mLastState = "";

        SpectrumAnalyzer.instances.push(this);
    }

    private setContext(context: AudioContext, input: AudioNode)
    {
        this.context = context;
        this.mLastState = context.state;
        this.spectrum = new AnalyserNode(context, {
            channelCount: 2,
            fftSize: 512,
            channelInterpretation: "discrete"
        });

        input.connect(this.spectrum);

        this.mData = new Uint8Array(this.spectrum.frequencyBinCount);
        this.input = input;
    }

    /**
     * Emplace module to the SpectrumAnalyzers.
     *
     * @param audioModule       - audio module to set, should be
     * @param resetAllAnalyzers - set to true if resetting due to error or
     *                            module crash, etc.
     */
    static setModule(audioModule: InsoundAudioModule,
        resetAllAnalyzers: boolean = false)
    {
        SpectrumAnalyzer.audioModule = audioModule;

        if (resetAllAnalyzers)
        {
            const instanceCount = SpectrumAnalyzer.instances.length;
            for (let i = 0; i < instanceCount; ++i)
            {
                const inst = SpectrumAnalyzer.instances[i];
                inst.context = undefined;
                inst.mData.fill(0);
                inst.mLastState = "";
            }
        }
    }

    /** Update the spectrum data with the current audio */
    update()
    {
        if (!this.context)
        {
            const audioModule = SpectrumAnalyzer.audioModule;
            if (audioModule.mContext && audioModule.mWorkletNode)
            {
                this.setContext(
                    audioModule.mContext,
                    audioModule.mWorkletNode);
            }
        }
        else
        {
            const state = this.context.state;
            if (state === "running")
            {
                this.spectrum?.getByteFrequencyData(this.mData);
            }
            else if (this.mLastState !== state)
            {
                // on first non-running state frame, clear spectrum
                this.data.fill(0);
            }

            this.mLastState = state;
        }
    }

    /**
     * Call this when no longer using this object.
     */
    close()
    {
        if (this.input !== undefined && this.spectrum !== undefined)
        {
            this.input.disconnect(this.spectrum);
            this.input = undefined;
            this.spectrum = undefined;
        }

        const instanceCount = SpectrumAnalyzer.instances.length;
        for (let i = 0; i < instanceCount; ++i)
        {
            if (this === SpectrumAnalyzer.instances[i])
            {
                SpectrumAnalyzer.instances.splice(i, 1);
                break;
            }
        }
    }
}
