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
#include <string>
#include <string_view>

namespace Insound
{
    class MultiTrackAudio;

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

    private:
        enum class Event {
            Init,
            Update,
            SyncPoint,
            Load,
            Unload,
            TrackEnd,
            MaxCount, // leave this last
        };
        
        struct Impl;
        Impl *m;
    };
}
