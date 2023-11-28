#pragma once
#include "Channel.h"
#include "MultiTrackAudio.h"

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
        AudioEngine();
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
         * @param seconds - time in seconds
         */
        void seek(double seconds);

        /**
         * Set paused status
         * @param pause - whether to pause
         */
        void setPause(bool pause, float seconds);

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
        float param_getInitValueByIndex(size_t index) const;
        [[nodiscard]]
        float param_getInitValue(const std::string &name) const;
        [[nodiscard]]
        float param_getByIndex(size_t index) const;
        [[nodiscard]]
        float param_get(const std::string &name) const;

        [[nodiscard]]
        const std::string &param_getName(size_t index) const;

        [[nodiscard]]
        size_t param_count() const;

        [[nodiscard]]
        size_t param_labelCount(size_t paramIndex) const;

        [[nodiscard]]
        const std::string &param_getLabelName(size_t paramIndex,
            size_t labelIndex) const;
        [[nodiscard]]
        float param_getLabelValue(size_t paramIndex, size_t labelIndex) const;

        void param_setByIndex(size_t index, float value);
        void param_setFromLabelByIndex(size_t index, const std::string &labelVal);
        void param_set(const std::string &name, float value);
        void param_setFromLabel(const std::string &name, const std::string &labelVal);
        void param_addLabel(const std::string &paramName,
            const std::string &label, float value);
    private:
        FMOD::System *sys;
        MultiTrackAudio *track;
        std::optional<Channel> master;
    };
}
