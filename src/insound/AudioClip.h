#pragma once

#include <stddef.h>
#include <vector>

// forward declaration
namespace FMOD
{
    class Sound;
    class System;
}

namespace Insound::Audio
{


    /**
     * Container for an FMOD::Sound
     */
    class AudioClip
    {
    public:
        [[nodiscard]]
        bool isLoaded() const;

        [[nodiscard]]
        bool isSubSound() const;
    private:
        // Low-level handling of lifetime and loading done by AudioPool object
        friend class AudioPool;

        AudioClip();
        ~AudioClip();

        /**
         * Emplaces an FMOD::Sound object directly into the AudioClip.
         * This is for placing a subsound of a multi-sound object.
         * It will throw an error if it is not a subsound.
         */
        void emplace(FMOD::Sound *subsound);

        /**
         * Load a sound from memory. This memory is copied and stored
         * internally, so the data passed into this function can be freed after
         * this function call.
         *
         * @param system     - system object to load sound with
         * @param data       - data pointer
         * @param byteLength - size of the data in bytes
         */
        void load(FMOD::System *system, void *data, size_t byteLength);

        void unload();

        static std::vector<AudioClip *> loadFSB(FMOD::System *sys, void *data, size_t byteLength);

        FMOD::Sound *sound;
    };
}
