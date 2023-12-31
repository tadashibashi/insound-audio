#pragma once
#include <string>
#include <string_view>
#include <vector>

namespace Insound
{
    class StringsParam
    {
    public:
        StringsParam() : values(), m_default() { }
        StringsParam(std::vector<std::string> values, size_t defaultValue = 0)
            : values(std::move(values)), m_default(defaultValue) { }

        [[nodiscard]]
        auto begin() { return values.begin(); }
        [[nodiscard]]
        auto begin() const { return values.begin(); }
        [[nodiscard]]
        auto end() { return values.end(); }
        [[nodiscard]]
        auto end() const { return values.end(); }

        [[nodiscard]]
        int operator[](std::string_view value) const { return at(value); }
        [[nodiscard]]
        int at(std::string_view value) const
        {
            int i = 0;
            for (auto &str : values)
            {
                if (str == value)
                    return i;
                ++i;
            }
            throw std::runtime_error("Value \"" + std::string(value) + "\" "
                "does not exist in paremeter.");
        }

        [[nodiscard]]
        std::string_view operator[](size_t index) const { return at(index); }
        [[nodiscard]]
        std::string_view at(size_t index) const { return values.at(index); }

        [[nodiscard]]
        auto size() const { return values.size(); }

        std::string &emplace_back(std::string_view value)
        { return values.emplace_back(value); }

        size_t defaultValue() const { return m_default; }
    public:
        std::vector<std::string> values;
        size_t m_default;
    };
}
