#include "PCH.hpp"

#include <Features/Logger.hpp>

#include "Defs/FLHookConfig.hpp"
#include "Features/Mail.hpp"
#include "Global.hpp"

#include <Features/CommandProcessors/UserCommandProcessor.hpp>

bool UserCommandProcessor::ProcessCommand(ClientId triggeringClient, std::wstring_view commandStr)
{
    this->client = triggeringClient;

    auto params = StringUtils::GetParams(commandStr, ' ');

    auto command = params.front();

    std::vector<std::wstring> paramsFiltered(params.begin(), params.end());
    paramsFiltered.erase(paramsFiltered.begin()); // Remove the first item which is the command

    const std::wstring character = Hk::Client::GetCharacterNameByID(triggeringClient).Unwrap();
    Logger::i()->Log(LogLevel::Info, std::format(L"{}: {}", character, commandStr));

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

void UserCommandProcessor::SetDieMessage(const std::wstring& param)
{
    if (!FLHookConfig::i()->chatConfig.dieMsg)
    {
        PrintUserCmdText(client, L"command disabled");
        return;
    }

    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set diemsg <param>\n"
                                         L"<param>: all,system,self or none";

    auto params = StringUtils::GetParams(param, ' ');
    const std::wstring dieMsgParam = StringUtils::ToLower(StringUtils::GetParam(params, 0));

    DIEMSGTYPE dieMsg;
    if (dieMsgParam == L"all")
    {
        dieMsg = DiemsgAll;
    }
    else if (dieMsgParam == L"system")
    {
        dieMsg = DIEMSG_SYSTEM;
    }
    else if (dieMsgParam == L"none")
    {
        dieMsg = DIEMSG_NONE;
    }
    else if (dieMsgParam == L"self")
    {
        dieMsg = DIEMSG_SELF;
    }
    else
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    const auto info = &ClientInfo[client];
    info->accountData["settings"]["dieMsg"] = std::to_string(dieMsg);
    info->SaveAccountData();

    info->dieMsg = dieMsg;

    // send confirmation msg
    PrintOk(client);
}

void UserCommandProcessor::SetDieMessageFontSize(const std::wstring& param)
{
    if (!FLHookConfig::i()->userCommands.userCmdSetDieMsgSize)
    {
        PrintUserCmdText(client, L"command disabled");
        return;
    }

    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set diemsgsize <size>\n"
                                         L"<size>: small, default";

    auto params = StringUtils::GetParams(param, ' ');
    const std::wstring dieMsgSizeParam = StringUtils::ToLower(StringUtils::GetParam(params, 0));

    CHATSIZE dieMsgSize;
    if (!dieMsgSizeParam.compare(L"small"))
    {
        dieMsgSize = CS_SMALL;
    }
    else if (!dieMsgSizeParam.compare(L"default"))
    {
        dieMsgSize = CS_DEFAULT;
    }
    else
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    const auto info = &ClientInfo[client];
    info->accountData["settings"]["dieMsgSize"] = std::to_string(dieMsgSize);
    info->SaveAccountData();

    info->dieMsgSize = dieMsgSize;

    // send confirmation msg
}

void UserCommandProcessor::SetChatFont(std::wstring_view fontSize, std::wstring_view fontType)
{
    if (!FLHookConfig::i()->userCommands.userCmdSetChatFont)
    {
        PrintUserCmdText(client, L"command disabled");
        return;
    }

    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set chatfont <size> <style>\n"
                                         L"<size>: small, default or big\n"
                                         L"<style>: default, bold, italic or underline";
    CHATSIZE chatSize;
    if (fontSize == L"small")
    {
        chatSize = CS_SMALL;
    }
    else if (fontSize == L"default")
    {
        chatSize = CS_DEFAULT;
    }
    else if (fontSize == L"big")
    {
        chatSize = CS_BIG;
    }
    else
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    CHATSTYLE chatStyle;
    if (fontType == L"default")
    {
        chatStyle = CST_DEFAULT;
    }
    else if (fontType == L"bold")
    {
        chatStyle = CST_BOLD;
    }
    else if (fontType == L"italic")
    {
        chatStyle = CST_ITALIC;
    }
    else if (fontType == L"underline")
    {
        chatStyle = CST_UNDERLINE;
    }
    else
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    // save to ini
    const auto info = &ClientInfo[client];
    info->accountData["settings"]["chatStyle"] = std::to_string(chatStyle);
    info->accountData["settings"]["chatSize"] = std::to_string(chatSize);
    info->SaveAccountData();

    info->chatSize = chatSize;
    info->chatStyle = chatStyle;

    // send confirmation msg
    PrintOk(client);
}

void UserCommandProcessor::IgnoreUser(std::wstring_view ignoredUser, std::wstring_view flags)
{
    if (!FLHookConfig::i()->userCommands.userCmdIgnore)
    {

        return;
    }

    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /ignore <charname> [<flags>]\n"
                                         L"<charname>: character name which should be ignored(case insensitive)\n"
                                         L"<flags>: combination of the following flags:\n"
                                         L" p - only affect private chat\n"
                                         L" i - <charname> may match partially\n"
                                         L"Examples:\n"
                                         L"\"/ignore SomeDude\" ignores all chatmessages from SomeDude\n"
                                         L"\"/ignore PlayerX p\" ignores all private-chatmessages from PlayerX\n"
                                         L"\"/ignore idiot i\" ignores all chatmessages from players whose \n"
                                         L"charname contain \"idiot\" (e.g. \"[XYZ]IdIOT\", \"MrIdiot\", etc)\n"
                                         L"\"/ignore Fool pi\" ignores all private-chatmessages from players \n"
                                         L"whose charname contain \"fool\"";

    const std::wstring allowedFlags = L"pi";

    if (ignoredUser.empty())
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    // check if flags are valid
    for (const auto flag : flags)
    {
        if (allowedFlags.find_first_of(flag) == std::wstring::npos)
        {
            PrintUserCmdText(client, errorMsg);
            return;
        }
    }

    if (ClientInfo[client].ignoreInfoList.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
    {
        PrintUserCmdText(client, L"Error: Too many entries in the ignore list, please delete an entry first!");
        return;
    }

    // save to ini
    const auto info = &ClientInfo[client];
    auto& list = info->accountData["settings"]["ignoreList"];
    list[StringUtils::wstos(std::wstring(ignoredUser))] = flags;

    info->SaveAccountData();

    IgnoreInfo ii;
    ii.character = ignoredUser;
    ii.flags = flags;
    info->ignoreInfoList.push_back(ii);

    PrintOk(client);
}

void UserCommandProcessor::IgnoreClientId(ClientId ignoredClient, std::wstring_view flags)
{
    if (!FLHookConfig::i()->userCommands.userCmdIgnore)
    {
        PrintUserCmdText(client, L"Command disabled");
        return;
    }

    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /ignoreid <client-id> [<flags>]\n"
                                         L"<client-id>: client-id of character which should be ignored\n"
                                         L"<flags>: if \"p\"(without quotation marks) then only affect private\n"
                                         L"chat";

    if (ignoredClient == UINT_MAX || !flags.empty() && flags != L"p")
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    if (ClientInfo[client].ignoreInfoList.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
    {
        PrintUserCmdText(client, L"Error: Too many entries in the ignore list, please delete an entry first!");
        return;
    }

    if (!Hk::Client::IsValidClientID(ignoredClient) || Hk::Client::IsInCharSelectMenu(ignoredClient))
    {
        PrintUserCmdText(client, L"Error: Invalid client-id");
        return;
    }

    std::wstring character = Hk::Client::GetCharacterNameByID(ignoredClient).Handle();

    // save to ini
    const auto info = &ClientInfo[client];
    auto& list = info->accountData["settings"]["ignoreList"];
    list[StringUtils::wstos(std::wstring(character))] = flags;

    info->SaveAccountData();

    IgnoreInfo ii;
    ii.character = character;
    ii.flags = flags;
    ClientInfo[client].ignoreInfoList.push_back(ii);

    PrintUserCmdText(client, std::format(L"OK, \"{}\" added to ignore list", character));
}

void UserCommandProcessor::GetIgnoreList()
{
    if (!FLHookConfig::i()->userCommands.userCmdIgnore)
    {
        PrintUserCmdText(client, L"command disabled");
        return;
    }

    PrintUserCmdText(client, L"Id | Character Name | flags");
    int i = 1;
    for (auto& ignore : ClientInfo[client].ignoreInfoList)
    {
        PrintUserCmdText(client, std::format(L"{} | %s | %s", i, ignore.character, ignore.flags));
        i++;
    }
    PrintOk(client);
}

void UserCommandProcessor::RemoveFromIgnored(const std::vector<std::wstring_view>& charactersToRemove)
{
    if (!FLHookConfig::i()->userCommands.userCmdIgnore)
    {
        PrintUserCmdText(client, L"Command disabled");
        return;
    }

    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /delignore <id> [<id2> <id3> ...]\n"
                                         L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)";
    if (charactersToRemove.empty())
    {
        PrintUserCmdText(client, errorMsg);
        return;
    }

    const auto info = &ClientInfo[client];
    if (charactersToRemove.front() == L"all")
    {
        info->accountData["settings"]["ignoreList"] = nlohmann::json::object();
        info->SaveAccountData();
        PrintOk(client);
        return;
    }

    std::vector<uint> idsToBeDeleted;
    for (auto name : charactersToRemove)
    {
        uint id = StringUtils::Cast<uint>(name);
        if (!id || id > ClientInfo[client].ignoreInfoList.size())
        {
            PrintUserCmdText(client, L"Error: Invalid Id");
            return;
        }

        idsToBeDeleted.push_back(id);
    }

    ClientInfo[client].ignoreInfoList.reverse();
    for (const auto& del : idsToBeDeleted)
    {
        uint currId = ClientInfo[client].ignoreInfoList.size();
        for (auto ignoreIt = ClientInfo[client].ignoreInfoList.begin(); ignoreIt != ClientInfo[client].ignoreInfoList.end(); ++ignoreIt)
        {
            if (currId == del)
            {
                ClientInfo[client].ignoreInfoList.erase(ignoreIt);
                break;
            }
            currId--;
        }
    }
    ClientInfo[client].ignoreInfoList.reverse();

    // send confirmation msg
    auto newList = nlohmann::json::object();
    int i = 1;
    for (const auto& ignore : ClientInfo[client].ignoreInfoList)
    {
        newList[StringUtils::wstos(ignore.character)] = ignore.flags;
        i++;
    }

    info->accountData["settings"]["ignoreList"] = newList;
    info->SaveAccountData();
    PrintOk(client);
}

void UserCommandProcessor::GetClientIds()
{
    for (auto& player : Hk::Admin::GetPlayers())
    {
        PrintUserCmdText(client, std::format(L"| {} = {}", player.character, player.client));
    }
    PrintUserCmdText(client, L"OK");
}

void UserCommandProcessor::GetSelfClientId() { PrintUserCmdText(client, std::format(L"Your client-id: {}", client)); }

// TODO: Move to utils.
void UserCommandProcessor::InvitePlayer(const std::wstring_view& characterName)
{
    const std::wstring XML = L"<TEXT>/i " + StringUtils::XmlText(characterName) + L"</TEXT>";

    // Allocates a stack-sized std::array once per run-time and clear every invocation.
    static std::array<char, USHRT_MAX> buf{};
    std::ranges::fill(buf, 0);

    uint retVal;
    if (Hk::Chat::FMsgEncodeXml(XML, buf.data(), sizeof buf, retVal).Raw().has_error())
    {
        PrintUserCmdText(client, L"Error: Could not encode XML");
        return;
    }
    // Mimics Freelancer's ingame invite system by using their chatID and chat commands from pressing the invite target button.
    CHAT_ID chatId;
    chatId.id = client;
    CHAT_ID chatIdTo;
    chatIdTo.id = 0x00010001;
    Server.SubmitChat(chatId, retVal, buf.data(), chatIdTo, -1);
}

void UserCommandProcessor::InvitePlayerByName(std::wstring_view invitee)
{
    if (!invitee.empty())
    {
        if (auto inviteeId = Hk::Client::GetClientIdFromCharName(std::wstring(invitee)).Raw();
            inviteeId.has_value() && !Hk::Client::IsInCharSelectMenu(inviteeId.value()))
        {
            InvitePlayer(invitee);
        }
    }
    else
    {
        auto targetClientId = Hk::Player::GetTargetClientID(client).Raw();
        if (targetClientId.has_value())
        {
            InvitePlayer(Hk::Client::GetCharacterNameByID(targetClientId.value()).Unwrap());
        }
    }
}

void UserCommandProcessor::InvitePlayerById(ClientId inviteeId)
{

    if (inviteeId == UINT_MAX || inviteeId == 0)
    {
        PrintUserCmdText(client, L"Error: Invalid parameters\nUsage: /invite$ <client-id>");
        return;
    }

    if (!Hk::Client::IsValidClientID(inviteeId) || Hk::Client::IsInCharSelectMenu(inviteeId))
    {
        PrintUserCmdText(client, L"Error: Invalid client-id");
        return;
    }

    InvitePlayer(Hk::Client::GetCharacterNameByID(inviteeId).Unwrap());
}

void UserCommandProcessor::FactionInvite(std::wstring_view factionTag)
{

    bool msgSent = false;

    if (factionTag.size() < 3)
    {
        PrintUserCmdText(client, L"ERR Invalid parameters");
        PrintUserCmdText(client, L"Usage: /factioninvite <tag> or /fi ...");
        return;
    }

    for (const auto& player : Hk::Admin::GetPlayers())
    {
        if (StringUtils::ToLower(player.character).find(factionTag) == std::wstring::npos)
        {
            continue;
        }

        if (player.client == client)
        {
            continue;
        }

        InvitePlayer(player.character);
        msgSent = true;
    }

    if (msgSent == false)
    {
        PrintUserCmdText(client, L"ERR No chars found");
    }
}

void UserCommandProcessor::DeleteMail(const std::wstring_view mailID, const std::wstring_view readOnlyDel)
{
    if (mailID == L"all")
    {
        const auto count = MailManager::i()->PurgeAllMail(client, readOnlyDel == L"readonly");
        if (count.has_error())
        {
            PrintUserCmdText(client, std::format(L"Error deleting mail: {}", count.error()));
            return;
        }

        PrintUserCmdText(client, std::format(L"Deleted {} mail", count.value()));
    }
    else
    {
        const auto index = StringUtils::Cast<int64>(mailID);
        if (const auto err = MailManager::i()->DeleteMail(client, index); err.has_error())
        {
            PrintUserCmdText(client, std::format(L"Error deleting mail: {}", err.error()));
            return;
        }

        PrintUserCmdText(client, L"Mail deleted");
    }
}

void UserCommandProcessor::ReadMail(uint mailId)
{
    if (mailId <= 0)
    {
        PrintUserCmdText(client, L"Id was not provided or was invalid.");
        return;
    }

    const auto mail = MailManager::i()->GetMailById(client, mailId);
    if (mail.has_error())
    {
        PrintUserCmdText(client, std::format(L"Error retreiving mail: {}", mail.error()));
        return;
    }

    const auto& item = mail.value();
    PrintUserCmdText(client, std::format(L"From: {}", item.author));
    PrintUserCmdText(client, std::format(L"Subject: {}", item.subject));
    PrintUserCmdText(client, std::format(L"Date: {:%F %T}", TimeUtils::UnixToSysTime(item.timestamp)));
    PrintUserCmdText(client, item.body);
}

void UserCommandProcessor::ListMail(int pageNumber, std::wstring_view unread)
{

    if (pageNumber <= 0)
    {
        PrintUserCmdText(client, L"Page was not provided or was invalid.");
        return;
    }

    const bool unreadOnly = (unread == L"unread");

    const auto mail = MailManager::i()->GetMail(client, unreadOnly, pageNumber);
    if (mail.has_error())
    {
        PrintUserCmdText(client, std::format(L"Error retrieving mail: {}", mail.error()));
        return;
    }

    const auto& mailList = mail.value();
    if (mailList.empty())
    {
        PrintUserCmdText(client, L"You have no mail.");
        return;
    }

    PrintUserCmdText(client, std::format(L"Printing mail of page {}", mailList.size()));
    for (const auto& item : mailList)
    {
        // |    Id.) Subject (unread) - Author - Time
        PrintUserCmdText(client,
                         std::format(L"|    {}.) {} {}- {} - {:%F %T}",
                                     item.id,
                                     item.subject,
                                     item.unread ? L"(unread) " : L"",
                                     item.author,
                                     TimeUtils::UnixToSysTime(item.timestamp)));
    }
}

// TODO: Implement GiveCash Target and by ID
void UserCommandProcessor::GiveCash(std::wstring_view characterName, std::wstring_view amount)
{

    const auto targetPlayer = Hk::Client::GetClientIdFromCharName(characterName).Unwrap();
    const auto cash = StringUtils::MultiplyUIntBySuffix(amount);
    const auto clientCash = Hk::Player::GetCash(client).Unwrap();

    if (client == targetPlayer)
    {
        PrintUserCmdText(client, L"Not sure this really accomplishes much, (Don't give cash to yourself.)");
        return;
    }

    if (cash == 0)
    {
        PrintUserCmdText(client, std::format(L"Err: Invalid cash amount."));
        return;
    }

    if (clientCash < cash)
    {
        PrintUserCmdText(client, std::format(L"Err: You do not have enough cash, you only have {}, and are trying to give {}.", clientCash, cash));
        return;
    }

    Hk::Player::RemoveCash(client, cash).Handle();
    Hk::Player::AddCash(targetPlayer, cash).Handle();

    Hk::Player::SaveChar(client);
    Hk::Player::SaveChar(targetPlayer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCommandProcessor::Help(std::wstring_view module, std::wstring_view command)
{
    if (const auto* config = FLHookConfig::c(); !config->userCommands.userCmdHelp)
    {
        PrintUserCmdText(client, L"The help command is disabled.");
        return;
    }

    const auto& pm = PluginManager::i();
    /*if (module.empty())
    {
        PrintUserCmdText(client, L"The following command modules are available to you. Use /help <module> [command] for detailed information.");
        PrintUserCmdText(client, L"core");
        for (const auto& userCommandProcessor : pm->userCommands)
        {
            if (const auto commands = plugin->GetCommands(); commands.empty())
            {
                continue;
            }

            PrintUserCmdText(client, StringUtils::ToLower(std::wstring(plugin->GetShortName())));
        }
        return;
    }*/

    /*auto params = StringUtils::GetParams(paramView, ' ');
    const auto mod = StringUtils::GetParam(params, 1);
    const auto cmd = StringUtils::ToLower(StringUtils::Trim(StringUtils::GetParam(params, 2)));

    if (mod == L"core")
    {
        if (cmd.empty())
        {
            for (const auto& i : UserCmds)
            {
                if (i.command.index() == 0)
                {
                    PrintUserCmdText(client, std::get<std::wstring_view>(i.command));
                }
                else
                {
                    PrintUserCmdText(client, i.usage);
                }
            }
        }
        else if (const auto& userCommand = std::ranges::find_if(UserCmds, [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
                 userCommand != UserCmds.end())
        {
            PrintUserCmdText(client, userCommand->usage);
            PrintUserCmdText(client, userCommand->description);
        }
        else
        {
            PrintUserCmdText(client, std::format(L"Command '{}' not found within module 'Core'", cmd.c_str()));
        }
        return;
    }

    const auto& pluginIterator = std::ranges::find_if(
        *plugins, [&mod](const std::shared_ptr<Plugin> plug) { return StringUtils::ToLower(std::wstring(plug->GetShortName())) == StringUtils::ToLower(mod); });

    if (pluginIterator == plugins->end())
    {
        PrintUserCmdText(client, L"Command module not found.");
        return;
    }

    const auto plugin = *pluginIterator;

    const auto commands = plugin->GetCommands();
    if (cmd.empty())
    {
        for (const auto& [command, usage, proc, description] : commands)
        {
            if (command.index() == 0)
            {
                PrintUserCmdText(client, std::get<std::wstring_view>(command));
            }
            else
            {
                PrintUserCmdText(client, usage);
            }
        }
    }
    else if (const auto& userCommand = std::ranges::find_if(commands, [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
             userCommand != commands.end())
    {
        PrintUserCmdText(client, userCommand->usage);
        PrintUserCmdText(client, userCommand->description);
    }
    else
    {
        PrintUserCmdText(client, std::format(L"Command '{}' not found within module '{}'", cmd, std::wstring(plugin->GetShortName())));
    }*/
}
