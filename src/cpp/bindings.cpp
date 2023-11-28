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
        .function("setPause", &T::setPause)
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
        .function("fadeTo", &T::fadeTo)
        .function("fadeChannelTo", &T::fadeChannelTo)
        .function("getChannelFadeLevel", &T::getChannelFadeLevel)
        .function("getFadeLevel", &T::getFadeLevel)
        .function("setSyncPointCallback", &T::setSyncPointCallback)
        .function("setEndCallback", &T::setEndCallback)

        .function("param_getInitValueByIndex", &T::param_getInitValueByIndex)
        .function("param_getInitValue", &T::param_getInitValue)
        .function("param_getByIndex", &T::param_getByIndex)
        .function("param_getName", &T::param_getName)
        .function("param_count", &T::param_count)
        .function("param_labelCount", &T::param_labelCount)
        .function("param_getLabelName", &T::param_getLabelName)
        .function("param_getLabelValue", &T::param_getLabelValue)
        .function("param_setByIndex", &T::param_setByIndex)
        .function("param_setFromLabelByIndex", &T::param_setByIndex)
        .function("param_set", &T::param_set)
        .function("param_setFromLabel", &T::param_setFromLabel)
        .function("param_addLabel", &T::param_addLabel)
        ;
}
