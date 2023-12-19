
export interface LuaCallbacks
{
    // Receive parameter through this function from LuaScript to set in on the
    // JS frontend.
    setParam(index: number, value: number): void,

    // Requests a parameter from JS to send to Lua.
    getParam(nameOrIndex: string | number): number

    // Called by C++ engine when sync points have been mutated.
    // The audio engine then updates its own local copy of sync opint info
    // to alter the frontend display to match.
    syncpointUpdated(): void;

    addPreset(name: string, volumes: number[]): void;
    applyPreset(indexOrName: number | string, seconds: number): void;
    getPreset(indexOrName: number | string): {name: string, volumes: number[]};
    getPresetCount(): number;
}
