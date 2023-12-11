#include "AudioEngine.h"
#include "MultiTrackAudio.h"
#include "common.h"

#include "insound/presets/PresetMgr.h"
#include "scripting/lua.hpp"
#include "scripting/Marker.h"

#include <any>
#include <fmod.hpp>
#include <fmod_errors.h>

#include <cassert>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <variant>

namespace Insound
{
    static void validateCallbacks(std::initializer_list<const char *> names, const emscripten::val &cbs)
    {
        for (auto name : names)
        {
            if (cbs[name].typeOf().as<std::string>() != "function")
                throw std::runtime_error("missing callback \"" +
                    std::string(name) + "\"");
        }
    }

    AudioEngine::AudioEngine(
        emscripten::val cbs): track(), sys(), master(), lua()
    {
        using IndexOrName = std::variant<int, std::string>;

#if INS_DEBUG
        validateCallbacks({"setParam", "getParam"}, cbs);
#endif

        auto populateEnv = [this, cbs](sol::table &env)
        {
            emscripten::val setParam = cbs["setParam"];
            emscripten::val getParam = cbs["getParam"];

            Scripting::Marker::inject("Marker", env);

            auto snd = env["snd"].get_or_create<sol::table>();

            snd.set_function("play", [this](float seconds=0) -> void
            {
                this->setPause(false, seconds);
            });

            snd.set_function("pause", [this](float seconds=0) -> void
            {
                this->setPause(true, seconds);
            });

            snd.set_function("paused",
            [this](std::optional<bool> pause={}, float seconds=0) -> bool
            {
                if (!pause)
                {
                    return getPause();
                }
                else
                {
                    this->setPause(pause.value(), seconds);
                    return pause.value();
                }
            });

            snd.set_function("main_volume",
            [this](std::optional<double> volume={})
            {
                if (volume)
                {
                    this->setMainVolume(volume.value());
                    return volume.value();
                }
                else
                {
                    return getMainVolume();
                }

            });

            snd.set_function("channel_volume",
            [this](int channel, std::optional<float> volume={})
            {
                --channel;
                if (volume)
                {
                    setChannelVolume(channel, volume.value());
                    return (double)volume.value();
                }
                else
                {
                    return this->getChannelVolume(channel);
                }
            });

            snd.set_function("channel_count",
            [this]()
            {
                return this->getChannelCount();
            });

            // param namespace
            auto param = snd["param"].get_or_create<sol::table>();
            param.set_function("set",
                [setParam](std::variant<int, std::string> index, float value)
                {
                    if (index.index() == 0)
                        setParam(std::get<int>(index), value);
                    else
                        setParam(std::get<std::string>(index), value);
                });
            param.set_function("get",
                [getParam](IndexOrName index)
                {
                    return (index.index() == 0) ?
                        getParam(std::get<int>(index)).as<float>() :
                        getParam(std::get<std::string>(index)).as<float>();
                });
            param.set_function("count",
                [this]()
                {
                    return track->params().size();
                });
            param.set_function("empty",
                [this]()
                {
                    return track->params().empty();
                });
            param.set_function("add_int",
                [this](std::string name, int min, int max, int defaultVal)
                {
                    track->params().addInt(name, min, max, defaultVal);
                });
            param.set_function("add_float",
                [this](std::string name, float min, float max, float step,
                    float defaultVal)
                {
                    track->params().addFloat(name, min, max, step, defaultVal);
                });
            param.set_function("add_checkbox",
                [this](std::string name, bool defaultVal)
                {
                    track->params().addBool(name, defaultVal);
                });
            param.set_function("add_options",
                [this](std::string name, std::vector<std::string> strings,
                    size_t defaultIndex)
                {
                    track->params().addStrings(name, strings, defaultIndex);
                });

            // marker namespace
            auto marker = snd["marker"].get_or_create<sol::table>();
            marker.set_function("count",
                [this]()
                {
                    return this->track->getSyncPointCount();
                });
            marker.set_function("empty",
                [this]()
                {
                    return this->track->getSyncPointsEmpty();
                });
            marker.set_function("get",
                [this](size_t index)
                {
                    --index; // offset index since lua indexes from 1
                    auto label = track->getSyncPointLabel(index);
                    auto offset = track->getSyncPointOffsetSeconds(index);
                    return Scripting::Marker{.name=label.data(), .seconds=offset};
                });
            marker.set_function("add",
            [this](std::string name, double seconds)
            {
                track->addSyncPointMS(name, seconds * 1000);
            });

            auto preset = snd["preset"].get_or_create<sol::table>();
            preset.set_function("apply",
            [this](IndexOrName indexOrName, float seconds)
            {
                if (indexOrName.index() == 0)
                {
                    track->applyPreset(std::get<int>(indexOrName),
                        seconds);
                }
                else
                {
                    track->applyPreset(std::get<std::string>(indexOrName),
                        seconds);
                }
            });

            preset.set_function("add",
            [this](std::string name, std::vector<float> values,
                std::optional<size_t> position)
            {
                if (position)
                    track->presets().insert(name, values, *position-1);
                else
                    track->presets().emplace_back(name, values);
            });

            preset.set_function("get",
            [this](IndexOrName indexOrName) -> const std::vector<float> &
            {
                return (indexOrName.index() == 0) ?
                    track->presets()[std::get<int>(indexOrName)-1].volumes :
                    track->presets()[std::get<std::string>(indexOrName)].volumes;
            });

            preset.set_function("count",
            [this]()
            {
                return track->presets().size();
            });

            preset.set_function("empty",
            [this]()
            {
                return track->presets().empty();
            });
        };

        // Prepare the lua driver envrionment callback -
        // This is what will be made available in each lua scripting context.
        lua.emplace(populateEnv);
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
        lua->doUpdate();
    }

    void AudioEngine::loadBank(size_t data, size_t bytelength)
    {
        assert(track);
        track->loadFsb((const char *)data, bytelength);
    }

    const std::string &AudioEngine::loadScript(const std::string &text)
    {
        static const std::string NoErrors{};

        auto result = lua->load(text);
        if (!result)
            return lua->getError();

        result = lua->doInit();
        if (!result)
            return lua->getError();

        result = lua->doLoad(*track);
        if (!result)
            return lua->getError();

        return NoErrors;
    }

    void AudioEngine::unloadBank()
    {
        assert(track);
        track->clear();
    }

    void AudioEngine::setPause(bool pause, float seconds)
    {
        assert(track);
        track->pause(pause, seconds);
    }

    bool AudioEngine::getPause() const
    {
        return track->paused();
    }

    double AudioEngine::getPosition() const
    {
        assert(track);
        return track->position();
    }

    double AudioEngine::getLength() const
    {
        assert(track);
        return track->length();
    }

    void AudioEngine::seek(double seconds)
    {
        assert(track);
        track->position(seconds);
    }

    void AudioEngine::setMainVolume(double vol)
    {
        assert(track);
        track->mainVolume(vol);
    }

    double AudioEngine::getMainVolume() const
    {
        assert(track);
        return track->mainVolume();
    }

    void AudioEngine::setChannelVolume(int ch, double vol)
    {
        assert(track);
        return track->channelVolume(ch, vol);
    }

    double AudioEngine::getChannelVolume(int ch) const
    {
        assert(track);
        return track->channelVolume(ch);
    }

    void AudioEngine::fadeChannelTo(int ch, float level, float seconds)
    {
        track->fadeChannelTo(ch, level, seconds);
    }

    float AudioEngine::getChannelFadeLevel(int ch, bool final) const
    {
        return track->channelFadeLevel(ch, final);
    }

    float AudioEngine::getFadeLevel(bool final) const
    {
        return track->fadeLevel(final);
    }

    int AudioEngine::getChannelCount() const
    {
        assert(track);
        return track->channelCount();
    }

    bool AudioEngine::isLooping() const
    {
        assert(track);
        return track->looping();
    }

    void AudioEngine::setLooping(bool looping)
    {
        assert(track);
        track->looping(looping);
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

        if (this->track)
        {
            this->track->clear();
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
        lua->doParam(param, value);
    }

    void AudioEngine::setMainReverbLevel(float level)
    {
        track->mainReverbLevel(level);
    }

    float AudioEngine::getMainReverbLevel() const
    {
        return track->mainReverbLevel();
    }

    void AudioEngine::setChannelReverbLevel(int ch, float level)
    {
        track->channelReverbLevel(ch, level);
    }

    float AudioEngine::getChannelReverbLevel(int ch) const
    {
        return track->channelReverbLevel(ch);
    }
}
