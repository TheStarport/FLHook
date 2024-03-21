#pragma once

#include "FlException.hpp"

class InvalidClientException final : public FlException
{
    public:
        explicit InvalidClientException(const ClientId client) : FlException(std::format(L"Invalid client ID was provided: {}", client.GetValue()))
        {}
};