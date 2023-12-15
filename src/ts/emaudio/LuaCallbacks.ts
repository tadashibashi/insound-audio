
export interface LuaCallbacks
{
    // Receive parameter through this function from LuaScript to set in on the
    // JS frontend.
    setParam: (index: number, value: number) => void,

    // Requests a parameter from JS to send to Lua.
    getParam: (nameOrIndex: string | number) => number

    syncpointUpdated: () => void;
}
