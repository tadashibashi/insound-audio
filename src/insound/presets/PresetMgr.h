#pragma once
#include "Preset.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Insound
{
    class PresetMgr
    {
    public:
        PresetMgr() : m_presets() { }

        [[nodiscard]]
        const Preset &operator[](size_t index) const
        {
            return m_presets[index];
        }

        [[nodiscard]]
        Preset &operator[](size_t index)
        {
            return m_presets[index];
        }

        [[nodiscard]]
        const Preset &operator[](std::string_view name) const
        {
            for (const auto &preset : m_presets)
                if (preset.name == name)
                    return preset;
            throw std::runtime_error("Could not find Preset with name " +
                std::string(name));
        }

        [[nodiscard]]
        Preset &operator[](std::string_view name)
        {
            return const_cast<Preset &>(operator[](name));
        }

        [[nodiscard]]
        auto size() const
        {
            return m_presets.size();
        }

        [[nodiscard]]
        auto empty() const
        {
            return m_presets.empty();
        }

        void clear()
        {
            m_presets.clear();
        }
    private:
        std::vector<Preset> m_presets;
    };
}
