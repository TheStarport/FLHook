#pragma once

// Included
#include <API/API.hpp>

namespace Plugins
{

    class BountyHunt final : public Plugin, public AbstractUserCommandProcessor
    {
            //! Structs
            struct Bounty
            {
                    std::wstring issuer;
                    uint cash;
                    mstime end;
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

                    Serialize(Config, levelProtect, minimalHuntTime, maximumHuntTime, defaultHuntTime);
            };

            std::unique_ptr<Config> config = nullptr;
            ReturnCode returnCode = ReturnCode::Default;

            std::map<ClientId, std::vector<Bounty>> bountiesOnPlayers;

            std::vector<std::pair<std::wstring, uint>> ClearPlayerOfBounties(ClientId client);
            void PrintBountyHunts(ClientId client);
            void TimeOutCheck();
            void BillCheck(ClientId& client, ClientId& killer);
            void CheckIfPlayerFled(ClientId& client);
            void LoadSettings();

            // Hook Functions
            void SendDeathMsg([[maybe_unused]] const std::wstring& msg, [[maybe_unused]] const SystemId& system, ClientId& clientVictim,
                              ClientId& clientKiller);
            void DisConnect(ClientId& client, [[maybe_unused]] const EFLConnection& state);
            void CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client);

            // User Commands
            void UserCmdBountyHunt( std::wstring_view target, uint prize, uint time);
            void UserCmdBountyHuntByClientID( ClientId target, uint credits, uint time);

            constexpr inline static std::array<CommandInfo<BountyHunt>, 4> commands = {
                {

                 AddCommand(BountyHunt, L"/bountyhunt", UserCmdBountyHunt,
                 L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
                 AddCommand(BountyHunt, L"/bh", UserCmdBountyHunt,
                 L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
                 AddCommand(BountyHunt, L"/bountyhunt$", UserCmdBountyHuntByClientID,
                 L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
                 AddCommand(BountyHunt, L"/bh$", UserCmdBountyHuntByClientID,
                 L"Places a bounty on the specified clientId. When another player kills them, they gain <credits>."),
                 }
            };

            SetupUserCommandHandler(BountyHunt, commands);

        public:
            explicit BountyHunt(PluginInfo& info);
    };
} // namespace Plugins
