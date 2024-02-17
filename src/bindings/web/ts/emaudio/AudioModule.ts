let Audio = {} as Insound.AudioModule;

export function getAudioModule(): Insound.AudioModule
{
    return Audio;
}


/**
 * Check if module was already loaded.
 *
 * @returns `true` if it was init, `false` if not.
 */
export function audioModuleWasInit(): boolean
{
    return !!Audio._free;
}

/**
 * Initialize a fresh Audio module.
 *
 * Calling this again after the first time resets the initial module with a new
 * object. This is useful for a restart if there was an unrecoverable error.
 *
 * After calling this function, the module object returned from
 * `getAudioModule` will be immediately updated, but will not be ready for
 * consumption until this function resolves.
 */
export async function initAudioModule(): Promise<Insound.AudioModule>
{
    // Ensure that the audio module function is available. It should be
    // attached to the document and available globally.
    if (AudioModule === undefined)
    {
        throw Error("Cannot start AudioEngine because its script was not loaded");
    }

    // Discard any previously loaded module
    if (audioModuleWasInit())
    {
        Audio = {} as Insound.AudioModule;
    }

    return new Promise<Insound.AudioModule>( (resolve, reject) => {
        // reject if it doesn't load in 10 seconds
        const timeout = setTimeout(reject, 10000);

        // resolve once it's initialized
        Audio.onRuntimeInitialized = () => {
            clearTimeout(timeout);
            resolve(Audio);
        };

        // asynchronously inject the module into the Audio object
        AudioModule(Audio);
    })
    .catch(err => {
        console.error(err);
        throw err;
    });
}
