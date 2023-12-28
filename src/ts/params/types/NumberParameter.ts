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
        // Validate step
        const diff = Math.abs(max - min);
        if (step <= 0 || step > diff)
            throw Error(
                `Step ${step} is out of range. Valid values: 0 to ${diff}`);

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
        let min: number, max: number;
        if (this.min < this.max)
        {
            min = this.min;
            max = this.max;
        }
        else
        {
            min = this.max;
            max = this.min;
        }

        v = Math.round(constant * (v-min) ) / constant + min;
        v = Math.max(Math.min(v, max), min);
        super.value = v;
    }

    override get value(): number
    {
        return this.m_value;
    }

    override transitionTo(value: number, seconds: number): void {
        if (isNaN(value))
            return;

        let min: number, max: number;
        if (this.min > this.max)
        {
            min = this.max;
            max = this.min;
        }
        else
        {
            min = this.min;
            max = this.max;
        }

        value = Math.min(Math.max(value, min), max);
        super.transitionTo(value, seconds);
    }
}
