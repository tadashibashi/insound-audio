#include "PresetMgr.h"

namespace Insound
{
    const Preset &PresetMgr::operator[](size_t index) const
    {
        return m_presets[index];
    }

    Preset &PresetMgr::operator[](size_t index)
    {
        return m_presets[index];
    }


    const Preset &PresetMgr::operator[](std::string_view name) const
    {
        for (const auto &preset : m_presets)
            if (preset.name == name)
                return preset;
        throw std::runtime_error("Could not find Preset with name " +
            std::string(name));
    }


    Preset &PresetMgr::operator[](std::string_view name)
    {
        return const_cast<Preset &>(operator[](name));
    }


    int PresetMgr::indexOf(std::string_view name) const
    {
        int i = 0;
        for (const auto &preset : m_presets)
        {
            if (preset.name == name)
                return i;
            ++i;
        }

        return -1;
    }
}
