#include "AudioEngine.h"
#include "MultiTrackAudio.h"
#include "common.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace Insound
{
    AudioEngine::AudioEngine(): track(), sys(), master()
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

    void AudioEngine::setPause(bool pause, float seconds)
    {
        assert(track);
        track->setPause(pause, seconds);
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

    void AudioEngine::fadeChannelTo(int ch, float level, float seconds)
    {
        track->fadeChannelTo(ch, level, seconds);
    }

    float AudioEngine::getChannelFadeLevel(int ch, bool final) const
    {
        return track->getChannelFadeLevel(ch, final);
    }

    float AudioEngine::getFadeLevel(bool final) const
    {
        return track->getFadeLevel(final);
    }

    int AudioEngine::trackCount() const
    {
        assert(track);
        return track->trackCount();
    }

    bool AudioEngine::isLooping() const
    {
        assert(track);
        return track->isLooping();
    }

    void AudioEngine::setLooping(bool looping)
    {
        assert(track);
        track->setLooping(looping);
    }

    void AudioEngine::fadeTo(float to, float seconds)
    {
        assert(track);
        track->fadeTo(to, seconds);
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

        FMOD::ChannelGroup *master;
        result = sys->getMasterChannelGroup(&master);
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
        this->master.emplace(Channel(master));
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

    void AudioEngine::setSyncPointCallback(emscripten::val callback)
    {
        auto callbackType = callback.typeOf().as<std::string>();
        if (callbackType != "function")
            throw std::runtime_error("Callback must be a function, got " +
                callbackType + " instead");
        if (!track)
            throw std::runtime_error("AudioEngine cannot set syncpoint "
                "callback because track was not loaded.");
        track->setSyncPointCallback(callback);
    }

    void AudioEngine::setEndCallback(emscripten::val callback)
    {
        auto callbackType = callback.typeOf().as<std::string>();
        if (callbackType != "function")
            throw std::runtime_error("Callback must be a function, got" +
                callbackType + " instead");
        if (!track)
            throw std::runtime_error("AudioEngine cannot set end callback"
                "because track was not loaded.");
        track->setEndCallback(callback);
    }

    const std::string &AudioEngine::param_getName(size_t index) const
    {
        return track->params()[index].getName();
    }

    ParamDesc::Type AudioEngine::param_getType(size_t index) const
    {
        return track->params()[index].getType();
    }

    const StringsParam &AudioEngine::param_getAsStrings(size_t index) const
    {
        return track->params()[index].getStrings();
    }

    const NumberParam &AudioEngine::param_getAsNumber(size_t index) const
    {
        return track->params()[index].getNumber();
    }

    size_t AudioEngine::param_count() const
    {
        return track->params().size();
    }

    void AudioEngine::param_send(size_t index, float value)
    {
        auto &param = track->params()[index];
        lua.doParam(param, value);
    }

    void AudioEngine::setParamReceiver(emscripten::val callback)
    {
        lua.setParamCallback(callback);
    }
}
