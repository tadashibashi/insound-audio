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
#include <emscripten/val.h>
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
        LuaDriver();
        ~LuaDriver();

        /**
         * Load and execute a lua script
         *
         * @param  userScript - string containing the user's lua code
         *
         * @return whether load was successful
         */
        bool load(std::string_view userScript) noexcept;

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
        std::string_view getError() const;

        bool doInit();
        bool doUpdate();
        bool doSyncPoint(const std::string &label, double seconds);
        bool doLoad(const MultiTrackAudio &track);
        bool doUnload();
        bool doTrackEnd();

        // To be called by the AudioEngine from JavaScript to let our Lua
        // API know that a parameter has been set.
        bool doParam(const ParamDesc &param, float value);

        // Received from JavaScript frontend Parameter API
        // This must be set before loading script via `load`.
        void paramSetCallback(emscripten::val callback);
        
        [[nodiscard]]
        const std::function<void(int, float)> & paramSetCallback() const;
        void paramGetCallback(emscripten::val callback);
        const std::function<float(const std::string &, float)> &paramGetCallback() const;
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
