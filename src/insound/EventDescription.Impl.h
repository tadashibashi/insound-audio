#pragma once
#include "EventDescription.h"

#include <insound/Int24.h>
#include <insound/common.h>
#include <insound/BankLoadError.h>
#include <fmod.hpp>
#include <fmod_errors.h>

#include <map>
#include <vector>

namespace Insound::Audio
{
    static std::map<FMOD::Sound *, std::vector<float>> pcmData{};

    /**
     * Reads PCM data from an opened FMOD::Sound and stores it in a global
     * storage container
     *
     * @param  pSnd    - sound pointer
     * @param  data    - bytes of pcm data
     * @param  datalen - length of pcm data in samples
     */
    static FMOD_RESULT F_CALL pcmReadCallback(FMOD_SOUND *pSnd, void *data,
        unsigned int datalen)
    {
        static_assert(sizeof(float) == 4, "Sizeof float must be 32-bit, 4 bytes");

        // Get callback objects
        auto sound = (FMOD::Sound *)pSnd;

        // Get audio format info
        FMOD_SOUND_TYPE type;
        FMOD_SOUND_FORMAT format;
        int bits;
        checkResult(sound->getFormat(&type, &format, nullptr, &bits));

        std::vector<float> res;

        // parse sample data based on format data
        switch(bits)
        {
        case 8:
            {
                res.reserve(datalen);
                for (size_t i = 0; i < datalen; ++i)
                {
                    auto val = *((uint8_t *)data + i);
                    res.push_back( (double)val / std::numeric_limits<uint8_t>::max() * 2.0 - 1.0);
                }
                break;
            }

        case 16:
            {
                auto len = datalen / 2;
                res.reserve(len);
                for (size_t i = 0; i < len; ++i)
                {
                    auto val = *((int16_t *)data + i);
                    res.push_back( (float)val / (std::numeric_limits<int16_t>::max()) );
                }
                break;
            }
        case 24:
            {
                auto len = datalen / 3;
                res.reserve(len);
                for (size_t i = 0; i < len; ++i)
                {
                    auto val = ((Int24 *)data + i)->value;
                    res.push_back( (float)val / 8'388'607.0 );
                }
                break;
            }
        case 32:
            {
                if (format == FMOD_SOUND_FORMAT_PCMFLOAT)
                {
                    auto len = datalen / 4;
                    res.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto val = *((float *)data + i);
                        res.push_back(val);
                    }
                }
                else
                {
                    auto len = datalen / 4;
                    res.reserve(len);
                    for (size_t i = 0; i < len; ++i)
                    {
                        auto val = *((int32_t *)data + i);
                        res.push_back( (double)val / std::numeric_limits<int32_t>::max());
                    }
                }

                break;
            }
        default:
            return FMOD_ERR_FORMAT;
        }

        // push to track pcm data
        pcmData.emplace(sound, res);
        return FMOD_OK;
    }

    class EventDescription::Impl
    {
    public:
        Impl(FMOD::System *sys) : fsb{}, sounds{}, sys{sys}
        {
            // check to make sure system is valid, will throw otherwise
            checkResult(sys->getVersion(nullptr));
        }

        [[nodiscard]]
        bool isFSB() const
        {
            return fsb;
        }

        [[nodiscard]]
        bool isLoaded() const
        {
            return !sounds.empty();
        }

        void loadFSB(const void *data, size_t size)
        {
            FMOD::Sound *newFSB;
            FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_LOOP_OFF;
            FMOD_CREATESOUNDEXINFO info{};
            info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            info.length = size;
            info.pcmreadcallback = pcmReadCallback; // not sure if this will work on an FSB, FMOD may have impelmented it

            auto result = sys->createSound((const char *)data, mode, &info, &newFSB);
            if (result != FMOD_OK)
            {
                throw BankLoadError(FMOD_ErrorString(result));
            }

            int subsoundCount;
            result = newFSB->getNumSubSounds(&subsoundCount);
            if (result != FMOD_OK)
            {
                newFSB->release();
                throw BankLoadError(FMOD_ErrorString(result));
            }

            std::vector<FMOD::Sound *> subsounds;

            for (int i = 0; i < subsoundCount; ++i)
            {
                FMOD::Sound *subsound;
                result = newFSB->getSubSound(i, &subsound);
                if (result != FMOD_OK)
                {
                    newFSB->release();
                    throw BankLoadError(FMOD_ErrorString(result));
                }

                subsounds.emplace_back(subsound);
            }

            unload();
            sounds.swap(subsounds);
            fsb = newFSB;
        }

        void loadSound(const void *data, size_t size)
        {
            if (fsb)
            {
                throw BankLoadError("FSBank already loaded, cannot call "
                    "`loadSound` until bank is unloaded");
            }

            FMOD::Sound *snd;
            FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_LOOP_OFF;
            FMOD_CREATESOUNDEXINFO info{};
            info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
            info.length = size;
            info.pcmreadcallback = pcmReadCallback;

            auto result = sys->createSound((const char *)data, mode, &info, &snd);
            if (result != FMOD_OK)
            {
                throw BankLoadError(FMOD_ErrorString(result));
            }

            sounds.emplace_back(snd);
        }

        /**
         * Release all sounds / memory
         */
        void unload()
        {
            if (fsb)
            {
                fsb->release();
                fsb = nullptr;

                // Just erase pcmData, fsb auto-releases all subsounds
                for (auto sound : sounds)
                {
                    pcmData.erase(sound);
                }

                sounds.clear();
            }
            else
            {
                for (auto sound : sounds)
                {
                    sound->release();
                    pcmData.erase(sound);
                }
                sounds.clear();
            }
        }

        FMOD::System *sys;
        FMOD::Sound *fsb;
        std::vector<FMOD::Sound *> sounds;
    };
}
