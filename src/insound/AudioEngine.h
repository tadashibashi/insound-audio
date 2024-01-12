#pragma once

#include <insound/Channel.h>
#include <insound/scripting/LuaDriver.h>
#include <insound/params/ParamDescMgr.h>
#include <insound/SampleDataInfo.h>

#include <emscripten/val.h>

#include <chrono>

// Forward declaration
namespace FMOD
{
    class System;
}

namespace Insound
{
    class Channel;
    class MultiTrackAudio;

    class AudioEngine
    {
    public:
        AudioEngine();
        ~AudioEngine();

    public:
        /**
         * Initialize audio engine
         * @return whether initialization was successful
         * @throws runtime error if there was an error, propagating to the
         *         frontend as a wasm error (if proper link flag is set)
         */
        bool init();

        /**
         * Update the audio engine to process all sound/params/levels/etc.
         * Should be called at least once every 20ms, if not faster for
         * priority of performance in-browser.
         */
        void update();

        /**
         * Resume the audio context
         */
        void resume();

        /**
         * Suspend the audio context
         */
        void suspend();

        /**
         * Create a MultitTrackAudio object that should be wrapped inside of
         * a MultiTrackChannel object.
         * @return ptr to the object for which JS code takes responsibility of.
         *             JS should free this manually when done with by calling
         *             `deleteTrack` on this pointer. It gets cleaned up during
         *             AudioEngine disposal, however.
         */
        uintptr_t createTrack();

        /**
         * Release memory and resources of track. Please call this with all
         * pointers retrieved via `createTrack` when done with.
         *
         * @param track - pointer to an MultiTrackAudio object
         */
        void deleteTrack(uintptr_t track);

        /**
         * Get the volume level of the master bus
         * @returns level - 0: off, 1: 100%
         */
        [[nodiscard]]
        float getMasterVolume() const;
        /**
         * Set the volume of the master bus
         * @param level - 0: off, 1: 100%
         */
        void setMasterVolume(float level);

        /**
         * Get the audibility of the master channel
         */
        [[nodiscard]]
        float getAudibility() const;

        /**
         * Total cpu usage of the underlying audio system
         */
        [[nodiscard]]
        float getCPUUsageTotal() const;

        /**
         * CPU usage of DSP units, part of the total retrieved from
         * `getCPUUsageTotal`
         */
        [[nodiscard]]
        float getCPUUsageDSP() const;
    private:
        /**
         * Called during destructor, invalidating all internals. Can be
         * called before object is destroyed to free resources in advance.
         */
        void close();
        FMOD::System *sys;
        std::optional<Channel> master;
        std::vector<MultiTrackAudio *> tracks;
    };
}
