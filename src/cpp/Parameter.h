#pragma once

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Insound
{


    class Parameter
    {
    public:
        struct Label
        {
            Label(const std::string &name, float value): name(name),
                value(value) { }

            const std::string name;
            const float value;
        };

        struct LabelMgr
        {
            auto begin() { return labels.begin(); }
            auto begin() const { return labels.begin(); }
            auto end() { return labels.end(); }
            auto end() const { return labels.end(); }
            auto find(std::string_view name)
            {
                for (auto it = labels.begin(); it != labels.end(); ++it)
                    if (it->name == name) return it;
                throw std::out_of_range("No label \"" + std::string(name) +
                    "\" in LabelMgr");
            }

            Label &operator[](size_t i) { return labels.at(i); }
            Label &operator[](std::string_view name)
            {
                for (auto &l : labels)
                    if (l.name == name)
                        return l;
                throw std::out_of_range("No label \"" + std::string(name) +
                    "\" in LabelMgr");
            }

            LabelMgr &add(std::string_view name, float value)
            {
                labels.emplace_back(name.data(), value);
                return *this;
            }

            size_t size() { return labels.size(); }

        private:
            std::vector<Label> labels;
        };

        Parameter(std::string_view name, float value = 0) :
            name(name), value(value), initValue(value) { }

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

        [[nodiscard]]
        float getInitValue() const { return initValue; }

        // Reset value to init value
        void reset()
        {
            this->value = this->initValue;
        }

        void setValue(float value)
        {
            this->value = value;
        }

        void setValue(std::string_view label)
        {
            value = m_labels[label].value;
        }

        [[nodiscard]]
        const std::string &getName() const { return name; }

        auto &labels() { return m_labels; }
        auto &labels() const { return m_labels; }

    private:
        std::string name;
        float value;
        const float initValue;
        LabelMgr m_labels;
    };

    class ParameterMgr
    {
    public:
        using iterator = std::vector<Parameter>::iterator;
        using const_iterator = std::vector<Parameter>::const_iterator;
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

        void set(size_t i, std::string_view label);
        void set(std::string_view name, std::string_view label);

        [[nodiscard]]
        float get(size_t i) const;
        [[nodiscard]]
        float get(std::string_view name) const;

        [[nodiscard]]
        float getInitValue(size_t i) const;
        [[nodiscard]]
        float getInitValue(std::string_view name) const;

        void reset(size_t i);
        void reset(std::string_view name);

        void setCallback(Callback callback);

        // Set all parameters to init values. Fires set callbacks.
        void resetAll();

        [[nodiscard]]
        const Callback &getCallback() const;

        [[nodiscard]]
        auto &getLabels(size_t i) { return m_params[i].labels(); }
        auto &getLabels(std::string_view paramName)
        { return operator[](paramName).labels(); }

        auto &getLabels(size_t i) const { return m_params[i].labels(); }
        auto &getLabels(std::string_view paramName) const
        { return operator[](paramName).labels(); }

        auto begin() { return m_params.begin(); }
        auto end() { return m_params.end(); }
        auto begin() const { return m_params.begin(); }
        auto end() const { return m_params.end(); }

        [[nodiscard]]
        const Parameter &operator[](std::string_view name) const;
        [[nodiscard]]
        const Parameter &operator[](size_t i) const { return m_params.at(i); }
    private:
        [[nodiscard]]
        Parameter &operator[](size_t i) { return m_params.at(i); }

        [[nodiscard]]
        Parameter &operator[](std::string_view name);

        int findParam(std::string_view name) const;
    private:
        std::vector<Parameter> m_params;
        Callback m_callback;
    };
}
