/**
 * Test driver code
 */
import {audioModuleWasInit, initAudioModule} from "./emaudio/AudioModule";

async function main()
{
    // initialize emscripten module
    const Audio = await initAudioModule();
    if (audioModuleWasInit())
    {
        console.log("Audio module was successfully initialized");
    }
    else
    {
        console.error("Audio module failed to initalize");
    }

    const exampleInstance = new Audio.ExampleClass(10);
    console.log("example instance value", exampleInstance.value);
    console.log("doubled", exampleInstance.doubled());

    // setup event loop
    const callback = (time) => {
        requestAnimationFrame(callback);

        // do something here...
    };

    requestAnimationFrame(callback);
}

main();
