#pragma once
#include <string>
#include <vector>

namespace Insound
{
    struct Preset
    {
        std::string name;
        std::vector<float> volumes;
    };
}
