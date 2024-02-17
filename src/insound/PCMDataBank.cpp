#include "PCMDataBank.h"
#include "Int24.h"
#include "common.h"

#include <stdexcept>

namespace Insound::Audio
{
    FMOD_RESULT F_CALL PCMDataBank::callback(FMOD_SOUND *pSnd, void *data, unsigned int datalen)
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
        PCMDataBank::data.emplace(sound, res);
        return FMOD_OK;
    }

    const std::vector<float> &PCMDataBank::get(FMOD::Sound *sound)
    {
        auto it = data.find(sound);
        if (it == data.end())
        {
            throw std::runtime_error("PCM data does not exist for sound");
        }

        return it->second;
    }

    bool PCMDataBank::contains(FMOD::Sound *sound)
    {
        return data.contains(sound);
    }

    bool PCMDataBank::unload(FMOD::Sound *sound)
    {
        auto it = data.find(sound);
        if (it != data.end())
        {
            data.erase(it);
            return true;
        }

        return false;
    }

    void PCMDataBank::clear()
    {
        data.clear();
    }

    std::map<FMOD::Sound *, std::vector<float>> PCMDataBank::data{};


}
