
/**
 * Basic event broadcaster for one event.
 * Doesn't support removing and adding callback during callback invoke.
 */
export class Callback<Args extends any[]> {

    private callbacks: ((...args: Args) => void)[];

    constructor()
    {
        this.callbacks = [];
    }

    addListener(listener: (...args: Args) => void)
    {
        this.callbacks.push(listener);
    }

    removeListener(listener: (...args: Args) => void)
    {
        const size = this.callbacks.length;
        for (let i = 0; i < size; ++i)
        {
            if (this.callbacks[i] === listener)
            {
                this.callbacks.splice(i, 1);
                break;
            }
        }
    }

    invoke(...args: Args): void
    {
        for (let i = 0; i < this.callbacks.length; ++i)
        {
            this.callbacks[i](...args);
        }
    }
}
