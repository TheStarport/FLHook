#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date unknown
     * @author ||KOS||Acid (Ported by Raikkonen 2022)
     * @defgroup BountyHunt Bounty Hunt
     * @brief
     * The "Bounty Hunt" plugin allows players to put bounties on each other that can be collected by destroying that player.
     *
     * @par configuration Configuration
     * @code
     * {
     *     "enableBountyHunt": true,
     *     "levelProtect": 0,
     *     "minimalHuntTime": 1,
     *     "maximumHuntTime": 240,
     *     "defaultHuntTime": 30
     * }
     * @endcode
     *
     * @par cmds Player Commands
     * All commands are prefixed with '/' unless explicitly specified.
     * - bountyhunt <player> <amount> [timelimit] - Places a bounty on the specified player. When another player kills them, they gain <credits>.
     * - bountyhuntid <id> <amount> [timelimit] - Same as above but with an id instead of a player name. Use /ids
     *
     * @par adminCmds Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    class BountyHuntPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            //! Structs
            struct Bounty
            {
                    ClientId issuer;
                    ClientId target;
                    uint cash;
                    int64 end;
            };

            //! Configurable fields for this plugin
            struct Config final
            {
                    int levelProtect = 0;
                    //! Minimal time a hunt can be set to, in minutes.
                    uint minimalHuntTime = 1;
                    //! Maximum time a hunt can be set to, in minutes.
                    uint maximumHuntTime = 240;
                    //! Hunt time in minutes, if not explicitly specified.
                    uint defaultHuntTime = 30;
            };

            Config config;

            std::array<std::vector<Bounty>, MaxClientId + 1> bountiesOnPlayers;

            std::vector<std::pair<ClientId, uint>> ClearPlayerOfBounties(ClientId client);
            void PrintBountyHunts(ClientId client);
            void TimeOutCheck();
            void BillCheck(ClientId client, ClientId killer);
            void CheckIfPlayerFled(ClientId client);

            // User Commands
            Task UserCmdBountyHunt(ClientId client, ClientId target, uint prize, uint time);

            inline static const std::array<CommandInfo<BountyHuntPlugin>, 1> commands = { { AddCommand(
                BountyHuntPlugin, Cmds(L"/bountyhunt", L"/bh"), UserCmdBountyHunt, L"/bountyhunt <targetId> <prize> [time]",
                L"Places a bounty on the specified player. When another player kills them, they gain <credits>.") } };

            SetupUserCommandHandler(BountyHuntPlugin, commands);

            // Hook Functions
            void OnSendDeathMessageAfter(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg) override;
            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnCharacterSelectAfter(ClientId client) override;
            bool OnLoadSettings() override;

        public:
            explicit BountyHuntPlugin(const PluginInfo& info);
    };
} // namespace Plugins
