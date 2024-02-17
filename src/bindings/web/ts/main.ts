// Test driver
import { initAudioModule } from "./emaudio/AudioModule";

async function main()
{
    // initialize emscripten module
    await initAudioModule();

    // setup event loop
    const callback = (time) => {
        requestAnimationFrame(callback);

        // do something here...
    };

    requestAnimationFrame(callback);
}

main();
