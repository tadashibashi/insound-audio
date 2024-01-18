import { ParameterBase } from "./params/types/ParameterBase";
import { NumberParameter } from "./params/types/NumberParameter";
import { MultiTrackControl } from "./MultiTrackControl";
import { ParamType } from "./params/ParamType";
import { BoolParameter } from "./params/types/BoolParameter";
import { StringsParameter } from "./params/types/StringsParameter";

interface AudioChannelParameters {
    readonly volume: NumberParameter;
    readonly panLeft: NumberParameter;
    readonly panRight: NumberParameter;
    readonly reverb: NumberParameter;
};

type ExtractParamType<T extends ParameterBase> =
    T extends NumberParameter ? number  :
    T extends StringsParameter ? string :
    T extends BoolParameter ? boolean   : never;

type AudioChannelPTypes = {
    [key in keyof AudioChannelParameters]: ExtractParamType<AudioChannelParameters[key]>;
};

/**
 * Set up channel parameter defaults
 *
 * @param {MultiTrackControl} ctrl    track controller
 * @param {number}            channel channel number
 */
function createDefaultParameters(ctrl: MultiTrackControl, channel: number)
{
    return {
        volume: new NumberParameter("Volume", channel, 0, 1.25, .01, 1, false,
            (i, val) => ctrl.track.setVolume(channel, val)),

        reverb: new NumberParameter("Reverb", channel, 0, 2, .01, 0, false,
            (i, val) => ctrl.track.setReverbLevel(channel, val)),

        panLeft: new NumberParameter("L", channel, 100, 0, 1, 100, true,
        (i, val) => ctrl.track.setPanLeft(channel, val * .01)),

        panRight: new NumberParameter("R", channel, 0, 100, 1, 100, true,
            (i, val) => ctrl.track.setPanRight(channel, val * .01)),
    };
}

/** Audio channel settings, aligns with AudioChannel members to be set */
export interface AudioChannelSettings
{
    /** Channel name */
    name: string;
    params: Partial<AudioChannelPTypes>;
}

export class AudioChannel
{
    name: string;
    readonly params: AudioChannelParameters;

    constructor(ctrl: MultiTrackControl, name: string, channel: number)
    {
        this.name = name;
        this.params = createDefaultParameters(ctrl, channel);
    }

    getCurrentSettings(): AudioChannelSettings
    {
        const params: Partial<AudioChannelPTypes> = {};

        for (const [key, value] of Object.entries(this.params))
        {
            params[key] = value.value;
        }

        return {
            name: this.name,
            params,
        };
    }

    getDefaultSettings(): AudioChannelSettings
    {
        const params: Partial<AudioChannelPTypes> = {};

        for (const [key, value] of Object.entries(this.params))
        {
            params[key] = value.defaultValue;
        }

        return {
            name: this.name,
            params,
        }
    }

    /**
     * Convenience method to apply many values at once
     *
     * @param settings - values to set
     * @param seconds  - time to transition to settings values in seconds
     */
    applySettings(settings: AudioChannelSettings, seconds: number = 0)
    {
        const keys = Object.keys(settings.params) as (keyof AudioChannel["params"])[];

        this.name = settings.name;

        for (const key of keys)
        {
            const member = this.params[key];
            const value = settings.params[key];

            if (member instanceof ParameterBase)
            {
                switch(member.type)
                {
                case ParamType.Bool:
                    {
                        if (typeof value !== "boolean")
                        {
                            throw Error(`AudioChannelSettings member "${key}" must be a boolean, but got ${typeof value} instead.`);
                        }

                        member.transitionTo(value ? 1 : 0, seconds);
                    }
                    break;

                case ParamType.Integer:
                case ParamType.Float:
                    {
                        if (typeof value !== "number")
                        {
                            throw Error(`AudioChannelSettings member "${key}" must be a number, but got ${typeof value} instead.`);
                        }

                        member.transitionTo(value, seconds);
                    }
                    break;

                case ParamType.Strings:
                    {
                        if (typeof value !== "string")
                        {
                            throw Error(`AudioChannelSettings member "${key}" must be a number, but got ${typeof value} instead.`);
                        }

                        // Immediately set regardless of transitionTime
                        (member as unknown as StringsParameter).label = value;
                    }
                    break;
                }
            }
            else // check for primitive types, set instantly
            {
                switch(typeof member)
                {
                case "number":
                    if (typeof value !== "number")
                        throw Error(`AudioChannelSettings member "${key}" must be a number, but got ${typeof value} instead.`);
                    (this[key] as unknown as number) = value;
                    break;

                case "string":
                    if (typeof value !== "string")
                        throw Error(`AudioChannelSettings member "${key}" must be a string, but got ${typeof value} instead.`);
                    (this[key] as unknown as string) = value;
                    break;

                default:
                    throw Error(`AudioChannelSettings member "${key}" type ${typeof member} is not supported.`);
                }
            }
        }
    }

    /**
     * Reset all values to their default values
     *
     * @param seconds - time to make transition in seconds
     */
    reset(seconds: number = 0)
    {
        for (const param of Object.values(this.params))
        {
            param.reset(seconds);
        }
    }

    /**
     * Use when cleaning up, clears any current intervals from any parameter
     * transitions.
     */
    clear()
    {
        for (const param of Object.values(this.params))
        {
            param.clear();
        }
    }
}
