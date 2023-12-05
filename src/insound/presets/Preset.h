#pragma once
#include <string>
#include <vector>

namespace Insound
{
    struct Preset
    {
        Preset(): name(), volumes() { }
        Preset(const std::string &name, const std::vector<float> &volumes) :
            name(name), volumes(volumes) { }

        std::string name;
        std::vector<float> volumes;
    };
}
