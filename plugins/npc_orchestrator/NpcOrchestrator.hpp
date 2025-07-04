#pragma once

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date September, 2024
     * @author IrateRedKite
     * @brief
     * The NPC Orchestrator plugin allows a server admin to spawn, manage and schedule NPC activity via commands.
     *
     * @par configuration Configuration
     * No configuration file is needed.
     *
     * @par Player Commands
     * There are no player commands in this plugin.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    class NpcOrchestratorPlugin final : public Plugin, public AbstractAdminCommandProcessor
    {
            std::vector<ClientId> awayClients;

            /**
             * @brief Spawns NPC(s) and Solars via command and assigns them to a group. By default, count and group are 1. If coordinates are omitted, the NPC
             * or solar will spawn at a random coordinate near the user's location.
             */

            concurrencpp::result<void>AdminCmdCreateNpc(ClientId client);

            // npc_spawn [group] [npc_class] [?count] [?system] [?coordinates] [?rank] [?delay] [?message]

            // clang-format off
            inline static const std::array<AdminCommandInfo<NpcOrchestratorPlugin>, 2> commands =
            {
                {
                    AddAdminCommand(NpcOrchestratorPlugin, Cmds(L".npc_create"), AdminCmdCreateNpc, GameOnly, Any, L".npc_create", L"Spawns NPCs and takes the following parameters: [group] [npc_class] [?count] [?system] [?coordinates] [?rank]"),
                 }
            };
            // clang-format on

            SetupAdminCommandHandler(NpcOrchestratorPlugin, commands);

        public:
            explicit NpcOrchestratorPlugin(const PluginInfo& info);
    };
} // namespace Plugins
