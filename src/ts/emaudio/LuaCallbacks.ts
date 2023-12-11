
export interface LuaCallbacks
{
    setParam: (index: number, value: number) => void,
    getParam: (nameOrIndex: string | number) => number
}
