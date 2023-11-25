#include "MultiTrackAudio.h"
#include "Channel.h"
#include "SyncPointManager.h"
#include "common.h"
#include "fmod_common.h"

#include <fmod.hpp>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

namespace Insound
{

    struct MultiTrackAudio::Impl
    {
    public:
        Impl(std::string_view name, FMOD::System *sys) : fsb(), chans(), main("main", sys)
        {
        }

        ~Impl()
        {
            if (fsb)
                fsb->release();
        }

        FMOD::Sound *fsb;
        std::vector<Channel> chans;
        Channel main;
        SyncPointManager points;
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
            chan.position(seconds);
    }


    double MultiTrackAudio::getPosition() const
    {
        if (m->chans.empty()) return 0;

        // get first channel, assuming each is synced
        return m->chans.at(0).position();
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
    }


    bool MultiTrackAudio::isLoaded() const
    {
        return static_cast<bool>(m->fsb);
    }


    void MultiTrackAudio::loadFsb(const char *data, size_t bytelength)
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
            FMOD_OPENMEMORY_POINT | FMOD_LOOP_NORMAL |
            FMOD_CREATECOMPRESSEDSAMPLE, &exinfo, &snd) );

        int numSubSounds;
        checkResult( snd->getNumSubSounds(&numSubSounds) );

        if (numSubSounds == 0)
            throw std::runtime_error("No subsounds in the fsbank file.");

        // Populate sync point container with first subsound
        FMOD::Sound *firstSound;
        checkResult( snd->getSubSound(0, &firstSound) );

        SyncPointManager syncPoints(firstSound);

        // find loop start / end points if they exist
        auto loopstart = syncPoints.getOffsetSamples("LoopStart");
        auto loopend = syncPoints.getOffsetSamples("LoopEnd");
        bool hasLoopPoints = (loopstart && loopend);

        std::vector<Channel> chans;
        for (int i = 0; i < numSubSounds; ++i)
        {
            FMOD::Sound *subsound;
            checkResult( snd->getSubSound(i, &subsound) );

            // set loop points if the bank has any
            if (hasLoopPoints)
            {
                checkResult(subsound->setLoopPoints(loopstart.value(),
                    FMOD_TIMEUNIT_PCM, loopend.value(), FMOD_TIMEUNIT_PCM));
            }

            // create the channel wrapper object from the subsound
            chans.emplace_back(subsound,
                (FMOD::ChannelGroup *)m->main.raw(), sys, i);
        }

        // success, commit changes
        unloadFsb();
        m->chans.swap(chans);
        m->fsb = snd;
        m->points = std::move(syncPoints);
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


    int MultiTrackAudio::trackCount() const
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
}
