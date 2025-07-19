#pragma once

#include "AbstractUserCommandProcessor.hpp"

class UserCommandProcessor final : public Singleton<UserCommandProcessor>, public AbstractUserCommandProcessor
{
        concurrencpp::result<void> GetClientIds(ClientId client);
        concurrencpp::result<void> SetDieMessage(ClientId client, std::wstring_view param);
        concurrencpp::result<void> SetDieMessageFontSize(ClientId client, std::wstring_view param);
        concurrencpp::result<void> SetChatFont(ClientId client, std::wstring_view fontSize, std::wstring_view fontType);
        concurrencpp::result<void> SetChatTime(ClientId client, std::wstring_view option);
        concurrencpp::result<void> ShowLastSender(ClientId client);
        concurrencpp::result<void> ShowSavedMsgs(ClientId client);
        concurrencpp::result<void> SetSavedMsg(ClientId client, uint index, std::wstring_view msg);
        concurrencpp::result<void> ReplyToLastMsg(ClientId client, std::wstring_view text);
        concurrencpp::result<void> MessageTarget(ClientId client, std::wstring_view text);
        concurrencpp::result<void> MessageTag(ClientId client, std::wstring_view tag, std::wstring_view msg);
        concurrencpp::result<void> IgnoreUser(ClientId client, std::wstring_view ignoredUser, std::wstring_view flags);
        concurrencpp::result<void> IgnoreClientId(ClientId client, ClientId ignoredClient, std::wstring_view flags);
        concurrencpp::result<void> GetIgnoreList(ClientId client);
        concurrencpp::result<void> RemoveFromIgnored(ClientId client, std::vector<std::wstring_view> charactersToRemove);
        concurrencpp::result<void> GetSelfClientId(ClientId client);
        concurrencpp::result<void> MarkTarget(ClientId client);
        concurrencpp::result<void> Rename(ClientId client, std::wstring_view newName);
        concurrencpp::result<void> InvitePlayer(ClientId client, ClientId otherClient);
        concurrencpp::result<void> LeaveGroup(ClientId client);
        concurrencpp::result<void> FactionInvite(ClientId client, std::wstring_view factionTag);
        concurrencpp::result<void> TransferCharacter(ClientId client, std::wstring_view cmd, std::wstring_view param1, std::wstring_view param2);
        // concurrencpp::result<void>DeleteMail(std::wstring_view mailID, std::wstring_view readOnlyDel);
        // concurrencpp::result<void>ReadMail(uint mailId);
        // concurrencpp::result<void>ListMail(int pageNumber, std::wstring_view unread);
        concurrencpp::result<void> GiveCash(ClientId client, std::wstring_view characterName, std::wstring_view amount);
        concurrencpp::result<void> Time(ClientId client);
        concurrencpp::result<void> Dice(ClientId client, uint sidesOfDice);
        concurrencpp::result<void> Coin(ClientId client);
        concurrencpp::result<void> Value(ClientId client);
        concurrencpp::result<void> DropRep(ClientId client);
        concurrencpp::result<void> Help(ClientId client, int page);

        // clang-format off
        inline static const std::array<CommandInfo<UserCommandProcessor>, 28> commands = {
            {
                AddCommand(UserCommandProcessor, Cmds( L"/id"sv ), GetSelfClientId, L"/id", L"Prints your client id"),
                AddCommand(UserCommandProcessor, Cmds( L"/ids"sv ), GetClientIds, L"/ids", L"Lists all the players and their internal client id numbers."),
                AddCommand(UserCommandProcessor, Cmds( L"/setdiemsgsize"sv ), SetDieMessageFontSize, L"/setdiemsgsize [option]",
                    L"Sets the text size of death chatConfig. Use without parameters to see available options."),
                AddCommand(UserCommandProcessor, Cmds( L"/setdiemsg"sv ), SetDieMessage, L"/setdiemsg [option]",
                    L"Change the scope of who's death messages you can see. Use without parameters for available options."),
                AddCommand(UserCommandProcessor, Cmds( L"/setchatfont"sv ), SetChatFont, L"/setchatfont [option]",
                    L"Sets the font style and size of the chat. Use without options for examples."),
                AddCommand(UserCommandProcessor, Cmds( L"/setchattime"sv ), SetChatTime, L"/setchattime [on|off]",
                    L"Adds a timestamp in front of all received chat messages"),
                AddCommand(UserCommandProcessor, Cmds( L"/lastpm"sv ), ShowLastSender, L"/lastpm",
                    L"Shows the name of the last received private message sender"),
                AddCommand(UserCommandProcessor, Cmds( L"/t"sv ), MessageTarget, L"/t <message>",
                    L"Sends the message to the player flying the selected ship"),
                AddCommand(UserCommandProcessor, Cmds( L"/fm"sv ), MessageTag, L"/fm <tag> <message>",
                    L"Sends the message to all players with defined tag"),
                AddCommand(UserCommandProcessor, Cmds( L"/reply"sv ), ReplyToLastMsg, L"/reply <message>",
                    L"Sends the provided message back to the sender of the last received private message"),
                AddCommand(UserCommandProcessor, Cmds( L"/setmsg"sv ), SetSavedMsg, L"/setmsg <0-9> <msg>", L"Saves provided message under the defined slot"),
                AddCommand(UserCommandProcessor, Cmds( L"/showmsgs"sv ), ShowSavedMsgs, L"/showmsgs", L"Lists all currently saved messages"),
                AddCommand(UserCommandProcessor, Cmds( L"/ignore"sv ), IgnoreUser, L"/ignore <name>", L"blocks any message sent by player specified"),
                AddCommand(UserCommandProcessor, Cmds( L"/getignorelist"sv ), GetIgnoreList, L"/getignorelist", L"prints the users you currently have ignored"),
                AddCommand(UserCommandProcessor, Cmds( L"/unignore"sv ), RemoveFromIgnored, L"/unignore <name ...>",
                    L"removes specified names from ignore list, typing /unignore all removes ignore list entierly."),
                AddCommand(UserCommandProcessor, Cmds( L"/invite"sv ), InvitePlayer, L"/invite <name/client id>", L"invites specified player to group"),
                AddCommand(UserCommandProcessor, Cmds( L"/leave"sv ), LeaveGroup, L"/leave", L"Leave your current group"),
                AddCommand(UserCommandProcessor, Cmds( L"/fi", L"/finvite"sv ), FactionInvite, L"/fi <prefix>", L"invites players that matches the listed prefix in their name"),
                //AddCommand(UserCommandProcessor, L"/delmail", DeleteMail, L"/delmail <id>", L"deletes specified mail"),
                //AddCommand(UserCommandProcessor, L"/readmail", ReadMail, L"/readmail <id>", L"prints specified mail."),
                //AddCommand(UserCommandProcessor, L"/listmail", ListMail, L"/listmail [page]", L"lists the mails of the specified page."),
                AddCommand(UserCommandProcessor, Cmds( L"/mark"sv ), MarkTarget, L"/mark", L"Marks the selected ship for all group members"),
                AddCommand(UserCommandProcessor, Cmds( L"/givecash", L"/gc", L"/sendcash", L"/sc"), GiveCash, L"/givecash <target> <amount>", L"gives specified amount of cash to target"),
                AddCommand(UserCommandProcessor, Cmds( L"/time"sv ), Time, L"/time", L"Prints current time"),
                AddCommand(UserCommandProcessor, Cmds( L"/rename"sv ), Rename, L"/rename <newName>", L"Renames the character. Kicks you upon completion."),
                AddCommand(UserCommandProcessor, Cmds( L"/value"sv ), Value, L"/value", L"Prints the current total worth of the character"),
                AddCommand(UserCommandProcessor, Cmds( L"/coin"sv ), Coin, L"/coin", L"Tosses a coin, heads or tails."),
                AddCommand(UserCommandProcessor, Cmds( L"/dice"sv ), Dice, L"/dice [numOfSides]", L"Rolls the dice with specified amount of sides, 6 if unspecified"),
                AddCommand(UserCommandProcessor, Cmds( L"/droprep"sv ), DropRep, L"/droprep", L"Removes your affiliation if you have one"),
                AddCommand(UserCommandProcessor, Cmds( L"/transferchar"sv ), TransferCharacter, L"/transferchar [setcode <code>|transfer <char> <code>|clearcode]", L"Set the character move code, or use the previously set code to transfer said character into current account"),
                AddCommand(UserCommandProcessor, Cmds( L"/help"sv, L"/h"sv, L"/?"sv), Help, L"/help [page]", L"Provides indepth help information")
            }
        };
        // clang-format on
        GetUserCommandsFunc(commands);

        template <int N>
        std::optional<concurrencpp::result<void>> MatchCommand(UserCommandProcessor* processor, ClientId triggeringClient, const std::wstring_view cmd,
                                                               std::vector<std::wstring_view>& paramVector)
        {
            const CommandInfo<UserCommandProcessor>& command = std::get<N - 1>(commands);
            for (const auto str : command.cmd)
            {
                if (cmd.starts_with(std::wstring(str) + L' ') || cmd == str)
                {
                    const auto countVal = std::ranges::count(str, L' ');
                    paramVector.erase(paramVector.begin() + 1, paramVector.begin() + std::clamp(countVal + 2, 2, 6));
                    return command.func(processor, paramVector);
                }
            }

            return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        std::optional<concurrencpp::result<void>> MatchCommand<0>(UserCommandProcessor* processor, ClientId triggeringClient, std::wstring_view fullCmdString,
                                                                  std::vector<std::wstring_view>& paramVector);

    public:
        std::optional<concurrencpp::result<void>> ProcessCommand(ClientId triggeringClient, const std::wstring_view fullCmdStr,
                                                                 std::vector<std::wstring_view>& paramVector) override;
        std::optional<concurrencpp::result<void>> ProcessCommand(ClientId triggeringClient, std::wstring_view clientStr, std::wstring_view commandStr);
};
