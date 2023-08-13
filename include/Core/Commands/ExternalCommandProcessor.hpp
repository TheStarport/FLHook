#pragma once

class ExternalCommandProcessor
{
        // clang-format off
        std::unordered_map<std::wstring, std::function<nlohmann::json(const nlohmann::json&)>> functions =
        {
            {
               std::wstring(L"beam"), Beam
            },
            {

            }
        };
        // clang-format on

       static nlohmann::json Beam(const nlohmann::json& parameters);

    public:
        std::optional<nlohmann::json> ProcessCommand(const nlohmann::json& command);
};
