#pragma once
#include "insound/LoopInfo.h"
#include <functional>
#include <string>
#include <string_view>

// Forward declaration
namespace FMOD {
    class System;
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
        MultiTrackAudio(std::string_view name, FMOD::System *sys);
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
        void loadFsb(const char *data, size_t bytelength, bool looping=true);

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

        [[nodiscard]]
        bool paused() const;




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
         * @return - current fade level (0=no sound, 1=full sound)
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
         * @return - current fade level of channel (0=no sound, 1=full sound)
         */
        [[nodiscard]]
        float channelFadeLevel(int ch, bool final=true) const;

        /**
         * Set whether the track should loop
         * @param looping - whether track should loop
         */
        void looping(bool looping);

        [[nodiscard]]
        bool looping() const;

        [[nodiscard]]
        double mainVolume() const;

        void mainVolume(double vol);

        [[nodiscard]]
        double channelVolume(int ch) const;

        void channelVolume(int ch, double vol);

        [[nodiscard]]
        int channelCount() const;

        [[nodiscard]]
        int samplerate() const;

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
        [[nodiscard]]

        /**
         * Get the callback that fires when current track has ended.
         * May or may not contain a target.
         */
        const std::function<void()> &getEndCallback() const;

        void addSyncPointMS(const std::string &name,
            unsigned int offset);
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

        ParamDescMgr &params();
        const ParamDescMgr &params() const;

        [[nodiscard]]
        float channelReverbLevel(int ch) const;

        void channelReverbLevel(int ch, float level);

        [[nodiscard]]
        float mainReverbLevel() const;

        void mainReverbLevel(float level);


        void loopSeconds(double loopstart, double loopend);
        void loopSamples(unsigned loopstart, unsigned loopend);

        LoopInfo<double> loopSeconds() const;
        LoopInfo<unsigned> loopSamples() const;
    private:
        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
