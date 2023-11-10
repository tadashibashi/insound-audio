#pragma once
#include <fmod_errors.h>

namespace Insound {
    /**
     * Check FMOD result - throw if an error occurred.
     */
    void checkResult(FMOD_RESULT result);
}
