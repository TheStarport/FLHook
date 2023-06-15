#pragma once

#include "AbstractUserCommandProcessor.hpp"

class UserCommandProcessor final : public Singleton<UserCommandProcessor>, public AbstractUserCommandProcessor
{
        void GetClientIds();

        constexpr inline static std::array<UserCommandInfo<UserCommandProcessor>, 1> commands = {
            AddUserCommand(UserCommandProcessor, L "ids", GetClientIds, L"/ids", L"Lists all the players and their internal client id numbers."),
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
