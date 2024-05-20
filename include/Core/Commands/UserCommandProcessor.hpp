#pragma once

#include "AbstractUserCommandProcessor.hpp"

class UserCommandProcessor final : public Singleton<UserCommandProcessor>, public AbstractUserCommandProcessor
{
        void GetClientIds();
        void SetDieMessage(std::wstring_view param);
        void SetDieMessageFontSize(std::wstring_view param);
        void SetChatFont(std::wstring_view fontSize, std::wstring_view fontType);
        void IgnoreUser(std::wstring_view ignoredUser, std::wstring_view flags);
        void IgnoreClientId(ClientId ignoredClient, std::wstring_view flags);
        void GetIgnoreList();
        void RemoveFromIgnored(std::vector<std::wstring_view> charactersToRemove);
        void GetSelfClientId();
        void InvitePlayer(const std::wstring_view& characterName);
        void InvitePlayerByName(std::wstring_view invitee);
        void InvitePlayerById(ClientId inviteeId);
        void FactionInvite(std::wstring_view factionTag);
        // void DeleteMail(std::wstring_view mailID, std::wstring_view readOnlyDel);
        // void ReadMail(uint mailId);
        // void ListMail(int pageNumber, std::wstring_view unread);
        void GiveCash(std::wstring_view characterName, std::wstring_view amount);
        void GiveCashById(ClientId targetClient, std::wstring_view amount);
        void Help(std::wstring_view module, std::wstring_view command);

        // clang-format off
        constexpr static std::array<CommandInfo<UserCommandProcessor>, 19> commands = {
            {AddCommand(UserCommandProcessor, L"/ids", GetClientIds, L"/ids", L"Lists all the players and their internal client id numbers."),
             AddCommand(UserCommandProcessor, L"/setdiemsgsize", SetDieMessageFontSize, L"/setdiemsgsize [option]",
             L"Sets the text size of death chatConfig. Use without parameters to see available options."),
             AddCommand(UserCommandProcessor, L"/setdiemsg", SetDieMessage, L"/setdiemsg [option]",
             L" Change the scope of who's death messages you can see. Use without parameters for available options."),
             AddCommand(UserCommandProcessor, L"/setchatfont", SetChatFont, L"/setchatfont [option]",
             L"Sets the font style and size of the chat. Use without options for examples."),
             AddCommand(UserCommandProcessor, L"/ignore", IgnoreUser, L"/ignore <name>", L"blocks any message sent by player specified"),
             AddCommand(UserCommandProcessor, L"getignorelist", GetIgnoreList, L"/getignorelist", L"prints the users you currently have ignored"),
             AddCommand(UserCommandProcessor, L"/unignore", RemoveFromIgnored, L"/unignore <name ...>",
             L"removes specified names from ignore list, typing /unignore all removes ignore list entierly."),
             AddCommand(UserCommandProcessor, L"/invite", InvitePlayerByName, L"/invite <name>", L"invites specified player to group"),
             AddCommand(UserCommandProcessor, L"/invite$", InvitePlayerById, L"/invite$ <id>", L"invites specified player to group by client id"),
             AddCommand(UserCommandProcessor, L"/finv", FactionInvite, L"/finv <prefix>", L"invites players that matches the listed prefix in their name"),
             //AddCommand(UserCommandProcessor, L"/delmail", DeleteMail, L"/delmail <id>", L"deletes specified mail"),
             //AddCommand(UserCommandProcessor, L"/readmail", ReadMail, L"/readmail <id>", L"prints specified mail."),
             //AddCommand(UserCommandProcessor, L"/listmail", ListMail, L"/listmail [page]", L"lists the mails of the specified page."),
             AddCommand(UserCommandProcessor, L"/givecash", GiveCash, L"/givecash <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, L"/gc", GiveCash, L"/gc <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, L"/givecash$", GiveCashById, L"/givecash <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, L"/gc$", GiveCashById, L"/gc$ <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, L"/sendcash", GiveCash, L"/sendcash <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, L"/sc", GiveCash, L"/sc <target> <amount>", L"gives specified amount of cash to named target"),
             AddCommand(UserCommandProcessor, L"/sendcash$", GiveCashById, L"/sendcash <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, L"/sc$", GiveCashById, L"/sc$ <targetId> <amount>", L"gives specified amount of cash to target"),
             AddCommand(UserCommandProcessor, L"/help", Help, L"/help [module] [command]", L"Provides indepth help information")}
        };
        // clang-format on
        GetCommandsFunc(commands);

        template <int N>
        bool MatchCommand(UserCommandProcessor* processor, ClientId triggeringClient, const std::wstring_view cmd, std::vector<std::wstring>& paramVector)
        {
            if (const CommandInfo<UserCommandProcessor> command = std::get<N - 1>(commands); command.cmd == cmd)
            {
                command.func(processor, paramVector);
                return true;
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
