#include "AudioEngine.h"
#include "common.h"

#include "scripting/lua.hpp"
#include "scripting/Marker.h"

#include <insound/MultiTrackAudio.h>

#include <fmod.hpp>
#include <fmod_errors.h>

#include <cassert>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <variant>

namespace Insound
{
    AudioEngine::AudioEngine(): sys(), master(), tracks()
    {}

    uintptr_t AudioEngine::createTrack()
    {
        return (uintptr_t)tracks.emplace_back(new MultiTrackAudio(sys));
    }

    void AudioEngine::deleteTrack(uintptr_t track)
    {
        auto size = tracks.size();
        for (int i = 0; i < size; ++i)
        {
            auto curTrack = tracks[i];
            if ((uintptr_t)curTrack == track)
            {
                curTrack->clear();
                delete curTrack;

                tracks.erase(tracks.begin() + i);
                break;
            }
        }
    }

    AudioEngine::~AudioEngine()
    {
        close();
    }

    void AudioEngine::resume()
    {
        checkResult(sys->mixerResume());
    }

    void AudioEngine::suspend()
    {
        checkResult(sys->mixerSuspend());
    }

    void AudioEngine::update()
    {
        checkResult(sys->update());
    }

    bool AudioEngine::init()
    {
        FMOD::System *sys;
        auto result = FMOD::System_Create(&sys);
        if (result != FMOD_OK)
        {
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        int system_rate;
        result = sys->getDriverInfo(0, nullptr, 0, nullptr, &system_rate,
            nullptr, nullptr);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        result = sys->setSoftwareFormat(system_rate, FMOD_SPEAKERMODE_DEFAULT,
            0);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        result = sys->setDSPBufferSize(2048, 2);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        result = sys->init(1024, FMOD_INIT_NORMAL, nullptr);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        FMOD_REVERB_PROPERTIES reverbPreset = FMOD_PRESET_CONCERTHALL;
        result = sys->setReverbProperties(0, &reverbPreset);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        FMOD::ChannelGroup *master;
        result = sys->getMasterChannelGroup(&master);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        result = sys->setUserData(this);
        if (result != FMOD_OK)
        {
            sys->release();
            std::cerr << FMOD_ErrorString(result) << '\n';
            return false;
        }

        if (this->sys)
        {
            this->sys->release();
        }

        this->sys = sys;
        this->master.emplace(master);
        return true;
    }

    void AudioEngine::close()
    {
        for (auto track : tracks)
        {
            track->clear();
            delete track;
        }
        tracks.clear();

        if (master)
        {
            master.reset();
        }

        if (sys)
        {
            sys->release();
            sys = nullptr;
        }
    }

    float AudioEngine::getMasterVolume() const
    {
        return master->volume();
    }

    void AudioEngine::setMasterVolume(float level)
    {
        master->volume(level);
    }

    float AudioEngine::getCPUUsageTotal() const
    {
        FMOD_CPU_USAGE usage;
        checkResult(sys->getCPUUsage(&usage));
        return usage.dsp + usage.stream + usage.update + usage.convolution1 +
            usage.convolution2;
    }

    float AudioEngine::getCPUUsageDSP() const
    {
        FMOD_CPU_USAGE usage;
        checkResult(sys->getCPUUsage(&usage));
        return usage.dsp;
    }

    float AudioEngine::getAudibility() const
    {
        return master->audibility();
    }
}
