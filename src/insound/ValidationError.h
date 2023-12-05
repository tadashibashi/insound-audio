#pragma once

#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace Insound
{
    class ValidationError : public std::exception
    {
    public:
        ValidationError() : m_errs() { }

        const std::vector<std::string> &getErrors()
        {
            return m_errs;
        }

        void emplace(const std::string &message)
        {
            m_errs.emplace_back(message);
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

        const char * what() const  noexcept override
        {
            try {
                if (m_errs.empty()) return "";

                if (m_msg.empty())
                {
                    std::string errs;
                    for (auto &m : m_errs)
                        errs.append(m + '\n');
                    std::swap(m_msg, errs);
                }

                return m_msg.c_str();
            }
            catch(...)
            {
                return "";
            }
        }
    private:
        std::vector<std::string> m_errs;
        mutable std::string m_msg;
    };
}
