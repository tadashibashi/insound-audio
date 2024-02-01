#include "LuaDriver.h"
#include "lua.hpp"

#include <insound/params/ParamDesc.h>

static auto DriverScript =
#include <insound/embed/driver.lua.h>
;

#include <charconv>
#include <chrono>
#include <string_view>

static auto NoErrors = "no errors.";

namespace Insound
{
    /** Data parsed from a sol::error */
    struct SolErrorData
    {
        /** Line number, or 0, if null */
        int lineNumber;
        /** Error message */
        std::string_view message;
    };

    /**
     * Get error data from a sol::error object.
     * Reliant on the message formatting, may break if Lua changes it's
     * error message format in a later version.
     *
     * @param err - error object to parse
     */
    static SolErrorData parseSolError(const std::exception &err)
    {
        int lineNumber;
        std::string_view errorMessage;

        std::string_view fullMessage = err.what();
        auto message = fullMessage.substr(0, fullMessage.find('\n'));

        // Error format:  [<filename or code>]:<line_number>:<error_message>

        // Get line number
        const auto beforeLineNumberIndex = message.rfind("]:");
        const auto afterLineNumberIndex = message.find(":",
            beforeLineNumberIndex + 2);
        if (beforeLineNumberIndex == std::string_view::npos ||
            afterLineNumberIndex == std::string_view::npos)
        {
            // couldn't parse line number, set just return full message
            lineNumber = 0;
            errorMessage = message;
        }
        else
        {
            // found line number, parse it
            auto numberStr = message.substr(beforeLineNumberIndex + 2, afterLineNumberIndex - (beforeLineNumberIndex + 2));
            auto result = std::from_chars(numberStr.data(), numberStr.data() + numberStr.size(), lineNumber);

            if (result.ec == std::errc::invalid_argument)
            {
                lineNumber = 0;
            }

            errorMessage = message.substr(afterLineNumberIndex + 1);
        }

        return {
            .lineNumber=lineNumber,
            .message=errorMessage
        };
    }


    /**
     * Implementation class for LuaDriver
     */
    struct LuaDriver::Impl
    {
        Impl(const std::function<void(sol::table &)> &populateEnv)
        : error(NoErrors), script(), lua(), onError(),
        populateEnv(populateEnv)
        {
        }

        // last error that occurred
        std::string error;
        // currently loaded script
        std::string script;
        // lua context
        sol::state lua;
        // callback populatates the env from owner
        std::function<void(sol::table &)> populateEnv;
        std::function<void(const std::string &, int)> onError;
    };

    LuaDriver::LuaDriver(const std::function<void(sol::table &)> &populateEnv)
    : m( new Impl(populateEnv) )
    {
        if (!m->populateEnv)
            throw std::runtime_error("No function target set on populateEnv");
    }

    LuaDriver::~LuaDriver() { delete m; }

    bool LuaDriver::load(std::string_view userScript)
    {
        try
        {
            // Create the lua state, providing std lib
            sol::state lua;
            lua.open_libraries(
                sol::lib::base,
                sol::lib::coroutine,
                sol::lib::math,
                sol::lib::string,
                sol::lib::table,
                sol::lib::utf8
            );

            // Event type enum used by `process_event` reducer
            lua["Event"] = lua.create_table_with(
                "Init", Event::Init,
                "Update", Event::Update,
                "SyncPoint", Event::SyncPoint,
                "Load", Event::Load,
                "Unload", Event::Unload,
                "TrackEnd", Event::TrackEnd,
                "ParamSet", Event::ParamSet
            );

            // Load the driver code
            auto result = lua.script(DriverScript);
            if (!result.valid())
            {
                sol::error err = result;
                m->error = err.what();
                return false;
            }

            // Populate the environment with basic lua libs and functions
            auto reset_env = lua["reset_env"].get<sol::function>();
            if (!reset_env.valid())
            {
                m->error =
                    "failed to get `reset_env` from Lua sandbox driver.";
                return false;
            }
            reset_env();

            // Get the env object to populate with custom engine functions
            auto env = lua["env"].get_or_create<sol::table>();
            if (!env.valid())
            {
                m->error = "failed to get `env` from Lua sandbox driver code.";
                return false;
            }

            // Populate the environment with custom engine functionality
            if (m->populateEnv) m->populateEnv(env);

            if (!userScript.empty())
            {
                // Get the load_script function to create sandbox with user script
                auto loadScript = lua.get<sol::unsafe_function>("load_script");
                if (!loadScript.valid())
                {
                    m->error = "failed to get `load_script` "
                        "function from the Lua sandbox driver code.";
                    return false;
                }

                // Just check that this function exists which is used as a reducer
                // to run events.
                auto processEvent =
                    lua.get<sol::unsafe_function>("process_event");
                if (!processEvent.valid())
                {
                    m->error = "failed to get `process_event` function from the "
                        "Lua sandbox driver code.";
                    return false;
                }

                // Finally load the script into the sandbox
                result = loadScript(userScript);
                if (!result.valid())
                {
                    sol::error err = result;
                    m->error = err.what();

                    // callback error message
                    if (m->onError)
                    {
                        auto data = parseSolError(err);
                        m->onError(data.message.data(), data.lineNumber);
                    }

                    return false;
                }
            }

            // Done, commit changes
            m->script = userScript;
            std::swap(m->lua, lua);
            m->error = NoErrors;

            return true;
        }
        catch (const std::exception &e)
        {
            m->error = e.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(e);
                m->onError(data.message.data(), data.lineNumber);
            }

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

    std::string LuaDriver::execute(std::string_view script)
    {
        auto execute_string = m->lua["execute_string"]
            .get<sol::unsafe_function>();
        if (!execute_string.valid())
        {
            throw std::runtime_error("Failed to get execute_string from lua driver");
        }

        try {
            auto result = execute_string(script);

            if (!result.valid())
            {
                sol::error err = result;
                m->error = err.what();

                if (m->onError)
                {
                    auto data = parseSolError(err);
                    m->onError(data.message.data(), data.lineNumber);
                }

                return m->error;
            }

            return result.get<std::string>();
        }
        catch(const sol::error &e)
        {
            if (m->onError)
            {
                auto data = parseSolError(e);
                m->onError(data.message.data(), data.lineNumber);
            }

            return e.what();
        }

    }

    bool LuaDriver::isLoaded() const
    {
        return !m->script.empty();
    }

    const std::string &LuaDriver::getError() const
    {
        return m->error;
    }

    bool LuaDriver::doInit()
    {
        if (!isLoaded())
        {
            m->error = "Script is not loaded";
            return false;
        }

        auto process_event = m->lua["process_event"]
            .get<sol::protected_function>();
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
            return false;
        }

        auto result = process_event(Event::Init);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    bool LuaDriver::doUpdate(double delta, double total)
    {
        if (!isLoaded()) return false; // no err set since it may be expensive?

        auto process_event = m->lua["process_event"]
            .get<sol::unsafe_function>();
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
            return false;
        }

        auto result = process_event(Event::Update, delta, total);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    bool LuaDriver::doSyncPoint(const std::string &label, double seconds)
    {
        if (!isLoaded())
        {
            m->error = "Script is not loaded";
            return false;
        }

        auto process_event = m->lua["process_event"]
            .get<sol::unsafe_function>();
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
        }

        auto result = process_event(Event::SyncPoint, label, seconds);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    bool LuaDriver::doLoad(const MultiTrackAudio &track)
    {
        if (!isLoaded())
        {
            m->error = "Script is not loaded";
            return false;
        }

        auto process_event = m->lua["process_event"]
            .get<sol::unsafe_function>();
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
        }

        auto result = process_event(Event::Load);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    bool LuaDriver::doUnload()
    {
        if (!isLoaded())
        {
            m->error = "Script is not loaded";
            return false;
        }

        auto process_event = m->lua["process_event"]
            .get<sol::unsafe_function>();
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
        }

        auto result = process_event(Event::Unload);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    bool LuaDriver::doTrackEnd()
    {
        if (!isLoaded())
        {
            m->error = "Script is not loaded";
            return false;
        }

        auto process_event = m->lua["process_event"]
            .get<sol::unsafe_function>();
        if (!process_event.valid())
        {
            m->error = "Could not get `process_event` function from lua "
                "driver.";
            return false;
        }

        auto result = process_event(Event::TrackEnd);
        if (!result.valid())
        {
            sol::error err = result;
            m->error = err.what();

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    bool LuaDriver::doParam(const ParamDesc &param, float value)
    {
        if (!isLoaded())
        {
            m->error = "Script is not loaded";
            return false;
        }

        sol::unsafe_function_result result;

        auto process_event = m->lua.get<sol::unsafe_function>(
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

            // callback error message
            if (m->onError)
            {
                auto data = parseSolError(err);
                m->onError(data.message.data(), data.lineNumber);
            }

            return false;
        }

        return true;
    }

    const sol::state &LuaDriver::context() const
    {
        return m->lua;
    }

    sol::state &LuaDriver::context()
    {
        return m->lua;
    }

    void LuaDriver::setErrorCallback(
        const std::function<void(const std::string &, int)> &callback)
    {
        m->onError = callback;
    }
}
