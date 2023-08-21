#pragma once

#include <API/API.hpp>

namespace Plugins
{
    class Betting final : public Plugin, public AbstractUserCommandProcessor
    {
            void ProcessFFA(ClientId client);
            void ProcessDuel(ClientId client);
            int DockCall(const unsigned int& ship, [[maybe_unused]] const unsigned int& dock, [[maybe_unused]] const int& cancel,
                         [[maybe_unused]] const DOCK_HOST_RESPONSE& response);
            void DisConnect(ClientId& client, [[maybe_unused]] const EFLConnection& state);
            void CharacterInfoReq(ClientId& client, [[maybe_unused]] const bool& param2);
            void SendDeathMessage([[maybe_unused]] const std::wstring& message, [[maybe_unused]] const uint& system, ClientId& clientVictim,
                                  [[maybe_unused]] const ClientId& clientKiller);


            //! A struct to hold a duel between two players. This holds the amount of cash they're betting on, and whether it's been accepted or not
            struct Duel
            {
                    uint client;
                    uint client2;
                    uint amount;
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
                    std::map<uint, Contestant> contestants;
                    uint entryAmount;
                    uint pot;
            };

            //! Global data for this plugin

            ReturnCode returnCode = ReturnCode::Default;
            std::list<Duel> duels;
            std::map<uint, FreeForAll> freeForAlls; // uint is systemId
      

            // User Commands.
            void UserCmdStartFreeForAll(uint amount);
            void UserCmdAcceptFFA();
            void UserCmdDuel(uint amount);
            void UserCmdAcceptDuel();
            void UserCmdCancel();

            constexpr inline static std::array<CommandInfo<Betting>, 5> commands = {
                {AddCommand(Betting, L"/ffa", UserCmdStartFreeForAll, L"Create an ffa and send an invite to everyone in the system. Winner gets the pot."),
                 AddCommand(Betting, L"/acceptffa", UserCmdAcceptFFA, L"Accept the current ffa request."),
                 AddCommand(Betting, L"/duel", UserCmdDuel, L"Create a duel request to the targeted player. Winner gets the pot."),
                 AddCommand(Betting, L"/acceptduel", UserCmdAcceptDuel, L"Accepts the current duel request."),
                 AddCommand(Betting, L"/cancel", UserCmdCancel, L"Cancel the current duel/ffa request.")}
            };

    public:
            explicit Betting(const PluginInfo& info);

    };
} // namespace Plugins