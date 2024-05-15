#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{

    class BountyHunt final : public Plugin, public AbstractUserCommandProcessor
    {
            //! Structs
            struct Bounty
            {
                    ClientId issuer;
                    ClientId target;
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
            };

            std::unique_ptr<Config> config = nullptr;
            ReturnCode returnCode = ReturnCode::Default;

            std::array<std::vector<Bounty>, MaxClientId + 1> bountiesOnPlayers;

            std::vector<std::pair<ClientId, uint>> ClearPlayerOfBounties(ClientId client);
            void PrintBountyHunts(ClientId client);
            void TimeOutCheck();
            void BillCheck(ClientId client, ClientId killer);
            void CheckIfPlayerFled(ClientId client);

            // User Commands
            void UserCmdBountyHunt(std::wstring_view target, uint prize, uint time);
            void UserCmdBountyHuntByClientID(ClientId target, uint credits, uint time);

            constexpr static std::array<CommandInfo<BountyHunt>, 4> commands = {
                {

                 AddCommand(BountyHunt, L"/bountyhunt", UserCmdBountyHunt, L"/bountyhunt <targetName> <prize> <time>",
                 L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
                 AddCommand(BountyHunt, L"/bh", UserCmdBountyHunt, L"/bh <targetName> <prize> <time>",
                 L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
                 AddCommand(BountyHunt, L"/bountyhunt$", UserCmdBountyHuntByClientID, L"/bountyhunt$ <targetId> <prize> <time>",
                 L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
                 AddCommand(BountyHunt, L"/bh$", UserCmdBountyHuntByClientID, L"/bh$ <targetId> <prize> <time>",
                 L"Places a bounty on the specified clientId. When another player kills them, they gain <credits>."),
                 }
            };

            SetupUserCommandHandler(BountyHunt, commands);

            // Hook Functions
            void OnSendDeathMessageAfter(ClientId killer, ClientId victim, SystemId system, std::wstring_view msg) override;
            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnCharacterSelectAfter(ClientId client) override;

        public:
            explicit BountyHunt(const PluginInfo& info);
    };
} // namespace Plugins
