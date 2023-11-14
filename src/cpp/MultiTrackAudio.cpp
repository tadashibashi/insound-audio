#include "MultiTrackAudio.h"
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
        Impl(std::string_view name, FMOD::System *sys) : group(), fsb()
        {
            FMOD::ChannelGroup *group;
            checkResult(sys->createChannelGroup(name.data(), &group));
            this->group = group;
        }

        ~Impl()
        {
            if (fsb)
                fsb->release();
            group->release();
        }

        FMOD::ChannelGroup *group;
        FMOD::Sound *fsb;

    };


    MultiTrackAudio::MultiTrackAudio(std::string_view name, FMOD::System *sys)
        : m(new Impl(name, sys))
    {

    }


    MultiTrackAudio::~MultiTrackAudio()
    {
        delete m;
    }


    void MultiTrackAudio::play()
    {
        FMOD::System *sys;
        checkResult( m->group->getSystemObject(&sys) );

        int numSounds;
        checkResult( m->fsb->getNumSubSounds(&numSounds) );

        m->group->stop();
        std::vector<FMOD::Channel *> chans;
        for (int i = 0; i < numSounds; ++i)
        {
            FMOD::Sound *sound;
            checkResult( m->fsb->getSubSound(i, &sound) );

            FMOD::Channel *chan;
            checkResult( sys->playSound(sound, m->group, true, &chan) );

            // prevent pops when changing channel volume
            checkResult( chan->setVolumeRamp(true) );

            chans.emplace_back(chan);
        }

        for (int i = 0; i < numSounds; ++i)
            checkResult(chans[i]->setPaused(false));
    }


    void MultiTrackAudio::setPause(bool pause)
    {
        checkResult( m->group->setPaused(pause) );
    }


    bool MultiTrackAudio::getPause() const
    {
        bool paused;
        checkResult( m->group->getPaused(&paused) );

        return paused;
    }


    void MultiTrackAudio::seek(double seconds)
    {
        int chanCount;
        checkResult( m->group->getNumChannels(&chanCount) );

        for (int i = 0; i < chanCount; ++i)
        {
            FMOD::Channel *chan;
            checkResult( m->group->getChannel(i, &chan) );

            checkResult( chan->setPosition(seconds * 1000, FMOD_TIMEUNIT_MS) );
        }
    }


    double MultiTrackAudio::getPosition() const
    {
        int chanCount;
        checkResult( m->group->getNumChannels(&chanCount) );
        if (chanCount == 0) return -1.0;

        FMOD::Channel *chan;
        checkResult( m->group->getChannel(0, &chan) );

        unsigned int position;
        checkResult( chan->getPosition(&position, FMOD_TIMEUNIT_MS) );

        return (double)position * .001;
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


    void MultiTrackAudio::stop()
    {
        m->group->stop();
    }


    void MultiTrackAudio::unloadFsb()
    {
        // Check if already unloaded
        if (!isLoaded()) return;

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
        auto exinfo{FMOD_CREATESOUNDEXINFO()};

        std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = bytelength;

        FMOD::System *sys;
        checkResult( m->group->getSystemObject(&sys) );

        FMOD::Sound *snd;
        checkResult( sys->createSound(data,
            FMOD_OPENMEMORY_POINT | FMOD_LOOP_NORMAL |
            FMOD_CREATECOMPRESSEDSAMPLE, &exinfo, &snd) );
        unloadFsb();
        m->fsb = snd;
    }


    void MultiTrackAudio::setMainVolume(double vol)
    {
        checkResult( m->group->setVolume(vol) );
    }


    double MultiTrackAudio::getMainVolume() const
    {
        float vol;
        checkResult( m->group->getVolume(&vol) );
        return static_cast<double>(vol);
    }


    void MultiTrackAudio::setChannelVolume(int ch, double vol)
    {
        FMOD::Channel *chan;
        auto result = m->group->getChannel(ch, &chan);
        if (result != FMOD_OK)
        {
            std::cerr << "Failed to get channel " << ch << '\n';
            return;
        }

        checkResult( chan->setVolume(vol) );
    }


    double MultiTrackAudio::getChannelVolume(int ch) const
    {
        FMOD::Channel *chan;
        auto result = m->group->getChannel(ch, &chan);
        if (result != FMOD_OK) return -1;

        float volume;
        checkResult( chan->getVolume(&volume) );

        return static_cast<double>(volume);
    }


    int MultiTrackAudio::trackCount() const
    {
        assert(m->fsb);

        int count;
        checkResult( m->fsb->getNumSubSounds(&count) );
        return count;
    }
}
