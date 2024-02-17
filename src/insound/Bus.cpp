#include "Bus.h"
#include "System.h"

#include "insound/common.h"
#include <fmod.hpp>

namespace Insound::Audio
{
    Bus::Bus(System *sys): m_group{}, m_panLeft{1.f}, m_panRight{1.f}
    {
        FMOD::ChannelGroup *group;
        checkResult(sys->handle()->createChannelGroup(nullptr, &group));

        // tie this Bus to the newly created group
        checkResult(group->setUserData(this));

        m_group = group;
    }

    Bus::Bus(FMOD::ChannelGroup *group): m_group{}, m_panLeft{1.f}, m_panRight{1.f}
    {
        // ensure group hasn't already been associated with another Bus
        void *data;
        checkResult(group->getUserData(&data));
        if (data)
        {
            throw std::runtime_error("FMOD::ChannelGroup has already been associated");
        }

        FMOD::System *sys;
        checkResult(group->getSystemObject(&sys));

        // tie this Bus to the group
        checkResult(group->setUserData(this));

        m_group = group;
    }

    Bus::~Bus()
    {
        if (m_group && !isMaster()) // master bus memory handled by FMOD
        {
            m_group->release();
        }
    }

    bool Bus::isMaster() const
    {
        FMOD::System *sys;
        checkResult(m_group->getSystemObject(&sys));

        FMOD::ChannelGroup *master;
        checkResult(sys->getMasterChannelGroup(&master));

        return (m_group == master);
    }

    unsigned long long Bus::clock() const
    {
        unsigned long long res;
        checkResult(m_group->getDSPClock(&res, nullptr));
        return res;
    }

    unsigned long long Bus::parentClock() const
    {
        unsigned long long res;
        checkResult(m_group->getDSPClock(nullptr, &res));
        return res;
    }

    float Bus::volume() const
    {
        float v;
        checkResult(m_group->getVolume(&v));
        return v;
    }

    void Bus::volume(float level)
    {
        checkResult(m_group->setVolume(level));
    }

    void Bus::panLeft(float level)
    {
        float values[4] = {level, 1.f - m_panRight, 1.f - level, m_panRight};

        checkResult(m_group->setMixMatrix(values, 2, 2, 2));
        m_panLeft = level;
    }

    void Bus::panRight(float level)
    {
        float values[4] = {m_panLeft, 1.f-level, 1.f - m_panLeft, level};

        checkResult(m_group->setMixMatrix(values, 2, 2, 2));

        m_panRight = level;
    }
}
