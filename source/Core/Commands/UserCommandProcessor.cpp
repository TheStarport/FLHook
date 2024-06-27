#include "PCH.hpp"

#include "Defs/FLHookConfig.hpp"

#include <API/Utils/Logger.hpp>

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "API/InternalApi.hpp"
#include "API/Utils/Random.hpp"

#include <Core/Commands/UserCommandProcessor.hpp>

bool UserCommandProcessor::ProcessCommand(ClientId triggeringClient, std::wstring_view commandStr)
{
    if (commandStr.length() < 2)
    {
        return false;
    }

    this->userCmdClient = triggeringClient;
    auto params = StringUtils::GetParams(commandStr, ' ');

    const auto& config = FLHook::GetConfig();

    for (const auto trimmedCmdStr = std::wstring_view(commandStr.data() + 1, commandStr.length() - 1);
         const auto& disabledCommand : config.userCommands.disabledCommands)
    {
        if (trimmedCmdStr.starts_with(disabledCommand))
        {
            userCmdClient.Message(L"This command is currently disabled.");
            return false;
        }
    }

    const auto character = triggeringClient.GetCharacterName().Unwrap();
    Logger::Trace(std::format(L"{}: {}", character, commandStr));

    std::vector paramsFiltered(params.begin(), params.end());
    return ProcessCommand(triggeringClient, commandStr, paramsFiltered);
}

template <>
bool UserCommandProcessor::MatchCommand<0>([[maybe_unused]] UserCommandProcessor* processor, ClientId triggeringClient, const std::wstring_view fullCmdString,
                                           std::vector<std::wstring_view>& paramVector)
{
    for (auto& user : PluginManager::i()->userCommands)
    {
        if (user.lock()->ProcessCommand(triggeringClient, fullCmdString, paramVector))
        {
            return true;
        }
    }

    return false;
}

bool UserCommandProcessor::ProcessCommand(ClientId triggeringClient, const std::wstring_view fullCmdStr, std::vector<std::wstring_view>& paramVector)
{
    return MatchCommand<commands.size()>(this, triggeringClient, fullCmdStr, paramVector);
}

void UserCommandProcessor::SetDieMessage(std::wstring_view param)
{
    DieMsgType dieMsg;
    if (param == L"all")
    {
        dieMsg = DieMsgType::All;
    }
    else if (param == L"system")
    {
        dieMsg = DieMsgType::System;
    }
    else if (param == L"none")
    {
        dieMsg = DieMsgType::None;
    }
    else if (param == L"self")
    {
        dieMsg = DieMsgType::Self;
    }
    else
    {
        (void)userCmdClient.Message(L"Error: Invalid parameters\n"
                                    L"Usage: /setdiemsg <param>\n"
                                    L"<param>: 'all', 'system', 'self' or 'none'");
        return;
    }

    auto dieMsgVal = StringUtils::wstos(magic_enum::enum_name(dieMsg));
    Database::SaveValueOnAccount(userCmdClient.GetAccount().Handle(), "dieMsg", { dieMsgVal });

    auto& info = userCmdClient.GetData();
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
        (void)userCmdClient.Message(errorMsg);
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
        (void)userCmdClient.Message(errorMsg);
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
        (void)userCmdClient.Message(errorMsg);
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

void UserCommandProcessor::SetChatTime(std::wstring_view option)
{
    auto& info = userCmdClient.GetData();
    if (option == L"on")
    {
        if (info.showChatTime)
        {
            (void)userCmdClient.Message(L"Chat Time already enabled!");
        }
        else
        {
            (void)userCmdClient.Message(L"Chat Time enabled");
            info.showChatTime = true;
        }
    }
    else if (option == L"off")
    {
        if (!info.showChatTime)
        {
            (void)userCmdClient.Message(L"Chat Time already disabled!");
        }
        else
        {
            (void)userCmdClient.Message(L"Chat Time disabled");
            info.showChatTime = false;
        }
    }
    else
    {
        (void)userCmdClient.Message(L"Invalid parameter");
    }
}

void UserCommandProcessor::ShowLastSender()
{
    auto& info = userCmdClient.GetData();
    if (info.lastPMSender.IsValidClientId())
    {
        (void)userCmdClient.Message(std::format(L"Last PM sender is {}", info.lastPMSender.GetCharacterName().Handle()));
    }
    else
    {
        (void)userCmdClient.Message(L"Last PM sender not found!");
    }
}

void UserCommandProcessor::ReplyToLastMsg(std::wstring_view response)
{
    // TODO: Add tracking of lastPMSender
    auto& info = userCmdClient.GetData();
    if (info.lastPMSender.IsValidClientId())
    {
        (void)userCmdClient.MessageFrom(userCmdClient, response);
        (void)info.lastPMSender.MessageFrom(userCmdClient, response);
    }
    else
    {
        (void)userCmdClient.Message(L"Last PM sender not found!");
    }
}

void UserCommandProcessor::MessageTarget(std::wstring_view text)
{
    const auto target = userCmdClient.GetShipId().Handle().GetTarget();
    if (!target.has_value())
    {
        (void)userCmdClient.Message(L"No target selected");
        return;
    }

    const auto targetPlayer = target.value().GetPlayer();
    if (!targetPlayer.has_value())
    {
        (void)userCmdClient.Message(L"Target is not a player");
        return;
    }

    (void)userCmdClient.MessageFrom(userCmdClient, text);
    (void)targetPlayer.value().MessageFrom(userCmdClient, text);
}

void UserCommandProcessor::MessageTag(std::wstring_view tag, std::wstring_view msg)
{
    PlayerData* pd = nullptr;
    bool foundTaggedPlayer = false;
    while (pd = Players.traverse_active(pd))
    {
        ClientId client = ClientId(pd->clientId);
        if (client == userCmdClient)
        {
            continue;
        }

        if (client.GetCharacterName().Handle().find(tag) != std::wstring_view::npos)
        {
            foundTaggedPlayer = true;
            (void)client.MessageFrom(userCmdClient, msg);
        }
    }

    if (foundTaggedPlayer)
    {
        (void)userCmdClient.MessageFrom(userCmdClient, msg);
    }
    else
    {
        (void)userCmdClient.Message(L"No players with matching tag found");
    }
}

void UserCommandProcessor::SetSavedMsg(uint index, std::wstring_view msg)
{
    if (index > 9)
    {
        (void)userCmdClient.Message(L"Invalid message index provided");
        return;
    }

    auto& info = userCmdClient.GetData().presetMsgs.at(index) = msg;
    PrintOk();
}

void UserCommandProcessor::ShowSavedMsgs()
{
    auto& info = userCmdClient.GetData();
    uint counter = 0;
    (void)userCmdClient.Message(L"Saved messages");
    for (const auto& msg : info.presetMsgs)
    {
        (void)userCmdClient.Message(std::format(L"{}: {}", counter++, msg));
    }
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
        (void)userCmdClient.Message(errorMsg);
        return;
    }

    auto ignoredLower = StringUtils::ToLower(ignoredUser);

    // check if flags are valid
    for (const auto flag : flags)
    {
        if (allowedFlags.find_first_of(flag) == std::wstring::npos)
        {
            (void)userCmdClient.Message(errorMsg);
            return;
        }
    }

    auto& info = userCmdClient.GetData();
    if (info.ignoreInfoList.size() > FLHook::GetConfig().userCommands.userCmdMaxIgnoreList)
    {
        (void)userCmdClient.Message(L"Error: Too many entries in the ignore list, please delete an entry first!");
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
        (void)userCmdClient.Message(errorMsg);
        return;
    }

    auto& data = userCmdClient.GetData();
    if (data.ignoreInfoList.size() > FLHook::GetConfig().userCommands.userCmdMaxIgnoreList)
    {
        (void)userCmdClient.Message(L"Error: Too many entries in the ignore list, please delete an entry first!");
        return;
    }

    if (ignoredClient.InCharacterSelect())
    {
        (void)userCmdClient.Message(L"Error: Invalid client id");
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

    (void)userCmdClient.Message(std::format(L"OK, \"{}\" added to ignore list", character));
}

void UserCommandProcessor::GetIgnoreList()
{
    (void)userCmdClient.Message(L"Id | Character Name | flags");
    int i = 1;
    auto& info = userCmdClient.GetData();
    for (auto& ignore : info.ignoreInfoList)
    {
        (void)userCmdClient.Message(std::format(L"{} | %s | %s", i, ignore.character, ignore.flags));
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
        (void)userCmdClient.Message(errorMsg);
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
            (void)userCmdClient.Message(L"Error: Invalid Id");
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

    (void)userCmdClient.Message(L"OK");
}

void UserCommandProcessor::GetSelfClientId() { userCmdClient.Message(std::format(L"Your userCmdClient-id: {}", userCmdClient)); }

void RenameCallback(ClientId client, std::wstring newName, const std::shared_ptr<void>& taskData)
{
    auto errorMessage = std::static_pointer_cast<std::wstring>(taskData);
    if (!errorMessage->empty())
    {
        (void)client.Message(*errorMessage);
        return;
    }
    std::wstring currName = client.GetCharacterName().Handle().data();

    const auto& [renameCost, cooldown] = FLHook::GetConfig().rename;
    if (renameCost)
    {
        if (client.GetCash().Handle() < renameCost)
        {
            (void)client.Message(L"Insufficient money!");
            return;
        }
        (void)client.RemoveCash(renameCost);
    }

    (void)client.Message(L"Renaming, you will be kicked.");
    Timer::AddOneShot(
        [client, currName, newName]()
        {
            (void)client.Kick();
            TaskScheduler::Schedule(std::bind(AccountManager::Rename, currName, newName));
        },
        3000);
}

void UserCommandProcessor::Rename(std::wstring_view newName)
{
    const auto& [renameCost, cooldown] = FLHook::GetConfig().rename;

    if (newName.find(L' ') != std::wstring_view::npos)
    {
        (void)userCmdClient.Message(L"No whitespaces allowed.");
        return;
    }

    if (newName.length() > 23)
    {
        (void)userCmdClient.Message(L"Name too long, max 23 characters allowed");
        return;
    }

    // Ban any name that is numeric and might interfere with commands
    if (const auto numeric = StringUtils::Cast<uint>(newName); numeric < 10000 && numeric != 0)
    {
        (void)userCmdClient.Message(L"Names that are strictly numerical must be at least 5 digits.");
        return;
    }

    if (renameCost)
    {
        if (userCmdClient.GetCash().Unwrap() < renameCost)
        {
            (void)userCmdClient.Message(L"Insufficient money!");
            return;
        }
    }

    if (cooldown)
    {
        const auto charData = userCmdClient.GetData().characterData;
        if (auto lastRename = charData.find("lastRenameTimestamp"); lastRename != charData.end())
        {
            const auto cooldownAsDays = std::chrono::days(cooldown);
            if (const std::chrono::milliseconds endOfCooldown =
                    lastRename->get_date().value + std::chrono::duration_cast<std::chrono::milliseconds>(cooldownAsDays);
                endOfCooldown.count() > TimeUtils::UnixTime<std::chrono::milliseconds>())
            {
                (void)userCmdClient.Message(std::format(L"Rename cooldown not elapsed yet, end of cooldown: {}",
                                                        TimeUtils::AsDate(std::chrono::duration_cast<std::chrono::seconds>(endOfCooldown))));
                return;
            }
        }
    }

    std::wstring newNameStr = newName.data();
    TaskScheduler::ScheduleWithCallback<std::wstring>(std::bind(AccountManager::CheckCharnameTaken, userCmdClient, newNameStr, std::placeholders::_1),
                                                      std::bind(RenameCallback, userCmdClient, newNameStr, std::placeholders::_1));
}

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
        (void)userCmdClient.Message(L"Error: Could not encode XML");
        return;
    }
    // Mimics Freelancer's ingame invite system by using their chatID and chat commands from pressing the invite target button.
    CHAT_ID chatId{};
    chatId.id = userCmdClient.GetValue();
    CHAT_ID chatIdTo{};
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

        (void)userCmdClient.Message(std::format(L"Failed to invite player: '{}'. They may not be online, or are otherwise unavailable.", invitee));
        return;
    }

    auto ship = userCmdClient.GetShipId().Raw();
    if (ship.has_error())
    {
        (void)userCmdClient.Message(L"No invitee was provided and no target selected.");
        return;
    }

    auto target = ship.value().GetTarget();
    if (!target.has_value())
    {
        (void)userCmdClient.Message(L"No invitee was provided and no target selected.");
        return;
    }

    if (const auto targetClient = target->GetPlayer(); targetClient.has_value())
    {
        InvitePlayer(targetClient.value().GetCharacterName().Unwrap());
        return;
    }

    (void)userCmdClient.Message(L"You cannot invite an NPC.");
}

void UserCommandProcessor::InvitePlayerById(const ClientId inviteeId)
{
    if (inviteeId.InCharacterSelect())
    {
        (void)userCmdClient.Message(L"Error: Invalid client id");
        return;
    }

    InvitePlayer(inviteeId.GetCharacterName().Unwrap());
}

void UserCommandProcessor::FactionInvite(std::wstring_view factionTag)
{

    bool msgSent = false;

    if (factionTag.size() < 3)
    {
        (void)userCmdClient.Message(L"ERR Invalid parameters");
        (void)userCmdClient.Message(L"Usage: /factioninvite <tag> or /fi ...");
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
        (void)userCmdClient.Message(L"ERR No chars found");
    }
}

void CharacterTransferCallback(ClientId client, const std::shared_ptr<void>& taskData)
{
    auto errorMessage = std::static_pointer_cast<std::wstring>(taskData);
    if (!errorMessage->empty())
    {
        (void)client.Message(*errorMessage);
        return;
    }

    (void)client.Kick(L"Transferring the character, you will be kicked.", 3);
}

void UserCommandProcessor::TransferCharacter(const std::wstring_view cmd, const std::wstring_view param1, const std::wstring_view param2)
{
    const auto db = FLHook::GetDbClient();
    if (cmd == L"clearcode")
    {
        std::wstring charName = userCmdClient.GetCharacterName().Handle().data();
        TaskScheduler::Schedule(std::bind(AccountManager::ClearCharacterTransferCode, charName));
        (void)userCmdClient.Message(L"Character transfer code cleared");
    }
    else if (cmd == L"setcode")
    {
        std::wstring charName = userCmdClient.GetCharacterName().Handle().data();
        std::wstring newCharCode = param1.data();
        TaskScheduler::Schedule(std::bind(AccountManager::SetCharacterTransferCode, charName, newCharCode));
        (void)userCmdClient.Message(L"Character transfer code set");
    }
    else if (cmd == L"transfer")
    {
        if (userCmdClient.GetData().account->characters.size() >= 5)
        {
            (void)userCmdClient.Message(L"This account cannot hold more characters");
            return;
        }
        AccountId accountId = userCmdClient.GetAccount().Handle();
        std::wstring charName = param1.data();
        std::wstring charCode = param2.data();
        TaskScheduler::ScheduleWithCallback<std::wstring>(std::bind(AccountManager::TransferCharacter, accountId, charName, charCode, std::placeholders::_1),
                                                          std::bind(CharacterTransferCallback, userCmdClient, std::placeholders::_1));
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
        (void)userCmdClient.Message(L"Not sure this really accomplishes much, (Don't give cash to yourself.)");
        return;
    }

    if (cash == 0)
    {
        (void)userCmdClient.Message(std::format(L"Err: Invalid cash amount."));
        return;
    }

    if (clientCash < cash)
    {
        (void)userCmdClient.Message(std::format(L"Err: You do not have enough cash, you only have {}, and are trying to give {}.", clientCash, cash));
        return;
    }

    client.RemoveCash(cash).Handle();
    targetPlayer.AddCash(cash).Handle();

    client.SaveChar().Handle();
    targetPlayer.SaveChar().Handle();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCommandProcessor::Time() { (void)userCmdClient.Message(std::format(L"{:%Y-%m-%d %X}", std::chrono::system_clock::now())); }

void UserCommandProcessor::Help(const std::wstring_view module, std::wstring_view command)
{
    const auto& pm = PluginManager::i();
    if (module.empty())
    {
        (void)userCmdClient.Message(L"The following command modules are available to you. Use /help <module> [command] for detailed information.");
        (void)userCmdClient.Message(L"core");
        for (const auto& plugin : pm->plugins)
        {
            if (dynamic_cast<const AbstractUserCommandProcessor*>(plugin.get()) == nullptr)
            {
                continue;
            }

            (void)userCmdClient.Message(StringUtils::ToLower(plugin->GetShortName()));
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
                (void)userCmdClient.Message(i.cmd.front());
            }
        }
        else if (const auto& userCommand =
                     std::ranges::find_if(commands,
                                          [&cmd](const auto& userCmd) {
                                              return std::ranges::any_of(userCmd.cmd, [&cmd](const auto str) { return cmd == str.substr(1, str.size() - 1); });
                                          });
                 userCommand != commands.end())
        {
            (void)userCmdClient.Message(userCommand->usage);
            (void)userCmdClient.Message(userCommand->description);
        }
        else
        {
            (void)userCmdClient.Message(std::format(L"Command '{}' not found within module 'core'", cmd));
        }
        return;
    }

    const auto& pluginIterator = std::ranges::find_if(
        pm->plugins, [&moduleLower](const std::shared_ptr<Plugin>& plug) { return StringUtils::ToLower(std::wstring(plug->GetShortName())) == moduleLower; });

    if (pluginIterator == pm->plugins.end())
    {
        (void)userCmdClient.Message(L"Command module not found.");
        return;
    }

    const auto plugin = *pluginIterator;

    const auto cmdProcessor = dynamic_cast<AbstractUserCommandProcessor*>(plugin.get());
    if (cmdProcessor == nullptr)
    {
        (void)userCmdClient.Message(L"Command module not found.");
        return;
    }

    if (cmd.empty())
    {
        for (const auto& [fullCmd, usage, description] : cmdProcessor->GetUserCommands())
        {
            (void)userCmdClient.Message(fullCmd.front());
        }
    }
    else if (const auto& userCommand =
                 std::ranges::find_if(commands,
                                      [&cmd](const auto& userCmd)
                                      { return std::ranges::any_of(userCmd.cmd, [&cmd](const auto str) { return cmd == str.substr(1, str.size() - 1); }); });
             userCommand != commands.end())
    {
        (void)userCmdClient.Message(userCommand->usage);
        (void)userCmdClient.Message(userCommand->description);
    }
    else
    {
        (void)userCmdClient.Message(std::format(L"Command '{}' not found within module '{}'", cmd, std::wstring(plugin->GetShortName())));
    }
}

void UserCommandProcessor::DropRep()
{
    const auto& config = FLHook::GetConfig().reputatation;

    if (config.creditCost)
    {
        if (const uint cash = userCmdClient.GetCash().Handle(); cash < config.creditCost)
        {
            (void)userCmdClient.Message(std::format(L"Not enough money, {} credits required", config.creditCost));
            return;
        }
        (void)userCmdClient.RemoveCash(config.creditCost);
    }
    const auto playerRep = userCmdClient.GetReputation().Handle();
    const auto playerAffliation = playerRep.GetAffiliation().Handle();
    if (playerAffliation.GetValue() == -1)
    {
        (void)userCmdClient.Message(L"No affiliation, can't drop one");
        return;
    }

    (void)playerRep.SetAttitudeTowardsRepGroupId(playerAffliation, 0.3f);
}

void UserCommandProcessor::Value() { (void)userCmdClient.Message(std::format(L"Your worth is ${} credits", userCmdClient.GetValue())); }

void UserCommandProcessor::Dice(const uint sidesOfDice)
{
    uint result;
    if (!sidesOfDice)
    {
        result = Random::Uniform(1u, 6u);
        userCmdClient.MessageLocal(std::format(L"{} has rolled {} out of 6", userCmdClient.GetCharacterName().Handle(), result));
    }
    else
    {
        result = Random::Uniform(1u, sidesOfDice);
        userCmdClient.MessageLocal(std::format(L"{} has rolled {} out of {}", userCmdClient.GetCharacterName().Handle(), result, sidesOfDice));
    }
}

void UserCommandProcessor::Coin()
{
    const uint result = Random::Uniform(0u, 1u);
    userCmdClient.MessageLocal(std::format(L"{} tossed a coin, it landed on {}", userCmdClient.GetCharacterName().Handle(), result ? L"heads" : L"tails"));
}
