#pragma once
#include "insound/Channel.h"
#include "insound/LoopInfo.h"
#include <functional>
#include <string>
#include <string_view>

// Forward declaration
namespace FMOD {
    class System;
    class Sound;
}

namespace Insound {
    class ParamDescMgr;
    class Preset;

    /**
     * Container of loaded audio tracks to be played in sync.
     *
     */
    class MultiTrackAudio {
    public:
        MultiTrackAudio(FMOD::System *sys);
        ~MultiTrackAudio();

        /**
         * Load fsb file from memory.
         * Reads all syncpoint/marker data from the first sound in the bank,
         * all other track syncoints are ignored.
         *
         * @param  data       memory pointer to the fsb
         * @param  bytelength byte size of the memory block
         *
         * @throw runtime_error on error, or FMODError, which is a subclass of
         *                      runtime_error.
         */
        void loadFsb(const char *data, size_t bytelength);

        /**
         * Add sounds separately. This is useful for testing audio without
         * needing a compiled FSBank.
         *
         * @param data       - pointer to the data
         * @param bytelength - byte size of the data
         */
        uintptr_t loadSound(const char *data, size_t bytelength);

        /**
         * Unload fsb file from memory and reset internals
         */
        void clear();

        /**
         * Check if bank is currently loaded
         */
        [[nodiscard]]
        bool isLoaded() const;

        /**
         * Seek current track to a position in the track (in seconds)
         *
         * @param seconds - position in the number of seconds to seek to.
         */
        void position(double seconds);

        /**
         * Get current seek position of the track in seconds.
         */
        [[nodiscard]]
        double position() const;

        /**
         * Get the length of the track in seconds.
         */
        [[nodiscard]]
        double length() const;

        /**
         * Set the paused status of the track
         *
         * @param pause   - whether to pause `true` or unpause `false` track
         * @param seconds - number of seconds to fade in/out pause
         */
        void pause(bool pause, float seconds);

        /**
         * Get the paused status of the track
         *
         * @returns true if paused, false if not.
         */
        [[nodiscard]]
        bool paused() const;

        /**
         * Get the approximated channel output taking all engine parameters
         * into account.
         */
        [[nodiscard]]
        float audibility() const;


        // Fade main volume to a certain level
        void fadeTo(float to, float seconds);

        void fadeChannelTo(int ch, float to, float seconds);

        /**
         * Get the current fade level of the track's master bus
         *
         * @param final - whether to get live calculated value `false`, or the
         *                value targeted by the last call to a fade function
         *                `true` (default behavior)
         *
         * @returns - current fade level (0=no sound, 1=full sound)
         */
        [[nodiscard]]
        float fadeLevel(bool final=true) const;

        /**
         * Get the current fade level of a channel
         *
         * @param final - whether to get live calculated value `false`, or the
         *                value targeted by the last call to a fade function
         *                `true` (default behavior)
         *
         * @returns - current fade level of channel (0=no sound, 1=full sound)
         */
        [[nodiscard]]
        float channelFadeLevel(int ch, bool final=true) const;

        /**
         * Get the volume level of the track's main bus
         *
         * @returns - current volume level where 0 = 0%, 1 = 100%; values over
         *            1 are allowed, and negative values mean reversed polarity
         */
        [[nodiscard]]
        double mainVolume() const;

        /**
         * Set the volume level of the track's main bus
         *
         * @param vol - volume level to set where 0 = 0%, 1 = 100%; values over
         *            1 are allowed, and negative values mean reversed polarity
         */
        void mainVolume(double vol);

        [[nodiscard]]
        double channelVolume(int ch) const;

        void channelVolume(int ch, double vol);

        [[nodiscard]]
        Channel &channel(int ch);
        [[nodiscard]]
        const Channel &channel(int ch) const;

        [[nodiscard]]
        int channelCount() const;

        void setSyncPointCallback(
            std::function<void(const std::string &, double, int)> callback);
        [[nodiscard]]
        const std::function<void(const std::string &, double, int)> &
        getSyncPointCallback() const;

        /**
         * This callback fires when the current track has reached its end point
         *
         * @param callback - callback to set
         */
        void setEndCallback(std::function<void()> callback);

        void setReadyCallback(std::function<void()> &&callback);

        /**
         * Get the callback that fires when current track has ended.
         * May or may not contain a target.
         */
        [[nodiscard]]
        const std::function<void()> &getEndCallback() const;

        bool addSyncPointMS(const std::string &name,
            double offset);

        bool editSyncPointMS(size_t i, const std::string &name,
            double offset);

        bool deleteSyncPoint(size_t i);

        /**
         * Get the number of sync points in the current track.
         * Only checks the first track.
         */
        [[nodiscard]]
        size_t getSyncPointCount() const;

        [[nodiscard]]
        bool getSyncPointsEmpty() const;

        [[nodiscard]]
        std::string_view getSyncPointLabel(size_t i) const;
        [[nodiscard]]
        double getSyncPointOffsetSeconds(size_t i) const;
        [[nodiscard]]
        double getSyncPointOffsetMS(size_t i) const;

        [[nodiscard]]
        float channelReverbLevel(int ch) const;

        void channelReverbLevel(int ch, float level);

        [[nodiscard]]
        float mainReverbLevel() const;

        void mainReverbLevel(float level);

        void loopMilliseconds(double loopstart, double loopend);
        void loopSeconds(double loopstart, double loopend);
        void loopSamples(unsigned loopstart, unsigned loopend);

        [[nodiscard]]
        LoopInfo<double> loopSeconds() const;
        [[nodiscard]]
        LoopInfo<unsigned> loopSamples() const;
        [[nodiscard]]
        LoopInfo<double> loopMilliseconds() const;

        [[nodiscard]]
        Channel &main();
        [[nodiscard]]
        const Channel &main() const;

        [[nodiscard]]
        const std::vector<float> &getSampleData(size_t index) const;

        [[nodiscard]]
        float samplerate() const;

        

    private:

        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
