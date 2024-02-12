#pragma once

#include <insound/SampleDataInfo.h>
#include <insound/SyncPointInfo.h>
#include <insound/LoopInfo.h>

#include <emscripten/val.h>

#include <cstddef>
#include <string>
#include <variant>

namespace Insound
{
    class LuaDriver;
    class MultiTrackAudio;

    /**
     * Class intended to be bound via Emscripten Embind for use in JavaScript
     * frontend code. A simple wrapper around MultiTrackAudio for controlling
     * the track play state and mix.
     */
    class MultiTrackControl
    {
    public:
        /**
         * Callbacks:
         *     - syncpoint callback (fires syncpoint as it happens to JS)
         */
        MultiTrackControl(uintptr_t track, emscripten::val callbacks);

        ~MultiTrackControl();

        // ----- Loading / Unloading ------------------------------------------
        void loadSound(size_t data, size_t bytelength);
        void loadBank(size_t data, size_t bytelength);

        /**
         * Load lua script
         *
         * @param  text - script text to load
         * @return        error string or empty string if successful load
         */
        const std::string &loadScript(const std::string &text);

        /**
         * Execute lua script in the current scripting context
         * @param  script - script text to load
         * @return        - string with result of script
         *
         * @throws sol::error on script error
         */
        std::string executeScript(const std::string &script);

        /** Unload any currently loaded sounds, script - resets state, etc. */
        void unload();

        /**
         * Calls update on the lua script engine
         * @param deltaTime - time passed since the last call to `update`
         */
        void update(float deltaTime);

        [[nodiscard]]
        bool isLoaded() const;

        void setPause(bool pause, float seconds = 0);

        [[nodiscard]]
        bool getPause() const;

        /**
         * Set the volume of a specific channel
         *
         * @param ch     - channel to affect (0 is main bus, 1-chSize are
         *               individual channels)
         * @param volume - level to set, where 0 is off, and 1 is 100%
         */
        void setVolume(int ch, float volume);

        /**
         * Get the volume of a specific channel
         *
         * @param ch    channel to get value from (0 is the main bus, 1-chSize
         *              are the individual sub-channels)
         */
        [[nodiscard]]
        float getVolume(int ch) const;

        void setReverbLevel(int ch, float level);

        [[nodiscard]]
        float getReverbLevel(int ch) const;

        void setPanLeft(int ch, float level);

        [[nodiscard]]
        float getPanLeft(int ch) const;

        void setPanRight(int ch, float level);

        [[nodiscard]]
        float getPanRight(int ch) const;

        /**
         * Set the playhead position of the track (in seconds)
         *
         * @param seconds - number of seconds to seek to in the track
         */
        void setPosition(float seconds);

        /**
         * Get the current position of the playhead in the track (in seconds)
         */
        [[nodiscard]]
        float getPosition() const;

        /**
         * Get loaded track's length in seconds
         */
        [[nodiscard]]
        float getLength() const;

        /**
         * Get the number of subchannels in the loaded track (doesn't include
         * the main bus)
         */
        [[nodiscard]]
        int getChannelCount() const;

        [[nodiscard]]
        float getAudibility(int ch) const;

        /**
         * Immediately transition to another position in the track
         * @param position - position within the track in seconds
         * @param inTime   - time in seconds to fade in new position, or to
         *                   delay entrance if `fadeIn` is false
         * @param fadeIn   - fade in new track portion (true),
         *                   or delay entrance (false)
         * @param outTime  - time in seconds to fade out current track position
         *                   or delay stop if `fadeOut` is false
         * @param fadeOut  - fade out current track portion (true),
         *                   or delay stop (false)
         */
        void transitionTo(float position, float inTime, bool fadeIn,
            float outTime, bool fadeOut, unsigned long clock = 0);

        /**
         * Set loop points (in seconds)
         *
         * @param loopstart - loop start point in seconds
         * @param loopend - loop end point in seconds
         */
        void setLoopPoint(double loopstart, double loopend);

        /**
         * Get loop points in seconds
         */
        [[nodiscard]]
        LoopInfo<double> getLoopPoint() const;

        bool addSyncPoint(const std::string &label, double seconds);
        bool deleteSyncPoint(int i);
        bool editSyncPoint(int i, const std::string &label, double seconds);

        [[nodiscard]]
        size_t getSyncPointCount() const;

        [[nodiscard]]
        SyncPointInfo getSyncPoint(int index) const;

        /**
         * Get pointer information about channel sample data
         *
         * @param index - channel index, 0-based
         */
        [[nodiscard]]
        SampleDataInfo getSampleData(int index) const;

        void onSyncPoint(emscripten::val callback);

        void doMarker(const std::string &name, double seconds);

        [[nodiscard]]
        float samplerate() const;
        [[nodiscard]]
        unsigned long dspClock() const;

        void setParameter(const std::string &name, emscripten::val value);

    private:
        void initScriptingEngine();
        MultiTrackAudio *track;
        LuaDriver *lua;
        emscripten::val callbacks;
        float totalTime;
    };
}
