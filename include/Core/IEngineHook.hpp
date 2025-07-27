#pragma once
#include "Core/VTables.hpp"
#include "FLHook.hpp"
#include "Utils/Detour.hpp"

class FLHook;
class IEngineHook
{
        friend FLHook;
        using SendCommType = int (*)(uint sender, uint receiver, uint voiceId, const Costume* costume, uint infocardId, uint* lines, int lineCount,
                                     uint infocardId2, float radioSilenceTimerAfter, bool global);

        enum ZoneDamageType
        {
            ZONEDMG_HULL = 1 << 0,
            ZONEDMG_SHIELD = 1 << 1,
            ZONEDMG_ENERGY = 1 << 2,
            ZONEDMG_CRUISE = 1 << 3,
        };

        struct ZoneSpecialData
        {
                uint dmgType = ZONEDMG_HULL;
                float percentageDamage;
                float flatDamage;

                float distanceScaling;
                float logScale;
                float shieldMult;
                float energyMult;
        };

        inline static std::unordered_map<Id, ZoneSpecialData> zoneSpecialData;

        struct LoadRepData
        {
                uint repId;
                float attitude;
        };

        struct RepDataList
        {
                uint dunno;
                LoadRepData* begin;
                LoadRepData* end;
        };

        struct SendCommData
        {
                struct Callsign
                {
                        static constexpr uint AlphaDesignation = 197808;
                        static constexpr uint FreelancerCommHash = 0xb967660b;
                        static constexpr uint FreelancerAffiliation = 0x1049;

                        uint lastFactionAff = 0;
                        uint factionLine = FreelancerCommHash;
                        uint formationLine;
                        uint number1;
                        uint number2;
                };

                struct FactionData
                {
                        uint msgId;
                        std::vector<uint> formationHashes;
                };

                std::unordered_map<uint, Callsign> callsigns;
                std::unordered_map<uint, FactionData> factions;
                std::unordered_map<uint, std::pair<uint, uint>> numberHashes; // number said in the middle and in the end
        };

        inline static SendCommData sendCommData;

        static void Init();

        inline static FARPROC oldLoadReputationFromCharacterFile =
            reinterpret_cast<FARPROC>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoadPatch) + 7);
        inline static FARPROC oldLaunchPosition;
        inline static FARPROC oldDisconnectPacketSent;
        inline static uint lastTicks;

        static int FreeReputationVibe(const int& p1);
        static void UpdateTime(double interval);
        static void __stdcall ElapseTime(float interval);
        static int DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response);
        static bool __stdcall LaunchPosition(uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock);

#define VTablePtr(x) static_cast<DWORD>(x)

        inline static VTableHook<VTablePtr(CShipVTable::Start), VTablePtr(CShipVTable::End)> cShipVTable{ "common" };
        inline static VTableHook<VTablePtr(CLootVTable::Start), VTablePtr(CLootVTable::End)> cLootVTable{ "common" };
        inline static VTableHook<VTablePtr(CSolarVTable::Start), VTablePtr(CSolarVTable::End)> cSolarVTable{ "common" };
        inline static VTableHook<VTablePtr(CGuidedVTable::Start), VTablePtr(CGuidedVTable::End)> cGuidedVTable{ "common" };
        inline static VTableHook<VTablePtr(CELauncherVTable::Start), VTablePtr(CELauncherVTable::End)> ceLauncherVTable{ "common" };

        inline static VTableHook<VTablePtr(IShipInspectVTable::Start), VTablePtr(IShipInspectVTable::End)> iShipVTable{ "server" };
        inline static VTableHook<VTablePtr(ISolarInspectVTable::Start), VTablePtr(ISolarInspectVTable::End)> iSolarVTable{ "server" };
        inline static VTableHook<VTablePtr(ILootInspectVTable::Start), VTablePtr(ILootInspectVTable::End)> iLootVTable{ "server" };
        inline static VTableHook<VTablePtr(IGuidedInspectVTable::Start), VTablePtr(IGuidedInspectVTable::End)> iGuidedVTable{ "server" };
        inline static VTableHook<VTablePtr(IMineInspectVTable::Start), VTablePtr(IMineInspectVTable::End)> iMineVTable{ "server" };

#undef VTablePtr

        static void __fastcall ShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId);
        static void __fastcall LootDestroy(Loot* loot, void* edx, DestroyType destroyType, Id killerId);
        static void __fastcall SolarDestroy(Solar* solar, void* edx, DestroyType destroyType, Id killerId);
        static void __fastcall MineDestroy(Mine* mine, void* edx, DestroyType destroyType, Id killerId);
        static void __fastcall GuidedDestroy(Guided* mine, void* edx, DestroyType destroyType, Id killerId);

        static void __fastcall ShipHullDamage(Ship* ship, void* edx, float damage, DamageList* dmgList);
        static void __fastcall SolarHullDamage(Solar* ship, void* edx, float damage, DamageList* dmgList);

        static void __fastcall SolarColGrpDestroy(Solar* solar, void* edx, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList, bool killParent);

        static void __fastcall ShipEquipDmg(Ship* ship, void* edx, CAttachedEquip* equip, float damage, DamageList* dmgList);
        static void __fastcall ShipEquipDestroy(Ship* ship, void* edx, CEquip* equip, DamageEntry::SubObjFate fate, DamageList* dmgList);

        static void __fastcall ShipColGrpDmg(Ship*, void* edx, CArchGroup* colGrp, float incDmg, DamageList* dmg);
        static void __fastcall ShipColGrpDestroy(Ship* ship, void* edx, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList,
                                                 bool killLinkedElements);

        static void __fastcall ShipDropAllCargo(Ship* ship, void* edx, const char* hardPoint, DamageList* dmgList);

        static void __fastcall ShipRadiationDamage(Ship* ship, void* edx, float deltaTime, DamageList* dmgList);

        static void __fastcall GuidedExplosionHit(Guided* guided, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);
        static void __fastcall SolarExplosionHit(Solar* guided, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);
        static void __fastcall ShipMunitionHit(Ship* ship, void* edx, MunitionImpactData* impact, DamageList* dmgList);
        static void __fastcall ShipExplosionHit(Ship* ship, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);

        static void __fastcall ShipShieldDmg(Ship* iobj, void* edx, CEShield* shield, float incDmg, DamageList* dmg);
        static void __fastcall ShipEnergyDmg(Ship* ship, void* edx, float incDmg, DamageList* dmgList);

        static void __fastcall CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams);
        static void __fastcall CSolarInit(CSolar* solar, void* edx, CSolar::CreateParms* creationParams);
        static void __fastcall CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* createParams);
        static void __fastcall CGuidedInit(CGuided* guided, void* edx, CGuided::CreateParms* creationParams);

        static void __fastcall ShipFuse(Ship* ship, void* edx, uint fuseCause, uint& fuseId, ushort sId, float radius, float lifetime);

        static int __fastcall GetAmmoCapacityDetourHash(CShip* cship, void* edx, Id ammoArch);
        static int __fastcall GetAmmoCapacityDetourEq(CShip* cship, void* edx, Archetype::Equipment* ammoType);
        static float __fastcall GetCargoRemaining(CShip* cship);
        static int __fastcall GetSpaceForCargoType(CShip* cship, void* edx, Archetype::Equipment* archEquip);

        static TractorFailureCode __fastcall CETractorVerifyTarget(CETractor* tractor, void* edx, CLoot* target);

        static FireResult __fastcall CELauncherFireAfter(CELauncher* launcher, void* edx, const Vector& pos);

        struct CallAndRet final : Xbyak::CodeGenerator
        {
                CallAndRet(void* toCall, void* ret);
        };

        struct DisconnectPacketSentAssembly final : Xbyak::CodeGenerator
        {
                DisconnectPacketSentAssembly();
        };

        struct LaunchPositionAssembly final : Xbyak::CodeGenerator
        {
                LaunchPositionAssembly();
        };

        struct LoadReputationFromCharacterFileAssembly final : Xbyak::CodeGenerator
        {
                LoadReputationFromCharacterFileAssembly();
        };

        inline static DisconnectPacketSentAssembly* disconnectPacketSentAssembly;
        inline static LoadReputationFromCharacterFileAssembly* loadReputationFromCharacterFileAssembly;
        inline static LaunchPositionAssembly launchPositionAssembly;

        static bool AllowPlayerDamage(ClientId client, ClientId clientTarget);

        static bool __stdcall LoadReputationFromCharacterFile(const RepDataList* savedReps, const LoadRepData* repToSave);
        static int SendCommDetour(uint sender, uint receiver, uint voiceId, const Costume* costume, uint infocardId, uint* lines, int lineCount,
                                  uint infocardId2, float radioSilenceTimerAfter, bool global);
        static bool __stdcall DisconnectPacketSent(ClientId client);
        static void SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller);

        using CGunWrapperShutdownFunc = void(__fastcall*)(void*);
        inline static std::unique_ptr<FunctionDetour<CGunWrapperShutdownFunc>> disconnectPacketDetour = nullptr;
        inline static std::unique_ptr<FunctionDetour<SendCommType>> sendCommDetour;

    public:
        static void OnPlayerLaunch(ClientId client);
        static void OnCharacterSelectAfter(ClientId client);
};
