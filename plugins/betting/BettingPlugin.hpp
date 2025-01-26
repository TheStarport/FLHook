#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date Jan, 2023
     * @author Raikkonen
     * @brief
     * A plugin that allows players to place bets and then duel, the winner getting the pot.
     * @par Configuration
     * This plugin has no configuration file.
     *
     * @par Player Commands
     * All commands are prefixed with '/' unless explicitly specified.
     * - acceptduel - Accepts the current duel request.
     * - acceptffa - Accept the current ffa request.
     * - cancel - Cancel the current duel/ffa request.
     * - duel <amount> - Create a duel request to the targeted player. Winner gets the pot.
     * - ffa <amount> - Create an ffa and send an invite to everyone in the system. Winner gets the pot.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    class BettingPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            void ProcessFFA(ClientId client);
            void ProcessDuel(ClientId client);

            /// @brief A struct to hold a duel between two players. This holds the amount of cash they're betting on, and whether it's been accepted or not
            struct Duel
            {
                    ClientId client;
                    ClientId client2;
                    uint betAmount;
                    bool accepted;
            };

            /// @brief A struct to hold a contestant for a Free-For-All
            struct Contestant
            {
                    bool accepted;
                    bool loser;
            };

            /// @brief A struct to hold a Free-For-All competition. This holds the contestants, how much it costs to enter, and the total pot to be won by the eventual winner
            struct FreeForAll
            {
                    std::unordered_map<ClientId, Contestant> contestants;
                    uint entryAmount;
                    uint pot;
            };

            //! Global data for this plugin

            std::list<Duel> duels;
            std::unordered_map<SystemId, FreeForAll> freeForAlls;

            // User Commands.
            Task UserCmdStartFreeForAll(ClientId client, uint amount);
            Task UserCmdAcceptFFA(ClientId client);
            Task UserCmdDuel(ClientId client, uint amount);
            Task UserCmdAcceptDuel(ClientId client);
            Task UserCmdCancel(ClientId client);

            inline static const std::array<CommandInfo<BettingPlugin>, 5> commands = {
                { AddCommand(BettingPlugin, Cmds(L"/ffa"sv), UserCmdStartFreeForAll, L"/ffa",
                 L"Create an ffa and send an invite to everyone in the system. Winner gets the pot."),
                 AddCommand(BettingPlugin, Cmds(L"/acceptffa"sv), UserCmdAcceptFFA, L"/acceptffa", L"Accept the current ffa request."),
                 AddCommand(BettingPlugin, Cmds(L"/duel"sv), UserCmdDuel, L"/duel", L"Create a duel request to the targeted player. Winner gets the pot."),
                 AddCommand(BettingPlugin, Cmds(L"/acceptduel"sv), UserCmdAcceptDuel, L"/acceptduel", L"Accepts the current duel request."),
                 AddCommand(BettingPlugin, Cmds(L"/cancel"sv), UserCmdCancel, L"/cancel", L"Cancel the current duel/ffa request.") }
            };

            SetupUserCommandHandler(BettingPlugin, commands);

            /// @brief Hook for disconnect. Treats a player as if they died if they were part of a duel
            void OnDisconnect(ClientId client, EFLConnection connection) override;
            /// @brief Hook for dock call. Treats a player as if they died if they were part of a duel
            void OnDockCallAfter(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response) override;
            /// @brief Hook for char info request (F1). Treats a player as if they died if they were part of a duel
            void OnCharacterInfoRequestAfter(ClientId client, bool unk1) override;
            /// @brief Hook for death to kick player out of duel
            void OnSendDeathMessageAfter(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg) override;

        public:
            explicit BettingPlugin(const PluginInfo& info);
    };
} // namespace Plugins
