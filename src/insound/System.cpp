#include "System.h"
#include "fmod.hpp"
#include "Bus.h"
#include "fmod_errors.h"

#include <insound/common.h>
#include <iostream>

namespace Insound::Audio
{
    System::System() : m_sys(), m_master() { }
    System::~System()
    {
        close();
    }

    bool System::init()
    {
        FMOD::System *sys;
        auto result = FMOD::System_Create(&sys);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        // tie the associated system object to it
        result = sys->setUserData(this);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        int systemRate;
        result = sys->getDriverInfo(0, nullptr, 0, nullptr, &systemRate,
            nullptr, nullptr);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        result = sys->setSoftwareFormat(systemRate,
            FMOD_SPEAKERMODE_DEFAULT, 0);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        result = sys->setDSPBufferSize(2048, 2);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        result = sys->init(1024, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        Bus *master;
        FMOD::ChannelGroup *mGroup;
        result = sys->getMasterChannelGroup(&mGroup);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            sys->release();
            return false;
        }

        try {
            master = new Bus(mGroup);
        }
        catch(const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            sys->release();
            return false;
        }
        catch(...)
        {
            sys->release();
            return false;
        }

        // Free any currently loaded system
        close();
        m_sys = sys;
        m_master = master;
        return true;
    }

    void System::close()
    {
        if (m_master)
        {
            delete m_master;
            m_master = nullptr;
        }

        if (m_sys)
        {
            m_sys->release();
            m_sys = nullptr;
        }
    }

    void System::resume()
    {
        checkResult(m_sys->mixerResume());
    }

    void System::suspend()
    {
        checkResult(m_sys->mixerSuspend());
    }

    void System::update()
    {
        checkResult(m_sys->update());
    }
}
