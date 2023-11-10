#pragma once

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

    private:
        FMOD::System *sys;
    };
}
