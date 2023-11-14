import { AudioEngine, initAudio } from "./AudioEngine";

async function main()
{
    await initAudio();
    const audio = new AudioEngine();

    const trackBuffer = await fetch("/demo.fsb")
        .then(res => res.arrayBuffer());

    audio.loadTrack(trackBuffer);

    let chanSelected = 0;

    window.addEventListener("keydown", ev => {
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

            case "ArrowDown": {
                const originalVol = audio.getChannelVolume(chanSelected);
                let targetVol = Math.max(originalVol - .1, 0);
                audio.setChannelVolume(chanSelected, targetVol);
                console.log("Ch " + (chanSelected + 1) + " vol: " + targetVol);
                console.log("From: " + originalVol);
                break;
            }

            case "ArrowUp": {
                const originalVol = audio.getChannelVolume(chanSelected);
                let targetVol = Math.min(originalVol + .1, 2);
                audio.setChannelVolume(chanSelected, targetVol);
                console.log("Ch " + (chanSelected + 1) + " vol: " + targetVol);
                console.log("From: " + originalVol);
                break;
            }
        }

        if (ev.code.startsWith("Digit")) {
            chanSelected = Number(ev.code[5]) - 1;
            console.log("Channel selected: " + (chanSelected + 1));
        }
    });

    const callback = (time) => {
        requestAnimationFrame(callback);
        audio.update();
    };

    requestAnimationFrame(callback);
}

main();
