#include "LuaDriver.h"
#include "lua.hpp"
#include "params/ParamDesc.h"

static auto DriverScript =
#include "embed/driver.lua.h"
;

static auto NoErrors = "no errors.";

namespace Insound
{
    struct LuaDriver::Impl
    {
        Impl() : error(NoErrors), script(), lua(),
            paramSetCallback([](int index, float value){}),
            paramGetCallback([](const std::string &name, float value)
                {return value;})
        {}

        std::string error;
        std::string script;
        sol::state lua;

        std::function<void(int, float)> paramSetCallback;
        std::function<float(const std::string &, float)> paramGetCallback;
    };

    LuaDriver::LuaDriver() : m(new Impl){ }
    LuaDriver::~LuaDriver() { delete m; }

    bool LuaDriver::load(std::string_view userScript) noexcept
    {
        try
        {
            if (userScript.empty())
            {
                m->error = "cannot load empty script.";
                return false;
            }

            sol::state lua;
            lua.open_libraries(
                sol::lib::base,
                sol::lib::coroutine,
                sol::lib::math,
                sol::lib::string,
                sol::lib::table,
                sol::lib::utf8
            );

            // Event type enum
            lua["Event"] = lua.create_table_with(
                "Init", Event::Init,
                "Update", Event::Update,
                "SyncPoint", Event::SyncPoint,
                "Load", Event::Load,
                "Unload", Event::Unload,
                "TrackEnd", Event::TrackEnd,
                "ParamSet", Event::ParamSet
            );

            auto result = lua.script(DriverScript);
            if (!result.valid())
            {
                sol::error err = result;
                m->error = err.what();
                return false;
            }

            auto loadScript = lua.get<sol::protected_function>("load_script");
            if (!loadScript.valid())
            {
                m->error = "failed to get `load_script` "
                    "function from the Lua sandbox driver code.";
                return false;
            }


            auto processEvent =
                lua.get<sol::protected_function>("process_event");

            if (!processEvent.valid())
            {
                m->error = "failed to get `process_event` function from the "
                    "Lua sandbox driver code.";
                return false;
            }

            result = loadScript(userScript, [&lua, this]() {
                auto env = lua["env"].get<sol::table>();
                auto ins = env["ins"].get_or_create<sol::table>();
                auto param = ins["param"].get_or_create<sol::table>();

                param["set"] = m->paramSetCallback;
                param["get"] = m->paramGetCallback;
            });

            if (!result.valid())
            {
                sol::error err = result;
                m->error = err.what();
                return false;
            }

            m->script = userScript;
            std::swap(m->lua, lua);

            m->error = NoErrors;
            return true;
        }
        catch (const std::exception &e)
        {
            m->error = e.what();
            return false;
        }
        catch (...)
        {
            m->error = "unknown error.";
            return false;
        }
    }

    bool LuaDriver::reload() noexcept
    {
        if (m->script.empty())
        {
            m->error = "cannot reload - no script to load.";
            return false;
        }

        return load(m->script);
    }

    bool LuaDriver::isLoaded() const
    {
        return !m->script.empty();
    }

    std::string_view LuaDriver::getError() const
    {
        return m->error;
    }

    bool LuaDriver::doInit()
    {
        auto result = m->lua["process_event"](Event::Init);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    bool LuaDriver::doUpdate()
    {
        auto result = m->lua["process_event"](Event::Update);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    bool LuaDriver::doSyncPoint(const std::string &label, double seconds)
    {
        auto result = m->lua["process_event"](Event::SyncPoint, label, seconds);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    bool LuaDriver::doLoad(const MultiTrackAudio &track)
    {
        auto result = m->lua["process_event"](Event::Load/*, track - need to bind this type to lua*/);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    bool LuaDriver::doUnload()
    {
        auto result = m->lua["process_event"](Event::Unload);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    bool LuaDriver::doTrackEnd()
    {
        auto result = m->lua["process_event"](Event::TrackEnd);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    bool LuaDriver::doParam(const ParamDesc &param, float value)
    {
        sol::protected_function_result result;

        auto process_event = m->lua.get<sol::protected_function>(
            "process_event");
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
            return false;
        }

        if (param.getType() == ParamDesc::Type::Strings)
        {
            auto &strings = param.getStrings();
            result = process_event(Event::ParamSet, param.getName(),
                strings.at((int)value));
        }
        else
        {
            result = process_event(Event::ParamSet, param.getName(),
                value);
        }

        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();
            return false;
        }

        return true;
    }

    void
    LuaDriver::paramSetCallback(emscripten::val callback)
    {
        if (callback.typeOf().as<std::string>() != "function")
            throw std::runtime_error("Callback must be a function");

        m->paramSetCallback = callback;
    }

    const std::function<void(int, float)> &
    LuaDriver::paramSetCallback() const
    {
        return m->paramSetCallback;
    }

    void LuaDriver::paramGetCallback(emscripten::val callback)
    {
        if (callback.typeOf().as<std::string>() != "function")
            throw std::runtime_error("Callback must be a function");

        m->paramGetCallback = [callback](const std::string &name, float value)
        {
            return callback(name, value).as<float>();
        };
    }

    const std::function<float(const std::string &, float)> &
    LuaDriver::paramGetCallback() const
    {
        return m->paramGetCallback;
    }
}
