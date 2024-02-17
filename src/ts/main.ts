// Test driver
import { initAudioModule } from "./emaudio/AudioModule";

async function main()
{
    await initAudioModule();
    const callback = (time) => {
        requestAnimationFrame(callback);

    };

    requestAnimationFrame(callback);
}

main();
