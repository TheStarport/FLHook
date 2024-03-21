#pragma once

#include <exception>

class FlException : public std::exception
{
        std::wstring wStr;
        std::string str;

    protected:
        explicit FlException(const std::wstring& msg)
        {
            wStr = msg;
            str = StringUtils::wstos(msg);
        }

    public:
        [[nodiscard]]
        const char* what() const override
        {
            return str.data();
        }

        virtual std::wstring_view Message() { return wStr; }
};
