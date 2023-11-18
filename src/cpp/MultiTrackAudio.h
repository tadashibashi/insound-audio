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
        void loadFsb(const char *data, size_t bytelength);

        void unloadFsb();

        [[nodiscard]]
        bool isLoaded() const;

        void play();


        void seek(double seconds);

        [[nodiscard]]
        double getPosition() const;

        [[nodiscard]]
        double getLength() const;

        void stop();


        void setPause(bool pause);

        [[nodiscard]]
        bool getPause() const;


        [[nodiscard]]
        bool isLooping() const;

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
    private:
        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
