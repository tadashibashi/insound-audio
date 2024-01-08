#pragma once

#include <insound/Channel.h>
#include <insound/scripting/LuaDriver.h>
#include <insound/MultiTrackAudio.h>
#include <insound/params/ParamDescMgr.h>

#include <emscripten/val.h>

#include <chrono>

// Forward declaration
namespace FMOD
{
    class System;
}

struct SampleDataInfo
{
    uintptr_t ptr;
    size_t byteLength;
};

namespace Insound
{
    class Channel;

    class AudioEngine
    {
    public:
        AudioEngine(emscripten::val cbs);
        ~AudioEngine();

    public:
        bool init();
        void close();
        void resume();
        void suspend();

        /**
         * Load bank from memory pointer. This memory must be available for
         * the lifetime of its use and is not managed by the audio engine.
         *
         * @param data       - data pointer (using number type to interop with js)
         * @param bytelength - byte size of the data
         */
        void loadBank(size_t data, size_t bytelength);
        void unloadBank();

        /**
         * Load multiple sounds into the track. Unloads any currently loaded
         * files or bank. Please keep the data valid for the lifetime of use
         * and dispose after unload has been called.
         * @param dataBuffers - pointers to data
         * @param byteLengths - bytelength of each buffer
         * @param bufferCount - number of buffers provided
         */
        void loadSound(size_t data, size_t bytelength);

        const std::string &loadScript(const std::string &text);

        [[nodiscard]]
        bool isBankLoaded() const { return track && track->isLoaded(); }

        void setMainVolume(double vol);

        [[nodiscard]]
        double getMainVolume() const;

        void setMasterVolume(double vol);

        [[nodiscard]]
        double getMasterVolume() const;

        /**
         * Set the volume of a channel currently playing
         *
         * @param ch  channel number
         * @param vol volume level (0 - off, 1 - 100%)
         *
         */
        void setChannelVolume(int ch, double vol);

        [[nodiscard]]
        double getChannelVolume(int ch) const;


        void fadeChannelTo(int ch, float level, float seconds);

        /**
         *
         * @param ch    - channel index (0-indexed)
         * @param final - whether to use current value (true), or
         *                target value (false)
         *
         * @return volume level of fade (different from channel volume)
         */
        [[nodiscard]]
        float getChannelFadeLevel(int ch, bool final=true) const;


        [[nodiscard]]
        float getFadeLevel(bool final=true) const;

        [[nodiscard]]
        double getPosition() const;

        [[nodiscard]]
        double getLength() const;

        /**
         * Seek to a postiion in the audio
         *
         * @param seconds - track time point in seconds
         */
        void seek(double seconds);

        /**
         * Set paused status
         * @param pause   - whether to pause
         * @param seconds - time to fade in/out of pause
         */
        void setPause(bool pause, float seconds = 0);

        /**
         * Get whether engine is paused.
         */
        [[nodiscard]]
        bool getPause() const;

        [[nodiscard]]
        bool isLooping() const;

        void setLooping(bool looping);

        void setLoopSeconds(double loopstart, double loopend);
        void setLoopSamples(unsigned loopstart, unsigned loopend);

        [[nodiscard]]
        LoopInfo<double> getLoopSeconds() const;
        [[nodiscard]]
        LoopInfo<unsigned> getLoopSamples() const;

        [[nodiscard]]
        int getChannelCount() const;

        void fadeTo(float to, float seconds);

        void update();

        void setSyncPointCallback(emscripten::val callback);
        void setEndCallback(emscripten::val callback);

        [[nodiscard]]
        ParamDesc::Type param_getType(size_t index) const;
        [[nodiscard]]
        const std::string &param_getName(size_t index) const;
        [[nodiscard]]
        const StringsParam &param_getAsStrings(size_t index) const;
        [[nodiscard]]
        const NumberParam &param_getAsNumber(size_t index) const;
        [[nodiscard]]
        size_t param_count() const;

        // To be called from JavaScript when a parameter has been set.
        // Notifies Lua Driver for scriptable callbacks.
        void param_send(size_t index, float value);

        void setMainReverbLevel(float level);
        [[nodiscard]]
        float getMainReverbLevel() const;

        void setChannelReverbLevel(int ch, float level);
        [[nodiscard]]
        float getChannelReverbLevel(int ch) const;

        [[nodiscard]]
        size_t getSyncPointCount() const;
        [[nodiscard]]
        std::string getSyncPointLabel(size_t index) const;
        [[nodiscard]]
        double getSyncPointOffsetSeconds(size_t index) const;

        void setChannelPanLeft(int ch, float level);
        void setChannelPanRight(int ch, float level);
        void setChannelPan(int ch, float left, float right);

        [[nodiscard]]
        float getChannelPanLeft(int ch) const;
        [[nodiscard]]
        float getChannelPanRight(int ch) const;

        void setMainPanLeft(float level);
        void setMainPanRight(float level);
        void setMainPan(float left, float right);

        [[nodiscard]]
        float getMainPanLeft() const;
        [[nodiscard]]
        float getMainPanRight() const;

        [[nodiscard]]
        float getCPUUsageTotal() const;
        [[nodiscard]]
        float getCPUUsageDSP() const;

        [[nodiscard]]
        SampleDataInfo getSampleData(size_t index) const;

        [[nodiscard]]
        MultiTrackAudio *getTrack() { return track; }

    private:
        FMOD::System *sys;
        MultiTrackAudio *track;
        std::optional<Channel> master;
        std::optional<LuaDriver> lua;

        // track time for update calls
        using time_point = std::chrono::time_point<std::chrono::system_clock>;
        time_point startTime, lastFrame;

        unsigned long long downtime;

        bool isSuspended;
    };
}
