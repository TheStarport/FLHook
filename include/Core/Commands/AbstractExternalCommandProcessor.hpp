#pragma once
#include "Defs/BsonWrapper.hpp"

class AbstractExternalCommandProcessor
{
    public:
        virtual ~AbstractExternalCommandProcessor() = default;
        virtual std::optional<BsonWrapper> ProcessCommand( command) = 0;
        virtual std::vector<std::wstring_view> GetCommands() = 0;
};
