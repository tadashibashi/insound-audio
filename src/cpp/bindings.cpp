#include "AudioEngine.h"
#include <emscripten/bind.h>

using namespace emscripten;

EMSCRIPTEN_BINDINGS() {
    using T = Insound::AudioEngine;

    class_<Insound::AudioEngine>("AudioEngine")
        .constructor()
        .function("init", &T::init)
        .function("resume", &T::resume)
        .function("suspend", &T::suspend)
        .function("update", &T::update)
        .function("loadBank", &T::loadBank)
        .function("unloadBank", &T::unloadBank)
        .function("isBankLoaded", &T::isBankLoaded)
        .function("play", &T::play)
        .function("setPause", &T::setPause)
        .function("stop", &T::stop)
        .function("seek", &T::seek)
        .function("getPosition", &T::getPosition)
        .function("getLength", &T::getLength)
        .function("getPause", &T::getPause)
        .function("setMainVolume", &T::setMainVolume)
        .function("getMainVolume", &T::getMainVolume)
        .function("setChannelVolume", &T::setChannelVolume)
        .function("getChannelVolume", &T::getChannelVolume)
        .function("trackCount", &T::trackCount)
        .function("isLooping", &T::isLooping)
        .function("setLooping", &T::setLooping)
        .function("fade", &T::fade)
        ;
}
