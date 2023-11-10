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
     * Follows the RAII pattern, where resources are freed when the object is
     * destroyed.
     */
    class MultiTrackAudio {
    public:
        MultiTrackAudio(std::string_view name, FMOD::System *sys);
        ~MultiTrackAudio();

    private:
        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
