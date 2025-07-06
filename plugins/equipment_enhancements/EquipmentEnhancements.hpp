#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date 2024
     * @author Aingar (Ported in 2025)
     * @brief
     * This plugin adds several combat mechanics such as armor, equipment shutdown on collision group loss etc.
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
     * This plugin has no player commands
     *
     * @par Admin Commands
     * This plugin has no admin commands
     */
    class EquipmentEnhancementsPlugin final : public Plugin
    {
            const int ARMOR_MOD = 100;

            struct ExplosionDamageData
            {
                    uint weaponType = 0;
                    float percentageDamageHull = 0.0f;
                    float percentageDamageShield = 0.0f;
                    float percentageDamageEnergy = 0.0f;
                    int armorPen = 0;
                    float detDist = 0.0f;
                    bool cruiseDisrupt = false;
                    bool damageSolars = true;
                    bool missileDestroy = false;
            };

            struct BurstFireGunData
            {
                    int maxMagSize;
                    int bulletsLeft;
                    float reloadTime;
            };

            struct BurstFireData
            {
                    int magSize;
                    float reloadTime;
            };

            struct MineInfo
            {
                    float armingTime = 0.0f;
                    float dispersionAngle = 0.0f;
                    bool detonateOnEndLifetime = false;
                    bool stopSpin = false;
            };

            struct GuidedData
            {
                    bool noTrackingAlert = false;
                    uint trackingBlacklist = 0;
                    float armingTime = 0.0f;
                    float topSpeed = 0.0f;
            };

            enum class ShieldSource
            {
                UNSET,
                CLOAK,
                MISC
            };
            struct ShieldState
            {
                    bool shieldState = true;
                    ShieldSource changeSource = ShieldSource::UNSET;
                    mstime boostUntil = 0;
                    float damageReduction = 0.0f;
                    float damageTaken = 0.0f;
            };

            struct ShieldSyncData
            {
                    ClientId client;
                    ClientId targetClient;
                    uint count = 0;
            };

            struct ShieldBoostData
            {
                    float durationPerBattery;
                    float minimumDuration;
                    float maximumDuration;
                    float damageReduction;
                    uint fuseId;

                    float hullBaseDamage = 0.0f;
                    float hullReflectDamagePercentage = 0.0f;
                    float hullDamageCap = 0.0f;
                    float energyBaseDamage = 0.0f;
                    float energyReflectDamagePercentage = 0.0f;
                    float energyDamageCap = 0.0f;
                    float radius = 0.0f;
                    uint explosionFuseId = 0;
            };

            struct ShieldBoostFuseInfo
            {
                    int64 lastUntil;
                    const ShieldBoostData* boostData;
            };

            struct ShipData
            {
                    int engineCount = 0;
                    bool internalEngine = false;
                    std::unordered_map<std::string, std::unordered_set<std::string>> engineHpMap;
                    std::unordered_map<ushort, std::vector<std::string>> colGrpHpMap;
                    std::unordered_map<uint, std::vector<std::string>> fuseHpMap;
            };

            struct EngineProperties
            {
                    bool ignoreCDWhenEKd = false;
                    float engineKillCDSpeedLimit;
                    std::string hpType;
            };

            enum class InvulType
            {
                ALL,
                HULLONLY,
                EQUIPONLY,
            };

            struct InvulData
            {
                    float minHpPerc;
                    InvulType invulType;
            };

            enum TRACKING_STATE
            {
                TRACK_ALERT,
                TRACK_NOALERT,
                NOTRACK_NOALERT
            };

            struct SpeedCheck
            {
                    float targetSpeed = 0;
                    uint checkCounter = 0;
            };

            //! Configurable fields for this plugin
            struct Config final
            {
                    // Reflectable fields
                    //! Setting this to true enables HitRay based missile damage detection.
                    //! This causes explosions to hit large targets without needing enormous explosion radiuses
                    //! Also distributes damage between rooted collision groups, preventing instakills on ships with many collision groups
                    bool hitRayMissileLogic = false;

                    //! Set this value to the explosion arch which will be used as a stand-in to deal damage
                    //! triggered by an EMP shield boost explosion. Leave blank if your mod doesn't use this feature.
                    std::string shieldExplosionArch;
            };

            std::unordered_map<uint, ExplosionDamageData> explosionTypeMap;
            std::unordered_map<ClientId, ShieldBoostFuseInfo> shieldFuseMap;
            std::unordered_map<uint, GuidedData> guidedDataMap;
            std::unordered_map<uint, ShipData> shipDataMap;
            inline static std::unordered_map<uint, MineInfo> mineInfoMap;
            std::unordered_map<uint, EngineProperties> engineData;
            std::unordered_map<uint, ShieldBoostData> shieldBoostMap;
            std::unordered_map<uint, std::unordered_map<uint, GoodId>> equipOverrideMap;

            std::unordered_map<uint, int> munitionArmorPenMap;
            std::unordered_map<uint, std::unordered_map<ushort, int>> shipArmorMap;
            std::unordered_map<uint, std::unordered_map<ushort, int>>::iterator shipArmorIter;
            std::vector<float> armorReductionVector;
            std::unordered_map<uint, std::unordered_map<ushort, BurstFireGunData>> shipGunData;
            std::unordered_map<uint, BurstFireData> burstGunData;
            std::unordered_map<uint, uint> NewMissileForcedUpdatePacketMap;

            std::unordered_map<uint, InvulData> invulMap;
            std::array<ShieldState, MaxClientId + 1> playerShieldState;
            std::vector<ShieldSyncData> shieldStateUpdateMap;
            std::vector<std::pair<ClientId, uint>> equipUpdateVector;
            std::unordered_map<uint, SpeedCheck> topSpeedWatch;

            Archetype::Explosion* shieldExplosion;
            
            bool usedBatts;

            bool armorEnabled;
            int weaponArmorPenValue;
            uint weaponArmorPenArch;
            int shipArmorRating;
            uint shipArmorArch;

            Config config;

            bool OnLoadSettings() override;
            void OnServerUpdateAfter() override;
            void OnClearClientInfo(ClientId client) override;
            bool VerifyEngines(ClientId client);
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;
            void OnCShipInitAfter(CShip* ship) override;
            void OnCELauncherFireAfter(CELauncher* launcher, const Vector& pos, FireResult fireResult) override;

            void OnRequestAddItem(ClientId client, GoodId& goodId, std::wstring_view hardpoint, int count, float status, bool mounted) override;

            void OnShipDespawn(Ship* ship) override;
            void OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) override;
            void OnMineDestroy(Mine * mine, DestroyType& destroyType, ShipId killerId) override;
            void OnGuidedDestroy(Guided * guided, DestroyType& destroyType, ShipId killerId) override;

            void OnShipEquipDmg(Ship* ship, CAttachedEquip* equip, float& incDmg, DamageList* dmg) override;
            void OnShipEquipDestroy(Ship*, CEquip*, DamageEntry::SubObjFate, DamageList*) override;

            void OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList) override;
            void OnSolarExplosionHit(Solar* solar, ExplosionDamageEvent* explosion, DamageList* dmgList) override;

            void OnShipHullDmg(Ship* ship, float& incDmg, DamageList* dmg) override;

            void OnCGuidedInitAfter(CGuided* guided) override;

            void OnSpRequestUseItem(ClientId client, const SSPUseItem& item) override;
            void OnSpRequestUseItemAfter(ClientId client, const SSPUseItem& item) override;

            void ShipExplosionHandlingExtEqColGrpHull(EqObj* iobj, ExplosionDamageEvent* explosion, DamageList* dmg, float& rootDistance,
                                                      ExplosionDamageData* explData);
            bool ShieldAndDistance(EqObj* iobj, ExplosionDamageEvent* explosion, DamageList* dmg, float& rootDistance, ExplosionDamageData* explData);
            void EnergyExplosionHit(EqObj* iobj, ExplosionDamageEvent* explosion, DamageList* dmg, const float rootDistance, ExplosionDamageData* explData);
            void FetchShipArmor(uint shipHash);
            void EqObjExplosionHit(EqObj* iobj, ExplosionDamageEvent* explosion, DamageList* dmg);

            void ReadMunitionDataFromInis();
            static void MineSpin(CMine* mine, Vector& spinVec);
            static void MineImpulse(CMine* mine, Vector& launchVec);

            static float __fastcall GetWeaponModifier(CEShield* shield, void* edx, uint& weaponType);

            // clang-format on

        public:
            explicit EquipmentEnhancementsPlugin(const PluginInfo& info);
    };
} // namespace Plugins
