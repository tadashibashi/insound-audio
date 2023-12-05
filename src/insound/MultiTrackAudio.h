#pragma once
#include <string>
#include <string_view>

// Forward declaration
namespace FMOD {
    class System;
}

namespace Insound {
    class ParamDescMgr;
    class PresetMgr;
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
         * Load fsb file from memory. Copies memory, safe to delete afterward.
         * @param  data       memory pointer to the fsb
         * @param  bytelength byte size of the memory block
         *
         * @throw FMODError on error
         */
        void loadFsb(const char *data, size_t bytelength, bool looping=true);

        void unloadFsb();

        [[nodiscard]]
        bool isLoaded() const;

        void seek(double seconds);

        [[nodiscard]]
        double getPosition() const;

        [[nodiscard]]
        double getLength() const;

        void setPause(bool pause, float seconds);

        [[nodiscard]]
        bool getPause() const;


        [[nodiscard]]
        bool isLooping() const;

        // Fade main volume to a certain level
        void fadeTo(float to, float seconds);

        void fadeChannelTo(int ch, float to, float seconds);

        [[nodiscard]]
        float getFadeLevel(bool final=true) const;

        [[nodiscard]]
        float getChannelFadeLevel(int ch, bool final=true) const;

        /**
         * Set whether the track should loop
         * @param looping - whether track should loop
         */
        void setLooping(bool looping);


        [[nodiscard]]
        double getMainVolume() const;

        void setMainVolume(double vol);

        [[nodiscard]]
        double getChannelVolume(int ch) const;

        void setChannelVolume(int ch, double vol);

        [[nodiscard]]
        int channelCount() const;

        [[nodiscard]]
        int samplerate() const;

        void setSyncPointCallback(
            std::function<void(const std::string &, double)> callback);
        [[nodiscard]]
        const std::function<void(const std::string &, double)> &
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

        PresetMgr &presets();
        const PresetMgr &presets() const;

        void applyPreset(std::string_view name, float seconds=1.f);
        void applyPreset(size_t index, float seconds=1.f);
        void applyPreset(const Preset &preset, float seconds);
    private:
        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
