#include "AudioPool.h"

namespace Insound::Audio
{
    AudioPool::AudioPool(System *sys) : sys(sys), clips()
    {
    }

    AudioClip *AudioPool::loadSound(void *data, size_t byteLength)
    {
        auto clip = new AudioClip();
        try {
            clip->load(sys->handle(), data, byteLength);
        }
        catch(...)
        {
            delete clip;
            throw;
        }

        clips.emplace_back(clip);
        return clip;
    }

    AudioClip *AudioPool::loadFSB(void *data, size_t byteLength)
    {
        auto clip = new AudioClip();
        try {
            clip->load(sys->handle(), data, byteLength);
        }
        catch(...)
        {
            delete clip;
            throw;
        }

        clips.emplace_back(clip);
        return clip;
    }

    bool AudioPool::unloadClip(AudioClip *clip)
    {
        for(auto it = clips.begin(); it != clips.end(); ++it)
        {
            if (*it == clip)
            {
                clips.erase(it);
                (*it)->unload();
                delete *it;
                return true;
            }
        }

        return false;

    }
}

