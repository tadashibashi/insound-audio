#include "MultiTrackControl.h"
#include "MultiTrackAudio.h"

#include <insound/scripting/Marker.h>
#include <insound/scripting/lua.hpp>
#include <insound/scripting/LuaDriver.h>
#include <insound/scripting/LuaError.h>

#include <string>

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
            emscripten::val addMarker = callbacks["addMarker"];
            emscripten::val editMarker = callbacks["editMarker"];
            emscripten::val getMarker = callbacks["getMarker"];
            emscripten::val getMarkerCount = callbacks["getMarkerCount"];
            emscripten::val setPause = callbacks["setPause"];
            emscripten::val setPosition = callbacks["setPosition"];
            emscripten::val setVolume = callbacks["setVolume"];
            emscripten::val setPanLeft = callbacks["setPanLeft"];
            emscripten::val setPanRight = callbacks["setPanRight"];
            emscripten::val setReverbLevel = callbacks["setReverbLevel"];
            emscripten::val getPresetName = callbacks["getPresetName"];
            emscripten::val getPresetCount = callbacks["getPresetCount"];
            emscripten::val applyPreset = callbacks["applyPreset"];
            emscripten::val print = callbacks["print"];
            emscripten::val clearConsole = callbacks["clearConsole"];
            emscripten::val setLoopPoint = callbacks["setLoopPoint"];
            emscripten::val transitionTo = callbacks["transitionTo"];

            env.set_function("raw_print", [this, print](int level, std::string name, std::string message)
            {
                print(level, name, message);
            });

            env.set_function("clear", [clearConsole]()
            {
                clearConsole();
            });

            Scripting::Marker::inject("Marker", env);

            auto snd = env["track"].get_or_create<sol::table>();

            snd.set_function("play", [this, setPause](std::optional<float> seconds={}) -> void
            {
                setPause(false, seconds.value_or(0));
            });

            snd.set_function("pause", [this, setPause](std::optional<float> seconds={}) -> void
            {
                setPause(true, seconds.value_or(0));
            });

            snd.set_function("paused",
            [this](std::optional<bool> pause={}, std::optional<float> seconds={}) -> bool
            {
                if (!pause)
                {
                    return getPause();
                }
                else
                {
                    this->setPause(pause.value(), seconds.value_or(0));
                    return pause.value();
                }
            });

            // doesn't need a callback, because the interface polls for value
            // on update
            snd.set_function("position",
            [this, setPosition](std::optional<double> seconds = {})
            {
                double pos;
                if (seconds)
                {
                    setPosition(seconds.value());
                }

                return getPosition();
            });

            snd.set_function("transition_to",
            [this, transitionTo](float position, float inTime, bool fadeIn, float outTime, bool fadeOut, std::optional<unsigned long long> clock={})
            {
                transitionTo(position, inTime, fadeIn, outTime, fadeOut, clock.value_or(0));
            });

            snd.set_function("volume",
            [this, setVolume](std::optional<int> ch={}, std::optional<float> volume={}, std::optional<float> seconds={})
            {
                if (volume)
                {
                    setVolume(ch.value_or(0), volume.value(), seconds.value_or(0));
                }

                return this->getVolume(ch.value_or(0));
            });

            snd.set_function("pan_left",
            [this, setPanLeft](std::optional<int> ch={}, std::optional<float> value = {}, std::optional<float> seconds={})
            {
                if (value)
                {
                    setPanLeft(ch.value_or(0), value.value(), seconds.value_or(0));
                }

                return this->getPanLeft(ch.value_or(0));
            });

            snd.set_function("pan_right",
            [this, setPanRight](std::optional<int> ch={}, std::optional<float> value = {}, std::optional<float> seconds={})
            {
                if (value)
                {
                    setPanRight(ch.value_or(0), value.value(), seconds.value_or(0));
                }

                return this->getPanRight(ch.value_or(0));
            });

            snd.set_function("reverb_level",
            [this, setReverbLevel](std::optional<int> ch={}, std::optional<float> value = {}, std::optional<float> seconds={})
            {
                if (value)
                {
                    setReverbLevel(ch.value_or(0), value.value(), seconds.value_or(0));
                }

                return this->getReverbLevel(ch.value_or(0));
            });


            snd.set_function("channel_count",
            [this]()
            {
                return this->getChannelCount();
            });

            snd.set_function("loop_point", // loopstart and loopend in milliseconds
            [this, setLoopPoint](std::optional<double> loopstart={}, std::optional<double> loopend={})
            {
                std::cout << "setting loop start: " << loopstart.value_or(0) << ", " << loopend.value_or(0) << '\n';
                if (loopstart)
                {
                    setLoopPoint(loopstart.value(), loopend.value_or(getLength() * 1000.0));
                }

                return this->getLoopPoint();
            });

            // marker namespace
            auto marker = snd["marker"].get_or_create<sol::table>();
            marker.set_function("count",
                [this, getMarkerCount]()
                {
                    return getMarkerCount().as<int>();
                });
            marker.set_function("get",
                [this, getMarker](std::variant<int, std::string> indexOrName)
                {
                    emscripten::val marker;
                    if (indexOrName.index() == 0)
                    {   // index
                        auto index = std::get<int>(indexOrName) - 1; // offset index since lua indexes from 1
                        marker = getMarker(index);

                        if (marker.isUndefined() || marker.isNull())
                        {
                            throw LuaError(lua, "marker at index " +
                                std::to_string(index+1) + " is out of range");
                        }
                    }
                    else // string name
                    {
                        auto name = std::get<std::string>(indexOrName);
                        marker = getMarker(name);
                    }



                    return Scripting::Marker{
                        .name=marker["name"].as<std::string>(),
                        .position=marker["position"].as<double>(),
                    };
                });
            marker.set_function("add",
            [this, addMarker](std::string name, double ms)
            {
                addMarker(name, ms);
            });
            marker.set_function("edit",
            [this, editMarker](std::variant<int, std::string> indexOrName, std::string name, double ms)
            {
                if (indexOrName.index() == 0)
                {
                    auto index = std::get<int>(indexOrName) - 1; // offset since lua indexes from 1
                    editMarker(index, name, ms);
                }
                else
                {
                    auto markerName = std::get<std::string>(indexOrName);
                    editMarker(markerName, name, ms);
                }
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

        emscripten::val print = callbacks["print"];
        this->lua->setErrorCallback(
            [print](const std::string &message, int line)
        {
            static const std::string name = "ERROR";
            std::cout << "Error line: " << line << '\n';
            print(4, name, message, line); // 4 is code for error originating from lua script
        });
    }
}
