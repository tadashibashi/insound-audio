#pragma once

#include <functional>
#include <string>
#include <string_view>
#include <vector>

namespace Insound
{
    class Parameter
    {
    public:
        Parameter(const std::string &name, float value = 0) :
            name(name), value(value) { }

        /**
         * Return value as an integer
         */
        [[nodiscard]]
        int getInt() const { return static_cast<int>(value); }

        /**
         * Return value
         */
        [[nodiscard]]
        float getFloat() const { return value; }

        void setValue(float value)
        {
            this->value = value;
        }

        const std::string &getName() const { return name; }


    private:
        std::string name;
        float value;
    };

    class ParameterMgr
    {
    public:
        using Callback =
            std::function<void(const std::string &, size_t, float)>;

        ParameterMgr() = default;

        [[nodiscard]]
        size_t size() const { return m_params.size(); }

        Parameter &emplace_back(const std::string &name, float value=0)
        {
            return m_params.emplace_back(name, value);
        }

        void clear() { m_params.clear(); }
        void reserve(size_t i) { m_params.reserve(i); }

        void set(size_t i, float value);
        void set(std::string_view name, float value);

        [[nodiscard]]
        float get(size_t i) const;
        [[nodiscard]]
        float get(std::string_view) const;

        void setCallback(Callback callback);

        [[nodiscard]]
        const Callback &getCallback() const;
    private:
        [[nodiscard]]
        Parameter &operator[](size_t i) { return m_params.at(i); }
        [[nodiscard]]
        const Parameter &operator[](size_t i) const { return m_params.at(i); }

        [[nodiscard]]
        Parameter &operator[](std::string_view name);
        [[nodiscard]]
        const Parameter &operator[](std::string_view name) const;
    private:
        std::vector<Parameter> m_params;
        Callback m_callback;
    };
}
