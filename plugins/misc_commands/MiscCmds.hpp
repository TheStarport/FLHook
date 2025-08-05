#pragma once

#include "API/FLHook/Plugin.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date July 2025
     * @author Aingar
     * @brief
     * Miscellaneous commands
     */
    class MiscCmdsPlugin final : public Plugin, public AbstractUserCommandProcessor
    {

            concurrencpp::result<void> UserCmdBountyScan(ClientId client);
            concurrencpp::result<void> UserCmdGroupSize(ClientId client, std::optional<GroupId> groupId);
            // clang-format off
            inline static const std::array<CommandInfo<MiscCmdsPlugin>, 10> commands =
            {
                {
                    AddCommand(MiscCmdsPlugin, Cmds(L"/bountyscan", L"/bs"), UserCmdBountyScan, L"/bountyscan", L"Prints basic data about the player"),
                    AddCommand(MiscCmdsPlugin, Cmds(L"/groupsize", L"/gs"), UserCmdGroupSize, L"/groupsize [groupId]", L"Prints group size of your group and of group of either provided ID or selected target"),
                }
            };
            SetupUserCommandHandler(MiscCmdsPlugin, commands);
            // clang-format on

        public:
            explicit MiscCmdsPlugin(const PluginInfo& info);
    };
} // namespace Plugins
