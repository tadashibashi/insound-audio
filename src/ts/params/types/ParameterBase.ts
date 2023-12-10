import { ParamType } from "../ParamType";

function lerp(value: number, target: number, percent: number)
{
    return (target - value) * percent + value;
}

// Use finalization registry to clean up any existing intervals
// However, this is a backup to explicitly calling `ParameterBase#clear`
const registry = new FinalizationRegistry((heldValue: any) => {
    if (heldValue && typeof heldValue.interval === "number" && heldValue.inverval !== 0)
    {
        clearInterval(heldValue.interval);
    }
});

export
class ParameterBase
{
    readonly name: string;
    readonly type: ParamType;

    // All parameters have underlying number as value
    protected m_value: number;

    private onSetCallback: (index: number, value: number) => void;
    readonly defaultValue: number;
    readonly index: number;

    private lastValue: number;
    private transitionInterval: { interval: number | null };

    constructor(name: string, index: number, type: ParamType,
        defaultValue: number,
        onSetCallback: (index: number, value: number) => void)
    {
        this.name = name;
        this.type = type;
        this.m_value = defaultValue;
        this.lastValue = this.m_value;
        this.defaultValue = defaultValue;
        this.onSetCallback = onSetCallback;
        this.index = index;

        this.transitionInterval = {interval: null};

        registry.register(this, this.transitionInterval, this);
    }

    get value()
    {
        return this.type === ParamType.Float ? this.m_value :
            Math.round(this.m_value);
    }

    set value(value: number)
    {
        if (this.type === ParamType.Integer)
            value = Math.round(value);

        const lastValue = this.m_value;
        this.lastValue = lastValue;

        if (value !== lastValue)
        {
            this.m_value = value;
            this.onSetCallback(this.index, value);
        }
    }

    get wasUpdated(): boolean
    {
        return this.m_value !== this.lastValue;
    }

    transitionTo(value: number, seconds: number)
    {
        if (isNaN(value))
            return;

        if (this.transitionInterval.interval)
        {
            clearInterval(this.transitionInterval.interval);
        }

        if (seconds <= 0)
        {
            this.value = value;
            return;
        }

        const milliseconds = seconds * 1000;
        const intervalTime = 1000/50; // 50fps
        let time = 0;
        this.transitionInterval.interval = setInterval(() => {
            time += intervalTime;
            this.value = lerp(this.value, value, Math.min(1, time/milliseconds)); // curved, maybe make linear?
            if (time >= milliseconds)
            {
                clearInterval(this.transitionInterval.interval);
                this.transitionInterval.interval = null;
            }
        }, intervalTime);
    }

    clear()
    {
        if (this.transitionInterval.interval)
        {
            clearInterval(this.transitionInterval.interval);
            this.transitionInterval.interval = null;
        }
    }
}
