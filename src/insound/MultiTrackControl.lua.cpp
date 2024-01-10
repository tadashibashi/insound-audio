#include "MultiTrackControl.h"
#include "MultiTrackAudio.h"

#include <insound/scripting/Marker.h>
#include <insound/scripting/lua.hpp>
#include <insound/scripting/LuaDriver.h>

namespace Insound
{
    const std::string &MultiTrackControl::loadScript(const std::string &text)
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

    void MultiTrackControl::initScriptingEngine()
    {
        // set up lua environment callback
        auto populateEnv = [this](sol::table &env)
        {
            // Javascript callbacks to directly go through the UI
            emscripten::val syncPointsUpdated = callbacks["syncPointsUpdated"];
            emscripten::val setPause = callbacks["setPause"];
            emscripten::val setVolume = callbacks["setVolume"];
            emscripten::val setPanLeft = callbacks["setPanLeft"];
            emscripten::val setPanRight = callbacks["setPanRight"];
            emscripten::val setReverbLevel = callbacks["setReverbLevel"];
            emscripten::val getPresetName = callbacks["getPresetName"];
            emscripten::val getPresetCount = callbacks["getPresetCount"];
            emscripten::val applyPreset = callbacks["applyPreset"];

            Scripting::Marker::inject("Marker", env);

            auto snd = env["track"].get_or_create<sol::table>();

            snd.set_function("play", [this, setPause](float seconds=0) -> void
            {
                setPause(false, seconds);
            });

            snd.set_function("pause", [this, setPause](float seconds=0) -> void
            {
                setPause(true, seconds);
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

            // doesn't need a callback, because the interface polls for value
            // on update
            snd.set_function("position",
            [this](std::optional<double> seconds = {})
            {
                double pos;
                if (seconds)
                {
                    setPosition(seconds.value());
                }

                return getPosition();
            });

            snd.set_function("volume",
            [this, setVolume](int ch = 0, std::optional<float> volume={})
            {
                if (volume)
                {
                    setVolume(ch, volume.value());
                    return volume.value();
                }
                else
                {
                    return this->getVolume(ch);
                }
            });

            snd.set_function("pan_left",
            [this, setPanLeft](int ch = 0, std::optional<float> value = {})
            {
                if (value)
                {
                    setPanLeft(ch, value.value());
                    return value.value();
                }
                else
                {
                    return this->getPanLeft(ch);
                }
            });

            snd.set_function("pan_right",
            [this, setPanRight](int ch = 0, std::optional<float> value = {})
            {
                if (value)
                {
                    setPanRight(ch, value.value());
                    return value.value();
                }
                else
                {
                    return this->getPanRight(ch);
                }
            });

            snd.set_function("reverb_level",
            [this, setReverbLevel](int ch = 0, std::optional<float> value = {})
            {
                if (value)
                {
                    setReverbLevel(ch, value.value());
                    return value.value();
                }
                else
                {
                    return this->getReverbLevel(ch);
                }
            });


            snd.set_function("channel_count",
            [this]()
            {
                return this->getChannelCount();
            });

            snd.set_function("loop_point", // loopstart and loopend in milliseconds
            [this](std::optional<unsigned> loopstart, std::optional<unsigned> loopend)
            {
                if (loopstart)
                {
                    setLoopPoint(loopstart.value(), loopend.value_or(getLength()));
                }

                return track->loopMilliseconds();
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
            [this, syncPointsUpdated](std::string name, double seconds)
            {
                this->track->addSyncPointMS(name, seconds * 1000);
                syncPointsUpdated(); // alert JS that syncpoints have been updated
            });

            auto preset = snd["preset"].get_or_create<sol::table>();
            preset.set_function("apply",
            [this, applyPreset](std::variant<int, std::string> indexOrName, float seconds = 0)
            {
                if (indexOrName.index() == 0)
                {
                    applyPreset(std::get<int>(indexOrName) - 1,
                        seconds);
                }
                else
                {
                    applyPreset(std::get<std::string>(indexOrName),
                        seconds);
                }
            });

            preset.set_function("get_name",
            [this, getPresetName](int index)
            {
                return getPresetName(index-1);
            });

            preset.set_function("count",
            [this, getPresetCount]()
            {
                return getPresetCount().as<size_t>();
            });
        };

        this->lua = new LuaDriver(populateEnv);
    }
}
