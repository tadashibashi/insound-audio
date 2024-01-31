#pragma once

#include <stdexcept>
#include <string>

#include <sol/error.hpp>

namespace Insound
{
    class LuaDriver;

    class LuaError : public sol::error
    {
    public:
        LuaError(LuaDriver *driver, std::string message);

        [[nodiscard]]
        const char *what() const noexcept override { return m_message.c_str(); }

        [[nodiscard]]
        int lineNumber() const { return m_lineNumber; }

    private:
        int m_lineNumber;
        std::string m_message;
    };
}
