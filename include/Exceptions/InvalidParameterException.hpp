#pragma once

#include "GameException.hpp"

class InvalidParameterException final : public GameException
{
    public:
        InvalidParameterException(std::wstring_view providedValue, size_t paramNumber, std::wstring_view expected)
            : GameException(std::format(L"A specified parameter was invalid.\nParam: {}.\nProvided Value: {}.\nExpected: {}", paramNumber, providedValue, expected)){};
};
