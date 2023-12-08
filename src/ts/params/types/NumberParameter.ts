import { ParamType } from "../ParamType";
import { ParameterBase } from "./ParameterBase";

export
class NumberParameter extends ParameterBase
{
    min: number;
    max: number;
    step: number;
    isInteger: boolean;

    constructor(name: string, index: number, min: number, max: number,
        step: number, defaultValue: number, isInteger: boolean,
        onSetCallback: (index: number, value: number) => void)
    {
        // Validate min, max, and step
        if (min > max)
            throw Error("Invalid arguments: Min is greater than Max");
        if (step <= 0 || step > max - min)
            throw Error(
                `Step ${step} is out of range. Valid values: 0 to ${max-min}`);

        super(name, index,
            isInteger ? ParamType.Integer : ParamType.Float,
            defaultValue,
            onSetCallback);

        this.min = min;
        this.max = max;
        this.step = step;
    }

    override set value(v: number)
    {
        const constant = 1/this.step;
        v = Math.round(constant * (v-this.min) ) / constant + this.min;
        v = Math.max(Math.min(v, this.max), this.min);
        super.value = v;
    }

    override get value(): number
    {
        return this.m_value;
    }
}
