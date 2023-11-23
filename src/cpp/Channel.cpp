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
            chan(), lastFadePoint(1.f), m_isGroup(false), samplerate(),
            m_isPaused()
    {
        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );

        FMOD::Channel *tempChan;
        checkResult( system->playSound(sound, group, false, &tempChan) );

        this->samplerate = rate;
        this->chan = static_cast<FMOD::ChannelControl *>(tempChan);
    }


    Channel::Channel(std::string_view name, FMOD::System *system) :
        chan(), lastFadePoint(1.f), m_isGroup(true), samplerate(),
        m_isPaused()
    {
        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );

        FMOD::ChannelGroup *group;
        checkResult( system->createChannelGroup(name.data(), &group) );

        this->chan = static_cast<FMOD::ChannelControl *>(group);
        this->samplerate = rate;
    }


    Channel::Channel(Channel &&other) : chan(other.chan),
        lastFadePoint(other.lastFadePoint), m_isGroup(other.m_isGroup),
        samplerate(other.samplerate), m_isPaused(other.m_isPaused)
    {
        other.chan = nullptr;
    }

    Channel &Channel::operator=(Channel &&other)
    {
        return {other};
    }


    Channel::~Channel()
    {
        if (chan) // check for chan, since it may have been moved
        {
            auto result = this->chan->stop();
            if (result != FMOD_OK)
            {
                std::cerr << "Channel failed to stop: " <<
                    FMOD_ErrorString(result) << '\n';
            }

            if (m_isGroup)
            {
                result = static_cast<FMOD::ChannelGroup *>(chan)->release();
                if (result != FMOD_OK)
                {
                    std::cerr << "Channel failed to release: " <<
                        FMOD_ErrorString(result) << '\n';
                }
            }
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


    Channel &Channel::paused(bool value, float seconds)
    {
        if (this->paused() == value) return *this;

        // Get current parent clock to time pause below
        unsigned long long clock;
        checkResult( chan->getDSPClock(nullptr, &clock) );

        if (value) // pause
        {
            // fade-out in `seconds`
            fadeTo(0, seconds);

            // pause at ramp end
            auto rampEnd = clock + samplerate * seconds;
            checkResult( chan->setDelay(0, rampEnd, false) );
        }
        else       // unpause
        {
            // fade-in in `seconds`
            fadeTo(1, seconds);

            // unpause right now
            checkResult( chan->setDelay(clock, 0, false) );
        }

        m_isPaused = value;
        return *this;
    }


    Channel &Channel::looping(bool set)
    {
        FMOD_MODE mode;
        checkResult( chan->getMode(&mode) );

        if (set)
        {
            mode &= ~FMOD_LOOP_OFF;
            checkResult( chan->setMode(mode | FMOD_LOOP_NORMAL) );
        }
        else
        {
            mode |= FMOD_LOOP_OFF;
            mode &= ~FMOD_LOOP_NORMAL;
        }

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


    Channel &Channel::position(float seconds)
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::position when "
                "underlying type is an FMOD::ChannelGroup");

        unsigned int pos = seconds * 1000;
        checkResult( static_cast<FMOD::Channel *>(chan)->setPosition(pos,
            FMOD_TIMEUNIT_MS) );

        return *this;
    }

    bool Channel::paused() const
    {
        return m_isPaused;
    }


    float Channel::volume() const
    {
        float volume;
        checkResult(chan->getVolume(&volume));
        return volume;
    }
}
