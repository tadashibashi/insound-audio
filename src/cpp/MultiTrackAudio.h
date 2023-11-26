#pragma once
#include <string_view>

// Forward declaration
namespace FMOD {
    class System;
}

namespace Insound {
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
        int trackCount() const;

        [[nodiscard]]
        int samplerate() const;

        void setSyncPointCallback(
            std::function<void(const std::string &, double)> callback);

        const std::function<void(const std::string &, double)> &
        getSyncPointCallback() const;

        void setEndCallback(std::function<void()> callback);

        const std::function<void()> &getEndCallback() const;

        [[nodiscard]]
        size_t getNumSyncPoints() const;
        [[nodiscard]]
        const std::string &getSyncPointLabel(size_t i) const;
        [[nodiscard]]
        double getSyncPointOffsetSeconds(size_t i) const;
    private:
        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
