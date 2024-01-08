#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct Int24
{
    int32_t value : 24;
};

#pragma pack(pop)

static_assert(sizeof(Int24) == 3, "Size of Int24 must be 3");
