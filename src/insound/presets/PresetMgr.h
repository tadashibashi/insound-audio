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

        /**
         * Get index of a preset via name, or -1 if none exists.
         * @param  name - name of the preset to find
         * @return zero-based index, or -1 if it wasn't found.
         */
        [[nodiscard]]
        int indexOf(std::string_view name) const
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

        auto emplace_back(const std::string &name,
            const std::vector<float> &values)
        {
            m_presets.emplace_back(name, values);
        }

        auto insert(const std::string &name,
            const std::vector<float> &values, size_t position)
        {
            m_presets.insert(m_presets.begin() + position,
                Preset{name, values});
        }
    private:
        std::vector<Preset> m_presets;
    };
}
