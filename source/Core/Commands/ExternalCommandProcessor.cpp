#include "PCH.hpp"

#include "Core/Commands/ExternalCommandProcessor.hpp"
/*
std::optional<nlohmann::json> ExternalCommandProcessor::ProcessCommand(const nlohmann::json& command)
{
    try
    {
        std::wstring functionName = command.at("command").get<std::wstring>();

        auto func = functions.find(functionName);
        if (func == functions.end())
        {
            return std::nullopt;
        }

        return func->second(command);
    }
    catch (nlohmann::json::exception& ex)
    {
        FLHook::GetLogger().Log(LogLevel::Warn,
                                std::format(L"Exception was thrown while trying to process an external command. {}", StringUtils::stows(ex.what())));
    }

    return std::nullopt;
}

std::vector<std::wstring_view> ExternalCommandProcessor::GetCommands()
{
    std::vector<std::wstring_view> commands;
    commands.reserve(functions.size());
    for (auto i : functions)
    {
        commands.emplace_back(i.first);
    }

    return commands;
}

nlohmann::json ExternalCommandProcessor::Beam(const nlohmann::json& parameters)
{
    const auto client = Hk::Client::GetClientIdFromCharName(parameters.at("characterName").get<std::wstring>()).Handle();
    Hk::Player::Beam(client, parameters.at("baseName").get<std::wstring>());

    return {};
}
*/