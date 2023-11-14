#include "AudioEngine.h"
#include "MultiTrackAudio.h"
#include "common.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace Insound {


    AudioEngine::AudioEngine(): track()
    {

    }

    AudioEngine::~AudioEngine()
    {
        close();
    }

    void AudioEngine::resume()
    {
        assert(sys);
        checkResult(sys->mixerResume());
    }

    void AudioEngine::suspend()
    {
        assert(sys);
        checkResult(sys->mixerSuspend());
    }

    void AudioEngine::update()
    {
        assert(sys);
        checkResult(sys->update());
    }

    void AudioEngine::loadBank(size_t data, size_t bytelength)
    {
        assert(track);
        track->loadFsb((const char *)data, bytelength);
    }

    void AudioEngine::unloadBank()
    {
        assert(track);
        track->unloadFsb();
    }

    void AudioEngine::play()
    {
        assert(track);
        track->play();
    }

    void AudioEngine::setPause(bool pause)
    {
        assert(track);
        track->setPause(pause);
    }

    bool AudioEngine::getPause() const
    {
        return track->getPause();
    }

    double AudioEngine::getPosition() const
    {
        assert(track);
        return track->getPosition();
    }

    double AudioEngine::getLength() const
    {
        assert(track);
        return track->getLength();
    }


    void AudioEngine::stop()
    {
        assert(track);
        track->stop();
    }


    void AudioEngine::seek(double seconds)
    {
        assert(track);
        track->seek(seconds);
    }

    void AudioEngine::setMainVolume(double vol)
    {
        assert(track);
        track->setMainVolume(vol);
    }

    double AudioEngine::getMainVolume() const
    {
        assert(track);
        return track->getMainVolume();
    }

    void AudioEngine::setChannelVolume(int ch, double vol)
    {
        assert(track);
        return track->setChannelVolume(ch, vol);
    }

    double AudioEngine::getChannelVolume(int ch) const
    {
        assert(track);
        return track->getChannelVolume(ch);
    }

    int AudioEngine::trackCount() const
    {
        assert(track);
        return track->trackCount();
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


        if (this->track)
        {
            this->track->unloadFsb();
            delete this->track;
        }

        if (this->sys)
        {
            this->sys->release();
        }

        this->sys = sys;
        this->track = new MultiTrackAudio("main", sys);
        return true;
    }

    void AudioEngine::close()
    {
        if (track)
        {
            delete track;
            track = nullptr;
        }

        if (sys)
        {
            sys->release();
            sys = nullptr;
        }
    }
}
