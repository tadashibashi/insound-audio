#include <insound/MultiTrackControl.h>
#include <insound/SyncPointInfo.h>
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
    value_object<SampleDataInfo>("SampleDataInfo")
        .field("ptr", &SampleDataInfo::ptr)
        .field("byteLength", &SampleDataInfo::byteLength);

    value_object<SyncPointInfo>("SyncPointInfo")
        .field("name", &SyncPointInfo::name)
        .field("offset", &SyncPointInfo::offset);

    value_object<Preset>("Preset")
        .field("name", &Preset::name)
        .field("volumes", &Preset::volumes);

    value_object<LoopInfo<unsigned>>("LoopInfo")
        .field("loopstart", &LoopInfo<unsigned>::start)
        .field("loopend", &LoopInfo<unsigned>::end)
        ;
    value_object<LoopInfo<double>>("LoopInfoSeconds")
        .field("start", &LoopInfo<double>::start)
        .field("end", &LoopInfo<double>::end)
        ;

    using T = AudioEngine;

    class_<AudioEngine>("AudioEngine")
        .constructor<>()
        .function("init", &T::init)
        .function("resume", &T::resume)
        .function("suspend", &T::suspend)
        .function("update", &T::update)
        .function("createTrack", &T::createTrack)
        .function("deleteTrack", &T::deleteTrack)
        .function("getMasterVolume", &T::getMasterVolume)
        .function("setMasterVolume", &T::setMasterVolume)
        .function("getAudibility", &T::getAudibility)
        .function("getCPUUsageTotal", &T::getCPUUsageTotal)
        .function("getCPUUsageDSP", &T::getCPUUsageDSP)
        ;

    class_<MultiTrackControl>("MultiTrackControl")
        .constructor<uintptr_t, emscripten::val>()
        .function("loadSound", &MultiTrackControl::loadSound)
        .function("loadBank", &MultiTrackControl::loadBank)
        .function("loadScript", &MultiTrackControl::loadScript)
        .function("executeScript", &MultiTrackControl::executeScript)
        .function("update", &MultiTrackControl::update)
        .function("unload", &MultiTrackControl::unload)
        .function("isLoaded", &MultiTrackControl::isLoaded)
        .function("setPause", &MultiTrackControl::setPause)
        .function("getPause", &MultiTrackControl::getPause)
        .function("setVolume", &MultiTrackControl::setVolume)
        .function("getVolume", &MultiTrackControl::getVolume)
        .function("setReverbLevel", &MultiTrackControl::setReverbLevel)
        .function("getReverbLevel", &MultiTrackControl::getReverbLevel)
        .function("setPanLeft", &MultiTrackControl::setPanLeft)
        .function("getPanLeft", &MultiTrackControl::getPanLeft)
        .function("setPanRight", &MultiTrackControl::setPanRight)
        .function("getPanRight", &MultiTrackControl::getPanRight)
        .function("setPosition", &MultiTrackControl::setPosition)
        .function("getPosition", &MultiTrackControl::getPosition)
        .function("transitionTo", &MultiTrackControl::transitionTo)
        .function("getLength", &MultiTrackControl::getLength)
        .function("getChannelCount", &MultiTrackControl::getChannelCount)
        .function("getAudibility", &MultiTrackControl::getAudibility)
        .function("setLoopPoint", &MultiTrackControl::setLoopPoint)
        .function("getLoopPoint", &MultiTrackControl::getLoopPoint)
        .function("addSyncPoint", &MultiTrackControl::addSyncPoint)
        .function("deleteSyncPoint", &MultiTrackControl::deleteSyncPoint)
        .function("editSyncPoint", &MultiTrackControl::editSyncPoint)
        .function("getSyncPointCount", &MultiTrackControl::getSyncPointCount)
        .function("getSyncPoint", &MultiTrackControl::getSyncPoint)
        .function("getSampleData", &MultiTrackControl::getSampleData)
        .function("onSyncPoint", &MultiTrackControl::onSyncPoint)
        .function("doMarker", &MultiTrackControl::doMarker)
        .function("samplerate", &MultiTrackControl::samplerate)
        .function("dspClock", &MultiTrackControl::dspClock)
        ;
}
