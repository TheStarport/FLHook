// ReSharper disable CppMemberFunctionMayBeStatic
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

std::optional<concurrencpp::result<void>> UserCommandProcessor::ProcessCommand(const ClientId triggeringClient, const std::wstring_view clientStr,
                                                                               const std::wstring_view commandStr)
{
    if (commandStr.length() < 2)
    {
        return std::nullopt;
    }

    auto params = StringUtils::GetParams(commandStr, ' ');

    const auto config = FLHook::GetConfig();

    for (const auto trimmedCmdStr = std::wstring_view(commandStr.data() + 1, commandStr.length() - 1);
         const auto& disabledCommand : config->userCommands.disabledCommands)
    {
        if (trimmedCmdStr.starts_with(disabledCommand))
        {
            triggeringClient.ToastMessage(L"Command Disabled", L"This command is currently disabled.", ToastType::Warning);
            return std::nullopt;
        }
    }

    const auto character = triggeringClient.GetCharacterName().Unwrap();

    TRACE("{{character}} {{command}}", { "character", character }, { "command", commandStr });

    std::vector paramsFiltered(params.begin(), params.end());
    paramsFiltered.insert(paramsFiltered.begin(), clientStr);

    return ProcessCommand(triggeringClient, commandStr, paramsFiltered);
}

template <>
std::optional<concurrencpp::result<void>> UserCommandProcessor::MatchCommand<0>([[maybe_unused]] UserCommandProcessor* processor, ClientId triggeringClient,
                                                                                const std::wstring_view fullCmdString,
                                                                                std::vector<std::wstring_view>& paramVector)
{
    return std::nullopt;
}

std::optional<concurrencpp::result<void>> UserCommandProcessor::ProcessCommand(const ClientId triggeringClient, const std::wstring_view fullCmdStr,
                                                                               std::vector<std::wstring_view>& paramVector)
{
    for (auto& cmdProcessor : PluginManager::i()->userCommands)
    {
        if (cmdProcessor.expired())
        {
            continue;
        }

        if (auto task = cmdProcessor.lock()->ProcessCommand(triggeringClient, fullCmdStr, paramVector); task.has_value())
        {
            return task;
        }
    }

    return MatchCommand<commands.size()>(this, triggeringClient, fullCmdStr, paramVector);
}

concurrencpp::result<void> UserCommandProcessor::SetDieMessage(const ClientId client, const std::wstring_view param)
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
        (void)client.Message(L"Error: Invalid parameters\n"
                             L"Usage: /setdiemsg <param>\n"
                             L"<param>: 'all', 'system', 'self' or 'none'");
        co_return;
    }

    auto dieMsgVal = StringUtils::wstos(magic_enum::enum_name(dieMsg));
    Database::SaveValueOnAccount(client.GetAccount().Handle(), "dieMsg", { dieMsgVal });

    auto& info = client.GetData();
    info.dieMsg = dieMsg;
    client.SaveChar();

    // send confirmation msg
    client.Message(L"OK");

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::SetDieMessageFontSize(const ClientId client, const std::wstring_view param)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /set diemsgsize <size>\n"
                                         L"<size>: small, default";

    const auto params = StringUtils::GetParams(param, ' ');
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
        (void)client.Message(errorMsg);
        co_return;
    }

    client.GetData().dieMsgSize = dieMsgSize;
    client.SaveChar();
    client.Message(L"OK");

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::SetChatFont(const ClientId client, const std::wstring_view fontSize, const std::wstring_view fontType)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /setchatfont <size> <style>\n"
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
        (void)client.Message(errorMsg);
        co_return;
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
        (void)client.Message(errorMsg);
        co_return;
    }

    auto& info = client.GetData();
    info.chatSize = chatSize;
    info.chatStyle = chatStyle;

    client.SaveChar();

    // send confirmation msg
    client.Message(L"OK");

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::SetChatTime(const ClientId client, const std::wstring_view option)
{
    auto& info = client.GetData();
    if (option == L"on")
    {
        if (info.showChatTime)
        {
            (void)client.Message(L"Chat Time already enabled!");
        }
        else
        {
            (void)client.Message(L"Chat Time enabled");
            info.showChatTime = true;
        }
    }
    else if (option == L"off")
    {
        if (!info.showChatTime)
        {
            (void)client.Message(L"Chat Time already disabled!");
        }
        else
        {
            (void)client.Message(L"Chat Time disabled");
            info.showChatTime = false;
        }
    }
    else
    {
        (void)client.Message(L"Invalid parameter");
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::ShowLastSender(const ClientId client)
{
    const auto& info = client.GetData();
    if (info.lastPMSender.IsValidClientId())
    {
        (void)client.Message(std::format(L"Last PM sender is {}", info.lastPMSender.GetCharacterName().Handle()));
    }
    else
    {
        (void)client.Message(L"Last PM sender not found!");
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::ReplyToLastMsg(const ClientId client, StrToEnd response)
{
    const auto& info = client.GetData();
    if (info.lastPMSender.IsValidClientId())
    {
        (void)client.MessageFrom(client, response.end);
        (void)info.lastPMSender.MessageFrom(client, response.end);
        info.lastPMSender.GetData().lastPMSender = client;
    }
    else
    {
        (void)client.Message(L"Last PM sender not found!");
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::MessageTarget(const ClientId client, StrToEnd text)
{
    const auto clientShip = client.GetShip().Handle();
    const auto target = clientShip.GetTarget().Handle();
    const auto targetPlayer = target.GetPlayer().Unwrap();

    if (!targetPlayer)
    {
        (void)client.Message(L"Target is not a player");
        co_return;
    }

    client.MessageFrom(client, text.end);
    targetPlayer.MessageFrom(client, text.end);

    targetPlayer.GetData().lastPMSender = client;

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::MessageTag(const ClientId client, const std::wstring_view tag, const std::wstring_view msg)
{
    PlayerData* pd = nullptr;
    bool foundTaggedPlayer = false;
    while (pd = Players.traverse_active(pd))
    {
        auto next = ClientId(pd->clientId);
        if (client == next)
        {
            continue;
        }

        if (next.GetCharacterName().Handle().find(tag) != std::wstring_view::npos)
        {
            foundTaggedPlayer = true;
            (void)next.MessageFrom(client, msg, L"00CCFF");
        }
    }

    if (foundTaggedPlayer)
    {
        (void)client.MessageFrom(client, msg, L"00CCFF");
    }
    else
    {
        (void)client.MessageErr(L"No players with matching tag found");
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::SetSavedMsg(const ClientId client, const uint index, const StrToEnd msg)
{
    if (index > 9)
    {
        (void)client.Message(L"Invalid message index provided");
        co_return;
    }

    auto c = client.GetData().characterData;
    auto orig = c->presetMsgs.at(index);
    c->presetMsgs.at(index) = StringUtils::wstos(msg.end);
    if (AccountManager::SaveSavedMsgs(std::wstring(client.GetCharacterName().Handle()), c->presetMsgs))
    {
        client.Message(L"Ok");
    }
    else
    {
        c->presetMsgs.at(index) = orig;
        client.MessageErr(L"Error saving to database - change rolled back");
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::ShowSavedMsgs(const ClientId client)
{
    (void)client.Message(L"Saved messages");
    std::array<std::string, 10> presetMsgs = client.GetData().characterData->presetMsgs;
    for (uint counter = 0; counter < 10; counter++)
    {
        std::wstring msg = StringUtils::stows(presetMsgs.at(counter));
        (void)client.Message(StringUtils::stows(std::to_string(counter)) + L": " + msg);
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::IgnoreUser(const ClientId client, const std::wstring_view ignoredUser, const std::wstring_view flags)
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
        (void)client.Message(errorMsg);
        co_return;
    }

    const auto ignoredLower = StringUtils::ToLower(ignoredUser);

    // check if flags are valid
    for (const auto flag : flags)
    {
        if (allowedFlags.find_first_of(flag) == std::wstring::npos)
        {
            (void)client.Message(errorMsg);
            co_return;
        }
    }

    if (flags.find_first_of(L"i") == std::wstring::npos && !AccountId::GetAccountFromCharacterName(ignoredUser).has_value())
    {
        (void)client.Message(L"Target character not found");
        co_return;
    }

    auto& info = client.GetData();
    if (info.ignoreInfoList.size() >= FLHook::GetConfig()->userCommands.userCmdMaxIgnoreList)
    {
        (void)client.Message(L"Error: Too many entries in the ignore list, please delete an entry first!");
        co_return;
    }

    IgnoreInfo ii;
    ii.character = ignoredLower;
    ii.flags = flags;
    info.ignoreInfoList.push_back(ii);

    client.SaveChar();
    client.Message(L"Ok");
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::IgnoreClientId(const ClientId client, const ClientId ignoredClient, const std::wstring_view flags)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /ignoreid <id> [<flags>]\n"
                                         L"<id>: client id of character which should be ignored\n"
                                         L"<flags>: if \"p\" (without quotation marks) then only affect private chat";

    if (!ignoredClient || (!flags.empty() && flags != L"p"))
    {
        (void)client.Message(errorMsg);
        co_return;
    }

    auto& data = client.GetData();
    if (data.ignoreInfoList.size() >= FLHook::GetConfig()->userCommands.userCmdMaxIgnoreList)
    {
        (void)client.Message(L"Error: Too many entries in the ignore list, please delete an entry first!");
        co_return;
    }

    if (ignoredClient.InCharacterSelect())
    {
        (void)client.Message(L"Error: Invalid client id");
        co_return;
    }

    auto character = StringUtils::ToLower(ignoredClient.GetCharacterName().Handle());

    IgnoreInfo ii;
    ii.character = character;
    ii.flags = flags;
    data.ignoreInfoList.push_back(ii);

    client.SaveChar();

    (void)client.Message(std::format(L"OK, \"{}\" added to ignore list", character));
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::GetIgnoreList(const ClientId client)
{
    (void)client.Message(L"Id | Character Name | flags");
    int i = 1;
    auto& info = client.GetData();
    for (auto& ignore : info.ignoreInfoList)
    {
        (void)client.Message(std::format(L"{} | {} | {}", i, ignore.character, ignore.flags));
        i++;
    }

    client.Message(L"Ok");
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::RemoveFromIgnored(const ClientId client, const std::vector<std::wstring_view> charactersToRemove)
{
    static const std::wstring errorMsg = L"Error: Invalid parameters\n"
                                         L"Usage: /delignore <id> [<id2> <id3> ...]\n"
                                         L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)";
    if (charactersToRemove.empty())
    {
        (void)client.Message(errorMsg);
        co_return;
    }

    auto& info = client.GetData();
    if (charactersToRemove.front() == L"all")
    {
        info.ignoreInfoList.clear();
        client.SaveChar();
        client.Message(L"Ok");
        co_return;
    }

    std::vector<uint> idsToBeDeleted;
    for (const auto name : charactersToRemove)
    {
        uint id = StringUtils::Cast<uint>(name);
        if (!id || id > info.ignoreInfoList.size())
        {
            (void)client.Message(L"Error: Invalid Id");
            co_return;
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

    client.SaveChar();
    client.Message(L"Ok");
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::GetClientIds(const ClientId client)
{
    for (auto& next : FLHook::Clients())
    {
        client.Message(std::format(L"| {} = {}", next.characterName, next.id));
    }

    (void)client.Message(L"OK");
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::GetSelfClientId(ClientId client)
{
    client.ToastMessage(L"Your Client Id", std::format(L"Your client id is currently: {}", client));

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::Rename(const ClientId client, const std::wstring_view newName)
{
    const auto& [renameCost, cooldown] = FLHook::GetConfig()->rename;

    if (newName.empty() || newName == client.GetCharacterName().Unwrap())
    {
        client.ToastMessage(L"Bad Name", L"A new name, that is not your current one, must be provided.", ToastType::Error);
        co_return;
    }

    if (newName.find(L' ') != std::wstring_view::npos)
    {
        (void)client.MessageErr(L"No whitespaces allowed.");
        co_return;
    }

    if (newName.length() > 23)
    {
        (void)client.MessageErr(L"Name too long, max 23 characters allowed");
        co_return;
    }

    // Ban any name that is numeric and might interfere with commands
    if (const auto numeric = StringUtils::Cast<uint>(newName); numeric < 10000 && numeric != 0)
    {
        (void)client.MessageErr(L"Names that are strictly numerical must be at least 5 digits.");
        co_return;
    }

    if (renameCost)
    {
        if (client.GetCash().Unwrap() < renameCost)
        {
            (void)client.MessageErr(L"Insufficient money!");
            co_return;
        }
    }

    if (cooldown)
    {
        const auto charData = client.GetData().characterData;
        if (auto lastRename = charData->characterDocument.find("lastRenameTimestamp"); lastRename != charData->characterDocument.end())
        {
            const auto cooldownAsDays = std::chrono::days(cooldown);
            if (const std::chrono::milliseconds endOfCooldown =
                    lastRename->get_date().value + std::chrono::duration_cast<std::chrono::milliseconds>(cooldownAsDays);
                endOfCooldown.count() > TimeUtils::UnixTime<std::chrono::milliseconds>())
            {
                (void)client.MessageErr(std::format(L"Rename cooldown not elapsed yet, end of cooldown: {}",
                                                    TimeUtils::AsDate(std::chrono::duration_cast<std::chrono::seconds>(endOfCooldown))));
                co_return;
            }
        }
    }

    std::wstring newNameStr = newName.data();

    auto errMsg = co_await AccountManager::CheckCharnameTaken(client, newNameStr);
    if (!errMsg.empty())
    {
        (void)client.MessageErr(errMsg);
        co_return;
    }
    std::wstring currName = client.GetCharacterName().Handle().data();

    if (renameCost)
    {
        if (client.GetCash().Handle() < renameCost)
        {
            (void)client.MessageErr(L"Insufficient money!");
            co_return;
        }

        (void)client.RemoveCash(renameCost);
    }

    client.Message(L"Renaming, you will be kicked.");
    co_await FLHook::GetTaskScheduler()->Delay(5s);
    client.Kick();
    co_await FLHook::GetTaskScheduler()->Delay(0.5s);
    co_await AccountManager::Rename(currName, newNameStr);
}

concurrencpp::result<void> UserCommandProcessor::InvitePlayer(const ClientId client, const ClientId otherClient)
{
    client.InvitePlayer(otherClient);

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::LeaveGroup(const ClientId client)
{
    auto group = client.GetGroup();
    if (group.HasError())
    {
        client.MessageErr(L"You are not currently in a group.");
        co_return;
    }

    group.Value().RemoveMember(client);
    client.Message(std::format(L"Group {} left", group.Unwrap().GetValue()));
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::FactionInvite(const ClientId client, const std::wstring_view factionTag)
{

    bool msgSent = false;

    if (factionTag.size() < 3)
    {
        (void)client.MessageErr(L"Invalid parameters. Usage: /factioninvite <tag> or /fi ...");
        co_return;
    }
    std::wstring factionTagLower = StringUtils::ToLower(std::wstring(factionTag));

    for (const auto& player : FLHook::Clients())
    {
        if (StringUtils::ToLower(player.characterName).find(factionTagLower) == std::wstring::npos)
        {
            continue;
        }

        if (player.id == client)
        {
            continue;
        }

        InvitePlayer(client, player.id);
        msgSent = true;
    }

    if (msgSent == false)
    {
        (void)client.MessageErr(L"No chars found");
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::TransferCharacter(const ClientId client, const std::wstring_view cmd, const std::wstring_view param1,
                                                                   const std::wstring_view param2)
{
    const auto db = FLHook::GetDbClient();
    if (cmd == L"clearcode")
    {
        const std::wstring charName = client.GetCharacterName().Handle().data();
        if (co_await AccountManager::ClearCharacterTransferCode(charName))
        {
            (void)client.Message(L"Character transfer code cleared");
        }
        else
        {
            (void)client.MessageErr(L"Failed to clear character transfer code");
        }
    }
    else if (cmd == L"setcode")
    {
        const std::wstring charName = client.GetCharacterName().Handle().data();
        if (param1.size() == 0)
        {
            (void)client.Message(L"Usage: /transferchar setcode <code>");
            co_return;
        }
        const std::wstring newCharCode = param1.data();
        if (co_await AccountManager::SetCharacterTransferCode(charName, newCharCode))
        {
            (void)client.Message(L"Character transfer code set");
        }
        else
        {
            (void)client.MessageErr(L"Failed to set character transfer code");
        }
    }
    else if (cmd == L"transfer")
    {
        if (client.GetData().account->characters.size() >= 5) // TODO: This number is WRONG
        {
            (void)client.Message(L"This account cannot hold more characters");
            co_return;
        }

        if (param1.size() == 0 || param2.size() == 0)
        {
            (void)client.Message(L"Usage: /transferchar transfer <char> <code>");
            co_return;
        }

        const AccountId accountId = client.GetAccount().Handle();

        const std::wstring charName = std::wstring(param1);
        const std::wstring charCode = std::wstring(param2);

        if (const auto err = co_await AccountManager::TransferCharacter(accountId, charName, charCode); !err.empty())
        {
            (void)client.MessageErr(err);
        }
        else
        {
            (void)client.Kick(L"Transferring the character.", 3);
        }
    }
    else
    {
        client.Message(L"Usage: /transferchar [setcode <code>|transfer <char> <code>|clearcode]");
    }

    co_return;
}

/*concurrencpp::result<void>UserCommandProcessor::DeleteMail(const std::wstring_view mailID, const std::wstring_view readOnlyDel)
{
    if (mailID == L"all")
    {
        const auto count = MailManager::i()->PurgeAllMail(client, readOnlyDel == L"readonly");
        if (count.has_error())
        {
            client.Message(std::format(L"Error deleting mail: {}", count.error()));
            co_return;
        }

        client.Message(std::format(L"Deleted {} mail", count.value()));
    }
    else
    {
        const auto index = StringUtils::Cast<int64>(mailID);
        if (const auto err = MailManager::i()->DeleteMail(client, index); err.has_error())
        {
            client.Message(std::format(L"Error deleting mail: {}", err.error()));
            co_return;
        }

        client.Message(L"Mail deleted");
    }
}

concurrencpp::result<void>UserCommandProcessor::ReadMail(uint mailId)
{
    if (mailId <= 0)
    {
        client.Message(L"Id was not provided or was invalid.");
        co_return;
    }

    const auto mail = MailManager::i()->GetMailById(client, mailId);
    if (mail.has_error())
    {
        client.Message(std::format(L"Error retreiving mail: {}", mail.error()));
        co_return;
    }

    const auto& item = mail.value();
    client.Message(std::format(L"From: {}", item.author));
    client.Message(std::format(L"Subject: {}", item.subject));
    client.Message(std::format(L"Date: {:%F %T}", TimeUtils::UnixToSysTime(item.timestamp)));
    client.Message(item.body);
}

concurrencpp::result<void>UserCommandProcessor::ListMail(int pageNumber, std::wstring_view unread)
{

    if (pageNumber <= 0)
    {
        client.Message(L"Page was not provided or was invalid.");
        co_return;
    }

    const bool unreadOnly = (unread == L"unread");

    const auto mail = MailManager::i()->GetMail(client, unreadOnly, pageNumber);
    if (mail.has_error())
    {
        client.Message(std::format(L"Error retrieving mail: {}", mail.error()));
        co_return;
    }

    const auto& mailList = mail.value();
    if (mailList.empty())
    {
        client.Message(L"You have no mail.");
        co_return;
    }

    client.Message(std::format(L"Printing mail of page {}", mailList.size()));
    for (const auto& item : mailList)
    {
        // |    Id.) Subject (unread) - Author - Time
        client.Message(std::format(
            L"|    {}.) {} {}- {} - {:%F %T}", item.id, item.subject, item.unread ? L"(unread) " : L"", item.author, TimeUtils::UnixToSysTime(item.timestamp)));
    }
}*/

// TODO: Implement GiveCash Target and by ID
concurrencpp::result<void> UserCommandProcessor::GiveCash(const ClientId client, const std::wstring_view characterName, const std::wstring_view amount)
{
    // TODO: resolve sending money to offline people
    const auto cash = StringUtils::MultiplyUIntBySuffix(amount);
    const auto targetPlayer = ClientId(characterName);
    const auto clientCash = client.GetCash().Unwrap();

    if (client == targetPlayer)
    {
        (void)client.Message(L"Not sure this really accomplishes much... (Don't give cash to yourself.)");
        co_return;
    }

    if (cash == 0)
    {
        (void)client.Message(std::format(L"Err: Invalid cash amount."));
        co_return;
    }

    if (clientCash < cash)
    {
        (void)client.Message(std::format(L"Err: You do not have enough cash, you only have {}, and are trying to give {}.", clientCash, cash));
        co_return;
    }

    client.RemoveCash(cash).Handle();
    targetPlayer.AddCash(cash).Handle();

    client.SaveChar().Handle();
    targetPlayer.SaveChar().Handle();

    co_return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

concurrencpp::result<void> UserCommandProcessor::Time(const ClientId client)
{
    (void)client.Message(std::format(L"{:%Y-%m-%d %X}", std::chrono::system_clock::now()));

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::Help(const ClientId client, std::optional<int> optPage)
{
    constexpr int itemsPerPage = 20;
    const auto& pm = PluginManager::i();

    struct ModuleInfo
    {
            const AbstractUserCommandProcessor* processor;
            const Plugin* plugin;
            int startingCommandIndex;
            int endingCommandIndex;
    };

    // list of pointers and their starting command index
    std::vector<ModuleInfo> processors;
    processors.reserve(pm->plugins.size());

    // Add core and set starting indexes
    int commandIndex = commands.size();
    int totalCommands = commands.size();
    processors.emplace_back(this, nullptr, 0, commandIndex);

    for (const auto& plugin : pm->plugins)
    {
        if (const auto cmdProcessor = dynamic_cast<const AbstractUserCommandProcessor*>(plugin.get()); cmdProcessor)
        {
            const auto& cmds = cmdProcessor->GetUserCommands();
            totalCommands += cmds.size();
            processors.emplace_back(cmdProcessor, plugin.get(), commandIndex, commandIndex + cmds.size());
            commandIndex += cmds.size();
        }
    }

    // Divide the total commands against items per page, rounding up
    const auto div = std::div(totalCommands, itemsPerPage);
    const int totalPages = std::clamp(div.rem ? div.quot + 1 : div.quot, 1, 100);
    int page = optPage.value_or(0);
    if (page > totalPages)
    {
        page = totalPages;
    }
    else if (page < 1)
    {
        page = 1;
    }

    client.Message(std::format(L"Displaying help commands, page {} of {}", page, totalPages));
    for (const int startingIndex = commandIndex = itemsPerPage * (page - 1); const auto& info : processors)
    {
        if (commandIndex > startingIndex + itemsPerPage)
        {
            break;
        }

        if (info.endingCommandIndex < commandIndex)
        {
            continue;
        }

        for (auto& cmds = info.processor->GetUserCommands();
             auto& cmd : cmds | std::ranges::views::drop(std::clamp(commandIndex - info.startingCommandIndex, 0, INT_MAX)))
        {
            if (++commandIndex > startingIndex + itemsPerPage)
            {
                break;
            }

            std::wstring_view description = std::get<2>(cmd);
            std::wstring_view usage = std::get<1>(cmd);

            client.Message(std::format(L"{} - {}", usage, description));
        }
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::DropRep(const ClientId client)
{
    const auto& config = FLHook::GetConfig()->reputation;

    if (config.creditCost)
    {
        if (const uint cash = client.GetCash().Handle(); cash < config.creditCost)
        {
            (void)client.ToastMessage(L"Cannot Afford", std::format(L"Not enough money, {} credits required", config.creditCost), ToastType::Error);
            co_return;
        }
        (void)client.RemoveCash(config.creditCost);
    }
    const auto playerRep = client.GetReputation().Handle();
    const auto playerAffliation = playerRep.GetAffiliation().Handle();
    if (playerAffliation.GetValue() == -1)
    {
        (void)client.ToastMessage(L"Cannot drop!", L"No affiliation, can't drop one");
        co_return;
    }

    (void)playerRep.SetAttitudeTowardsRepGroupId(playerAffliation, 0.3f);

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::Value(const ClientId client)
{
    (void)client.ToastMessage(L"Total Ship Value", std::format(L"Your worth is ${} credits", client.GetValue()));

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::Pos(const ClientId client)
{
    const auto clientShip = client.GetShip().Handle();
    auto [pos, orientation] = clientShip.GetPositionAndOrientation().Handle();
    Vector erot = orientation.ToEuler(true);
    (void)client.Message(std::format(L"Position {:0.0f} {:0.0f} {:0.0f} Orient {:0.0f} {:0.0f} {:0.0f}", pos.x, pos.y, pos.z, erot.x, erot.y, erot.z));
    co_return;
}

concurrencpp::result<void> UserCommandProcessor::Dice(const ClientId client, uint sidesOfDice)
{
    // TODO: Support for more than 1 dice, e.g. 6d10
    if (!sidesOfDice)
    {
        sidesOfDice = 6;
    }

    uint result = Random::Uniform(1u, sidesOfDice);
    const std::wstring msg = std::format(L"{} has rolled {} out of {}", client.GetCharacterName().Handle(), result, sidesOfDice);

    if (client.IsDocked())
    {
        client.ToastMessage(std::format(L"1d{} Dice Roll", sidesOfDice), msg);
    }
    else
    {
        client.MessageLocal(msg);
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::Coin(const ClientId client)
{
    const uint result = Random::Uniform(0u, 1u);

    const auto msg = std::format(L"{} tossed a coin, it landed on {}", client.GetCharacterName().Handle(), result ? L"heads" : L"tails");

    if (client.IsDocked())
    {
        client.ToastMessage(L"Coin Toss", msg);
    }
    else
    {
        client.MessageLocal(msg);
    }

    co_return;
}

concurrencpp::result<void> UserCommandProcessor::MarkTarget(const ClientId client)
{
    const auto ship = client.GetShip().Handle();
    const auto group = client.GetGroup().Handle();
    const auto target = ship.GetTarget().Handle();
    const auto otherPlayer = target.GetPlayer().Unwrap();

    if (!otherPlayer)
    {
        (void)client.MessageErr(L"Target not a player");
        co_return;
    }

    uint targetShip = target.GetId().Unwrap().GetValue();

    auto charName = client.GetCharacterName().Handle();
    auto targetName = otherPlayer.GetCharacterName().Handle();

    // std::wstring message1 = std::format(L"Target: {}", targetName);
    const std::wstring message2 = std::format(L"{} has set {} as group target.", charName, targetName);

    // std::wstring_view msgView1 = message1;
    std::wstring_view msgView2 = message2;

    FmtStr caption(0, nullptr);
    caption.begin_mad_lib(526999);
    caption.end_mad_lib();
    group.ForEachGroupMember(
        [msgView2, targetShip](const ClientId& groupMemberClientId)
        {
            if (const uint groupMemShip = Players[groupMemberClientId.GetValue()].shipId; !groupMemShip || groupMemShip == targetShip)
            {
                return std::nullopt;
            }

            (void)groupMemberClientId.Message(msgView2);
            // TODO: Reintroduce when IDS override clienthook is made
            // HkChangeIDSString(gm->iClientID, 526999, message1);

            // pub::Player::DisplayMissionMessage(groupMemberClientId.GetValue(), caption, MissionMessageType::MissionMessageType_Type2, true);

            if (groupMemberClientId.GetData().markedTarget)
            {
                pub::Player::MarkObj(groupMemberClientId.GetValue(), groupMemberClientId.GetData().markedTarget, 0);
            }

            pub::Player::MarkObj(groupMemberClientId.GetValue(), targetShip, 1);
            groupMemberClientId.GetData().markedTarget = targetShip;

            return std::nullopt;
        },
        false);

    (void)client.Message(L"OK");

    co_return;
}
