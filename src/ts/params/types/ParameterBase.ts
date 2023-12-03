import { ParamType } from "../ParamType";

export class ParameterBase
{
    readonly name: string;
    readonly type: ParamType;

    // All parameters have underlying number as value
    private m_value: number;

    private onSetCallback: (index: number, value: number) => void;
    readonly defaultValue: number;
    readonly index: number;

    constructor(name: string, index: number, type: ParamType,
        defaultValue: number,
        onSetCallback: (index: number, value: number) => void)
    {
        this.name = name;
        this.type = type;
        this.m_value = defaultValue;
        this.defaultValue = defaultValue;
        this.onSetCallback = onSetCallback;
        this.index = index;
    }

    get value()
    {
        return this.type === ParamType.Float ? this.m_value :
            Math.floor(this.m_value);
    }

    set value(value: number)
    {
        if (this.type === ParamType.Integer)
            value = Math.floor(value);
        this.m_value = value;
        this.onSetCallback(this.index, value);
    }

    transitionTo(value: number, seconds: number)
    {
        throw "not implemented error";
        // TODO: implement via tweens
    }
}
