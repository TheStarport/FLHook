#pragma once

#include "AbstractExternalCommandProcessor.hpp"

class ExternalCommandProcessor final : public AbstractExternalCommandProcessor, public Singleton<ExternalCommandProcessor>
{
        // clang-format off
        std::unordered_map<std::wstring, std::function<nlohmann::json(const nlohmann::json&)>> functions =
        {
            {
               std::wstring(L"beam"), Beam
            }
        };
        // clang-format on

        static bsoncxx::document::view Beam( bsoncxx::document::view parameters);

    public:
        std::shared_ptr<BsonWrapper> ProcessCommand(bsoncxx::document::view command) override;
        std::vector<std::wstring_view> GetCommands() override;
};
