#pragma once
#include <fmod_errors.h>
#include <string_view>

namespace Insound {
    /**
     * Check FMOD result - throw if an error occurred.
     */
    void checkResult(FMOD_RESULT result);

    [[nodiscard]]
    bool compareCaseInsensitive(std::string_view a, std::string_view b);
}
