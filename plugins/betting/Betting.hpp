#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    class Betting final : public Plugin, public AbstractUserCommandProcessor
    {
            void ProcessFFA(ClientId client);
            void ProcessDuel(ClientId client);

            //! A struct to hold a duel between two players. This holds the amount of cash they're betting on, and whether it's been accepted or not
            struct Duel
            {
                    ClientId client;
                    ClientId client2;
                    uint betAmount;
                    bool accepted;
            };

            //! A struct to hold a contestant for a Free-For-All
            struct Contestant
            {
                    bool accepted;
                    bool loser;
            };

            //! A struct to hold a Free-For-All competition. This holds the contestants, how much it costs to enter, and the total pot to be won by the eventual
            //! winner
            struct FreeForAll
            {
                    std::unordered_map<ClientId, Contestant> contestants;
                    uint entryAmount;
                    uint pot;
            };

            //! Global data for this plugin

            ReturnCode returnCode = ReturnCode::Default;
            std::list<Duel> duels;
            std::unordered_map<SystemId, FreeForAll> freeForAlls;

            // User Commands.
            void UserCmdStartFreeForAll(uint amount);
            void UserCmdAcceptFFA();
            void UserCmdDuel(uint amount);
            void UserCmdAcceptDuel();
            void UserCmdCancel();

            constexpr static std::array<CommandInfo<Betting>, 5> commands = {
                { AddCommand(Betting, L"/ffa", UserCmdStartFreeForAll, L"/ffa",
                 L"Create an ffa and send an invite to everyone in the system. Winner gets the pot."),
                 AddCommand(Betting, L"/acceptffa", UserCmdAcceptFFA, L"/acceptffa", L"Accept the current ffa request."),
                 AddCommand(Betting, L"/duel", UserCmdDuel, L"/duel", L"Create a duel request to the targeted player. Winner gets the pot."),
                 AddCommand(Betting, L"/acceptduel", UserCmdAcceptDuel, L"/acceptduel", L"Accepts the current duel request."),
                 AddCommand(Betting, L"/cancel", UserCmdCancel, L"/cancel", L"Cancel the current duel/ffa request.") }
            };

            SetupUserCommandHandler(Betting, commands);

            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnDockCallAfter(ShipId shipId, ObjectId spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response) override;
            void OnCharacterInfoRequestAfter(ClientId client, bool unk1) override;
            void OnSendDeathMessageAfter(ClientId killer, ClientId victim, SystemId system, std::wstring_view msg) override;

        public:
            explicit Betting(const PluginInfo& info);
    };
} // namespace Plugins
