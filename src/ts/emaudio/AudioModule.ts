export const Audio = {} as InsoundAudioModule;

/**
 * Initialize Audio module
 */
export async function initAudioModule()
{
    if (Audio._free)
    {
        console.warn("initAudioModule: Audio module was already initialized.");
        return;
    }

    try {
        await new Promise<void>( (resolve, reject) => {
            const timeout = setTimeout(reject, 7500); // timeout if it doesn't load in 7.5 seconds.
            Audio.onRuntimeInitialized = () => {
                clearTimeout(timeout);
                resolve();
            };

            AudioModule(Audio);
        });

    } catch(err) {
        console.error(err);
        throw err;
    }
}

export function audioModuleWasInit()
{
    return (!!Audio._free);
}
