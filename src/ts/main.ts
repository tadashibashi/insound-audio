import Module from "../../build/insound-audio.js";

const Audio = {} as InsoundAudioModule;

try {
    await Module(Audio);
} catch(err) {
    console.error(err);
    throw err;
}

const engine = new Audio.AudioEngine();
engine.init();


engine.close();
