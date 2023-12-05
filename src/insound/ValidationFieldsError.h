#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>

namespace Insound
{
    class ValidationFieldsError : public std::exception
    {
    public:
        ValidationFieldsError() : m_errs() { }

        const std::map< std::string, std::vector<std::string> > &getErrors()
        {
            return m_errs;
        }

        void emplace(const std::string &key, const std::string &message)
        {
            m_errs[key].emplace_back(message);
        }

        [[nodiscard]]
        size_t errorCount() const
        {
            if (m_errs.empty()) return 0;

            size_t count = 0;
            for (auto &field : m_errs)
            {
                count += field.second.size();
            }

            return count;
        }

        [[nodiscard]]
        auto begin() { return m_errs.begin(); }
        [[nodiscard]]
        auto begin() const { return m_errs.begin(); }
        [[nodiscard]]
        auto end() { return m_errs.end(); }
        [[nodiscard]]
        auto end() const { return m_errs.end(); }

        // Get number of fields
        [[nodiscard]]
        auto size() { return m_errs.size(); }

        // Whether number of fields is empty
        [[nodiscard]]
        auto empty() { return m_errs.empty(); }

        // Whether there are no errors
        [[nodiscard]]
        bool valid() const { return m_errs.empty(); }
    private:
        std::map< std::string, std::vector<std::string> > m_errs;
    };
}
