#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date August, 2022
     * @author Raikkonen
     * @brief
     * The AFK plugin allows you to set yourself as Away from Keyboard.
     * This will notify other players if they try and speak to you, that you are not at your desk.
     *
     * @par configuration Configuration
     * No configuration file is needed.
     *
     * @par Player Commands
     * - afk - Sets your status to Away from Keyboard. Other players will notified if they try to speak to you.
     * - back - Removes the AFK status.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    class AfkPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            std::vector<ClientId> awayClients;

            /**
             * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let
             * anyone who messages them know too.
             */
            Task UserCmdAfk(ClientId);

            /**
             * @brief This command is called when a player types /back. It removes the afk status and welcomes the player back.
             * who messages them know too.
             */
            Task UserCmdBack(ClientId);

            void OnClearClientInfo(ClientId client) override;
            void OnSendChat(ClientId fromClient, ClientId targetClient, uint size, void* rdl) override;
            void OnSubmitChat(ClientId fromClient, unsigned long lP1, const void* rdlReader, ClientId to, int dunno) override;

            // clang-format off
            inline static const std::array<CommandInfo<AfkPlugin>, 2> commands =
            {
                {
                    AddCommand(AfkPlugin, Cmds(L"/afk"), UserCmdAfk, L"/afk", L"Sets your status to \"Away from Keyboard\". Other players will notified if they try to speak to you."),
                    AddCommand(AfkPlugin, Cmds(L"/back"), UserCmdBack, L"/back", L"Removes the AFK status."),
                 }
            };
            // clang-format on

            template <int N>
            std::optional<Task> MatchCommand(AfkPlugin* processor, ClientId triggeringClient, const std::wstring_view cmd,
                                             std::vector<std::wstring_view>& paramVector)
            {
                const CommandInfo<AfkPlugin> command = std::get<N - 1>(AfkPlugin ::commands);
                for (auto& str : command.cmd)
                {
                    if (cmd.starts_with(str))
                    {
                        paramVector.erase(paramVector.begin(), paramVector.begin() + (std::clamp(std::ranges::count(str, L' '), 1, 5)));
                        return command.func(processor, paramVector);
                    }
                }
                return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);
            }
            template <>
            std::optional<Task> MatchCommand<0>(AfkPlugin* processor, ClientId triggeringClient, std::wstring_view cmd,
                                                std::vector<std::wstring_view>& paramVector)
            {
                return std::nullopt;
            }
            std::optional<Task> ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring_view>& paramVector) override
            {
                return MatchCommand<commands.size()>(this, triggeringClient, cmd, paramVector);
            }

        public:
            const std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>>& GetUserCommands() const override
            {
                static std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> info;
                if (info.empty())
                {
                    for (const auto& cmd : commands)
                    {
                        info.emplace_back(cmd.cmd, cmd.usage, cmd.description);
                    }
                }
                return info;
            };
            ;

        private:
            ;

        public:
            explicit AfkPlugin(const PluginInfo& info);
    };
} // namespace Plugins
