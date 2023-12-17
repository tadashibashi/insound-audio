#pragma once

#include <type_traits>

namespace Insound
{
    template <typename T> requires std::is_arithmetic_v<T>
    struct LoopInfo
    {
        T loopstart;
        T loopend;
    };
}
