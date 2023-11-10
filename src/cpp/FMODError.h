#pragma once
#include <stdexcept>

namespace Insound {
    class FMODError : public std::runtime_error {
    public:
        explicit FMODError(int code);

    private:
        int code;
    };
}
