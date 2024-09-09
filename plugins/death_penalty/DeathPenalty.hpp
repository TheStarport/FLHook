#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    class DeathPenaltyPlugin final : public Plugin, public AbstractUserCommandProcessor, public PacketInterface
    {
            struct CLIENT_DATA
            {
                    bool displayDPOnLaunch = true;
                    bool isJumping = false;
                    uint deathPenaltyCredits = 0;
            };

            //! Configurable fields for this plugin
            struct Config final
            {
                    // Reflectable fields
                    //! Percentage of player's worth deducted upon death, where 1.0f stands for all of his worth.
                    float DeathPenaltyFraction = 0.0f;
                    //! Percentage of death penalty transferred to the killer, where 1.0f means the killer gets as much as the victim lost.
                    float DeathPenaltyFractionKiller = 0.0f;
                    //! List of system where death penalty/kill reward is disabled in.
                    std::unordered_set<SystemId> ExcludedSystems = {};
                    //! Map of ship archetypes to a penalty multiplier.
                    //! For example, {li_elite, 2} causes Defenders to lose twice the amount compared to unlisted ships on death.
                    std::unordered_map<EquipmentId, float> FractionOverridesByShip = {};

                    bool KillOnDisconnect = false;
            };

            std::unordered_set<SystemId> ExcludedSystemsIds;
            std::unordered_map<EquipmentId, float> FractionOverridesByShipIds;

            std::unordered_map<ClientId, CLIENT_DATA> ClientInfo;

            Config config;
            bool OnLoadSettings() override;
            void OnClearClientInfo(ClientId client) override;
            void OnPlayerLaunchAfter(ClientId client, ShipId ship) override;
            void OnCharacterSelectAfter(ClientId client) override;
            void OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document) override;
            void OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) override;
            void KillIfInJumpTunnel(const ClientId client);
            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnJumpInComplete(SystemId system, ShipId ship) override;
            void OnCharacterInfoRequest(ClientId client, bool unk1) override;
            bool OnSystemSwitchOutPacket(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet) override;

            [[nodiscard]]
            bool IsInExcludedSystem(ClientId client) const;
            float GetShipFractionOverride(ClientId client);
            void PenalizeDeath(ClientId client, ClientId killerId);

            Task UserCmdDeathPenalty(ClientId client, std::wstring_view param);

            // clang-format off
            inline static const std::array<CommandInfo<DeathPenaltyPlugin>, 1> commands =
            {
                {
                    AddCommand(DeathPenaltyPlugin, Cmds(L"/dp"), UserCmdDeathPenalty, L"/dp", L""),
                }
            };
            // clang-format on

            SetupUserCommandHandler(DeathPenaltyPlugin, commands);

        public:
            explicit DeathPenaltyPlugin(const PluginInfo& info);
    };
} // namespace Plugins
