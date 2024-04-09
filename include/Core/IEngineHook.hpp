#pragma once
#include "Core/VTables.hpp"
#include "Utils/Detour.hpp"
#include "FLHook.hpp"

class FLHook;
class IEngineHook
{
        friend FLHook;

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

        inline static FARPROC oldInitCShip;
        inline static FARPROC oldDestroyCShip;
        inline static FARPROC oldDestroyCLoot;
        inline static FARPROC oldDestroyCSolar;
        inline static FARPROC oldLoadReputationFromCharacterFile = reinterpret_cast<FARPROC>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoadPatch) + 7);
        inline static FARPROC oldLaunchPosition;
        inline static FARPROC oldGuidedHit;
        inline static FARPROC oldDamageHit;
        inline static FARPROC oldDamageHit2;
        inline static FARPROC nonGunWeaponHitsBaseRetAddress;
        inline static FARPROC oldNonGunWeaponHitsBase;
        inline static FARPROC oldDisconnectPacketSent;
        inline static FARPROC oldShipDestroyed;
        inline static uint lastTicks;

        static int FreeReputationVibe(const int& p1);
        static void UpdateTime(double interval);
        static void __stdcall ElapseTime(float interval);
        static int DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response);
        static bool __stdcall LaunchPosition(uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock);
        static void __fastcall DaLibDisconnect(void* ecx);

#define VTablePtr(x) static_cast<DWORD>(x)

        inline static VTableHook<CShip, VTablePtr(CShipVTable::Start), VTablePtr(CShipVTable::End)> cShipVTable;
        inline static VTableHook<CLoot, VTablePtr(CLootVTable::Start), VTablePtr(CLootVTable::End)> cLootVTable;
        inline static VTableHook<CShip, VTablePtr(CSolarVTable::Start), VTablePtr(CSolarVTable::End)> cSolarVtable;

        inline static VTableHook<Ship, VTablePtr(IShipInspectVTable::Start), VTablePtr(IShipInspectVTable::End)> iShipVTable;
        inline static VTableHook<Solar, VTablePtr(ISolarInspectVTable::Start), VTablePtr(ISolarInspectVTable::End)> iSolarVTable;
        inline static VTableHook<Loot, VTablePtr(ILootInspectVTable::Start), VTablePtr(ILootInspectVTable::End)> iLootVTable;

#undef VTablePtr

        static void __fastcall ShipDestroy(Ship* ship, void* edx, bool isKill, uint killerId);
        static void __fastcall LootDestroy(Loot* ship, void* edx, bool isKill, uint killerId);
        static void __fastcall SolarDestroy(Solar* ship, void* edx, bool isKill, uint killerId);

        static void __fastcall ShipHullDamage(Ship* ship, void* edx, float damage, DamageList* dmgList);
        static void __fastcall SolarHullDamage(Solar* ship, void* edx, float damage, DamageList* dmgList);

        static void __fastcall ShipExplosionHit(Ship* ship, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList);

        static void __fastcall CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams);
        static void __fastcall CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* createParams);
        static void __fastcall CSolarInit(CSolar* solar, void* edx, CSolar::CreateParms* createParms);

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

        // The pointers are for

        inline static DisconnectPacketSentAssembly* disconnectPacketSentAssembly;
        inline static LoadReputationFromCharacterFileAssembly* loadReputationFromCharacterFileAssembly;
        inline static LaunchPositionAssembly launchPositionAssembly;

        static bool AllowPlayerDamage(ClientId client, ClientId clientTarget);

        static bool __stdcall LoadReputationFromCharacterFile(const RepDataList* savedReps, const LoadRepData* repToSave);
        static void BaseDestroyed(ObjectId objectId, ClientId clientBy);
        static bool __stdcall DisconnectPacketSent(ClientId client);
        static void SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller);

        using CGunWrapperShutdownFunc = void (__fastcall*)(void*);
        inline static std::unique_ptr<FunctionDetour<CGunWrapperShutdownFunc>> disconnectPacketDetour = nullptr;
};
