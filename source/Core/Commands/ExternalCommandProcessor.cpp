#include "PCH.hpp"

#include "Core/Commands/ExternalCommandProcessor.hpp"

#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/types.hpp>

std::pair<bool, std::shared_ptr<BsonWrapper>> ExternalCommandProcessor::ProcessCommand(const bsoncxx::document::view document)
{
    try
    {
        bsoncxx::document::view subDoc;
        std::string_view command;
        for (auto& data : document)
        {
            if (const std::string_view key = data.key(); key == "command")
            {
                command = data.get_string();
            }
            else if (key == "data")
            {
                subDoc = data.get_document();
            }
        }

        if (command.empty())
        {
            // TODO: Handle malformed command
            return { false, {} };
        }

        const auto func = functions.find(command);
        if (func == functions.end())
        {
            // Check the plugin command handlers
            for (auto plugin : PluginManager::i()->externalCommands)
            {
                const auto locked = plugin.lock();
                if (!locked)
                {
                    continue;
                }

                // If success was true, or success was false and we were given a document; either way we got into a command function somewhere
                if (auto response = locked->ProcessCommand(command, subDoc); response.first || response.second)
                {
                    return response;
                }

                // Otherwise we should try another handler
            }

            return { false, BsonWrapper::CreateErrorDocument({ "Command was not found." }) };
        }

        return func->second(subDoc);
    }
    catch (bsoncxx::exception& ex)
    {
        Logger::Warn(std::format(L"Exception was thrown while trying to process an external command. {}", StringUtils::stows(ex.what())));
    }

    return { false, BsonWrapper::CreateErrorDocument({ "An unknown error occured and the command was unable to be processed." }) };
}

std::vector<std::wstring> ExternalCommandProcessor::GetCommands() const
{
    static std::vector<std::wstring> commands;
    commands.reserve(functions.size());
    for (auto i : functions)
    {
        commands.emplace_back(StringUtils::stows(i.first));
    }

    return commands;
}

std::pair<bool, std::shared_ptr<BsonWrapper>> ExternalCommandProcessor::Beam(bsoncxx::document::view parameters)
{
    struct BeamInfo
    {
            std::string_view characterName;
            std::string_view baseName;
            uint baseHash = 0;
    };

    BeamInfo info;

    for (auto& data : parameters)
    {
        if (const std::string_view key = data.key(); key == "characterName")
        {
            info.characterName = data.get_string();
        }
        else if (key == "baseName")
        {
            info.baseName = data.get_string();
        }
        else if (key == "baseHash")
        {
            info.baseHash = static_cast<uint>(data.get_int32());
        }
    }

    std::vector<std::string> errors;

    if (info.characterName.empty())
    {
        errors.emplace_back("'characterName' was not provided.");
    }

    if (!info.baseName.empty())
    {
        info.baseHash = CreateID(info.baseName.data());
    }
    else if (!info.baseHash)
    {
        errors.emplace_back("'baseName' nor 'baseHash' was provided.");
    }

    if (errors.size() == 0)
    {
        return { false, BsonWrapper::CreateErrorDocument(errors) };
    }

    if (const auto base = Universe::get_base(info.baseHash); !base)
    {
        errors.emplace_back("The base provided did not match a valid base.");
    }

    const std::wstring characterName = StringUtils::stows(info.characterName);
    const auto client = ClientId(characterName);
    if (!client)
    {
        errors.emplace_back("The provided character is not currently online or does not exist.");
    }

    if (!errors.empty())
    {
        return { false, BsonWrapper::CreateErrorDocument(errors) };
    }

    auto result = client.Beam(BaseId(info.baseHash));
    result.Handle(
        [&errors](Error errorCode, const std::wstring_view errString)
        {
            errors.emplace_back(std::format("Error while trying to beam: {} ({})", StringUtils::wstos(errString), static_cast<int>(errorCode)));
            return true;
        });

    if (!errors.empty())
    {
        return { false, BsonWrapper::CreateErrorDocument(errors) };
    }

    return { true, nullptr };
}
