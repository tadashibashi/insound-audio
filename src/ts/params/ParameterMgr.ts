import { BoolParameter } from "./types/BoolParameter";
import { NumberParameter } from "./types/NumberParameter";
import { StringsParameter } from "./types/StringsParameter";
import { Callback } from "../Callback";
import { MultiTrackControl } from "../MultiTrackControl";

// Contains non-virtual children of ParameterBase
export type Param = NumberParameter | StringsParameter | BoolParameter;

export interface ParamConfig {
    name: string;
    defaultValue: number;
    type: "strings" | "float" | "int" | "bool";

    /**
     * Choices to pick from from a strings parameter.
     * If `type === "strings"`, this field will be available
     */
    strings?: string[];
    number?: {
        min?: number;
        max?: number;
        step?: number;
    };
}

/**
 * Create parameter from config
 */
function makeParam(config: ParamConfig, onset: (index: number, value: number) => void, index: number): Param
{
    switch(config.type)
    {
        case "strings":
        {
            return new StringsParameter(config.name, index, config.strings || [],
                config.defaultValue, onset);
        }
        case "float":
        {
            const num = config.number;
            if (!num)
            {
                throw Error("Float parameter must have number field");
            }

            return new NumberParameter(config.name, index,
                num.min || Number.MIN_VALUE, num.max || Number.MAX_VALUE,
                config.number.step || .001, config.defaultValue, false, onset);
        }
        case "int":
        {
            const num = config.number;
            if (!num)
            {
                throw Error("Int parameter must have number field");
            }

            return new NumberParameter(config.name, index,
                num.min ? Math.floor(num.min) : Number.MIN_SAFE_INTEGER,
                num.max ? Math.floor(num.max) : Number.MAX_SAFE_INTEGER,
                1, config.defaultValue, true, onset);
        }
        case "bool":
        {
            return new BoolParameter(config.name, index, !!config.defaultValue, onset);
        }
        default:
        {
            throw Error("Invalid parameter type provided");
        }
    }

}

export class ParameterMgr
{
    private m_params: Param[];
    private m_map: Map<string, Param>;
    private track: MultiTrackControl;

    /** Read-only, reactive on new additions or clearance */
    get params() { return this.m_params; }

    /** Called when parameter map is updated */
    readonly onupdated: Callback<[]>;

    constructor(track: MultiTrackControl)
    {
        this.m_map = new Map;
        this.m_params = [];
        this.track = track;

        this.handleParamSet = this.handleParamSet.bind(this);
    }

    /**
     * Add a parameter to the container from a config
     */
    addParameter(config: ParamConfig)
    {
        const newParam = makeParam(config, this.handleParamSet,
            this.m_params.length);

        this.m_params.push(newParam);
        this.m_map.set(newParam.name, newParam);

        // notify frontend library
        this.m_params = this.m_params;
    }

    /** On param set */
    private handleParamSet(index: number, value: number)
    {
        // send signal to lua
        const p = this.m_params[index];
        if (p)
        {
            this.track.track.setParameter(p.name, p.value);
        }
    }

    /**
     * Set a parameter
     * @param index - numbered index of value or name of parameter
     * @param value - value to set
     */
    setParameter(index: number | string, value: number)
    {
        if (typeof index === "number")
        {
            const p = this.m_params[index];
            if (p)
            {
                p.value = value;
            }
        }
        else
        {
            const p = this.m_map.get(index);
            if (p)
            {
                p.value = value;
            }
        }

    }

    /**
     * Clear the ParameterMgr with option to emplace new internals
     * @param params? - new parameter array
     * @param pMap?   - new parameter map
     */
    clear(): void
    {
        this.m_map.clear();
        this.m_params.length = 0;

        this.m_params = this.m_params;
    }

    /**
     * Get the number of parameters stored in this container
     */
    get size(): number
    {
        return this.m_params.length;
    }

    /**
     * Get a parameter
     * @param {number | string} indexOrName - index or name of parameter to get
     *
     * @returns parameter or undefined, if it could not be found.
     *
     */
    get(indexOrName: number | string):
        Param | undefined
    {
        if (typeof indexOrName === "number")
        {
            const val = this.m_params.at(indexOrName);
            if (val === undefined)
                throw RangeError("Index is out of range");
            return val;
        }
        else
        {
            const val = this.m_map.get(indexOrName);
            if (val === undefined)
                throw RangeError("No parameter with that name exists");
            return val;
        }
    }
}
