#pragma once

#include <cstddef>
#include <cstdint>

namespace Insound
{
    struct SampleDataInfo
    {
        uintptr_t ptr;
        size_t byteLength;
    };
}
