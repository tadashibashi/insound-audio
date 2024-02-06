import { ParamType } from "../ParamType";
import { ParameterBase } from "./ParameterBase";

export class StringsParameter extends ParameterBase
{
    private m_values: string[];

    get values() { return this.m_values; }

    constructor(name: string, index: number, values: string[],
        defaultValue: number = 0,
        onSetCallback: (index: number, value: number) => void)
    {
        super(name, index, ParamType.Strings, defaultValue, onSetCallback);
        this.m_values = values;
    }

    /**
     * Set label by name or directly by index.
     * If a name is passed, and none could be found, set will be ignored.
     *
     * @param nameOrIndex - label name or underlying index value
     */
    set label(nameOrIndex: string | number)
    {
        if (typeof nameOrIndex === "string")
        {
            const index = this.findLabel(nameOrIndex);
            if (index !== -1)
                this.value = index;
        }
        else
        {
            this.value = nameOrIndex;
        }
    }

    /**
     * Get the label for the current value
     */
    get label(): string
    {
        return this.m_values[Math.floor(this.value)];
    }

    private findLabel(name: string): number
    {
        for (let index = 0; index < this.m_values.length; ++index)
            if (name === this.m_values[index])
                return index;
        return -1;
    }

    get labelCount(): number { return this.m_values.length; }

    /**
     * Get the label for a specific index
     *
     * @param  index - the index to query
     *
     * @returns label string at the specified index
     *
     * @throws RangeError, if index is out of range.
     */
    labelAt(index: number): string
    {
        const val = this.m_values.at(index);
        if (val === undefined)
            throw RangeError("Label index is out of range");
        return val;
    }

    copyLabels(): string[]
    {
        return [...this.m_values];
    }
}
