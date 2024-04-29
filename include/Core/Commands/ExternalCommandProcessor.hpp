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

        static nlohmann::json Beam(const nlohmann::json& parameters);

    public:
        std::optional<nlohmann::json> ProcessCommand(const nlohmann::json& command) override;
        std::vector<std::wstring_view> GetCommands() override;
};
