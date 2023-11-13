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
         * @param data       - data pointer (binary data)
         * @param bytelength - byte size of the data
         */
        void loadBank(const char *data, size_t bytelength);
        void unloadBank();

        [[nodiscard]]
        bool isBankLoaded() const { return track.isLoaded(); }

        void update();

    private:
        FMOD::System *sys;
        MultiTrackAudio track;
    };
}
