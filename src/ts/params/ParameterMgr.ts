import { AudioEngine } from "../AudioEngine";
import { NumberParameter } from "./types/NumberParameter";
import { StringsParameter } from "./types/StringsParameter";
import { ParamType } from "./Paramtype";

export class ParameterMgr
{
    private params: (NumberParameter | StringsParameter)[];
    private paramMap: Map<string, NumberParameter | StringsParameter>;
    private audio: AudioEngine;

    constructor(audio?: AudioEngine)
    {
        this.params = [];
        this.paramMap = new Map;
        if (audio)
            this.load(audio);

        // bind this to callbacks
        this.handleParamSet = this.handleParamSet.bind(this);
        this.handleParamReceive = this.handleParamReceive.bind(this);
    }

    handleParamSet(index: number, value: number)
    {
        if (!this.audio) return;

        this.audio.engine.param_send(index, value);
    }

    handleParamReceive(index: number | string, value: number)
    {
        if (typeof index === "number")
            this.params[index].value = value;
        else
            this.paramMap.get(index).value = value;
    }

    /**
     * Load parameter info from the currently loaded track in the audio engine
     *
     * @param {AudioEngine} audio - the audio engine to load the params from
     */
    load(audio: AudioEngine)
    {
        const paramCount = audio.engine.param_count();
        const params: (NumberParameter | StringsParameter)[] = [];
        const paramMap: Map<string, NumberParameter | StringsParameter> =
            new Map;

        for (let i = 0; i < paramCount; ++i)
        {
            const name = audio.engine.param_getName(i);
            const type = audio.engine.param_getType(i);
            if (type === ParamType.Strings)
            {
                const p = audio.engine.param_getAsStrings(i);
                const newParam = new StringsParameter(name, i, p.values,
                 p.defaultValue, this.handleParamSet);
                params.push(newParam);
                paramMap.set(name, newParam);
            }
            else
            {
                const p = audio.engine.param_getAsNumber(i);
                const newParam = new NumberParameter(name, i, p.min, p.max,
                    p.step, p.defaultValue, type === ParamType.Integer,
                    this.handleParamSet);
                params.push(newParam);
                paramMap.set(name, newParam);
            }
        }

        this.params = params;
        this.paramMap = paramMap;
        this.audio = audio;
    }

    clear(): void
    {
        this.params.length = 0;
        this.paramMap.clear();
    }

    /**
     * Get the number of parameters stored in this container
     */
    get count(): number
    {
        return this.params.length;
    }

    /**
     * Get a parameter
     * @param {number | string} indexOrName - index or name of parameter to get
     *
     * @returns parameter or undefined, if it could not be found.
     *
     */
    get(indexOrName: number | string):
        NumberParameter | StringsParameter | undefined
    {
        if (typeof indexOrName === "number")
        {
            const val = this.params.at(indexOrName);
            if (val === undefined)
                throw RangeError("Index is out of range");
            return val;
        }
        else
        {
            const val = this.paramMap.get(indexOrName);
            if (val === undefined)
                throw RangeError("No parameter with that name exists");
            return val;
        }
    }
}
