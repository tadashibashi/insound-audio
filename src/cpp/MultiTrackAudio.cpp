#include "MultiTrackAudio.h"
#include "common.h"
#include "fmod_common.h"

#include <fmod.hpp>
#include <cstdlib>
#include <vector>

namespace Insound {
    struct MultiTrackAudio::Impl {
    public:
        Impl(std::string_view name, FMOD::System *sys) : group(), fsb()
        {
            FMOD::ChannelGroup *group;
            checkResult(sys->createChannelGroup(name.data(), &group));
            this->group = group;
        }

        ~Impl()
        {
            if (fsb)
                fsb->release();
            group->release();
        }

        FMOD::ChannelGroup *group;
        FMOD::Sound *fsb;

    };

    MultiTrackAudio::MultiTrackAudio(std::string_view name, FMOD::System *sys)
        : m(new Impl(name, sys))
    {

    }

    MultiTrackAudio::~MultiTrackAudio()
    {
        delete m;
    }

    void MultiTrackAudio::play()
    {
        FMOD::System *sys;
        checkResult( m->group->getSystemObject(&sys) );

        int numSounds;
        checkResult( m->fsb->getNumSubSounds(&numSounds) );

        m->group->stop();
        std::vector<FMOD::Channel *> chans;
        for (int i = 0; i < numSounds; ++i)
        {
            FMOD::Sound *sound;
            checkResult( m->fsb->getSubSound(i, &sound) );

            FMOD::Channel *chan;

            checkResult( sys->playSound(sound, m->group, true, &chan) );
            chans.emplace_back(chan);
        }

        for (int i = 0; i < numSounds; ++i)
            checkResult(chans[i]->setPaused(false));
    }

    void MultiTrackAudio::setPause(bool pause)
    {
        FMOD::System *sys;
        checkResult( m->group->getSystemObject(&sys) );
        FMOD::ChannelGroup *chan;
        checkResult( sys->getMasterChannelGroup(&chan) );

        checkResult( chan->setPaused(pause) );
    }

    void MultiTrackAudio::unloadFsb()
    {
        // Check if already unloaded
        if (!isLoaded()) return;

        // Release bank
        m->fsb->release();
        m->fsb = nullptr;
    }

    bool MultiTrackAudio::isLoaded() const
    {
        return static_cast<bool>(m->fsb);
    }

    void MultiTrackAudio::loadFsb(const char *data, size_t bytelength)
    {
        auto exinfo{FMOD_CREATESOUNDEXINFO()};

        std::memset(&exinfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
        exinfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
        exinfo.length = bytelength;

        FMOD::System *sys;
        checkResult( m->group->getSystemObject(&sys) );

        FMOD::Sound *snd;
        checkResult( sys->createSound(data,
            FMOD_OPENMEMORY_POINT | FMOD_LOOP_NORMAL |
            FMOD_CREATECOMPRESSEDSAMPLE, &exinfo, &snd) );

        unloadFsb();
        m->fsb = snd;
    }
}
