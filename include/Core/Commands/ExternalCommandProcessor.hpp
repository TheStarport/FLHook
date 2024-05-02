#pragma once

#include "AbstractExternalCommandProcessor.hpp"

class ExternalCommandProcessor final : public Singleton<ExternalCommandProcessor>
{
        // clang-format off
        std::unordered_map<std::string, std::function<std::pair<bool, std::shared_ptr<BsonWrapper>>(bsoncxx::document::view)>,
            StringHash, std::equal_to<>> functions =
        {
            {
               std::string("beam"), Beam
            }
        };
        // clang-format on

        static std::pair<bool, std::shared_ptr<BsonWrapper>> Beam(bsoncxx::document::view parameters);

    public:
        std::pair<bool, std::shared_ptr<BsonWrapper>> ProcessCommand(bsoncxx::document::view document);
        std::vector<std::wstring> GetCommands() const;
};
