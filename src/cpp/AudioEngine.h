#pragma once

#include "MultiTrackAudio.h"

// Forward declaration
namespace FMOD {
    class System;
}

namespace Insound {
    class AudioEngine {
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

        /**
         * Play current loaded bank, if any is loaded.
         */
        void play();

        void stop();

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
        void setPause(bool pause);

        void update();

    private:
        FMOD::System *sys;
        MultiTrackAudio *track;
    };
}
