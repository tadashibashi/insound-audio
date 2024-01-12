#include "MultiTrackControl.h"
#include <insound/MultiTrackAudio.h>
#include <insound/scripting/LuaDriver.h>

namespace Insound
{
    MultiTrackControl::MultiTrackControl(uintptr_t track,
        emscripten::val callbacks) : lua(), track((MultiTrackAudio *)track),
        callbacks(callbacks), totalTime()
    {
        initScriptingEngine();
    }

    MultiTrackControl::~MultiTrackControl()
    {
        delete lua;
    }

    void MultiTrackControl::loadSound(size_t data, size_t bytelength)
    {
        track->loadSound((const char *)data, bytelength);
        totalTime = 0;
    }

    void MultiTrackControl::loadBank(size_t data, size_t bytelength)
    {
        track->loadFsb((const char *)data, bytelength);
        totalTime = 0;
    }

    void MultiTrackControl::unload()
    {
        track->clear();
    }

    void MultiTrackControl::update(float deltaTime)
    {
        lua->doUpdate(deltaTime, totalTime);
        totalTime += deltaTime;
    }

    bool MultiTrackControl::isLoaded() const
    {
        return track->isLoaded();
    }

    void MultiTrackControl::setPause(bool pause, float seconds)
    {
        track->pause(pause, seconds);
    }

    bool MultiTrackControl::getPause() const
    {
        return track->paused();
    }

    void MultiTrackControl::setVolume(int ch, float volume)
    {
        if (ch == 0)
            track->mainVolume(volume);
        else
            track->channelVolume(ch-1, volume);
    }

    float MultiTrackControl::getVolume(int ch) const
    {
        return (ch == 0) ?
            track->mainVolume() :
            track->channelVolume(ch-1);
    }

    void MultiTrackControl::setReverbLevel(int ch, float level)
    {
        if (ch == 0)
            track->mainReverbLevel(level);
        else
            track->channelReverbLevel(ch-1, level);
    }

    float MultiTrackControl::getReverbLevel(int ch) const
    {
        return (ch == 0) ?
            track->mainReverbLevel() :
            track->channelReverbLevel(ch-1);
    }

    void MultiTrackControl::setPanLeft(int ch, float level)
    {
        if (ch == 0)
            track->main().panLeft(level);
        else
            track->channel(ch-1).panLeft(level);
    }

    float MultiTrackControl::getPanLeft(int ch) const
    {
        return (ch == 0) ?
            track->main().panLeft() :
            track->channel(ch-1).panLeft();
    }

    void MultiTrackControl::setPanRight(int ch, float level)
    {
        if (ch == 0)
            track->main().panRight(level);
        else
            track->channel(ch-1).panRight(level);
    }

    float MultiTrackControl::getPanRight(int ch) const
    {
        return (ch == 0) ?
            track->main().panRight() :
            track->channel(ch-1).panRight();
    }

    void MultiTrackControl::setPosition(float seconds)
    {
        track->position(seconds);
    }

    float MultiTrackControl::getPosition() const
    {
        return track->position();
    }

    float MultiTrackControl::getLength() const
    {
        return track->length();
    }

    int MultiTrackControl::getChannelCount() const
    {
        return track->channelCount();
    }

    float MultiTrackControl::getAudibility(int ch) const
    {
        return (ch == 0) ?
            track->main().audibility() :
            track->channel(ch-1).audibility();
    }

    void MultiTrackControl::setLoopPoint(unsigned loopstart, unsigned loopend)
    {
        track->loopMilliseconds(loopstart, loopend);
    }

    LoopInfo<unsigned> MultiTrackControl::getLoopPoint() const
    {
        return track->loopMilliseconds();
    }

    size_t MultiTrackControl::getSyncPointCount() const
    {
        return track->getSyncPointCount();
    }

    SyncPointInfo MultiTrackControl::getSyncPoint(int index) const
    {
        return {
            .name=track->getSyncPointLabel(index).data(),
            .offset=track->getSyncPointOffsetMilliseconds(index)
        };
    }

    SampleDataInfo MultiTrackControl::getSampleData(int index) const
    {
        auto &data = track->getSampleData(index);
        return {
            .ptr=(uintptr_t)data.data(),
            .byteLength=data.size()
        };
    }

    void MultiTrackControl::onSyncPoint(emscripten::val callback)
    {
        track->setSyncPointCallback(
            [callback, this]
            (const std::string &name, double offset, int index)
            {
                callback(name, offset, index);
                this->lua->doSyncPoint(name, offset, index);
            });
    }
}
