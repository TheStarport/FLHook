#pragma once

#include "AbstractUserCommandProcessor.hpp"

class UserCommandProcessor final : public Singleton<UserCommandProcessor>, public AbstractUserCommandProcessor
{
        void GetClientIds(ClientId& client);
        void PrintUserCmdText(ClientId client, std::wstring_view text);
        void PrintLocalUserCmdText(ClientId client, std::wstring_view msg, float distance);
        void SetDieMessage(ClientId& client, const std::wstring& param);
        void SetDieMessageFontSize(ClientId& client, const std::wstring& param);
        void SetChatFont(ClientId& client, std::wstring_view fontSize, std::wstring_view fontType);
        void IgnoreUser(ClientId& client, std::wstring_view ignoredUser, std::wstring_view flags);
        void IgnoreClientId(ClientId& client, ClientId ignoredClient, std::wstring_view flags);
        void GetIgnoreList(ClientId& client);
        void RemoveFromIgnored(ClientId& client, std::vector<std::wstring_view> charactersToRemove);
        void GetSelfClientId(ClientId& client);
        void InvitePlayer(ClientId& client, const std::wstring_view& characterName);
        void InvitePlayerByName(ClientId& client, std::wstring_view invitee);
        void InvitePlayerById(ClientId& client, ClientId inviteeId);
        void FactionInvite(ClientId& client, std::wstring_view factionTag);
        void DeleteMail(ClientId& client, const std::wstring_view mailID, const std::wstring_view readOnlyDel);
        void ReadMail(ClientId& client, uint mailId);
        void ListMail(ClientId& client, int pageNumber, std::wstring_view unread);
        void GiveCash(ClientId& client, std::wstring_view characterName, std::wstring_view amount);

        constexpr inline static std::array<UserCommandInfo<UserCommandProcessor>, 13> commands = {
            {AddUserCommand(UserCommandProcessor, L"ids", GetClientIds, L"/ids", L"Lists all the players and their internal client id numbers."),
             AddUserCommand(UserCommandProcessor, L"/setdiemsgsize", SetDieMessageFontSize, L "/setdiemsgsize", L"Sets the text size of death chatConfig."),
             AddUserCommand(UserCommandProcessor, L"/setdiemsg", SetDieMessage, L"/setdiemsg", L" insert help text here"),
             AddUserCommand(UserCommandProcessor, L"/setchatfont", SetChatFont, L"setchatfont", L"sets the font style and size of the chat"),
             AddUserCommand(UserCommandProcessor, L"/ignore", IgnoreUser, L"/ignore <name>", L"blocks any message sent by player specified"),
             AddUserCommand(UserCommandProcessor, L"getignorelist", GetIgnoreList, L"/getignorelist", L"prints the users you currently have ignored"),
             AddUserCommand(UserCommandProcessor, L"/unignore", RemoveFromIgnored, L"/unignore <name ...>",
             L"removes specified names from ignore list, typing /unignore all removes ignore list entierly."),
             AddUserCommand(UserCommandProcessor, L"/invite", InvitePlayerByName, L"/invite name", L"invites specified player to group"),
             AddUserCommand(UserCommandProcessor, L"finv", FactionInvite, L"/finv tag", L"invites players that match the listed tag in their name"),
             AddUserCommand(UserCommandProcessor, L"/delmail", DeleteMail, L"/delmail ", L"deletes specified mail"),
             AddUserCommand(UserCommandProcessor, L"/readmail", ReadMail, L"/readmail id", L"prints specified mail."),
             AddUserCommand(UserCommandProcessor, L"/listmail", ListMail, L"listmail page", L"lists the mails of the specified page."),
             AddUserCommand(UserCommandProcessor, L"/givecash", GiveCash, L"/givecash target amount", L"gives speicified amount of cash to named target")}
        };

        template <int N>
        bool MatchCommand(UserCommandProcessor* processor, ClientId client, const std::wstring_view cmd, const std::vector<std::wstring>& paramVector)
        {
            if (const UserCommandInfo<UserCommandProcessor> command = std::get<N - 1>(commands); command.cmd == cmd)
            {
                command.func(*processor, client, paramVector);
                return true;
            }

            return MatchCommand<N - 1>(processor, client, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        bool MatchCommand<0>(UserCommandProcessor* processor, ClientId client, std::wstring_view cmd, const std::vector<std::wstring>& paramVector)
        {
            std::ranges::any_of(PluginManager::i()->userCommands,
                                [=](const std::weak_ptr<AbstractUserCommandProcessor>& weakPtr)
                                { return weakPtr.lock()->ProcessCommand(client, cmd, paramVector); });

            // No matching command was found.
            return false;
        }

    public:
        bool ProcessCommand(ClientId client, std::wstring_view cmd, const std::vector<std::wstring>& paramVector) override;
        bool ProcessCommand(ClientId client, std::wstring_view commandStr);
};
