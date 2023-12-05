#include "MultiTrackAudio.h"
#include "Channel.h"
#include "common.h"
#include "params/ParamDescMgr.h"
#include "presets/PresetMgr.h"
#include "SyncPointMgr.h"

#include <fmod.hpp>
#include <fmod_common.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <functional>
#include <vector>

namespace Insound
{
    struct MultiTrackAudio::Impl
    {
    public:
        Impl(std::string_view name, FMOD::System *sys) : fsb(), chans(),
            main("main", sys), points(), syncpointCallback(), endCallback(),
            params()
        {
        }

        ~Impl()
        {
            chans.clear();

            if (fsb)
                fsb->release();
        }

        std::vector<Channel> chans;
        FMOD::Sound *fsb;

        Channel main;
        SyncPointMgr points;
        std::function<void(const std::string &, double)> syncpointCallback;
        std::function<void()> endCallback;
        ParamDescMgr params;
        PresetMgr presets;
    };


    MultiTrackAudio::MultiTrackAudio(std::string_view name, FMOD::System *sys)
        : m(new Impl(name, sys))
    {

    }


    MultiTrackAudio::~MultiTrackAudio()
    {
        delete m;
    }


    void MultiTrackAudio::fadeTo(float to, float seconds)
    {
        m->main.fadeTo(to, seconds);
    }


    void MultiTrackAudio::fadeChannelTo(int ch, float to, float seconds)
    {
        m->chans.at(ch).fadeTo(to, seconds);
    }


    float MultiTrackAudio::getChannelFadeLevel(int ch, bool final) const
    {
        return m->chans.at(ch).fadeLevel(final);
    }


    float MultiTrackAudio::getFadeLevel(bool final) const
    {
        return m->main.fadeLevel(final);
    }


    void MultiTrackAudio::setPause(bool pause, float seconds)
    {
        if (!isLoaded() || getPause() == pause) return;

        m->main.paused(pause, seconds);
    }


    bool MultiTrackAudio::getPause() const
    {
        return m->main.paused();
    }


    void MultiTrackAudio::seek(double seconds)
    {
        for (auto &chan : m->chans)
            chan.ch_position(seconds);
    }


    double MultiTrackAudio::getPosition() const
    {
        if (m->chans.empty()) return 0;

        // get first channel, assuming each is synced
        return m->chans.at(0).ch_position();
    }


    double MultiTrackAudio::getLength() const
    {
        int numSounds;
        checkResult( m->fsb->getNumSubSounds(&numSounds) );

        unsigned int maxLength = 0;
        for (int i = 0; i < numSounds; ++i)
        {
            FMOD::Sound *sound;
            checkResult( m->fsb->getSubSound(i, &sound));

            unsigned int length;
            checkResult( sound->getLength(&length, FMOD_TIMEUNIT_MS) );

            if (length > maxLength)
                maxLength = length;
        }

        return (double)maxLength * .001;
    }


    void MultiTrackAudio::unloadFsb()
    {
        // Check if already unloaded
        if (!this->isLoaded()) return;

        m->chans.clear();

        // Release bank
        m->fsb->release();
        m->fsb = nullptr;
        m->params.clear();
    }


    bool MultiTrackAudio::isLoaded() const
    {
        return static_cast<bool>(m->fsb);
    }

    static FMOD_RESULT F_CALLBACK channelCallback(
        FMOD_CHANNELCONTROL *chanCtrl,
        FMOD_CHANNELCONTROL_TYPE chanType,
        FMOD_CHANNELCONTROL_CALLBACK_TYPE callbackType,
        void *commanddata1,
        void *commanddata2)
    {
        // Get `MultiTrackAudio` object
        auto chan = (FMOD::ChannelControl *)chanCtrl;
        MultiTrackAudio *track;

        auto result = chan->getUserData((void **)&track);
        if (result != FMOD_OK)
            return result;

        // Determine action based on callback type
        switch (callbackType)
        {
        case FMOD_CHANNELCONTROL_CALLBACK_SYNCPOINT:
            {
                auto pointIndex = (int)(uintptr_t)commanddata1;
                auto callback = track->getSyncPointCallback();

                // Invoke callback, if any was set
                if (callback)
                    callback(
                        track->getSyncPointLabel(pointIndex).data(),
                        track->getSyncPointOffsetSeconds(pointIndex)
                    );

                break;
            }

        case FMOD_CHANNELCONTROL_CALLBACK_END:
            {
                auto callback = track->getEndCallback();
                if (callback)
                    callback();
                break;
            }

        default: break;
        }

        return FMOD_OK;
    }

    size_t MultiTrackAudio::getSyncPointCount() const
    {
        return m->points.size();
    }

    bool MultiTrackAudio::getSyncPointsEmpty() const
    {
        return m->points.empty();
    }

    std::string_view MultiTrackAudio::getSyncPointLabel(size_t i) const
    {
        return m->points.getLabel(i);
    }

    double MultiTrackAudio::getSyncPointOffsetSeconds(size_t i) const
    {
        return (double)m->points.getOffsetSeconds(i);
    }


    void MultiTrackAudio::loadFsb(const char *data, size_t bytelength,
        bool looping)
    {
        // Set relevant info to load the fsb
        auto exinfo{FMOD_CREATESOUNDEXINFO()};

        std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = bytelength;

        FMOD::System *sys;
        checkResult( m->main.raw()->getSystemObject(&sys) );

        // Load the bank
        FMOD::Sound *snd;
        checkResult( sys->createSound(data,
            FMOD_OPENMEMORY_POINT |
                (looping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF) |
                FMOD_CREATECOMPRESSEDSAMPLE,
            &exinfo, &snd)
        );

        int numSubSounds;
        checkResult( snd->getNumSubSounds(&numSubSounds) );

        if (numSubSounds == 0)
            throw std::runtime_error("No subsounds in the fsbank file.");

        // Populate sync point container with first subsound
        FMOD::Sound *firstSound;
        checkResult( snd->getSubSound(0, &firstSound) );

        SyncPointMgr syncPoints(firstSound);
        unsigned int length;
        checkResult( firstSound->getLength(&length, FMOD_TIMEUNIT_PCM) );

        if (length == 0)
            throw std::runtime_error("Invalid subsound, 0 length.");

        // find loop start / end points if they exist
        auto loopstart = syncPoints.getOffsetSamples("LoopStart");
        auto loopend = syncPoints.getOffsetSamples("LoopEnd");
        bool didSetLoop = false;
        // if loop start or loop end were not found, add it automatically
        if (!loopstart)
        {
            checkResult( firstSound->addSyncPoint(0, FMOD_TIMEUNIT_PCM,
                "LoopStart", nullptr) );
            loopstart.emplace(0);
            didSetLoop = true;
        }

        if (!loopend)
        {
            checkResult( firstSound->addSyncPoint(length,
                FMOD_TIMEUNIT_PCM, "LoopEnd", nullptr) );
            loopend.emplace(length);
            didSetLoop = true;
        }

        if (didSetLoop)
            syncPoints = SyncPointMgr(firstSound);

        if (loopend.value() < loopstart.value())
            throw std::runtime_error("LoopStart comes after LoopEnd.");

        std::vector<Channel> chans;
        for (int i = 0; i < numSubSounds; ++i)
        {
            FMOD::Sound *subsound;
            checkResult( snd->getSubSound(i, &subsound) );

            // set loop points
            checkResult(
                subsound->setLoopPoints(
                    loopstart.value(), FMOD_TIMEUNIT_PCM,
                    loopend.value(), FMOD_TIMEUNIT_PCM)
            );

            // create the channel wrapper object from the subsound
            chans.emplace_back(subsound,
                (FMOD::ChannelGroup *)m->main.raw(), sys, i);
        }

        // set callback
        if (!chans.empty())
        {
            checkResult( chans[0].raw()->setUserData(this));
            checkResult( chans[0].raw()->setCallback(channelCallback) );
        }

        // success, commit changes
        unloadFsb();
        m->chans.swap(chans);
        m->fsb = snd;
        std::swap(m->points, syncPoints);
        setPause(true, 0);
    }


    void MultiTrackAudio::setMainVolume(double vol)
    {
        m->main.volume(vol);
    }


    double MultiTrackAudio::getMainVolume() const
    {
        return m->main.volume();
    }


    void MultiTrackAudio::setChannelVolume(int ch, double vol)
    {
        m->chans.at(ch).volume(vol);
    }


    double MultiTrackAudio::getChannelVolume(int ch) const
    {
        return m->chans.at(ch).volume();
    }


    int MultiTrackAudio::channelCount() const
    {
        assert(m->fsb);

        int count;
        checkResult( m->fsb->getNumSubSounds(&count) );
        return count;
    }


    bool MultiTrackAudio::isLooping() const
    {
        // only need to check first channel, since all should be uniform
        return m->chans.at(0).looping();
    }


    void MultiTrackAudio::setLooping(bool looping)
    {
        for (auto &chan : m->chans)
        {
            chan.looping(looping);
        }
    }


    void MultiTrackAudio::setSyncPointCallback(
        std::function<void(const std::string &, double)> callback)
    {
        m->syncpointCallback = std::move(callback);
    }


    const std::function<void(const std::string &, double)> &
    MultiTrackAudio::getSyncPointCallback() const
    {
        return m->syncpointCallback;
    }


    void MultiTrackAudio::addSyncPointMS(const std::string &name,
        unsigned int offset)
    {
        m->points.emplace(name.data(), offset, FMOD_TIMEUNIT_MS);
    }


    void MultiTrackAudio::setEndCallback(
        std::function<void()> callback)
    {
        m->endCallback = std::move(callback);
    }


    const std::function<void()> &
    MultiTrackAudio::getEndCallback() const
    {
        return m->endCallback;
    }


    ParamDescMgr &MultiTrackAudio::params()
    {
        return m->params;
    }


    const ParamDescMgr &MultiTrackAudio::params() const
    {
        return m->params;
    }


    PresetMgr &MultiTrackAudio::presets()
    {
        return m->presets;
    }


    const PresetMgr &MultiTrackAudio::presets() const
    {
        return m->presets;
    }


    void MultiTrackAudio::applyPreset(std::string_view name, float seconds)
    {
        applyPreset(m->presets[name], seconds);
    }


    void MultiTrackAudio::applyPreset(size_t index, float seconds)
    {
        applyPreset(m->presets[index], seconds);
    }


    void MultiTrackAudio::applyPreset(const Preset &preset, float seconds)
    {
        auto chanSize = m->chans.size();

        if (preset.volumes.size() < chanSize)
        {
            throw std::runtime_error("Preset " + preset.name + " does "
                "not have enough volume settings for number of channels");
        }

        for (size_t i = 0; i < chanSize; ++i)
        {
            m->chans[i].fadeTo(preset.volumes[i], seconds);
        }
    }
}
