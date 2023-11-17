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
        static int FreeReputationVibe(const int& p1);
        static void UpdateTime(double interval);
        static void __stdcall ElapseTime(float interval);
        static int _cdecl DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response);
        static bool __stdcall LaunchPosition(uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, int dock);
        static void __stdcall CShipInit(CShip* ship);

        static void NakedCShipDestroy();
        static void NakedCShipInit();
        static void NakedShipDestroyed();
        static void NakedGuidedHit();
        static void NakedDamageHit();
        static void NakedDamageHit2();
        static void NakedNonGunWeaponHitsBase();
        static void NakedDisconnectPacketSent();
        static void NakedAddDamageEntry();
        static void NakedLaunchPosition();
        static void NakedLoadReputationFromCharacterFile();

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
};
