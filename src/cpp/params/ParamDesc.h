#pragma once

#include "types/NumberParam.h"
#include "types/StringsParam.h"

#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace Insound
{
    /**
     * Parameter Template. Actual parameter implemented in JavaScript.
     */
    class ParamDesc
    {
    public:
        ParamDesc(const std::string &name, int min, int max, int defaultValue)
            : name(name), type(Type::Integer),
            param{NumberParam(min, max, defaultValue)} { }

        ParamDesc(const std::string &name, float min, float max, float step,
            float value) : name(name), type(Type::Float), param{
            NumberParam(min, max, step, value)} { }

        ParamDesc(const std::string &name,
            const std::vector<std::string> &values, size_t defaultValue=0) :
            name(name), type(Type::Strings),
            param{StringsParam(values, defaultValue)} { }

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
}
