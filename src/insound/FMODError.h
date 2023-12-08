#pragma once
#include <stdexcept>

namespace Insound {
    class FMODError : public std::runtime_error {
    public:
        explicit FMODError(int code);

        /**
         * FMOD Error code, directly corresponds with the FMOD_RESULT enum
         */
        int code;

        /**
         * The error message in a human-readable form
         */
        const char *message;
    };
}
