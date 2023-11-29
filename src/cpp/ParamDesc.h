#pragma once

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Insound
{
    class StringsParam
    {
    public:
        StringsParam() : values() { }
        StringsParam(std::vector<std::string> values)
            : values(std::move(values)) { }

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
    public:
        std::vector<std::string> values;
    };

    class NumberParam
    {
    public:
        NumberParam() : min(), max(), step(), value() { }
        NumberParam(int min, int max, int def) :
            min(min), max(max), step(1), value(def) { }
        NumberParam(float min, float max, float step,
            float def) : min(min), max(max), step(step), value(def) { }

        float min, max, step, value;
    };

    /**
     * Parameter Template. Actual parameter implemented in JavaScript.
     */
    class ParamDesc
    {
    public:
        ParamDesc(const std::string &name, int min, int max, int value) :
            name(name), type(Type::Integer),
            param{NumberParam(min, max, value)} { }

        ParamDesc(const std::string &name, float min, float max, float step,
            float value) : name(name), type(Type::Float), param{
            NumberParam(min, max, step, value)} { }

        ParamDesc(const std::string &name,
            const std::vector<std::string> &values) :
            name(name), type(Type::Strings),
            param{StringsParam(values)} { }

        ParamDesc(ParamDesc &&other) : type(other.type), param(other.param)
        { }

        enum class Type
        {
            Integer,
            Float,
            Strings,
        };

        [[nodiscard]]
        const NumberParam &getNumber() const
        {
            return std::get<NumberParam>(param);
        }

        [[nodiscard]]
        const StringsParam &getStrings() const
        {
            return std::get<StringsParam>(param);
        }

        [[nodiscard]]
        Type getType() const { return type; }

        const std::string &getName() const { return name; }

    public:
        std::string name;
        Type type;
        std::variant<NumberParam, StringsParam> param;
    };

    class ParamDescMgr
    {
    public:
        ParamDescMgr() : m_params() { }

        void addInt(const std::string &name, int min, int max, int def)
        {
            m_params.emplace_back(name, min, max, def);
        }

        void addFloat(const std::string &name, float min, float max, float step, float def)
        {
            m_params.emplace_back(name, min, max, step, def);
        }

        void addStrings(const std::string &name, const std::vector<std::string> &strings)
        {
            m_params.emplace_back(name, strings);
        }

        [[nodiscard]]
        const ParamDesc &operator[](size_t i) const
        {
            return m_params.at(i);
        }

        [[nodiscard]]
        const ParamDesc &operator[](const std::string &name) const
        {
            for (auto &p : m_params)
                if (p.getName() == name)
                    return p;
            throw std::runtime_error("Parameter with name: \"" + name + "\" "
                "does not exist in this container.");
        }

        [[nodiscard]]
        auto size() const { return m_params.size(); }

        [[nodiscard]]
        bool empty() const { return m_params.empty(); }

        void clear() { m_params.clear(); }

    private:
        std::vector<ParamDesc> m_params;
    };
}
