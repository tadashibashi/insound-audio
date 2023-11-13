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
        ;
}
