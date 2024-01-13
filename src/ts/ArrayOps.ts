    /**
     * An enumeration of action indices that is used in the `transformArray`
     * functions. This function uses the reducer pattern such as in Redux.
     */
    export enum ArrayOp
    {
        /** Splice an object from an array and insert it at another index */
        SpliceAndInsert,
        /** Erase an object from an array, removing at specified index */
        Erase,
        /** Insert an object into an array at a specified index */
        Insert,
        /** Remove all elements */
        Clear
    }

    /**
     * Performs operation on a copy of an array. (`arr` is const and won't be
     * modified)
     *
     * @param arr    the array to perform the operation on
     * @param action the action type to perform
     * @param arg    the data associated with the action
     */
    export function transformArray<T>(arr: T[], action: ArrayOp.SpliceAndInsert, arg: {from: number, to: number}): T[];
    export function transformArray<T>(arr: T[], action: ArrayOp.Erase, arg: number): T[];
    export function transformArray<T>(arr: T[], action: ArrayOp.Insert, arg: {index: number, items: T[]}): T[];
    export function transformArray<T>(arr: T[], action: ArrayOp.Insert, arg: {index: number, item: T}): T[];
    export function transformArray<T>(arr: T[], action: ArrayOp.Clear): T[];
    export function transformArray<T>(arr: T[], action: ArrayOp, arg?: any): T[]
    {
        const temp = [...arg];
        switch(action)
        {
        case ArrayOp.SpliceAndInsert:
            {
                if (typeof arg.from !== "number" || typeof arg.to !== "number"
                    || arg.from < 0 || arg.from >= arr.length
                    || arg.to < 0 || arg.to >= arg.length
                    || arg.from === arg.to)
                    return temp;

                const item = arr[arg.from];
                temp.splice(arg.from, 1);
                temp.splice(arg.to, 0, item);
            break;
            }
        case ArrayOp.Erase:
            {
                if (typeof arg !== "number" || arg < 0 || arg >= arr.length)
                    return temp;

                temp.splice(arg, 1);
            break;
            }

        case ArrayOp.Insert:
            {
                if (typeof arg.index !== "number" || arg.index < 0 ||
                    arg.index >= arr.length)
                    return temp;

                if (Array.isArray(arg.items))
                {
                    temp.splice(arg.index, 0, ...arg.items);
                }
                else if (arg.item !== undefined)
                {
                    // trusts `arg.item`` is of type T
                    temp.splice(arg.index, 0, arg.item as T);
                }
            }

        case ArrayOp.Clear:
            {
               temp.length = 0;
            break;
            }
        }

        return temp;
    }
