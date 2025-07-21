#pragma once

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date 2010
     * @author Cannon (Ported/Updated by Aingar 2022-2025)
     * @brief
     * This plugin is used to allow players to enable an otherwise unusable Cloaking Device type equipment
     *
     * @par Configuration
     * The configuration file is generated at config/cloak.json. An example configuration is as follows:
     * @code
     * {
     *     "restrictedSystems": [ "Li01", "Li02" ],
     *     "targetBase": "Li02_01_Base",
     *     "targetSystem": "Li02"
     * }
     * @endcode
     *
     * @par Player Commands
     * - cloak - toggles the cloak on and off.
     *
     * @par Admin Commands
     * - cloak - toggles the cloak on and off with no way to
     *
     * @note All player commands are prefixed with '/', all admin commands are prefixed with a '.'
     */
    class CloakPlugin : public Plugin, public AbstractUserCommandProcessor, public AbstractAdminCommandProcessor, public PacketInterface
    {

            enum class CloakState
            {
                Invalid,
                Off,
                Charging,
                On,
            };

            struct FuelUsage
            {
                    float staticUse;
                    float linearUse;
                    float squaredUse;
            };

            struct CloakInfo
            {
                    int64 warmupTime; // time in miliseconds
                    int64 cooldownTime;
                    int64 activationTime;
                    rfl::Skip<uint> usableClasses;
                    std::vector<std::string> usableClassesString;
                    float detectionRange;
                    float soundRange;
                    Id soundId;
                    bool dropShieldsOnCloak;
                    bool disruptChargeOnDisruptorHit;
                    std::unordered_map<Id, FuelUsage> fuelMap;
            };

            struct DisruptorInfo
            {
                    float range;
                    int64 cooldown;
                    int64 disruptTime;
                    Id ammoType;
                    ushort ammoAmount;
                    Id effect;
            };

            struct Config final
            {
                    bool enableSystemSpoofing;

                    std::unordered_map<Id, CloakInfo> cloakInfoMap;
                    std::unordered_map<Id, DisruptorInfo> disruptorInfoMap;
                    std::unordered_map<SystemId, SystemId> systemObscureMap;
            };

            Config config;

            struct PlayerCloakData
            {
                    CloakState cloakState = CloakState::Off;
                    ushort cloakSlot;
                    float fuelUsageOverflow;
                    bool admin;
                    bool disableJumpFuelConsumption;
                    int64 cloakTime;
                    int64 disruptedUntilTime;

                    CloakInfo* cloakInfo = nullptr;
            };

            struct PlayerDisruptorData
            {
                    int64 cooldownUntil;
                    DisruptorInfo* disruptorInfo;
            };

            std::unordered_map<ClientId, PlayerCloakData> clientCloakData;
            std::unordered_map<ClientId, PlayerDisruptorData> clientDisruptorData;

            concurrencpp::result<void> UserCmdCloak(ClientId client);
            concurrencpp::result<void> UserCmdDisruptor(ClientId client);

            concurrencpp::result<void> AdminCmdCloak(ClientId client);

            bool ConsumeFuel(ClientId client);
            void ProcessFuel();
            void ProcessDisruptors();
            void CloakDisruptor(ShipId ship, float disruptRange, int64 disruptTime);
            void EnableFuelConsumption(ClientId client);
            void InitCloak(ClientId client, float range);
            void OnClearClientInfo(ClientId client);
            void ObscureSystemList(ClientId client);
            void SetCloak(ClientId client, bool cloakState);
            void SetState(ClientId client, CloakState newState);
            void SendUncloakPacket(ClientId packetReceivingClient, Id uncloakingShipId, ushort cloakSId);
            bool IsClientJumping(ClientId client);

            bool OnLoadSettings() override;
            void OnCreateShipPacketAfter(ClientId client, FLPACKET_CREATESHIP& ship) override;
            void OnSpRequestUseItem(ClientId client, const SSPUseItem& item) override;
            void OnDockCallAfter(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response) override;
            void OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList) override;
            void OnPlayerLaunch(ClientId client, const ShipId& ship) override;

            #ifdef CLOAK_PLUGIN
            // clang-format off
            inline static const std::array<CommandInfo<CloakPlugin>, 2> commands =
            {
                {            
                    AddCommand(CloakPlugin, Cmds(L"/cloak"), UserCmdCloak, L"/cloak", L"Toggles your cloak state on/off"),
                    AddCommand(CloakPlugin, Cmds(L"/disruptor"), UserCmdDisruptor, L"/disruptor", L"Activates the cloak disruptor."),
                }
            };
            SetupUserCommandHandler(CloakPlugin, commands);
            // clang-format on

            // clang-format off
            inline static const std::array<AdminCommandInfo<CloakPlugin>, 2> adminCommands =
            {
                {
                    AddAdminCommand(CloakPlugin, Cmds(L".cloak"), AdminCmdCloak, GameOnly, Any, L".cloak", L"Toggles you cloak state on/off, impossible to forcefully detect, uses no fuel."),
                }
            };
            // clang-format on

            SetupAdminCommandHandler(CloakPlugin, adminCommands);
            #endif
        public:
            explicit CloakPlugin(const PluginInfo& info);

            static constexpr std::wstring_view pluginName = L"cloak";
            virtual bool IsClientCloaked(ClientId client);
    };
} // namespace Plugins
