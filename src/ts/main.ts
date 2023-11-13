import { AudioEngine, initAudio } from "./AudioEngine";

async function main()
{
    await initAudio();
    const audio = new AudioEngine();

    const trackBuffer = await fetch("/demo.fsb")
        .then(res => res.arrayBuffer());

    audio.loadTrack(trackBuffer);

    window.addEventListener("keypress", ev => {
        if (ev.code === "KeyP") {
            console.log("Start track!");
            audio.play();
        }
    });

    const callback = (time) => {
        requestAnimationFrame(callback);
        audio.update();
    };

    requestAnimationFrame(callback);
}

main();
