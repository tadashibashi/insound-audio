#include "Channel.h"

#include "common.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <iostream>
#include <vector>

namespace Insound
{
    Channel::Channel(FMOD::Sound *sound, FMOD::ChannelGroup *group,
        FMOD::System *system) :
            chan(), lastFadePoint(1.f), m_isGroup(false), samplerate()
    {
        FMOD::Channel *tempChan;
        checkResult( system->playSound(sound, group, true, &tempChan) );

        this->chan = tempChan;

        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );
        this->samplerate = rate;
    }


    Channel::Channel(FMOD::ChannelGroup *chan) :
        chan(chan), lastFadePoint(1.f), m_isGroup(true), samplerate()
    {
        FMOD::System *system;
        checkResult( chan->getSystemObject(&system) );

        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );
        this->samplerate = rate;
    }


    Channel::~Channel()
    {
        const auto result = this->chan->stop();
        if (result != FMOD_OK)
        {
            std::cerr << "Channel failed to stop: " <<
                FMOD_ErrorString(result) << '\n';
        }
    }


    Channel &Channel::volume(float val)
    {
        checkResult(chan->setVolume(val));
        return *this;
    }


    float Channel::fadeLevel() const
    {
        unsigned long long clock;
        checkResult(chan->getDSPClock(nullptr, &clock));

        unsigned int numPoints;
        checkResult(chan->getFadePoints(&numPoints, nullptr, nullptr));

        if (numPoints)
        {
            std::vector<unsigned long long> clocks(numPoints, 0);
            std::vector<float> volumes(numPoints, 0);

            checkResult(chan->getFadePoints(&numPoints, clocks.data(),
                volumes.data()));

            for (int i = 0, end = (int)numPoints-1; i < end; ++i)
            {
                if (clocks[i] <= clock && clocks[i+1] >= clock)
                {
                    float percentage =
                        ((float)clock - clocks[i]) / (clocks[i+1] - clocks[i]);
                    return std::abs(volumes[i+1] - volumes[i] * percentage);
                }
            }

            return this->lastFadePoint;
        }
        else
        {
            return this->lastFadePoint;
        }
    }


    Channel &Channel::fade(float from, float to, float seconds)
    {

        unsigned long long clock;
        checkResult( chan->getDSPClock(nullptr, &clock) );

        checkResult( chan->removeFadePoints(clock, clock + 60 * samplerate));
        checkResult( chan->addFadePoint(clock, from) );
        checkResult( chan->addFadePoint(clock + seconds * samplerate, to) );
        return *this;
    }


    Channel &Channel::fadeTo(float vol, float seconds)
    {
        return fade(fadeLevel(), vol, seconds);
    }

    Channel &Channel::looping(bool set)
    {
        FMOD_MODE mode;
        checkResult( chan->getMode(&mode) );

        mode &= ~FMOD_LOOP_OFF;
        checkResult( chan->setMode(mode | FMOD_LOOP_NORMAL) );
        return *this;
    }


    bool Channel::looping() const
    {
        FMOD_MODE mode;
        checkResult( chan->getMode(&mode) );

        return mode & (FMOD_LOOP_NORMAL | FMOD_LOOP_BIDI);
    }


    float Channel::position() const
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::position when "
                "underlying type is an FMOD::ChannelGroup");
        unsigned int position;
        checkResult( static_cast<FMOD::Channel *>(chan)->getPosition(&position,
            FMOD_TIMEUNIT_MS) );
        return position * 0.001f;
    }


    bool Channel::paused() const
    {
        bool p;
        checkResult( chan->getPaused(&p) );
        return p;
    }


    float Channel::volume() const
    {
        float volume;
        checkResult(chan->getVolume(&volume));
        return volume;
    }
}
