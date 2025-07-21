#pragma once

#include "../cloak/Cloak.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include <httplib.h>

namespace Plugins
{
    /**
     * @date July 2025
     * @author Aingar
     * @brief
     * This plugin is used to spawn temporary jump holes to specified locations.
     *
     * @par Configuration
     * The configuration file is generated at config/hyperjump.json.
     * @endcode
     *
     * @par Player Commands
     * //TODO
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/', all admin commands are prefixed with a '.'
     */
    class HyperjumpPlugin : public Plugin, public AbstractUserCommandProcessor
    {
            struct SystemJumpCoords
            {
                    SystemId system;
                    std::wstring sector;
                    Transform position;
            };

            struct JumpDriveData
            {
                    int64 chargeTime; // in miliseconds
                    int64 jumpHoleDuration;
                    uint jumpCapacity;
                    uint jumpRange;
                    std::vector<Id> chargeFuses;
                    Id jumpFuse;
                    uint shipClasses;

                    int64 spawnDelay;

                    Id entryJumpHoleArchetype;
                    Id entryJumpHoleLoadout;
                    Id exitJumpHoleArchetype;
                    Id exitJumpHoleLoadout;

                    std::map<Id, std::vector<uint>> fuelPerDistanceMap;
                    bool cdDisruptsCharge;
            };

            struct BeaconData
            {
                    float inaccuracy;
                    uint jumpRangeExtension;
                    Id fuel;
                    uint fuelAmount;
                    Id beaconFuse;
            };

            struct SystemData
            {
                    std::unordered_map<uint, std::vector<SystemId>> availableSystemsPerDepth;
                    std::vector<SystemJumpCoords> systemCoords;
            };

            struct Config final
            {
                    float jumpCargoRestriction = 100000;
                    std::unordered_set<Id> jumpRestrictedShips;
                    std::unordered_map<Id, JumpDriveData> jumpDriveMap;
                    std::unordered_map<Id, BeaconData> beaconMap;
                    std::unordered_map<SystemId, SystemData> jumpSystemData;

                    SystemId blindJumpOverrideSystem = SystemId();
                    int64 beaconRequestTimeout = 15000;
                    uint blindJumpRange = 0;
            };

            Config config;

            struct HyperjumpClientData
            {
                    JumpDriveData* jumpDriveInfo;

                    uint consumeFuelTarget;
                    ushort fuelToConsumePerSecond;
                    uint consumedFuel;
                    Id selectedFuel;

                    ClientId targetClient;

                    SystemId targetSystem;
                    Transform targetLocation;

                    Id currentJumpFuse;

                    bool isCharging;
                    bool isBlindJumping;
            };

            struct BeaconClientData
            {
                    std::unordered_set<ClientId> incomingClients;
                    BeaconData* beaconInfo;
            };

            std::unordered_map<ClientId, HyperjumpClientData> clientData;
            std::unordered_map<ClientId, BeaconClientData> beaconData;

            std::unordered_map<ClientId, ClientId> pendingBeaconRequestMap;

            bool IsPlayerCloaked(ClientId client);
            void BeaconRequestTimeout(ClientId targetClient);
            void ProcessChargingJumpDrives();

            void ShutdownJumpDrive(ClientId client, bool graceful = false);
            bool JumpDriveCheck(ClientId client);

            void InitiateJump(ClientId client, HyperjumpClientData& jumpDrive, SystemId targetSystem, bool printSectors);
            bool SetFuelForRange(ClientId client, uint range);
            void ListJumpableSystems(ClientId client);
            std::optional<uint> GetJumpRange(ClientId client, SystemId targetSystem);
            bool CanBeaconJump(ClientId client, ClientId targetClient);
            bool BeaconCheck(ClientId client);
            bool TryFuelConsume(ClientId client, Id fuel, ushort amount);
            bool CheckFuel(ClientId client, Id fuel, ushort amount);
            void SpawnJumpHole(ClientId client);

            /**
             * @brief Used to return from the arena system.
             */
            concurrencpp::result<void> UserCmdJump(ClientId client, std::optional<SystemId> system );
            concurrencpp::result<void> UserCmdJumpList(ClientId client);
            concurrencpp::result<void> UserCmdJumpStop(ClientId client);
            concurrencpp::result<void> UserCmdJumpSectors(ClientId client, std::optional<SystemId> system );
            concurrencpp::result<void> UserCmdJumpBlind(ClientId client);
            concurrencpp::result<void> UserCmdCanJump(ClientId client, SystemId targetSystem);
            concurrencpp::result<void> UserCmdCanBeacon(ClientId client, ClientId targetClient);
            concurrencpp::result<void> UserCmdJumpBeacon(ClientId client, ClientId targetClient);
            concurrencpp::result<void> UserCmdAcceptBeacon(ClientId client);
            concurrencpp::result<void> UserCmdSetSector(ClientId client, uint index);

            bool OnLoadSettings() override;
            void OnPlayerLaunchAfter(ClientId client, const ShipId& shipId) override;
            void OnClearClientInfo(ClientId client) override;


            // clang-format off
            inline static const std::array<CommandInfo<HyperjumpPlugin>, 10> commands =
            {
                {
                    AddCommand(HyperjumpPlugin, Cmds(L"/jump"), UserCmdJump, L"/jump", L" Sends you to the designated arena system."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/jump list"), UserCmdJumpList, L"/jump stop", L" Sends you to the designated arena system."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/jump stop"), UserCmdJumpStop, L"/jump stop", L" Sends you to the designated arena system."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/jump sectors"), UserCmdJumpSectors, L"/jump sectors", L" Sends you to the designated arena system."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/jump blind"), UserCmdJumpBlind, L"/jump blind", L" Jumps to a random valid system."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/canjump"), UserCmdCanJump, L"/canjump <systemName>", L" Tells you if selected system is in your jump range."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/canbeacon"), UserCmdCanBeacon, L"/canbeacon <playerName/Id>", L" Tells you if you can perform a beacon jump to target player."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/jumpbeacon"), UserCmdJumpBeacon, L"/canjump <systemName>", L" Sends a beacon jump request to the target player."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/acceptbeacon"), UserCmdAcceptBeacon, L"/acceptbeacon <playerName/Id>", L" Accepts selected beacon jump request."),
                    AddCommand(HyperjumpPlugin, Cmds(L"/setsector"), UserCmdSetSector, L"/setsector <index>", L" Sets the jump sector within the system to the selected one."),
                }
            };
            SetupUserCommandHandler(HyperjumpPlugin, commands);
            // clang-format on

        public:
            explicit HyperjumpPlugin(const PluginInfo& info);
            bool IsPlayerJumping(ClientId client);
            static constexpr std::wstring_view pluginName = L"hyperjump";
    };
} // namespace Plugins
