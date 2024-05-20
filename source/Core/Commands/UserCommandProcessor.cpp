#include "PCH.hpp"

#include "Defs/FLHookConfig.hpp"

#include <API/Utils/Logger.hpp>

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"

#include <Core/Commands/UserCommandProcessor.hpp>

bool UserCommandProcessor::ProcessCommand(ClientId triggeringClient, std::wstring_view commandStr)
{
    this->userCmdClient = triggeringClient;

    auto params = StringUtils::GetParams(commandStr, ' ');

    const auto& config = FLHook::GetConfig();
    const auto command = params.front();
    if (command.length() < 2)
    {
        return false;
    }

    if (std::ranges::find(config.userCommands.disabledCommands, std::wstring_view(command.begin() + 1, command.end())) !=
        config.userCommands.disabledCommands.end())
    {
        (void)this->userCmdClient.Message(L"This command is currently disabled.");
        return false;
    }

    std::vector<std::wstring> paramsFiltered(params.begin(), params.end());
    paramsFiltered.erase(paramsFiltered.begin()); // Remove the first item which is the command

    const auto character = triggeringClient.GetCharacterName().Unwrap();
    Logger::Info(std::format(L"{}: {}", character, commandStr));

    return ProcessCommand(triggeringClient, command, paramsFiltered);
}

template <>
bool UserCommandProcessor::MatchCommand<0>([[maybe_unused]] UserCommandProcessor* processor, ClientId triggeringClient, const std::wstring_view cmd,
                                           std::vector<std::wstring>& paramVector)
{
    for (auto user : PluginManager::i()->userCommands)
    {
        if (user.lock()->ProcessCommand(triggeringClient, cmd, paramVector))
        {
            return true;
        }
    }

    return false;
}

bool UserCommandProcessor::ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring>& paramVector)
{
    return MatchCommand<commands.size()>(this, triggeringClient, cmd, paramVector);
}

void UserCommandProcessor::SetDieMessage(std::wstring_view param)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set diemsg <param>\n"
                                         L"<param>: all,system,self or none";

    auto params = StringUtils::GetParams(param, ' ');
    const std::wstring dieMsgParam = StringUtils::ToLower(StringUtils::GetParam(params, 0));

    DieMsgType dieMsg;
    if (dieMsgParam == L"all")
    {
        dieMsg = DieMsgType::All;
    }
    else if (dieMsgParam == L"system")
    {
        dieMsg = DieMsgType::System;
    }
    else if (dieMsgParam == L"none")
    {
        dieMsg = DieMsgType::None;
    }
    else if (dieMsgParam == L"self")
    {
        dieMsg = DieMsgType::Self;
    }
    else
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    auto& info = userCmdClient.GetData();
    // TODO: Update save to db
    // info.accountData["settings"]["dieMsg"] = StringUtils::wstos(std::wstring(magic_enum::enum_name(dieMsg)));
    info.dieMsg = dieMsg;

    // send confirmation msg
    PrintOk();
}

void UserCommandProcessor::SetDieMessageFontSize(std::wstring_view param)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set diemsgsize <size>\n"
                                         L"<size>: small, default";

    auto params = StringUtils::GetParams(param, ' ');
    const std::wstring dieMsgSizeParam = StringUtils::ToLower(StringUtils::GetParam(params, 0));

    ChatSize dieMsgSize;
    if (dieMsgSizeParam == L"small")
    {
        dieMsgSize = ChatSize::Small;
    }
    else if (dieMsgSizeParam == L"default")
    {
        dieMsgSize = ChatSize::Default;
    }
    else
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    auto& info = FLHook::Clients()[userCmdClient];
    // TODO: Update save to db
    // info.accountData["settings"]["dieMsgSize"] = StringUtils::wstos(std::wstring(magic_enum::enum_name(dieMsgSize)));

    info.dieMsgSize = dieMsgSize;

    PrintOk();
}

void UserCommandProcessor::SetChatFont(std::wstring_view fontSize, std::wstring_view fontType)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set chatfont <size> <style>\n"
                                         L"<size>: small, default or big\n"
                                         L"<style>: default, bold, italic or underline";
    ChatSize chatSize;
    if (fontSize == L"small")
    {
        chatSize = ChatSize::Small;
    }
    else if (fontSize == L"default")
    {
        chatSize = ChatSize::Default;
    }
    else if (fontSize == L"big")
    {
        chatSize = ChatSize::Big;
    }
    else
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    ChatStyle chatStyle;
    if (fontType == L"default")
    {
        chatStyle = ChatStyle::Default;
    }
    else if (fontType == L"bold")
    {
        chatStyle = ChatStyle::Bold;
    }
    else if (fontType == L"italic")
    {
        chatStyle = ChatStyle::Italic;
    }
    else if (fontType == L"underline")
    {
        chatStyle = ChatStyle::Underline;
    }
    else
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    // save to ini
    // TODO: Update save to db
    auto& info = userCmdClient.GetData();
    /*info.accountData["settings"]["chatStyle"] = StringUtils::wstos(std::wstring(magic_enum::enum_name(chatStyle)));
    info.accountData["settings"]["chatSize"] = StringUtils::wstos(std::wstring(magic_enum::enum_name(chatSize)));*/

    info.chatSize = chatSize;
    info.chatStyle = chatStyle;

    // send confirmation msg
    PrintOk();
}

void UserCommandProcessor::IgnoreUser(std::wstring_view ignoredUser, std::wstring_view flags)
{
    static const std::wstring errorMsg =
        L"Error: Invalid parameters\n"
        L"Usage: /ignore <charname> [<flags>]\n"
        L"<charname>: character name which should be ignored(case insensitive)\n"
        L"<flags>: combination of the following flags:\n"
        L" p - only affect private chat\n"
        L" i - <charname> may match partially\n"
        L"Examples:\n"
        L"\"/ignore SomeDude\" ignores all chatmessages from SomeDude\n"
        L"\"/ignore PlayerX p\" ignores all private-chatmessages from PlayerX\n"
        L"\"/ignore idiot i\" ignores all chatmessages from players whose charname contain \"idiot\" (e.g. \"[XYZ]IdIOT\", \"MrIdiot\", etc)\n"
        L"\"/ignore Fool pi\" ignores all private-chatmessages from players whose charname contain \"fool\"";

    const std::wstring allowedFlags = L"pi";

    if (ignoredUser.empty())
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    auto ignoredLower = StringUtils::ToLower(ignoredUser);

    // check if flags are valid
    for (const auto flag : flags)
    {
        if (allowedFlags.find_first_of(flag) == std::wstring::npos)
        {
            userCmdClient.Message(errorMsg);
            return;
        }
    }

    auto& info = userCmdClient.GetData();
    if (info.ignoreInfoList.size() > FLHook::GetConfig().userCommands.userCmdMaxIgnoreList)
    {
        userCmdClient.Message(L"Error: Too many entries in the ignore list, please delete an entry first!");
        return;
    }

    // TODO: Update save to db
    /*auto& list = info.accountData["settings"]["ignoreList"];
    list[StringUtils::wstos(std::wstring(ignoredLower))] = flags;

    IgnoreInfo ii;
    ii.character = ignoredLower;
    ii.flags = flags;
    info.ignoreInfoList.push_back(ii);*/

    PrintOk();
}

void UserCommandProcessor::IgnoreClientId(ClientId ignoredClient, std::wstring_view flags)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /ignoreid <id> [<flags>]\n"
                                         L"<id>: client id of character which should be ignored\n"
                                         L"<flags>: if \"p\"(without quotation marks) then only affect private chat";

    if (!ignoredClient || (!flags.empty() && flags != L"p"))
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    auto& data = userCmdClient.GetData();
    if (data.ignoreInfoList.size() > FLHook::GetConfig().userCommands.userCmdMaxIgnoreList)
    {
        userCmdClient.Message(L"Error: Too many entries in the ignore list, please delete an entry first!");
        return;
    }

    if (ignoredClient.InCharacterSelect())
    {
        userCmdClient.Message(L"Error: Invalid client id");
        return;
    }

    auto character = StringUtils::ToLower(ignoredClient.GetCharacterName().Handle());

    // save to ini

    auto& info = userCmdClient.GetData();
    /*auto& list = info.accountData["settings"]["ignoreList"];
    list[StringUtils::wstos(std::wstring(character))] = flags;*/
    // TODO: Update save to db

    IgnoreInfo ii;
    ii.character = character;
    ii.flags = flags;
    info.ignoreInfoList.push_back(ii);

    userCmdClient.Message(std::format(L"OK, \"{}\" added to ignore list", character));
}

void UserCommandProcessor::GetIgnoreList()
{
    userCmdClient.Message(L"Id | Character Name | flags");
    int i = 1;
    auto& info = userCmdClient.GetData();
    for (auto& ignore : info.ignoreInfoList)
    {
        userCmdClient.Message(std::format(L"{} | %s | %s", i, ignore.character, ignore.flags));
        i++;
    }
    PrintOk();
}

void UserCommandProcessor::RemoveFromIgnored(std::vector<std::wstring_view> charactersToRemove)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /delignore <id> [<id2> <id3> ...]\n"
                                         L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)";
    if (charactersToRemove.empty())
    {
        userCmdClient.Message(errorMsg);
        return;
    }

    auto& info = userCmdClient.GetData();
    if (charactersToRemove.front() == L"all")
    {
        // TODO: Update save to db
        // info.accountData["settings"]["ignoreList"] = nlohmann::json::object();
        PrintOk();
        return;
    }

    std::vector<uint> idsToBeDeleted;
    for (const auto name : charactersToRemove)
    {
        uint id = StringUtils::Cast<uint>(name);
        if (!id || id > info.ignoreInfoList.size())
        {
            userCmdClient.Message(L"Error: Invalid Id");
            return;
        }

        idsToBeDeleted.push_back(id);
    }

    info.ignoreInfoList.reverse();
    for (const auto& del : idsToBeDeleted)
    {
        uint currId = info.ignoreInfoList.size();
        for (auto ignoreIt = info.ignoreInfoList.begin(); ignoreIt != info.ignoreInfoList.end(); ++ignoreIt)
        {
            if (currId == del)
            {
                info.ignoreInfoList.erase(ignoreIt);
                break;
            }
            currId--;
        }
    }

    info.ignoreInfoList.reverse();

    // send confirmation msg
    // TODO: Update save to db
    /*auto newList = nlohmann::json::object();
    int i = 1;
    for (const auto& ignore : info.ignoreInfoList)
    {
        newList[StringUtils::wstos(ignore.character)] = ignore.flags;
        i++;
    }*/

    // info.accountData["settings"]["ignoreList"] = newList;
    PrintOk();
}

void UserCommandProcessor::GetClientIds()
{
    for (auto& client : FLHook::Clients())
    {
        userCmdClient.Message(std::format(L"| {} = {}", client.characterName, client.id));
    }

    userCmdClient.Message(L"OK");
}

void UserCommandProcessor::GetSelfClientId() { userCmdClient.Message(std::format(L"Your userCmdClient-id: {}", userCmdClient)); }

// TODO: Move to utils.
void UserCommandProcessor::InvitePlayer(const std::wstring_view& characterName)
{
    const std::wstring XML = L"<TEXT>/i " + StringUtils::XmlText(characterName) + L"</TEXT>";

    // Allocates a stack-sized std::array once per run-time and clear every invocation.
    static std::array<char, USHRT_MAX> buf{};
    std::ranges::fill(buf, 0);

    uint retVal;
    if (InternalApi::FMsgEncodeXml(XML, buf.data(), sizeof buf, retVal).Raw().has_error())
    {
        userCmdClient.Message(L"Error: Could not encode XML");
        return;
    }
    // Mimics Freelancer's ingame invite system by using their chatID and chat commands from pressing the invite target button.
    CHAT_ID chatId;
    chatId.id = userCmdClient.GetValue();
    CHAT_ID chatIdTo;
    chatIdTo.id = static_cast<int>(SpecialChatIds::System);
    Server.SubmitChat(chatId, retVal, buf.data(), chatIdTo, -1);
}

void UserCommandProcessor::InvitePlayerByName(std::wstring_view invitee)
{
    if (!invitee.empty())
    {
        if (const auto inviteeId = ClientId(invitee); inviteeId && !inviteeId.InCharacterSelect())
        {
            InvitePlayer(invitee);
            return;
        }

        userCmdClient.Message(std::format(L"Failed to invite player: '{}'. They may not be online, or are otherwise unavailable.", invitee));
        return;
    }

    auto ship = userCmdClient.GetShipId().Raw();
    if (ship.has_error())
    {
        userCmdClient.Message(L"No invitee was provided and no target selected.");
        return;
    }

    auto target = ship.value().GetTarget();
    if (!target.has_value())
    {
        userCmdClient.Message(L"No invitee was provided and no target selected.");
        return;
    }

    if (const auto targetClient = target->GetPlayer(); targetClient.has_value())
    {
        InvitePlayer(targetClient.value().GetCharacterName().Unwrap());
        return;
    }

    userCmdClient.Message(L"You cannot invite an NPC.");
}

void UserCommandProcessor::InvitePlayerById(const ClientId inviteeId)
{
    if (inviteeId.InCharacterSelect())
    {
        userCmdClient.Message(L"Error: Invalid client id");
        return;
    }

    InvitePlayer(inviteeId.GetCharacterName().Unwrap());
}

void UserCommandProcessor::FactionInvite(std::wstring_view factionTag)
{

    bool msgSent = false;

    if (factionTag.size() < 3)
    {
        userCmdClient.Message(L"ERR Invalid parameters");
        userCmdClient.Message(L"Usage: /factioninvite <tag> or /fi ...");
        return;
    }

    for (const auto& player : FLHook::Clients())
    {
        if (StringUtils::ToLower(player.characterName).find(factionTag) == std::wstring::npos)
        {
            continue;
        }

        if (player.id == userCmdClient)
        {
            continue;
        }

        InvitePlayer(player.characterName);
        msgSent = true;
    }

    if (msgSent == false)
    {
        userCmdClient.Message(L"ERR No chars found");
    }
}

/*void UserCommandProcessor::DeleteMail(const std::wstring_view mailID, const std::wstring_view readOnlyDel)
{
    if (mailID == L"all")
    {
        const auto count = MailManager::i()->PurgeAllMail(userCmdClient, readOnlyDel == L"readonly");
        if (count.has_error())
        {
            userCmdClient.Message(std::format(L"Error deleting mail: {}", count.error()));
            return;
        }

        userCmdClient.Message(std::format(L"Deleted {} mail", count.value()));
    }
    else
    {
        const auto index = StringUtils::Cast<int64>(mailID);
        if (const auto err = MailManager::i()->DeleteMail(userCmdClient, index); err.has_error())
        {
            userCmdClient.Message(std::format(L"Error deleting mail: {}", err.error()));
            return;
        }

        userCmdClient.Message(L"Mail deleted");
    }
}

void UserCommandProcessor::ReadMail(uint mailId)
{
    if (mailId <= 0)
    {
        userCmdClient.Message(L"Id was not provided or was invalid.");
        return;
    }

    const auto mail = MailManager::i()->GetMailById(userCmdClient, mailId);
    if (mail.has_error())
    {
        userCmdClient.Message(std::format(L"Error retreiving mail: {}", mail.error()));
        return;
    }

    const auto& item = mail.value();
    userCmdClient.Message(std::format(L"From: {}", item.author));
    userCmdClient.Message(std::format(L"Subject: {}", item.subject));
    userCmdClient.Message(std::format(L"Date: {:%F %T}", TimeUtils::UnixToSysTime(item.timestamp)));
    userCmdClient.Message(item.body);
}

void UserCommandProcessor::ListMail(int pageNumber, std::wstring_view unread)
{

    if (pageNumber <= 0)
    {
        userCmdClient.Message(L"Page was not provided or was invalid.");
        return;
    }

    const bool unreadOnly = (unread == L"unread");

    const auto mail = MailManager::i()->GetMail(userCmdClient, unreadOnly, pageNumber);
    if (mail.has_error())
    {
        userCmdClient.Message(std::format(L"Error retrieving mail: {}", mail.error()));
        return;
    }

    const auto& mailList = mail.value();
    if (mailList.empty())
    {
        userCmdClient.Message(L"You have no mail.");
        return;
    }

    userCmdClient.Message(std::format(L"Printing mail of page {}", mailList.size()));
    for (const auto& item : mailList)
    {
        // |    Id.) Subject (unread) - Author - Time
        userCmdClient.Message(std::format(
            L"|    {}.) {} {}- {} - {:%F %T}", item.id, item.subject, item.unread ? L"(unread) " : L"", item.author, TimeUtils::UnixToSysTime(item.timestamp)));
    }
}*/

// TODO: Implement GiveCash Target and by ID
void UserCommandProcessor::GiveCash(std::wstring_view characterName, std::wstring_view amount)
{
    // TODO: resolve sending money to offline people
    const auto cash = StringUtils::MultiplyUIntBySuffix(amount);
    const auto targetPlayer = ClientId(characterName);
    const auto client = ClientId(userCmdClient);
    const auto clientCash = client.GetCash().Unwrap();

    if (userCmdClient == targetPlayer)
    {
        userCmdClient.Message(L"Not sure this really accomplishes much, (Don't give cash to yourself.)");
        return;
    }

    if (cash == 0)
    {
        userCmdClient.Message(std::format(L"Err: Invalid cash amount."));
        return;
    }

    if (clientCash < cash)
    {
        userCmdClient.Message(std::format(L"Err: You do not have enough cash, you only have {}, and are trying to give {}.", clientCash, cash));
        return;
    }

    client.RemoveCash(cash).Handle();
    targetPlayer.AddCash(cash).Handle();

    client.SaveChar().Handle();
    targetPlayer.SaveChar().Handle();
}

void UserCommandProcessor::GiveCashById(ClientId targetClient, std::wstring_view amount)
{
    const auto name = targetClient.GetCharacterName().Handle();
    return GiveCash(name, amount);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCommandProcessor::Help(const std::wstring_view module, std::wstring_view command)
{
    const auto& pm = PluginManager::i();
    if (module.empty())
    {
        userCmdClient.Message(L"The following command modules are available to you. Use /help <module> [command] for detailed information.");
        userCmdClient.Message(L"core");
        for (const auto& plugin : pm->plugins)
        {
            if (dynamic_cast<const AbstractUserCommandProcessor*>(plugin.get()) == nullptr)
            {
                continue;
            }

            userCmdClient.Message(StringUtils::ToLower(plugin->GetShortName()));
        }
        return;
    }

    const auto cmd = StringUtils::ToLower(StringUtils::Trim(command));

    const auto moduleLower = StringUtils::ToLower(module);

    if (moduleLower == L"core")
    {
        if (cmd.empty())
        {
            for (const auto& i : commands)
            {
                userCmdClient.Message(i.cmd);
            }
        }
        else if (const auto& userCommand =
                     std::ranges::find_if(commands, [&cmd](const auto& userCmd) { return cmd == userCmd.cmd.substr(1, userCmd.cmd.size() - 1); });
                 userCommand != commands.end())
        {
            userCmdClient.Message(userCommand->usage);
            userCmdClient.Message(userCommand->description);
        }
        else
        {
            userCmdClient.Message(std::format(L"Command '{}' not found within module 'core'", cmd));
        }
        return;
    }

    const auto& pluginIterator = std::ranges::find_if(
        pm->plugins, [&moduleLower](const std::shared_ptr<Plugin>& plug) { return StringUtils::ToLower(std::wstring(plug->GetShortName())) == moduleLower; });

    if (pluginIterator == pm->plugins.end())
    {
        userCmdClient.Message(L"Command module not found.");
        return;
    }

    const auto plugin = *pluginIterator;

    const auto cmdProcessor = dynamic_cast<AbstractUserCommandProcessor*>(plugin.get());
    if (cmdProcessor == nullptr)
    {
        userCmdClient.Message(L"Command module not found.");
        return;
    }

    if (cmd.empty())
    {
        for (const auto& [fullCmd, usage, description] : cmdProcessor->GetCommands())
        {
            userCmdClient.Message(fullCmd);
        }
    }
    else if (const auto& userCommand =
                 std::ranges::find_if(commands, [&cmd](const auto& userCmd) { return cmd == userCmd.cmd.substr(1, userCmd.cmd.size() - 1); });
             userCommand != commands.end())
    {
        userCmdClient.Message(userCommand->usage);
        userCmdClient.Message(userCommand->description);
    }
    else
    {
        userCmdClient.Message(std::format(L"Command '{}' not found within module '{}'", cmd, std::wstring(plugin->GetShortName())));
    }
}
