import { AudioEngine } from "./AudioEngine";

class ParameterBase
{
    readonly name: string;
    readonly type: ParamType;
    value: number;
    readonly defaultValue: number;

    constructor(name: string, type: ParamType, defaultValue: number)
    {
        this.name = name;
        this.type = type;
        this.value = defaultValue;
        this.defaultValue = defaultValue;
    }

    getValue()
    {
        return this.type === ParamType.Float ? this.value :
            Math.floor(this.value);
    }

    setValue(value: number)
    {
        this.value = this.type === ParamType.Float ? value :
            Math.floor(value);
    }

    transitionTo(value: number, seconds: number)
    {
        throw "not implemented error";
        // TODO: implement via tweens
    }
}

export class NumberParameter extends ParameterBase
{
    min: number;
    max: number;
    step: number;
    isInteger: boolean;

    private interval: NodeJS.Timeout | null;

    constructor(name: string, min: number, max: number, step: number,
        defaultValue: number, isInteger: boolean)
    {
        super(name, isInteger ? ParamType.Integer : ParamType.Float, defaultValue);
        this.min = min;
        this.max = max;
        this.step = step;

        this.interval = null;
    }




}

interface StringValue
{
    value: string;
    index: number;
}

export class StringsParameter extends ParameterBase
{
    values: StringValue[];

    constructor(name: string, values: string[], defaultValue: number = 0)
    {
        super(name, ParamType.Strings, defaultValue);

    }

    setLabel(name: string)
    {
        const index = this.findLabel(name);
        if (index !== -1)
        {
            this.value = index;
        }
    }

    getLabel()
    {
        return this.values[this.getValue()].value;
    }

    private findLabel(name: string)
    {
        for (let i = 0; i < this.values.length; ++i)
            if (name === this.values[i].value)
                return i;
        return -1;
    }
}

export class ParameterMgr
{
    constructor(audio: AudioEngine)
    {

    }

    load(audio: AudioEngine)
    {
        const count = audio.engine.param_count();

        for (let i = 0; i < count; ++i)
        {

        }
    }


}
