#include "AudioEngine.h"
#include "params/ParamDesc.h"
#include <emscripten/bind.h>

using namespace emscripten;
using namespace Insound;

EMSCRIPTEN_BINDINGS(Params)
{
    value_object<StringsParam>("StringsParam")
        .field("values", &StringsParam::values)
        .field("defaultValue", &StringsParam::m_default)
        ;

    value_object<NumberParam>("NumberParam")
        .field("min", &NumberParam::m_min)
        .field("max", &NumberParam::m_max)
        .field("step", &NumberParam::m_step)
        .field("defaultValue", &NumberParam::m_default)
        ;

    enum_<ParamDesc::Type>("ParamType")
        .value("Float", ParamDesc::Type::Float)
        .value("Integer", ParamDesc::Type::Integer)
        .value("Strings", ParamDesc::Type::Strings)
        ;
}

EMSCRIPTEN_BINDINGS(AudioEngine) {
    using T = AudioEngine;

    class_<AudioEngine>("AudioEngine")
        .constructor<emscripten::val, emscripten::val>()
        .function("init", &T::init)
        .function("resume", &T::resume)
        .function("suspend", &T::suspend)
        .function("update", &T::update)
        .function("loadBank", &T::loadBank)
        .function("loadScript", &T::loadScript)
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

        .function("param_count", &T::param_count)
        .function("param_getName", &T::param_getName)
        .function("param_getType", &T::param_getType)
        .function("param_getAsNumber", &T::param_getAsNumber)
        .function("param_getAsStrings", &T::param_getAsStrings)
        .function("param_send", &T::param_send)
        ;
}
