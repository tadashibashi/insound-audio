#pragma once

#include "fmod.hpp"

#include <map>

namespace Insound::Audio
{
    class PCMDataBank
    {
    public:
        /**
         * PCM data callback on FMOD::Sound creation
         */
        static FMOD_RESULT F_CALL callback(FMOD_SOUND *pSnd, void *data, unsigned int datalen);

        [[nodiscard]]
        static const std::vector<float> &get(FMOD::Sound *sound);
        [[nodiscard]]
        static bool contains(FMOD::Sound *sound);

        static void clear();

        /**
         * Unload PCM data for a sound entry
         * @param  sound - sound with pcm data to remove
         * @return       whether sound data was removed, it will return false
         *               if sound data doesn't exist in this container
         */
        static bool unload(FMOD::Sound *sound);
    private:
        static std::map<FMOD::Sound *, std::vector<float>> data;
    };
}
