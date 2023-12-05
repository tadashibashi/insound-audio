#pragma once
#ifdef __EMSCRIPTEN__

#include <insound/Channel.h>
#include <insound/scripting/LuaDriver.h>
#include <insound/MultiTrackAudio.h>
#include <insound/params/ParamDescMgr.h>

#include <emscripten/val.h>

// Forward declaration
namespace FMOD
{
    class System;
}

namespace Insound
{
    class Channel;

    class AudioEngine
    {
    public:
        AudioEngine(
            emscripten::val setParam,
            emscripten::val getParam);
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

        const std::string &loadScript(const std::string &text);

        [[nodiscard]]
        bool isBankLoaded() const { return track && track->isLoaded(); }

        void setMainVolume(double vol);

        [[nodiscard]]
        double getMainVolume() const;

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

        [[nodiscard]]
        int trackCount() const;

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

    private:
        FMOD::System *sys;
        MultiTrackAudio *track;
        std::optional<Channel> master;
        std::optional<LuaDriver> lua;
    };
}

#endif