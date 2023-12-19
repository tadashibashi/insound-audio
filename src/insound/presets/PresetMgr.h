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
        const Preset &operator[](size_t index) const;

        [[nodiscard]]
        Preset &operator[](size_t index);

        [[nodiscard]]
        const Preset &operator[](std::string_view name) const;

        [[nodiscard]]
        Preset &operator[](std::string_view name);

        /**
         * Get index of a preset via name, or -1 if none exists.
         * @param  name - name of the preset to find
         * @return zero-based index, or -1 if it wasn't found.
         */
        [[nodiscard]]
        int indexOf(std::string_view name) const;

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
            const std::vector<double> &values)
        {
            m_presets.emplace_back(name, values);
        }

        auto insert(const std::string &name,
            const std::vector<double> &values, size_t position)
        {
            m_presets.insert(m_presets.begin() + position,
                Preset{name, values});
        }
    private:
        std::vector<Preset> m_presets;
    };
}
