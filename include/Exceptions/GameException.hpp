#pragma once

#ifdef SERVER
    #include "ErrorInfo.hpp"
#endif

#include "UnsupportedException.hpp"

class DLL GameException : std::exception
{
        std::wstring msg;
        std::wstring trace;

    public:
#ifdef SERVER
        explicit GameException(const std::wstring& msg, const Error err);
#endif

        explicit GameException(const std::wstring& msg);
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
        [[nodiscard]]
        std::wstring_view Msg() const
        {
            return msg;
        }

        [[nodiscard]]
        std::wstring_view Trace() const
        {
            return trace;
        }
};
