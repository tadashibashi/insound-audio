
#pragma once
#include <cstdlib>

namespace FMOD
{
    class System;
}

namespace Insound::Audio
{
    /**
     * Interface for loading and unloading a collection of sounds
     */
    class EventDescription {
    public:
        /**
         * @param sys - pointer reference to the FMOD core system for
         *              creating sounds from
         */
        EventDescription(FMOD::System *sys);
        ~EventDescription();

        /**
         * Load a sound from an FSB file
         * @param data - pointer to the fsb file data in-memory
         * @param size - size of the data in bytes
         */
        void loadFSB(void *data, size_t size);

        /**
         * Load a sound into the Bank
         * @param  data - pointer to data
         * @param  size - size of data in bytes
         *
         * @return      sound index
         */
        int loadSound(void *data, size_t size);
        void unload();

        [[nodiscard]]
        bool isLoaded() const;
        [[nodiscard]]
        bool isFSB() const;

    private:
        class Impl;
        Impl *m;
    };
}
