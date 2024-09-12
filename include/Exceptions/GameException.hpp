#pragma once

#ifdef SERVER
    #include "ErrorInfo.hpp"
#endif

#include "UnsupportedException.hpp"

class DLL GameException : std::exception
{
        std::wstring msg;

    public:
#ifdef SERVER
        explicit GameException(std::wstring msg, const Error err) : msg(std::format(L"{}\n{}", std::move(msg), ErrorInfo::GetText(err))) {}
#endif

        explicit GameException(std::wstring msg) : msg(std::move(msg)) {}
        ~GameException() noexcept override = default;

        /**
         * @deprecated Using the const char* what() on custom exceptions is not supported.
         */
        [[nodiscard]]
        const char* what() const noexcept override
        {
            throw UnsupportedException();
        }

        /**
         * @brief Retrieve the underlying error message from the exception
         */
        [[nodiscard]] std::wstring_view Msg() const { return msg; }
};
