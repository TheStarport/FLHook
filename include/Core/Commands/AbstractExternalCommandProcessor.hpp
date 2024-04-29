#pragma once
#include "Defs/BsonWrapper.hpp"

class AbstractExternalCommandProcessor
{
    public:
        virtual ~AbstractExternalCommandProcessor() = default;
        virtual std::shared_ptr<BsonWrapper> ProcessCommand(bsoncxx::document::view) = 0;
        virtual std::vector<std::wstring_view> GetCommands() = 0;
};
