#include "AudioEngine.h"
#include "MultiTrackAudio.h"
#include "Parameter.h"
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


    float AudioEngine::param_getInitValueByIndex(size_t index) const
    {
        return track->params().getInitValue(index);
    }

    float AudioEngine::param_getInitValue(const std::string &name) const
    {
        return track->params().getInitValue(name);
    }

    float AudioEngine::param_getByIndex(size_t index) const
    {
        return track->params().get(index);
    }

    float AudioEngine::param_get(const std::string &name) const
    {
        return track->params().get(name);
    }

    const std::string &AudioEngine::param_getName(size_t index) const
    {
        const auto &params = track->params();
        return params[index].getName();
    }


    size_t AudioEngine::param_count() const
    {
        return track->params().size();
    }


    size_t AudioEngine::param_labelCount(size_t paramIndex) const
    {
        return track->params().getLabels(paramIndex).size();
    }


    const std::string &AudioEngine::param_getLabelName(size_t paramIndex,
        size_t labelIndex) const
    {
        return track->params().getLabels(paramIndex)[labelIndex].name;
    }

    float AudioEngine::param_getLabelValue(size_t paramIndex,
        size_t labelIndex) const
    {
        return track->params().getLabels(paramIndex)[labelIndex].value;
    }

    void AudioEngine::param_setByIndex(size_t index, float value)
    {
        track->params().set(index, value);
    }

    void AudioEngine::param_setFromLabelByIndex(size_t index,
        const std::string &labelVal)
    {
        track->params().set(index, labelVal.data());
    }
    void AudioEngine::param_set(const std::string &name, float value)
    {
        track->params().set(name, value);
    }

    void AudioEngine::param_setFromLabel(const std::string &name,
        const std::string &labelVal)
    {
        track->params().set(name, labelVal);
    }

    void AudioEngine::param_addLabel(const std::string &paramName,
        const std::string &label, float value)
    {
        track->params().getLabels(paramName).add(label, value);
    }
}
