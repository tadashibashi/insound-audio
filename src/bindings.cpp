#include <insound/presets/Preset.h>
#include <insound/AudioEngine.h>
#include <insound/params/ParamDesc.h>
#include <insound/LoopInfo.h>

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
    register_vector<float>("FloatVector");

    value_object<SampleDataInfo>("SampleDataInfo")
        .field("ptr", &SampleDataInfo::ptr)
        .field("byteLength", &SampleDataInfo::byteLength);

    value_object<Preset>("Preset")
        .field("name", &Preset::name)
        .field("volumes", &Preset::volumes);

    value_object<LoopInfo<unsigned>>("LoopInfoSamples")
        .field("loopstart", &LoopInfo<unsigned>::loopstart)
        .field("loopend", &LoopInfo<unsigned>::loopend)
        ;
    value_object<LoopInfo<double>>("LoopInfoSeconds")
        .field("loopstart", &LoopInfo<double>::loopstart)
        .field("loopend", &LoopInfo<double>::loopend)
        ;

    using T = AudioEngine;

    class_<AudioEngine>("AudioEngine")
        .constructor<emscripten::val>()
        .function("init", &T::init)
        .function("resume", &T::resume)
        .function("suspend", &T::suspend)
        .function("update", &T::update)
        .function("loadBank", &T::loadBank)
        .function("loadSound", &T::loadSound)
        .function("loadScript", &T::loadScript)
        .function("unloadBank", &T::unloadBank)
        .function("isBankLoaded", &T::isBankLoaded)
        .function("setPause", &T::setPause)
        .function("seek", &T::seek)
        .function("getPosition", &T::getPosition)
        .function("getLength", &T::getLength)
        .function("getPause", &T::getPause)
        .function("getAudibility", &T::getAudibility)
        .function("setMasterVolume", &T::setMasterVolume)
        .function("getMasterVolume", &T::getMasterVolume)
        .function("setMainVolume", &T::setMainVolume)
        .function("getMainVolume", &T::getMainVolume)
        .function("getMainReverbLevel", &T::getMainReverbLevel)
        .function("setMainReverbLevel", &T::setMainReverbLevel)
        .function("setMainPanLeft", &T::setMainPanLeft)
        .function("setMainPanRight", &T::setMainPanRight)
        .function("setMainPan", &T::setMainPan)
        .function("getMainPanLeft", &T::getMainPanLeft)
        .function("getMainPanRight", &T::getMainPanRight)
        .function("setChannelVolume", &T::setChannelVolume)
        .function("getChannelVolume", &T::getChannelVolume)
        .function("getChannelCount", &T::getChannelCount)
        .function("getChannelReverbLevel", &T::getChannelReverbLevel)
        .function("setChannelReverbLevel", &T::setChannelReverbLevel)
        .function("setChannelPanLeft", &T::setChannelPanLeft)
        .function("setChannelPanRight", &T::setChannelPanRight)
        .function("setChannelPan", &T::setChannelPan)
        .function("getChannelPanLeft", &T::getChannelPanLeft)
        .function("getChannelPanRight", &T::getChannelPanRight)
        .function("isLooping", &T::isLooping)
        .function("setLooping", &T::setLooping)
        .function("setLoopSeconds", &T::setLoopSeconds)
        .function("setLoopSamples", &T::setLoopSamples)
        .function("getLoopSeconds", &T::getLoopSeconds)
        .function("getLoopSamples", &T::getLoopSamples)
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

        .function("getSyncPointCount", &T::getSyncPointCount)
        .function("getSyncPointOffsetSeconds", &T::getSyncPointOffsetSeconds)
        .function("getSyncPointLabel", &T::getSyncPointLabel)

        .function("getCPUUsageTotal", &T::getCPUUsageTotal)
        .function("getCPUUsageDSP", &T::getCPUUsageDSP)
        .function("getSampleData", &T::getSampleData)
        ;
}
