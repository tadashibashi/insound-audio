#include "Channel.h"

#include "common.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <iostream>
#include <vector>

namespace Insound
{
    static const unsigned int MaxFadePoints = 2;

    Channel::Channel(FMOD::Sound *sound, FMOD::ChannelGroup *group,
        FMOD::System *system) :
            chan(), lastFadePoint(1.f), m_isGroup(false), samplerate(),
            m_isPaused(true), m_leftPan(1.f), m_rightPan(1.f),
            m_isMaster(false)
    {
        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );

        FMOD::Channel *tempChan;
        checkResult( system->playSound(sound, group, true, &tempChan) );

        checkResult( tempChan->setUserData(this) );
        checkResult( tempChan->setReverbProperties(0, 0) );

        this->samplerate = rate;
        this->chan = static_cast<FMOD::ChannelControl *>(tempChan);
    }


    Channel::Channel(FMOD::System *system) :
        chan(), lastFadePoint(1.f), m_isGroup(true), samplerate(),
        m_isPaused(false), m_leftPan(1.f), m_rightPan(1.f),
        m_isMaster(false)
    {
        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );

        FMOD::ChannelGroup *group;
        checkResult( system->createChannelGroup(nullptr, &group) );

        checkResult( group->setUserData(this) );
        checkResult( group->setReverbProperties(0, 0) );

        this->chan = static_cast<FMOD::ChannelControl *>(group);
        this->samplerate = rate;
    }


    Channel::Channel(FMOD::ChannelGroup *group) : chan(group),
        lastFadePoint(1.f), m_isGroup(true), samplerate(),
        m_isPaused(false), m_leftPan(1.f), m_rightPan(1.f),
        m_isMaster(false)
    {
        FMOD::System *system;
        checkResult( group->getSystemObject(&system) );

        int rate;
        checkResult( system->getSoftwareFormat(&rate, nullptr, nullptr) );

        checkResult( group->setUserData(this) );

        // Need to check against master channel, since reverb send cannot be
        // set here, or it would cause an infinite loop as reverbs are
        // fed into it.
        FMOD::ChannelGroup *master;
        checkResult(system->getMasterChannelGroup(&master));
        if (group != master)
        {
            checkResult( group->setReverbProperties(0, 0) );
        }
        else
        {
            m_isMaster = true;
        }
        this->samplerate = rate;
        this->chan = group;
    }


    Channel::Channel(Channel &&other) : chan(other.chan),
        lastFadePoint(other.lastFadePoint), m_isGroup(other.m_isGroup),
        samplerate(other.samplerate), m_isPaused(other.m_isPaused),
        m_leftPan(1.f), m_rightPan(1.f), m_isMaster(other.m_isMaster)
    {
        other.chan = nullptr;
    }


    Channel &Channel::operator=(Channel &&other)
    {
        return {other};
    }


    Channel::~Channel()
    {
        release();
    }


    void Channel::release()
    {
        if (chan) // check for chan, since it may have been moved
        {
            auto result = this->chan->stop();
            if (result != FMOD_OK)
            {
                std::cerr << "Channel failed to stop: " <<
                    FMOD_ErrorString(result) << '\n';
            }

            if (m_isGroup && !m_isMaster)
            {
                result = static_cast<FMOD::ChannelGroup *>(chan)->release();
                if (result != FMOD_OK)
                {
                    std::cerr << "Channel failed to release: " <<
                        FMOD_ErrorString(result) << '\n';
                }
            }

            chan = nullptr;
        }
    }


    Channel &Channel::volume(float val)
    {
        checkResult(chan->setVolume(val));
        return *this;
    }


    float Channel::fadeLevel(bool final, unsigned long long targetClock) const
    {
        if (!final) return lastFadePoint;

        if (targetClock == 0)
        {
            checkResult(chan->getDSPClock(nullptr, &targetClock));
        }

        unsigned int numPoints;
        checkResult(chan->getFadePoints(&numPoints, nullptr, nullptr));

        if (numPoints >= 2)
        {
            unsigned long long clocks[MaxFadePoints];
            float volumes[MaxFadePoints];

            if (numPoints > 2)
                numPoints = 2;
            checkResult( chan->getFadePoints(&numPoints, clocks, volumes) );

            if (clocks[0] <= targetClock && clocks[1] > targetClock)
            {
                float percentage =
                    ((float)targetClock - clocks[0]) / (clocks[1] - clocks[0]);
                return std::abs(volumes[1] - volumes[0] * percentage);
            }
            else if (targetClock > clocks[1])
            {
                return volumes[1];
            }
        }

        return this->lastFadePoint;
    }


    Channel &Channel::fade(float from, float to, float seconds, unsigned long long targetClock)
    {
        unsigned long long currentClock;
        checkResult( chan->getDSPClock(nullptr, &currentClock) );

        if (targetClock == 0)
        {
            targetClock = currentClock;
        }

        auto rampEnd = targetClock + seconds * samplerate;
        if (rampEnd == targetClock)
            ++rampEnd;

        // Remove any fade points in between region we will create new ones
        // hack to only remove fadepoints when unpausing
        if (to != 0)
        {
            checkResult( chan->removeFadePoints(targetClock-1, targetClock + 60  * samplerate));
        }

        checkResult( chan->addFadePoint(targetClock, from) );
        checkResult( chan->addFadePoint(rampEnd, to) );

        this->lastFadePoint = to;
        return *this;
    }


    Channel &Channel::fadeTo(float vol, float seconds, unsigned long long clock)
    {
        return fade(fadeLevel(true, clock), vol, seconds, clock);
    }


    Channel &Channel::pause(bool value, float seconds, bool performFade, unsigned long long clock)
    {
        if (paused() == value) return *this;

        // Get current parent clock to time pause below
        if (clock == 0)
        {
            checkResult( chan->getDSPClock(nullptr, &clock) );
        }



        if (value) // pause
        {
            if (performFade)
            {
                // fade-out in `seconds`
                fadeTo(0, seconds, clock);
            }
            else
            {
                fadeTo(0, 0, clock);
            }


            // pause at ramp end
            auto rampEnd = clock + samplerate * seconds;
            checkResult( chan->setDelay(0, rampEnd, false) );
        }
        else       // unpause
        {
            // Unset main pause mechanism if set
            bool chanPaused;
            checkResult( chan->getPaused(&chanPaused) );
            if (chanPaused)
            {
                checkResult( chan->setPaused(false) );
            }

            // Do pause behavior based on `performFade`
            if (performFade)
            {
                // unpause at clock point
                checkResult( chan->setDelay(clock, 0, false) );

                // fade-in from clock point to `seconds` afterward
                fade(0.f, 1.f, seconds, clock);
            }
            else
            {
                // unpause at the delayed time
                auto targetClock = clock + samplerate * seconds;
                checkResult( chan->setDelay(targetClock, 0, false) );
                fade(0, 1.f, 0, clock);
            }
        }

        m_isPaused = value;
        return *this;
    }


    float Channel::ch_position() const
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::position when "
                "underlying type is an FMOD::ChannelGroup");
        unsigned int position;
        checkResult( static_cast<FMOD::Channel *>(chan)->getPosition(&position,
            FMOD_TIMEUNIT_MS) );
        return position * 0.001f;
    }


    unsigned int Channel::ch_positionSamples() const
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::position when "
                "underlying type is an FMOD::ChannelGroup");
        unsigned int position;
        checkResult( static_cast<FMOD::Channel *>(chan)->getPosition(&position,
            FMOD_TIMEUNIT_PCM) );
        return position;
    }


    Channel &Channel::ch_position(float seconds)
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::position when "
                "underlying type is an FMOD::ChannelGroup");

        unsigned int pos = seconds * 1000;
        checkResult( static_cast<FMOD::Channel *>(chan)->setPosition(pos,
            FMOD_TIMEUNIT_MS) );

        return *this;
    }

    Channel &Channel::ch_positionSamples(float samples)
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::position when "
                "underlying type is an FMOD::ChannelGroup");

        checkResult( static_cast<FMOD::Channel *>(chan)->setPosition(samples,
            FMOD_TIMEUNIT_PCM) );

        return *this;
    }

    Channel &Channel::ch_loopMS(unsigned loopstart, unsigned loopend)
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::ch_loopSeconds "
                "when underlying type is an FMOD::ChannelGroup");
        checkResult(static_cast<FMOD::Channel *>(chan)->setLoopPoints(
            loopstart, FMOD_TIMEUNIT_MS,
            loopend, FMOD_TIMEUNIT_MS)
        );

        return *this;
    }

    Channel &Channel::ch_loopPCM(unsigned loopstart, unsigned loopend)
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::ch_loopSamples "
                "when underlying type is an FMOD::ChannelGroup");

        checkResult(static_cast<FMOD::Channel *>(chan)->setLoopPoints(
            loopstart, FMOD_TIMEUNIT_PCM,
            loopend, FMOD_TIMEUNIT_PCM)
        );

        return *this;
    }

    LoopInfo<unsigned> Channel::ch_loopMS() const
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::ch_loopSeconds "
                "when underlying type is an FMOD::ChannelGroup");

        unsigned start, end;
        checkResult(static_cast<FMOD::Channel *>(chan)->getLoopPoints(
            &start, FMOD_TIMEUNIT_MS,
            &end, FMOD_TIMEUNIT_MS)
        );

        return {.start=start, .end=end};
    }

    LoopInfo<unsigned> Channel::ch_loopPCM() const
    {
        if (m_isGroup)
            throw std::runtime_error("Cannot call Channel::ch_loopSamples "
                "when underlying type is an FMOD::ChannelGroup");

        unsigned start, end;
        checkResult(static_cast<FMOD::Channel *>(chan)->getLoopPoints(
            &start, FMOD_TIMEUNIT_PCM,
            &end, FMOD_TIMEUNIT_PCM)
        );

        return {.start=start, .end=end};
    }

    Channel *Channel::group() const
    {
        FMOD::ChannelGroup *group;

        if (m_isGroup)
        {
            checkResult(
                static_cast<FMOD::ChannelGroup *>(chan)->getParentGroup(&group));
        }
        else
        {
            checkResult(
                static_cast<FMOD::Channel *>(chan)->getChannelGroup(&group) );
        }

        Channel *ch = nullptr;
        checkResult( group->getUserData((void **)&ch) );

        return ch;
    }

    Channel &Channel::ch_group(Channel &group)
    {
        if (!group.isGroup())
            throw std::runtime_error("Cannot call Channel::ch_group when "
                "passed channel does not have underlying FMOD::ChannelGroup.");

        if (this->isGroup())
        {
            throw std::runtime_error("Cannot call Channel::ch_group when "
                "underlying object is not an FMOD::Channel");
        }

        checkResult(
            static_cast<FMOD::Channel *>(chan)->setChannelGroup(
                static_cast<FMOD::ChannelGroup *>(group.raw())
            )
        );

        return *this;
    }

    bool Channel::paused() const
    {
        bool chanPaused;
        checkResult(chan->getPaused(&chanPaused));

        return m_isPaused || chanPaused;
    }


    float Channel::volume() const
    {
        float volume;
        checkResult(chan->getVolume(&volume));
        return volume;
    }

    float Channel::reverbLevel() const
    {
        float level;
        checkResult(chan->getReverbProperties(0, &level));
        return level;
    }

    Channel &Channel::reverbLevel(float level)
    {
        checkResult(chan->setReverbProperties(0, level));
        return *this;
    }


    Channel &Channel::panLeft(float value)
    {
        float values[4] = {value, 1.f - m_rightPan, 1.f - value, m_rightPan};

        checkResult(chan->setMixMatrix(values, 2, 2, 2));
        m_leftPan = value;
        return *this;
    }

    float Channel::panLeft() const
    {
        return m_leftPan;
    }

    Channel &Channel::panRight(float value)
    {
        float values[4] = {m_leftPan, 1.f-value, 1.f - m_leftPan, value};

        checkResult(chan->setMixMatrix(values, 2, 2, 2));

        m_rightPan = value;
        return *this;
    }

    float Channel::panRight() const
    {
        return m_rightPan;
    }

    Channel &Channel::pan(float left, float right)
    {
        float values[4] = {left, 1.f-right, 1.f-left, right};

        checkResult(chan->setMixMatrix(values, 2, 2, 2));

        m_leftPan = left;
        m_rightPan = right;
        return *this;
    }

    float Channel::audibility() const
    {
        float level;
        checkResult(chan->getAudibility(&level));

        return level;
    }

}
