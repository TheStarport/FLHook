#pragma once

class AbstractExternalCommandProcessor
{
    public:
        virtual ~AbstractExternalCommandProcessor() = default;
        virtual std::optional<nlohmann::json> ProcessCommand(const nlohmann::json& command) = 0;
        virtual std::vector<std::wstring_view> GetCommands() = 0;
};
