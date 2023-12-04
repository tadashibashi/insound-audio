#include "common.h"
#include <stdexcept>
#include <string>

namespace Insound {
    void checkResult(FMOD_RESULT result)
    {
        if (result != FMOD_OK)
            throw std::runtime_error(FMOD_ErrorString(result));
    }

    bool compareCaseInsensitive(std::string_view a, std::string_view b)
    {
        const auto size = a.size();
        if (size != b.size()) return false;

        for (auto aIt = a.begin(), bIt = b.begin(); aIt != a.end(); ++aIt, ++bIt)
        {
            if (std::tolower(*aIt) != std::tolower(*bIt))
                return false;
        }

        return true;
    }
}
