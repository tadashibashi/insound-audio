#include "MultiTrackAudio.h"
#include "common.h"

#include <fmod.hpp>
#include <cstdlib>
#include <vector>

namespace Insound {
    struct MultiTrackAudio::Impl {
    public:
        Impl(std::string_view name, FMOD::System *sys) : group(), fsb()
        {
            FMOD::SoundGroup *group;
            checkResult(sys->createSoundGroup(name.data(), &group));
            this->group = group;
        }

        ~Impl()
        {
            group->release();
        }

        FMOD::SoundGroup *group;
        FMOD::Sound *fsb;

    };

    void MultiTrackAudio::unloadFsb()
    {
        if (!m->fsb) return;

        int count;
        checkResult( m->fsb->getNumSubSounds(&count) );

        for (int i = 0; i < count; ++i)
        {
            FMOD::Sound *subsound;
            checkResult( m->fsb->getSubSound(i, &subsound) );
            subsound->setSoundGroup(nullptr);
        }

        m->fsb->release();
        m->fsb = nullptr;
    }

    void MultiTrackAudio::loadFsb(const char *data, size_t bytelength)
    {
        // Unload any previously loaded fsbank file
        unloadFsb();

        auto exinfo = FMOD_CREATESOUNDEXINFO();
        std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = bytelength;

        FMOD::System *sys;
        checkResult( m->group->getSystemObject(&sys) );

        FMOD::Sound *snd;
        checkResult( sys->createSound((const char *)data, FMOD_DEFAULT, &exinfo, &snd) );

        int count;
        checkResult( snd->getNumSubSounds(&count) );

        for (int i = 0; i < count; ++i)
        {
            FMOD::Sound *subsound;
            checkResult( snd->getSubSound(i, &subsound) );
            checkResult( subsound->setSoundGroup(m->group) );
        }

        m->fsb = snd;
    }
}
