#pragma once

#include <insound/SampleDataInfo.h>
#include <insound/SyncPointInfo.h>
#include <insound/LoopInfo.h>

#include <emscripten/val.h>

#include <cstddef>
#include <string>

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
         * Set loop points (in milliseconds)
         *
         * @param loopstart - loop start point in milliseconds
         * @param loopend - loop end point in milliseconds
         */
        void setLoopPoint(unsigned loopstart, unsigned loopend);

        /**
         * Get loop points in milliseconds
         */
        [[nodiscard]]
        LoopInfo<unsigned> getLoopPoint() const;

        [[nodiscard]]
        size_t getSyncPointCount() const;

        [[nodiscard]]
        SyncPointInfo getSyncPoint(int index) const;

        [[nodiscard]]
        SampleDataInfo getSampleData(int index) const;

        void onSyncPoint(emscripten::val callback);

    private:
        void initScriptingEngine();
        MultiTrackAudio *track;
        LuaDriver *lua;
        emscripten::val callbacks;
        float totalTime;
    };
}
