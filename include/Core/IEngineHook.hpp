#pragma once

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

        static void __stdcall CShipDestroy(CShip* ship);
        static void __stdcall CLootDestroy(CLoot* loot);
        static void __stdcall CSolarDestroy(CSolar* solar);
        static int FreeReputationVibe(const int& p1);
        static void UpdateTime(double interval);
        static void __stdcall ElapseTime(float interval);
        static int DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response);
        static bool __stdcall LaunchPosition(uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock);
        static void __stdcall CShipInit(CShip* ship);

        struct CallAndRet : Xbyak::CodeGenerator
        {
                CallAndRet(void* toCall, void* ret);
        };

        inline static struct CShipInitAssembly : Xbyak::CodeGenerator
        {
                CShipInitAssembly();
        } cShipInitAssembly;

        inline static struct ShipDestroyAssembly : Xbyak::CodeGenerator
        {
                ShipDestroyAssembly();
        } shipDestroyAssembly;

        inline static struct GuidedHitAssembly : Xbyak::CodeGenerator
        {
                GuidedHitAssembly();
        } guidedHitAssembly;

        inline static struct NonGunWeaponHitBaseAssembly : Xbyak::CodeGenerator
        {
                NonGunWeaponHitBaseAssembly();
        } nonGunWeaponHitBaseAssembly;

        inline static struct DisconnectPacketSentAssembly : Xbyak::CodeGenerator
        {
                DisconnectPacketSentAssembly();
        } disconnectPacketSentAssembly;

        inline static struct AddDamageEntryAssembly : Xbyak::CodeGenerator
        {
                AddDamageEntryAssembly();
        } addDamageEntryAssembly;

        inline static struct LaunchPositionAssembly : Xbyak::CodeGenerator
        {
                LaunchPositionAssembly();
        } launchPositionAssembly;

        inline static struct LoadReputationFromCharacterFileAssembly : Xbyak::CodeGenerator
        {
                LoadReputationFromCharacterFileAssembly();
        } loadReputationFromCharacterFileAssembly;

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
        inline static CallAndRet cLootDestroyAssembly { IEngineHook::CLootDestroy, &oldDestroyCLoot };;
        inline static CallAndRet damageHitAssembly1{ IEngineHook::DamageHit, &oldDamageHit };
        inline static CallAndRet damageHitAssembly2{ IEngineHook::DamageHit, &oldDamageHit2 };
};
