#include "common.h"
#include <stdexcept>

namespace Insound {
    void checkResult(FMOD_RESULT result)
    {
        if (result != FMOD_OK)
            throw std::runtime_error(FMOD_ErrorString(result));
    }
}
