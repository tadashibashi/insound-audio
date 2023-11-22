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
        Impl(std::string_view name, FMOD::System *sys) : group(), fsb(), paused(false), lastFadePoint(1.f)
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

        FMOD::System *systemObject() const
        {
            FMOD::System *system;
            checkResult( group->getSystemObject(&system) );
            return system;
        }

        [[nodiscard]]
        int samplerate() const
        {
            int samplerate;
            checkResult( systemObject()->getSoftwareFormat(&samplerate,
                nullptr, nullptr) );
            return samplerate;
        }

        FMOD::ChannelGroup *parentGroup() const
        {
            FMOD::ChannelGroup *parent;
            checkResult( systemObject()->getMasterChannelGroup(&parent) );
            return parent;
        }

        FMOD::ChannelGroup *group;
        FMOD::Sound *fsb;
        bool paused;
        float lastFadePoint;

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

        m->paused = false;
    }


    int MultiTrackAudio::fadeTo(float to, float seconds)
    {
        // Get parent clock
        unsigned long long parentclock;
        auto parent = m->parentGroup();
        checkResult( parent->getDSPClock(nullptr, &parentclock) );

        // Get the current fade level
        unsigned int numPoints;
        checkResult( parent->getFadePoints(&numPoints, nullptr, nullptr) );

        float from;
        if (numPoints)
        {
            std::vector<unsigned long long> clocks(numPoints, 0);
            std::vector<float> volumes(numPoints, 0);

            checkResult( parent->getFadePoints(&numPoints, clocks.data(),
                volumes.data()) );

            int numPointsInt = (int)numPoints;
            for (int i = 0; i < numPointsInt-1; ++i)
            {
                if (clocks[i] <= parentclock && clocks[i+1] >= parentclock)
                {
                    float percentage = ((float)parentclock - clocks[i]) / (clocks[i+1] - clocks[i]);
                    from = (volumes[i+1] - volumes[i]) * percentage;
                    break;
                }
            }
        }
        else
        {
            from = m->lastFadePoint;
        }

        m->lastFadePoint = to;

        // Remove pre-existing fade points from now to track-length from now.
        auto length = getLength() * m->samplerate();
        checkResult( parent->removeFadePoints(
            0, parentclock + length) );

        if (seconds == 0)
        {
            auto currentPosition = getPosition();
            if (currentPosition == 0)
            {
                checkResult( parent->addFadePoint(0, to) );
                return parentclock;
            }
            else
            {
                checkResult( parent->setFadePointRamp(parentclock, to));
                return parentclock + 64;
            }
        }
        else
        {
            // Fade from
            checkResult( parent->addFadePoint(
                parentclock, from) );

            // Fade to target volume
            const int rampEnd = parentclock + m->samplerate() * seconds;
            checkResult( parent->addFadePoint(rampEnd, to) );

            return rampEnd;
        }

    }


    void MultiTrackAudio::setPause(bool pause, float seconds)
    {
        if (!isLoaded() || getPause() == pause) return;

        if (pause)
        {
            // fade to zero quickly, and delay pause
            auto rampEnd = fadeTo(0, seconds);

            checkResult( m->group->setDelay(0, rampEnd, false) );
            m->paused = pause;
        }
        else
        {
            // fade-in
            auto rampEnd = fadeTo(1, seconds);

            unsigned long long parentclock;
            checkResult( m->group->getDSPClock(nullptr, &parentclock) );

            checkResult( m->group->setDelay(parentclock, 0, false) );
            m->paused = pause;
        }
    }


    bool MultiTrackAudio::getPause() const
    {
        return m->paused;
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
        m->lastFadePoint = 1.f;
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

    bool MultiTrackAudio::isLooping() const
    {
        assert(m->fsb);

        // only need to check first channel, since all should be uniform
        FMOD::Channel *chan;
        auto result = m->group->getChannel(0, &chan);
        assert(result == FMOD_OK);

        FMOD_MODE mode;
        checkResult( chan->getMode(&mode) );

        return mode & (FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI);
    }

    void MultiTrackAudio::setLooping(bool looping)
    {
        assert(m->fsb);

        int chans;
        checkResult( m->group->getNumChannels(&chans) );

        for (int i = 0; i < chans; ++i)
        {
            FMOD::Channel *chan;
            checkResult( m->group->getChannel(i, &chan) );
            FMOD_MODE mode;
            checkResult( chan->getMode(&mode) );

            mode &= ~FMOD_LOOP_OFF; // unset loop off bit
            checkResult( chan->setMode(mode | FMOD_LOOP_NORMAL) );
        }
    }
}
