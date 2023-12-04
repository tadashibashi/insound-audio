import { ParamType } from "../ParamType";
import { ParameterBase } from "./ParameterBase";

// Offshoot of C++ NumberParam, simply uses value as 0 or 1
export
class BoolParameter extends ParameterBase
{
    constructor(name: string, index: number, defaultValue: boolean,
        onSetCallback: (index: number, value: number) => void)
    {
        super(name, index, ParamType.Bool, defaultValue ? 1 : 0,
            onSetCallback);
    }
}
