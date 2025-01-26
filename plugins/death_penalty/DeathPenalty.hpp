#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date unknown
     * @author Cannon (Ported in 2022)
     * @brief
     * Penalize player deaths by loss of credits, optionally reward the killer player.
     *
     * @par Configuration
     * @code
     * {
     *     "DeathPenaltyFraction": 0.0,
     *     "DeathPenaltyFractionKiller": 0.0,
     *     "ExcludedSystems": ["Li01", "Li02"],
     *     "penalizePvpOnly": true,
     *     "FractionOverridesByShip": [{"ge_fighter", 0.2}, {"li_elite", 0.15}]
     * }
     * @endcode
     *
     * @par Player Commands
     * All commands are prefixed with '/' unless explicitly specified.
     * - dp - Set amount of credits player will lose upon death, enable/disable notifications.
     *
     * @par Admin Commands
     * This plugin has no admin commands
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
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
                    //! Only penalize players that died after sustaining damage from a player
                    bool penalizePvpOnly = true;
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
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;
            void OnCharacterSelectAfter(ClientId client) override;
            void OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document) override;
            void OnSendDeathMessageAfter(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg) override;
            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnJumpInComplete(SystemId system, const ShipId& ship) override;
            void OnCharacterInfoRequest(ClientId client, bool unk1) override;
            bool OnSystemSwitchOutPacket(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet) override;

            void KillIfInJumpTunnel(ClientId client);
            [[nodiscard]]
            bool IsInExcludedSystem(ClientId client) const;
            float GetShipFractionOverride(ClientId client);
            void PenalizeDeath(ClientId client, ClientId killerId);
            /// @brief /dp command. Shows information about death penalty
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
