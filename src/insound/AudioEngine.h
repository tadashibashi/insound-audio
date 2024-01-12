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
        bool init();
        void close();

        void update();
        void resume();
        void suspend();

        uintptr_t createTrack();
        void deleteTrack(uintptr_t track);

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
        FMOD::System *sys;
        std::optional<Channel> master;
        std::vector<MultiTrackAudio *> tracks;
    };
}
