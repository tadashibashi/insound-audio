#pragma once
#include <stdexcept>

namespace Insound
{
    class SoundLengthMismatch : std::runtime_error
    {
    public:
        SoundLengthMismatch() : std::runtime_error("Sound lengths do not match.")
        {

        }
    };
}
