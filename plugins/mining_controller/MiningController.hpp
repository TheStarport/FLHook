#pragma once

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"
#include "Core/FlufInfocardHelper.hpp"

namespace Plugins
{
    /**
     * @date Jul 2025
     * @author Aingar
     * @brief
     * This plugin controls economy related logic. With clienthook support allows for separated buy/sell prices.
     * It also overrides loot-dropping logic
     */
    class MiningControllerPlugin final : public Plugin, public FlufInfocardHelper//, public AbstractAdminCommandProcessor
    {

            struct MiningNodeInfo
            {
                    Id itemArchId;
                    Id lootArchId;
                    float countMin;
                    float countMax;
            };

            struct MiningArchetype
            {
                    Id archetype;
                    Id loadout;
            };

            struct MiningSpawnPointDB
            {
                    SystemId system;
                    std::string miningNodeGroupName;
                    std::vector<Vector> positions;
                    std::vector<MiningArchetype> nodeArchetypes;
                    std::vector<std::string> nicknamesVector;
                    std::string nodeName;
                    uint respawnCD;
                    uint cdProgress = 0;
                    uint maxSpawnCount;
                    uint spawnedNodesCount = 0;
                    rfl::Skip<uint> idsNumber;
            };

            struct NodeInfo
            {
                    MiningSpawnPointDB* miningDB;
                    std::string nickname;
            };

            struct MiningBonus
            {
                    float bonusStandard = 1.0f;
                    float bonusAsteroidNodes = 1.0f;
                    float bonusAsteroidDestruction = 1.0f;
            };

            struct ZONE_BONUS
            {
                    std::wstring zoneName;

                    // The loot bonus multiplier.
                    float multiplier;

                    // The hash of the item to replace the dropped
                    Id replacementLootID;

                    // The recharge rate of the zone. This is the number of units of ore added
                    // to the reserve per minute.
                    float rechargeRate;

                    // The current amount of ore in the zone. When this gets low, ore gets harder
                    // to mine. When it gets to 0, ore is impossible to mine.
                    float currReserve = 50000;

                    // The maximum limit for the amount of ore in the field
                    float maxReserve = 50000;

                    // The amount of ore that has been mined.
                    float mined;
            };

            enum class MiningType
            {
                DynamicAsteroid,
                StaticAsteroidNode,
                StaticAsteroidCode
            };
            //! Configurable fields for this plugin
            struct Config final
            {
                    // Reflectable fields

                    bool scaleFieldRechargeWithPlayerCount = false;

                    //! Maximum expected dynamic asteroid mine events per second, triggers silent logging of a player.
                    float miningCheatLogThreshold = 2.0f;

                    Id miningMunition = Id("mining_gun_ammo");

                    Id deployableContainerCommodity = Id("commodity_deployable_container");
                    Id deployableContainerArchetype = Id("dsy_playerbase_01");
                    Id deployableContainerLoadout = Id("dsy_playerbase_01");

                    Id insufficientCargoSound = Id("insufficient_cargo_space");

                    //! Time after which a partially damaged asteroid will despawn
                    uint miningAsteroidDestructionTimer = 180;

                    float globalModifier = 1.0f;
                    float miningContainerModifier = 1.05f;

                    int containerJettisonCount = 1000;

                    std::unordered_map<uint, MiningBonus> shipClassModifiersMap;

                    std::unordered_map<Id, std::array<MiningNodeInfo, 32>> miningSolarMap;

                    std::unordered_map<Id, ZONE_BONUS> mapZoneBonus;

                    std::unordered_map<Id, std::unordered_map<Id, float>> idBonusMap;
            };
            Config config;

            struct MiningAreasConfig final
            {
                    std::vector<MiningSpawnPointDB> miningAreas;
            };
            MiningAreasConfig miningAreasConfig;

            struct CONTAINER_DATA
            {
                    Id loot1Id;
                    uint loot1Count;
                    std::wstring loot1Name;
                    Id loot2Id;
                    uint loot2Count;
                    std::wstring loot2Name;
                    uint nameIDS;
                    std::wstring solarName;
                    SystemId systemId;
                    Vector jettisonPos;
                    ClientId clientId;
            };

            std::unordered_map<Id, CONTAINER_DATA> mapMiningContainers;

            struct CLIENT_DATA
            {
                    bool initialized;
                    Id equippedID;
                    float equippedVolume;
                    Id lootID;
                    uint itemCount;
                    uint miningEvents;
                    uint miningSampleStart;
                    float overminedFraction;
                    Id deployedContainerId;
                    Id lastValidContainerId;
                    CONTAINER_DATA* lastValidContainer;
                    MiningBonus shipClassMiningBonus;

                    time_t LastTimeMessageAboutBeingFull;
            };
            std::unordered_map<ClientId, CLIENT_DATA> mapClients;

            std::unordered_map<Id, NodeInfo> miningNodeMap;
            std::unordered_map<uint, int64> pendingDestructionNodes;

            void DestroyContainer(ClientId clientID);
            void CheckClientSetup(ClientId client);
            void LogAsteroidField(std::wstring_view& zoneNick);

            float GetMiningYieldBonus(Id playerId, Id lootId);

            uint GetAsteroidMiningYield(const MiningNodeInfo& node, ClientId client, bool isNode);

            void SpawnNode(MiningSpawnPointDB& ms);

            void SpawnNewNodes();

            void DestroyPendingNodes();

            void SaveZoneStatusToDb();

            bool OnLoadSettings() override;
            void OnClearClientInfo(ClientId client) override;
            void OnCharacterSelect(ClientId client) override;
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;
            void OnDisconnect(ClientId client, EFLConnection conn) override;
            void OnBaseEnter(BaseId base, ClientId client) override;
            void OnSystemSwitchOutComplete(ClientId client, const ShipId& ship) override;
            void OnSolarDestroy(Solar* solar, DestroyType& destroyType, ShipId killerId) override;
            void OnMineAsteroid(ClientId client, SystemId system, const Vector& pos, Id crateId, Id lootId, uint count) override;
            void OnSpMunitionCollision(ClientId client, const SSPMunitionCollisionInfo& info) override;
            void OnCargoJettison(ClientId client, const XJettisonCargo& cargo) override;
            void OnSolarColGrpDestroy(Solar* solar, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList, bool killParented) override;


            // clang-format off
            //inline static const std::array<AdminCommandInfo<MiningControllerPlugin>, 1> commands =
            //{
            //    {
            //        AddAdminCommand(MiningControllerPlugin, Cmds(L".reloadMining"), AdminCmdReloadConfig, GameAndConsole, Any, L".reloadMining", L"Reloads mining plugin config from the json file"),
            //    }
            //};
            // clang-format on

            //SetupAdminCommandHandler(MiningControllerPlugin, commands);

        public:
            explicit MiningControllerPlugin(const PluginInfo& info);
    };
} // namespace Plugins
