#pragma once

#include "System.h"
#include "AudioClip.h"

#include <cstddef>

namespace Insound::Audio
{
    /**
     * Session store for loaded FMOD::Sound objects
     */
    class AudioPool
    {
    public:
        explicit AudioPool(System *sys);

        AudioClip *loadSound(void *data, size_t byteLength);
        AudioClip *loadFSB(void *data, size_t byteLength);

        /**
         * Unload an AudioClip
         * @param  clip - clip to unload
         * @return      whether clip was unloaded or not; if clip doesn't
         *              belong to this pool, it will not unload it
         */
        bool unloadClip(AudioClip *clip);
    private:
        System *sys;
        std::vector<AudioClip *> clips;
    };
}
