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
        emscripten::val cbs): track(), sys(), master(), lua(),
        startTime(), lastFrame(), isSuspended(false), downtime()
    {
        using IndexOrName = std::variant<int, std::string>;

        startTime = std::chrono::system_clock::now();
        lastFrame = startTime;

#if INS_DEBUG
        validateCallbacks({"setParam", "getParam", "syncpointUpdated",
            "addPreset", "applyPreset", "getPreset", "getPresetCount"}, cbs);
#endif

        auto populateEnv = [this, cbs](sol::table &env)
        {
            emscripten::val setParam = cbs["setParam"];
            emscripten::val getParam = cbs["getParam"];
            emscripten::val syncpointUpdated = cbs["syncpointUpdated"];
            emscripten::val addPreset = cbs["addPreset"];
            emscripten::val applyPreset = cbs["applyPreset"];
            emscripten::val getPreset = cbs["getPreset"];
            emscripten::val getPresetCount = cbs["getPresetCount"];

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

            snd.set_function("position",
            [this](std::optional<double> seconds = {})
            {
                double pos;
                if (seconds)
                {
                    seek(seconds.value());
                }

                return getPosition();
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

            snd.set_function("main_pan_left",
            [this](std::optional<float> value = {})
            {
                if (value)
                {
                    this->setMainPanLeft(value.value());
                    return value.value();
                }
                else
                {
                    return this->getMainPanLeft();
                }
            });

            snd.set_function("main_pan_right",
            [this](std::optional<float> value = {})
            {
                if (value)
                {
                    this->setMainPanRight(value.value());
                    return value.value();
                }
                else
                {
                    return this->getMainPanRight();
                }
            });

            snd.set_function("main_reverb",
            [this](std::optional<float> value = {})
            {
                if (value)
                {
                    this->setMainReverbLevel(value.value());
                    return value.value();
                }
                else
                {
                    return this->getMainReverbLevel();
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

            snd.set_function("channel_pan_left",
            [this](int channel, std::optional<float> value = {})
            {
                --channel;
                if (value)
                {
                    this->setChannelPanLeft(channel, value.value());
                    return value.value();
                }
                else
                {
                    return this->getChannelPanLeft(channel);
                }
            });

            snd.set_function("channel_pan_right",
            [this](int channel, std::optional<float> value = {})
            {
                --channel;
                if (value)
                {
                    this->setChannelPanRight(channel, value.value());
                    return value.value();
                }
                else
                {
                    return this->getChannelPanRight(channel);
                }
            });

            snd.set_function("channel_reverb",
            [this](int channel, std::optional<float> value={})
            {
                --channel;
                if (value)
                {
                    this->setChannelReverbLevel(channel, value.value());
                    return value.value();
                }
                else
                {
                    return this->getChannelReverbLevel(channel);
                }
            });

            snd.set_function("channel_count",
            [this]()
            {
                return this->getChannelCount();
            });

            snd.set_function("loop_point",
            [this](std::optional<double> loopstart, std::optional<double> loopend)
            {
                if (loopstart)
                {
                    setLoopSeconds(loopstart.value(), loopend.value_or(getLength()));
                }

                return track->loopSeconds();
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
            [this, syncpointUpdated](std::string name, double seconds)
            {
                track->addSyncPointMS(name, seconds * 1000);
                syncpointUpdated();
            });

            auto preset = snd["preset"].get_or_create<sol::table>();
            preset.set_function("apply",
            [this, applyPreset](IndexOrName indexOrName, float seconds = 0)
            {
                if (indexOrName.index() == 0)
                {
                    applyPreset(std::get<int>(indexOrName),
                        seconds);
                }
                else
                {
                    applyPreset(std::get<std::string>(indexOrName),
                        seconds);
                }
            });

            preset.set_function("add",
            [this, addPreset](std::string name, sol::object valArrayp)
            {
                auto values = emscripten::val::array();

                auto valArray = valArrayp.as<sol::table>();
                auto size = valArray.size();

                for (int i = 0; i < size; ++i)
                    values.set(i, valArray[i+1].get<double>());

                addPreset(name, values);
            });

            preset.set_function("get",
            [this, getPreset](IndexOrName indexOrName)
            {
                emscripten::val preset;
                if (indexOrName.index() == 0)
                {
                    preset = getPreset(std::get<int>(indexOrName));
                }
                else
                {
                    preset = getPreset(std::get<std::string>(indexOrName));
                }

                sol::table res;
                res["name"] = preset["name"].as<std::string>();
                auto luaVolumes = res["volumes"].get_or_create<sol::table>();

                auto volumes = preset["volumes"];
                size_t length = volumes["length"].as<size_t>();
                for (size_t i = 0; i < length; ++i)
                {
                    luaVolumes[i+1] = volumes[i].as<double>();
                }

                return res;
            });

            preset.set_function("count",
            [this, getPresetCount]()
            {
                return getPresetCount().as<size_t>();
            });

            preset.set_function("empty",
            [this, getPresetCount]()
            {
                return getPresetCount().as<size_t>() == 0;
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
        checkResult(sys->mixerResume());
        isSuspended = false;
    }

    void AudioEngine::suspend()
    {
        checkResult(sys->mixerSuspend());
        isSuspended = true;
    }

    void AudioEngine::update()
    {
        auto current = std::chrono::system_clock::now();
        auto delta = current - lastFrame;
        auto total = current - startTime;

        // auto-suspend when there is no activity
        if (!isSuspended)
        {
            if (track->isLoaded())
            {
                // Suspend audio engine after 5 seconds of pause
                if (track->paused())
                {
                    downtime += delta.count();

                    if (downtime > 5000000)
                    {
                        suspend();
                        downtime = 0;
                    }
                }
            }
            else
            {
                downtime += delta.count();

                if (downtime > 5000000)
                {
                    suspend();
                    downtime = 0;
                }
            }
        }

        checkResult(sys->update());
        lua->doUpdate(delta.count() * .001, total.count() * .001);

        lastFrame = current;
    }

    void AudioEngine::loadSound(size_t data, size_t bytelength)
    {
        track->loadSound((const char *)data, bytelength);

        // set clock
        startTime = std::chrono::system_clock::now();
        lastFrame = startTime;
    }

    void AudioEngine::loadBank(size_t data, size_t bytelength)
    {
        track->loadFsb((const char *)data, bytelength);

        // set clock
        startTime = std::chrono::system_clock::now();
        lastFrame = startTime;
    }

    const std::string &AudioEngine::loadScript(const std::string &text)
    {
        static const std::string NoErrors{};

        auto result = lua->load(text);
        if (!result)
            return lua->getError();

        if (!text.empty())
        {
            result = lua->doInit();
            if (!result)
                return lua->getError();

            result = lua->doLoad(*track);
            if (!result)
                return lua->getError();
        }

        return NoErrors;
    }

    void AudioEngine::unloadBank()
    {
        track->clear();
    }

    void AudioEngine::setPause(bool pause, float seconds)
    {
        track->pause(pause, seconds);

        if (!pause && isSuspended)
            resume();
    }

    bool AudioEngine::getPause() const
    {
        return track->paused();
    }

    double AudioEngine::getPosition() const
    {
        return track->position();
    }

    double AudioEngine::getLength() const
    {
        return track->length();
    }

    void AudioEngine::seek(double seconds)
    {
        track->position(seconds);
    }

    void AudioEngine::setMainVolume(double vol)
    {
        track->mainVolume(vol);
    }

    double AudioEngine::getMainVolume() const
    {
        return track->mainVolume();
    }

    void AudioEngine::setChannelVolume(int ch, double vol)
    {
        return track->channelVolume(ch, vol);
    }

    double AudioEngine::getChannelVolume(int ch) const
    {
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
        return track->channelCount();
    }

    bool AudioEngine::isLooping() const
    {
        return track->looping();
    }

    void AudioEngine::setLooping(bool looping)
    {
        track->looping(looping);
    }

    void AudioEngine::fadeTo(float to, float seconds)
    {
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
        track->setSyncPointCallback(
            [callback, this](const std::string &name, double offset, int index)
            {
                callback(name, offset, index);
                lua->doSyncPoint(name, offset, index);
            });
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


    size_t AudioEngine::getSyncPointCount() const
    {
        return track->getSyncPointCount();
    }

    std::string AudioEngine::getSyncPointLabel(size_t index) const
    {
        return track->getSyncPointLabel(index).data();
    }

    double AudioEngine::getSyncPointOffsetSeconds(size_t index) const
    {
        return track->getSyncPointOffsetSeconds(index);
    }

    void AudioEngine::setLoopSeconds(double loopstart, double loopend)
    {
        track->loopSeconds(loopstart, loopend);
    }

    void AudioEngine::setLoopSamples(unsigned loopstart, unsigned loopend)
    {
        track->loopSamples(loopstart, loopend);
    }

    LoopInfo<double> AudioEngine::getLoopSeconds() const
    {
        return track->loopSeconds();
    }

    LoopInfo<unsigned> AudioEngine::getLoopSamples() const
    {
        return track->loopSamples();
    }

    void AudioEngine::setChannelPanLeft(int ch, float level)
    {
        track->channel(ch).panLeft(level);
    }

    void AudioEngine::setChannelPanRight(int ch, float level)
    {
        track->channel(ch).panRight(level);
    }

    void AudioEngine::setChannelPan(int ch, float left, float right)
    {
        track->channel(ch).pan(left, right);
    }

    float AudioEngine::getChannelPanLeft(int ch) const
    {
        return track->channel(ch).panLeft();
    }

    float AudioEngine::getChannelPanRight(int ch) const
    {
        return track->channel(ch).panRight();
    }

    void AudioEngine::setMainPanLeft(float level)
    {
        track->main().panLeft(level);
    }

    void AudioEngine::setMainPanRight(float level)
    {
        track->main().panRight(level);
    }
    void AudioEngine::setMainPan(float left, float right)
    {
        track->main().pan(left, right);
    }

    float AudioEngine::getMainPanLeft() const
    {
        return track->main().panLeft();
    }

    float AudioEngine::getMainPanRight() const
    {
        return track->main().panRight();
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
}
