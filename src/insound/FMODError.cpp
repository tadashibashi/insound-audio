#include "FMODError.h"

#include <fmod_errors.h>

#include <string>

namespace Insound {
    FMODError::FMODError(int code) :
        std::runtime_error("FMOD Error code " + std::to_string(code) + ": " +
            std::string(FMOD_ErrorString((FMOD_RESULT)code))),
        code(code),
        errorMessage(FMOD_ErrorString((FMOD_RESULT)code))
    {

    }
}
