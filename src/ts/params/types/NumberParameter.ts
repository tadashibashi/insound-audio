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
        v = Math.max(Math.min(v, this.max), this.min);
        super.value = v;
    }

    override get value(): number
    {
        return this.m_value;
    }
}
