/**
 * @file LuaDriver.h
 *
 * Contains driver for user script files.
 *
 * The Lua language was chosen over JavaScript, since it would protect the
 * frontend from unwanted browser manipulations, either from malicious code or
 * protecting the user from themselves. (Enabling `eval` in client code is
 * considered a significant security vulnerability).
 *
 * Inside the driver sandbox, only functions pertaining to the AudioEngine are
 * provided.
 *
 */
#pragma once

#include <sol/forward.hpp>

#include <functional>
#include <string>
#include <string_view>

namespace Insound
{
    class MultiTrackAudio;
    class ParamDesc;

    class LuaDriver
    {
    public:
        explicit LuaDriver(const std::function<void(sol::table &)> &populateEnv);
        ~LuaDriver();

        /**
         * Load and execute a lua script
         *
         * @param  userScript - string containing the user's lua code
         *
         * @return whether load was successful
         */
        bool load(std::string_view userScript);

        /**
         * Execute script in the current context
         *
         * @param script - valid lua script to execute
         *
         * @returns result as a string to display
         */
        std::string execute(std::string_view script);

        /**
         * Reload the last successfully loaded script
         *
         * @return whether load was successful
         */
        bool reload() noexcept;

        /**
         * Whether a script was successfully loaded into the driver.
         */
        [[nodiscard]]
        bool isLoaded() const;

        /**
         * Get the result message from the last call to `LuaDriver::load` or
         * `LuaDriver::reload`
         *
         * Check this if the result of either of these functions is `false`
         *
         * @return last error message.
         */
        [[nodiscard]]
        const std::string &getError() const;

        bool doInit();
        bool doUpdate(double delta, double total);
        bool doSyncPoint(const std::string &label, double seconds);
        bool doLoad(const MultiTrackAudio &track);
        bool doUnload();
        bool doTrackEnd();

        // To be called by the AudioEngine from JavaScript to let our Lua
        // API know that a parameter has been set.
        bool doParam(const ParamDesc &param, float value);

        /**
         * Set a callback that fires when an error occurs.
         * The first parameter contains the error message, and the second
         * is an integer with the line that the error occurred.
         *
         * @param callback - callback to set
         */
        void setErrorCallback(const std::function<void(const std::string &, int)> &callback);

        [[nodiscard]]
        sol::state &context();
        [[nodiscard]]
        const sol::state &context() const;
    private:
        enum class Event {
            Init,
            Update,
            SyncPoint,
            Load,
            Unload,
            TrackEnd,
            ParamSet,
            MaxCount, // leave this last
        };
        
        struct Impl;
        Impl *m;
    };
}
