
export interface LuaCallbacks
{
    // ----- Markers ----------------------------------------------------------
    addMarker(name: string, ms: number): void;
    editMarker(index: number | string, name: string, ms: number): void;
    getMarker(index: number | string): {name: string, position: number};
    getMarkerCount(): number;

    setLoopPoint(start: number, end: number): void;

    // ----- controls ---------------------------------------------------------

    setPause(pause: boolean, seconds?: number): void;
    setPosition(seconds: number): void;

    transitionTo(position: number, inTime: number, fadeIn: boolean, outTime: number, fadeOut: boolean): void;

    // --- mixer parameters ---------------------------------------------------

    setVolume(ch: number, level: number, seconds?: number): void;
    setPanLeft(ch: number, level: number, seconds?: number): void;
    setPanRight(ch: number, level: number, seconds?: number): void;
    setReverbLevel(ch: number, level: number, seconds?: number): void;

    print(level: number, name: string, message: string): void
    clearConsole(): void;

    // ----- presets ----------------------------------------------------------

    // Take snapshot of the current engine and add preset with following name
    getPresetName(index: number): string;
    getPresetCount(): number;
    applyPreset(indexOrName: number | string, seconds?: number): void;
}
