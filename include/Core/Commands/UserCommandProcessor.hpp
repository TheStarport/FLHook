#pragma once

#include "AbstractUserCommandProcessor.hpp"

class UserCommandProcessor final : public Singleton<UserCommandProcessor>, public AbstractUserCommandProcessor
{
        Task GetClientIds(ClientId client);
        Task SetDieMessage(ClientId client, std::wstring_view param);
        Task SetDieMessageFontSize(ClientId client, std::wstring_view param);
        Task SetChatFont(ClientId client, std::wstring_view fontSize, std::wstring_view fontType);
        Task SetChatTime(ClientId client, std::wstring_view option);
        Task ShowLastSender(ClientId client);
        Task ShowSavedMsgs(ClientId client);
        Task SetSavedMsg(ClientId client, uint index, std::wstring_view msg);
        Task ReplyToLastMsg(ClientId client, std::wstring_view text);
        Task MessageTarget(ClientId client, std::wstring_view text);
        Task MessageTag(ClientId client, std::wstring_view tag, std::wstring_view msg);
        Task IgnoreUser(ClientId client, std::wstring_view ignoredUser, std::wstring_view flags);
        Task IgnoreClientId(ClientId client, ClientId ignoredClient, std::wstring_view flags);
        Task GetIgnoreList(ClientId client);
        Task RemoveFromIgnored(ClientId client, std::vector<std::wstring_view> charactersToRemove);
        Task GetSelfClientId(ClientId client);
        Task MarkTarget(ClientId client);
        Task Rename(ClientId client, std::wstring_view newName);
        Task InvitePlayer(ClientId client, ClientId otherClient);
        Task FactionInvite(ClientId client, std::wstring_view factionTag);
        Task TransferCharacter(ClientId client, std::wstring_view cmd, std::wstring_view param1, std::wstring_view param2);
        // Task DeleteMail(std::wstring_view mailID, std::wstring_view readOnlyDel);
        // Task ReadMail(uint mailId);
        // Task ListMail(int pageNumber, std::wstring_view unread);
        Task GiveCash(ClientId client, std::wstring_view characterName, std::wstring_view amount);
        Task Time(ClientId client);
        Task Dice(ClientId client, uint sidesOfDice);
        Task Coin(ClientId client);
        Task Value(ClientId client);
        Task DropRep(ClientId client);
        Task Help(ClientId client, int page);

        // clang-format off
        inline static const std::array<CommandInfo<UserCommandProcessor>, 27> commands = {
            {AddCommand(UserCommandProcessor, Cmds( L"/ids"sv ), GetClientIds, L"/ids", L"Lists all the players and their internal client id numbers."),
             AddCommand(UserCommandProcessor, Cmds( L"/setdiemsgsize"sv ), SetDieMessageFontSize, L"/setdiemsgsize [option]",
             L"Sets the text size of death chatConfig. Use without parameters to see available options."),
             AddCommand(UserCommandProcessor, Cmds( L"/setdiemsg"sv ), SetDieMessage, L"/setdiemsg [option]",
             L" Change the scope of who's death messages you can see. Use without parameters for available options."),
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
             AddCommand(UserCommandProcessor, Cmds( L"/fi", L"/finv"sv ), FactionInvite, L"/finv <prefix>", L"invites players that matches the listed prefix in their name"),
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
             AddCommand(UserCommandProcessor, Cmds( L"/transferchar"sv ), TransferCharacter, L"/transferchar", L"Set the character move code, or use the previously set code to transfer said character into current account"),
             AddCommand(UserCommandProcessor, Cmds( L"/help"sv, L"/h"sv, L"/?"sv), Help, L"/help [page]", L"Provides indepth help information")
            }
        };
        // clang-format on
        GetUserCommandsFunc(commands);

        template <int N>
        std::optional<Task> MatchCommand(UserCommandProcessor* processor, ClientId triggeringClient, const std::wstring_view cmd,
                                         std::vector<std::wstring_view>& paramVector)
        {
            const CommandInfo<UserCommandProcessor>& command = std::get<N - 1>(commands);
            for (const auto str : command.cmd)
            {
                if (cmd.starts_with(str))
                {
                    paramVector.erase(paramVector.begin(), paramVector.begin() + std::clamp(std::ranges::count(str, L' '), 1, 5));
                    return command.func(processor, paramVector);
                }
            }

            return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        std::optional<Task> MatchCommand<0>(UserCommandProcessor* processor, ClientId triggeringClient, std::wstring_view fullCmdString,
                                            std::vector<std::wstring_view>& paramVector);

    public:
        std::optional<Task> ProcessCommand(ClientId triggeringClient, const std::wstring_view fullCmdStr, std::vector<std::wstring_view>& paramVector) override;
        std::optional<Task> ProcessCommand(ClientId triggeringClient, std::wstring_view clientStr, std::wstring_view commandStr);
};
