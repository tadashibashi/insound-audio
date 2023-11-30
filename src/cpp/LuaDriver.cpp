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
        Impl() : error(NoErrors), script(), lua()
        {}

        std::string error;
        std::string script;
        sol::state lua;

        std::function<void(int, float)> paramCallback;
        std::function<void(int, float)> paramSetFromLuaToJS;
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

            auto env = lua["env"];
            auto ins = env["ins"] = lua.create_table();

            ins["param_set"] = m->paramSetFromLuaToJS;

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

            result = loadScript(userScript);

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

    void LuaDriver::setParamCallback(emscripten::val callback)
    {
        if (callback.typeOf().as<std::string>() != "function")
            throw std::runtime_error("Callback must be a function");

        m->paramCallback = [&callback](int index, float value) -> void {
            callback(index, value);
        };
    }

    const std::function<void(int, float)> &
    LuaDriver::getParamCallback() const
    {
        return m->paramCallback;
    }
}
