#include "PCH.hpp"
#include "Core/Commands/ExternalCommandProcessor.hpp"
#include "API/API.hpp"



std::optional<nlohmann::json> ExternalCommandProcessor::ProcessCommand(const nlohmann::json& command)
{
    try
    {
        std::wstring functionName = command.at("command").get<std::wstring>();

        return functions[functionName](command);
    }
    catch (nlohmann::json::exception& ex)
    {
        Logger::i()->Log(LogLevel::Warn, std::format(L"Exception was thrown while trying to process an external command. {}",StringUtils::stows(ex.what())));
    }

}

nlohmann::json ExternalCommandProcessor::Beam(const nlohmann::json& parameters)
{
    const auto client = Hk::Client::GetClientIdFromCharName(parameters.at("characterName").get<std::wstring>()).Handle();
    Hk::Player::Beam(client, parameters.at("baseName").get<std::wstring>());

    return {};
}