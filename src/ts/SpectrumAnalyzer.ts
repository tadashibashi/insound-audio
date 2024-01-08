
export class SpectrumAnalyzer
{
    private audioModule: InsoundAudioModule;
    private input?: AudioNode;
    private context?: AudioContext;
    private spectrum?: AnalyserNode;
    private mData: Uint8Array;

    get data(): Uint8Array { return this.mData; }

    constructor(audioModule: InsoundAudioModule)
    {
        this.audioModule = audioModule;
        this.spectrum = undefined;
    }

    private setContext(context: AudioContext, input: AudioNode)
    {
        this.context = context;
        this.spectrum = new AnalyserNode(context, {
            channelCount: 2,
            fftSize: 512,
            channelInterpretation: "discrete"
        });

        input.connect(this.spectrum);

        this.mData = new Uint8Array(this.spectrum.frequencyBinCount);
        this.input = input;
    }

    /** For when audio engine resets, emplace a new audio context */
    setModule(audioModule: InsoundAudioModule)
    {
        this.context = undefined;
        this.audioModule = audioModule;
    }

    /** Update the spectrum data with the current audio */
    update()
    {
        if (!this.context)
        {
            if (this.audioModule.mContext && this.audioModule.mWorkletNode)
            {
                this.setContext(this.audioModule.mContext,
                    this.audioModule.mWorkletNode);
            }
        }
        else
        {
            if (this.context.state === "running")
            {
                this.spectrum?.getByteFrequencyData(this.mData);
            }
            else
            {
                this.data.fill(0);
            }

        }
    }

    close()
    {
        if (this.input !== undefined && this.spectrum !== undefined)
        {
            this.input.disconnect(this.spectrum);
            this.input = undefined;
            this.spectrum = undefined;
        }
    }
}
