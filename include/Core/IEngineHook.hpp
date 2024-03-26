#pragma once
#include "Core/VTables.hpp"
#include "Utils/Detour.hpp"

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
        inline static FARPROC oldLoadReputationFromCharacterFile;
        inline static FARPROC oldLaunchPosition;
        inline static FARPROC oldGuidedHit;
        inline static FARPROC oldDamageHit;
        inline static FARPROC oldDamageHit2;
        inline static ulong nonGunWeaponHitsBaseRetAddress;
        inline static FARPROC oldNonGunWeaponHitsBase;
        inline static FARPROC oldDisconnectPacketSent;
        inline static FARPROC oldShipDestroyed;
        inline static uint lastTicks;

        static int FreeReputationVibe(const int& p1);
        static void UpdateTime(double interval);
        static void __stdcall ElapseTime(float interval);
        static int DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response);
        static bool __stdcall LaunchPosition(uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock);

#define VTablePtr(x) static_cast<DWORD>(x)

        inline static VTableHook<CShip, VTablePtr(CShipVTable::Start), VTablePtr(CShipVTable::End)> cShipVTable;
        inline static VTableHook<CLoot, VTablePtr(CLootVTable::Start), VTablePtr(CLootVTable::End)> cLootVTable;
        inline static VTableHook<CShip, VTablePtr(CSolarVTable::Start), VTablePtr(CSolarVTable::End)> cSolarVtable;

#undef VTablePtr

        static void __fastcall CShipDestroy(CShip* ship);
        static void __fastcall CLootDestroy(CLoot* loot);
        static void __fastcall CSolarDestroy(CSolar* solar);
        static void __fastcall CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams);
        static void __fastcall CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* createParams);
        static void __fastcall CSolarInit(CSolar* solar, void* edx, CSolar::CreateParms* createParms);

        struct CallAndRet final : Xbyak::CodeGenerator
        {
                CallAndRet(void* toCall, void* ret);
        };

        struct CShipInitAssembly final : Xbyak::CodeGenerator
        {
                CShipInitAssembly();
        };

        struct ShipDestroyAssembly final : Xbyak::CodeGenerator
        {
                ShipDestroyAssembly();
        };

        struct GuidedHitAssembly final : Xbyak::CodeGenerator
        {
                GuidedHitAssembly();
        };

        struct NonGunWeaponHitBaseAssembly final : Xbyak::CodeGenerator
        {
                NonGunWeaponHitBaseAssembly();
        };

        struct DisconnectPacketSentAssembly final : Xbyak::CodeGenerator
        {
                DisconnectPacketSentAssembly();
        };

        struct AddDamageEntryAssembly final : Xbyak::CodeGenerator
        {
                AddDamageEntryAssembly();
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

        inline static CShipInitAssembly* cShipInitAssembly;
        inline static ShipDestroyAssembly* shipDestroyAssembly;
        inline static NonGunWeaponHitBaseAssembly* nonGunWeaponHitBaseAssembly;
        inline static DisconnectPacketSentAssembly* disconnectPacketSentAssembly;
        inline static AddDamageEntryAssembly* addDamageEntryAssembly;
        inline static LoadReputationFromCharacterFileAssembly* loadReputationFromCharacterFileAssembly;
        inline static LaunchPositionAssembly launchPositionAssembly;
        inline static GuidedHitAssembly guidedHitAssembly;

        static bool AllowPlayerDamage(ClientId client, ClientId clientTarget);

        static bool __stdcall LoadReputationFromCharacterFile(const RepDataList* savedReps, const LoadRepData* repToSave);
        static void NonGunWeaponHitsBaseAfter();
        static void BaseDestroyed(ObjectId objectId, ClientId clientBy);
        static void __stdcall DamageHit(char* ecx);
        static bool __stdcall GuidedHit(char* ecx, char* p1, DamageList* dmgList);
        static void __stdcall AddDamageEntry(DamageList* dmgList, unsigned short subObjId, float hitPts, DamageEntry::SubObjFate fate);
        static void __stdcall NonGunWeaponHitsBaseBefore(const char* ECX, const char* p1, const DamageList* dmg);
        static int __stdcall DisconnectPacketSent(ClientId client);
        static void SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller);
        static void __stdcall ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill);

        inline static CallAndRet cShipDestroyAssembly{ IEngineHook::CShipDestroy, &oldDestroyCShip };
        inline static CallAndRet cSolarDestroyAssembly{ IEngineHook::CSolarDestroy, &oldDestroyCSolar };
        inline static CallAndRet cLootDestroyAssembly{ IEngineHook::CLootDestroy, &oldDestroyCLoot };
        ;
        inline static CallAndRet damageHitAssembly1{ IEngineHook::DamageHit, &oldDamageHit };
        inline static CallAndRet damageHitAssembly2{ IEngineHook::DamageHit, &oldDamageHit2 };
};
