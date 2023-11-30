#pragma once

#include "ParamDesc.h"

#include <string>
#include <vector>

namespace Insound
{
    /**
     * A container and manager for a list of parameter descriptions
     */
    class ParamDescMgr
    {
    public:
        ParamDescMgr() : m_params() { }

        /**
         * Add an integer parameter to this manager
         *
         * @param name - parameter name
         * @param min  - minimum value (inclusive)
         * @param max  - maximum value (inclusive)
         * @param def  - default value
         */
        void addInt(const std::string &name, int min, int max, int def)
        {
            m_params.emplace_back(name, min, max, def);
        }

        /**
         * Add a float parameter to this manager
         *
         * @param name - parameter name
         * @param min  - minimum value (inclusive)
         * @param max  - maximum value (inclusive)
         * @param step - incremental interval between values - how
         *               fine-grained the slider will be
         * @param def  - default value
         */
        void addFloat(const std::string &name, float min, float max,
            float step, float def)
        {
            m_params.emplace_back(name, min, max, step, def);
        }

        /**
         * Add an enumerated parameter to this manager (dropdown menu of
         * strings)
         *
         * @param name    - parameter name
         * @param strings - list of strings declaring enumerated value names
         */
        void addStrings(const std::string &name, const std::vector<std::string> &strings)
        {
            m_params.emplace_back(name, strings);
        }

        /**
         * Get parameter at index, throws if out of range.
         */
        [[nodiscard]]
        const ParamDesc &operator[](size_t i) const
        {
            return m_params.at(i);
        }

        /**
         * Get parameter by name, throws runtime_error if it doesn't exist.
         */
        [[nodiscard]]
        const ParamDesc &operator[](const std::string &name) const
        {
            for (auto &p : m_params)
                if (p.getName() == name)
                    return p;
            throw std::runtime_error("Parameter with name: \"" + name + "\" "
                "does not exist in this container.");
        }

        /**
         * Get the number of parameters in the container
         */
        [[nodiscard]]
        auto size() const { return m_params.size(); }

        /**
         * Check if no params exist in the container
         */
        [[nodiscard]]
        bool empty() const { return m_params.empty(); }

        /**
         * Delete all parameters in the container
         */
        void clear() { m_params.clear(); }

    private:
        std::vector<ParamDesc> m_params;
    };
}
