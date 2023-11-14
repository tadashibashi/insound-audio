import { AudioEngine, initAudio } from "./AudioEngine";

async function main()
{
    await initAudio();
    const audio = new AudioEngine();

    const trackBuffer = await fetch("/demo.fsb")
        .then(res => res.arrayBuffer());

    audio.loadTrack(trackBuffer);

    window.addEventListener("keypress", ev => {
        switch(ev.code)
        {
        case "KeyP":
            console.log("Start track!");
            audio.play();
            break;

        case "KeyZ":
            console.log("Seek Left");
            audio.seek(audio.position-.75);
            console.log("CurrentPosition:", audio.position);
            break;

        case "KeyX":
            console.log("Seek Right");
            audio.seek(audio.position+.75);
            console.log("CurrentPosition:", audio.position);
            break;
        }
    });

    const callback = (time) => {
        requestAnimationFrame(callback);
        audio.update();
    };

    requestAnimationFrame(callback);
}

main();
