#include "MultiTrackAudio.h"
#include <fmod.hpp>
#include <vector>

namespace Insound {
    struct MultiTrackAudio::Impl {
        FMOD::SoundGroup *group;
        std::vector<FMOD::Sound *> sounds;
    };
}
