#include "AudioClip.h"
#include "fmod_errors.h"
#include "insound/BankLoadError.h"
#include "insound/EventDescription.Impl.h"
#include "PCMDataBank.h"
#include "insound/common.h"

#include <fmod.hpp>

namespace Insound::Audio
{
    AudioClip::AudioClip(): sound()
    {

    }

    AudioClip::~AudioClip()
    {
        unload();
    }

    void AudioClip::unload()
    {
        if (sound)
        {
            sound->release();
            sound = nullptr;
        }
    }

    void AudioClip::emplace(FMOD::Sound *sound)
    {
        unload();
        checkResult(sound->setUserData(this));

        this->sound = sound;
    }

    void AudioClip::load(FMOD::System *sys, void *data, size_t byteLength)
    {
        FMOD::Sound *snd;
        FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_LOOP_OFF;
        FMOD_CREATESOUNDEXINFO info{};
        info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        info.length = byteLength;
        info.pcmreadcallback = PCMDataBank::callback;

        auto result = sys->createSound((const char *)data, mode, &info, &snd);
        if (result != FMOD_OK)
        {
            throw BankLoadError(FMOD_ErrorString(result));
        }

        unload();
        this->sound = snd;
    }

    std::vector<AudioClip *> AudioClip::loadFSB(FMOD::System *sys, void *data, size_t byteLength)
    {
        std::vector<AudioClip *> res;

        FMOD::Sound *fsb;
        FMOD_MODE mode = FMOD_OPENMEMORY | FMOD_CREATESAMPLE | FMOD_LOOP_OFF;
        FMOD_CREATESOUNDEXINFO info{};
        info.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        info.length = byteLength;
        info.pcmreadcallback = PCMDataBank::callback; // not sure if this will work on an FSB, FMOD may have impelmented it

        auto result = sys->createSound((const char *)data, mode, &info, &fsb);
        if (result != FMOD_OK)
        {
            throw BankLoadError(FMOD_ErrorString(result));
        }

        int subsoundCount;
        result = fsb->getNumSubSounds(&subsoundCount);
        if (result != FMOD_OK)
        {
            fsb->release();
            throw BankLoadError(FMOD_ErrorString(result));
        }

        std::vector<AudioClip *> clips;

        for (int i = 0; i < subsoundCount; ++i)
        {
            FMOD::Sound *subsound;
            result = fsb->getSubSound(i, &subsound);
            if (result != FMOD_OK)
            {
                fsb->release();
                throw BankLoadError(FMOD_ErrorString(result));
            }

            auto clip = new AudioClip();
            clip->emplace(subsound);

            clips.emplace_back(clip);
        }

        return clips;
    }

    bool AudioClip::isLoaded() const
    {
        return static_cast<bool>(this->sound);
    }

    bool AudioClip::isSubSound() const
    {
        FMOD::Sound *parent{};
        sound && sound->getSubSoundParent(&parent);

        return static_cast<bool>(parent);
    }
}
