
export interface LuaCallbacks
{
    // Called by C++ engine when sync points have been mutated.
    // The audio engine then updates its own local copy of sync opint info
    // to alter the frontend display to match.
    syncPointsUpdated(): void;

    // ----- controls ---------------------------------------------------------

    setPause(pause: boolean, seconds?: number): void;
    setPosition(seconds: number): void;

    // --- mixer parameters ---------------------------------------------------

    setVolume(ch: number, level: number, seconds?: number): void;
    setPanLeft(ch: number, level: number, seconds?: number): void;
    setPanRight(ch: number, level: number, seconds?: number): void;
    setReverbLevel(ch: number, level: number, seconds?: number): void;

    // ----- presets ----------------------------------------------------------

    // Take snapshot of the current engine and add preset with following name
    getPresetName(index: number): string;
    getPresetCount(): number;
    applyPreset(indexOrName: number | string, seconds?: number): void;
}
