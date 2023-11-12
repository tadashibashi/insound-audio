import { AudioEngine, initAudio } from "./AudioEngine";

async function main()
{
    await initAudio();
    const audio = new AudioEngine();

    const callback = (time) => {
        requestAnimationFrame(callback);
        audio.update();
    };

    requestAnimationFrame(callback);
}

main();
