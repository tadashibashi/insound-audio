#include "FMODError.h"
#include <fmod_errors.h>
#include <format>

namespace Insound {
    FMODError::FMODError(int code) :
        std::runtime_error(std::format("FMOD Error code {}: \"{}\"",
            code, FMOD_ErrorString((FMOD_RESULT)code))), code(code)
    {

    }
}
