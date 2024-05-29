#pragma once

#include "AbstractUserCommandProcessor.hpp"

class UserCommandProcessor final : public Singleton<UserCommandProcessor>, public AbstractUserCommandProcessor
{
        void GetClientIds();
        void SetDieMessage(std::wstring_view param);
        void SetDieMessageFontSize(std::wstring_view param);
        void SetChatFont(std::wstring_view fontSize, std::wstring_view fontType);
        void SetChatTime(std::wstring_view option);
        void ShowLastSender();
        void ShowSavedMsgs();
        void SetSavedMsg(uint index, std::wstring_view msg);
        void ReplyToLastMsg(std::wstring_view text);
        void MessageTarget(std::wstring_view text);
        void MessageTag(std::wstring_view tag, std::wstring_view msg);
        void IgnoreUser(std::wstring_view ignoredUser, std::wstring_view flags);
        void IgnoreClientId(ClientId ignoredClient, std::wstring_view flags);
        void GetIgnoreList();
        void RemoveFromIgnored(std::vector<std::wstring_view> charactersToRemove);
        void GetSelfClientId();
        void Rename(std::wstring_view newName);
        void InvitePlayer(const std::wstring_view& characterName);
        void InvitePlayerByName(std::wstring_view invitee);
        void InvitePlayerById(ClientId inviteeId);
        void FactionInvite(std::wstring_view factionTag);
        void TransferCharacter(std::wstring_view cmd, std::wstring_view param1, std::wstring_view param2);
        // void DeleteMail(std::wstring_view mailID, std::wstring_view readOnlyDel);
        // void ReadMail(uint mailId);
        // void ListMail(int pageNumber, std::wstring_view unread);
        void GiveCash(std::wstring_view characterName, std::wstring_view amount);
        void GiveCashById(ClientId targetClient, std::wstring_view amount);
        void Time();
        void Dice(uint sidesOfDice);
        void Coin();
        void Value();
        void DropRep();
        void Help(std::wstring_view module, std::wstring_view command);

        // clang-format off
        inline static const std::array<CommandInfo<UserCommandProcessor>, 33> commands = {
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
             AddCommand(UserCommandProcessor, Cmds( L"getignorelist"sv ), GetIgnoreList, L"/getignorelist", L"prints the users you currently have ignored"),
             AddCommand(UserCommandProcessor, Cmds( L"/unignore"sv ), RemoveFromIgnored, L"/unignore <name ...>",
             L"removes specified names from ignore list, typing /unignore all removes ignore list entierly."),
             AddCommand(UserCommandProcessor, Cmds( L"/invite"sv ), InvitePlayerByName, L"/invite <name>", L"invites specified player to group"),
             AddCommand(UserCommandProcessor, Cmds( L"/invite$"sv ), InvitePlayerById, L"/invite$ <id>", L"invites specified player to group by client id"),
             AddCommand(UserCommandProcessor, Cmds( L"/finv"sv ), FactionInvite, L"/finv <prefix>", L"invites players that matches the listed prefix in their name"),
             //AddCommand(UserCommandProcessor, L"/delmail", DeleteMail, L"/delmail <id>", L"deletes specified mail"),
             //AddCommand(UserCommandProcessor, L"/readmail", ReadMail, L"/readmail <id>", L"prints specified mail."),
             //AddCommand(UserCommandProcessor, L"/listmail", ListMail, L"/listmail [page]", L"lists the mails of the specified page."),
             AddCommand(UserCommandProcessor, Cmds( L"/givecash"sv ), GiveCash, L"/givecash <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, Cmds( L"/gc"sv ), GiveCash, L"/gc <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, Cmds( L"/givecash$"sv ), GiveCashById, L"/givecash <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, Cmds( L"/gc$"sv ), GiveCashById, L"/gc$ <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, Cmds( L"/sendcash"sv ), GiveCash, L"/sendcash <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, Cmds( L"/sc"sv ), GiveCash, L"/sc <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, Cmds( L"/sendcash$"sv ), GiveCashById, L"/sendcash <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, Cmds( L"/sc$"sv ), GiveCashById, L"/sc$ <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, Cmds( L"/time"sv ), Time, L"/time", L"Prints current time"),
             AddCommand(UserCommandProcessor, Cmds( L"/rename"sv ), Rename, L"/rename <newName>", L"Renames the character. Kicks you upon completion."),
             AddCommand(UserCommandProcessor, Cmds( L"/value"sv ), Value, L"/value", L"Prints the current total worth of the character"),
             AddCommand(UserCommandProcessor, Cmds( L"/coin"sv ), Coin, L"/coin", L"Tosses a coin, heads or tails."),
             AddCommand(UserCommandProcessor, Cmds( L"/dice"sv ), Dice, L"/dice [numOfSides]", L"Rolls the dice with specified amount of sides, 6 if undefined"),
             AddCommand(UserCommandProcessor, Cmds( L"/droprep"sv ), DropRep, L"/droprep", L"Removes your affiliation if you have one"),
             AddCommand(UserCommandProcessor, Cmds( L"/transferchar"sv ), TransferCharacter, L"/transferchar", L"Set the character move code, or use the previously set code to transfer said character into current account"),
             AddCommand(UserCommandProcessor, Cmds( L"/help"sv, L"/h"sv, L"?"sv), Help, L"/help [module] [command]", L"Provides indepth help information")}
        };
        // clang-format on
        GetUserCommandsFunc(commands);

        template <int N>
        bool MatchCommand(UserCommandProcessor* processor, ClientId triggeringClient, const std::wstring_view cmd, std::vector<std::wstring>& paramVector)
        {
            const CommandInfo<UserCommandProcessor>& command = std::get<N - 1>(commands);
            for (const auto str : command.cmd)
            {
                if (str == cmd)
                {
                    command.func(processor, paramVector);
                    return true;
                }
            }

            return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        bool MatchCommand<0>(UserCommandProcessor* processor, ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring>& paramVector);

    public:
        bool ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring>& paramVector) override;
        bool ProcessCommand(ClientId triggeringClient, std::wstring_view commandStr);
};
