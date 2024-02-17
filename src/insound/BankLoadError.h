#pragma once
#include <stdexcept>
#include <string_view>

namespace Insound::Audio
{
    /**
     * Contains a client message indicating sound-loading error
     */
    class BankLoadError : public std::runtime_error
    {
    public:
        BankLoadError(std::string_view message) : std::runtime_error(message.data())
        {

        }
    };
}
