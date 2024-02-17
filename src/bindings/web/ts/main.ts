/**
 * Test driver code
 */
import {audioModuleWasInit, initAudioModule} from "./emaudio/AudioModule";

async function main()
{
    // initialize emscripten module
    await initAudioModule();
    if (audioModuleWasInit())
    {
        console.log("Audio module was successfully initialized");
    }
    else
    {
        console.error("Audio module failed to initalize");
    }

    // setup event loop
    const callback = (time) => {
        requestAnimationFrame(callback);

        // do something here...
    };

    requestAnimationFrame(callback);
}

main();
