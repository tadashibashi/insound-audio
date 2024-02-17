type LuaCallbacks = import("./LuaCallbacks").LuaCallbacks;

declare namespace Insound
{
    interface AudioModule extends EmscriptenModule {
        System: {
            new (): Insound.System;
        }

        ExampleClass: {
            new (value?: number): Insound.ExampleClass;
        }

        /** Web audio context, set after audio initializes */
        mContext?: AudioContext;
        /** Main web audio output node, set after audio initializes */
        mWorkletNode?: AudioWorkletNode;
    }

    interface System {
        /**
         * Initialize the Audio Engine. Must be called before any other function of
         * the audio engine is called.
         *
         * @return whether initialization was successful.
         */
        init(): boolean;

        /**
         * Explicitly delete the underlying Audio Engine object.
         * The browser will attempt to use a FinalizationRegistry, to clean up resources,
         * but the standard does not guarantee a call to the destructor will be made,
         * so it is recommended to call this manually.
         */
        delete(): void;

        /**
         * Resume the audio engine.
         *
         * @throws FMODError if there was an error.
         */
        resume(): void;

        /**
         * Pause the audio engine.
         */
        suspend(): void;

        /**
         * Update audio engine state, should be called at least 20 times per second
         */
        update(): void;
    }

    interface ExampleClass {
        value: number;
        doubled(): number;
    }
}

declare const AudioModule: (module: any) => Promise<void>;
