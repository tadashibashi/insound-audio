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
         * Load fsb file from a memory point. Does not copy the memory or own
         * it. That memory should be stored until no longer needed by the
         * MultiTrackAudio object.
         *
         * @param  data       memory pointer to the fsb
         * @param  bytelength byte size of the memory block
         *
         * @throw FMODError on error
         */
        void loadFsb(const char *data, size_t bytelength);

        void unloadFsb();
    private:
        // Pimple idiom
        struct Impl;
        Impl *m;
    };
}
