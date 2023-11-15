#pragma once

#include "AddressList.hpp"

class InternalApi;
class IServerImplHook;
class StartupCache;
class ClientList;
class InfocardManager;
class Logger;
class TempBanManager;

class FLHook final
{
        friend IServerImplHook;
        friend IClientImpl;
        friend InternalApi;
        friend ClientId;
        friend ShipId;

        // Static things

        inline static FLHook* instance;

        inline static HMODULE moduleFLHook;
        inline static HMODULE serverDll;
        inline static HMODULE commonDll;
        inline static HMODULE contentDll;
        inline static HMODULE dalibDll;
        inline static HMODULE remoteClient;
        inline static HMODULE flProc;

        inline static FARPROC oldShipDestroyed;
        inline static FARPROC oldNonGunWeaponHitsBase;
        inline static FARPROC oldDamageHit;
        inline static FARPROC oldDamageHit2;
        inline static FARPROC oldDisconnectPacketSent;
        inline static FARPROC oldGuidedHit;

        inline static RCSendChatMsgT rcSendChatMsg;
        inline static CRCAntiCheatT crcAntiCheat;
        inline static GetShipInspectT getShipInspect;

        inline static bool g_NonGunHitsBase;
        inline static float g_LastHitPts;

        inline static CDPServer* cdpServer;
        inline static CDPClientProxy** clientProxyArray;
        inline static IClientImpl* fakeClientImpl;
        inline static IClientImpl* hookClientImpl;
        inline static char* oldClientImpl;

        struct PatchInfo
        {
                const char* BinName;
                ulong baseAddress;

                struct Entry
                {
                        ulong address;
                        void* newValue;
                        uint size;
                        void* oldValue;
                        bool allocated;
                };

                std::array<Entry, 128> entries;
        };

        static PatchInfo exePatch;
        static PatchInfo contentPatch;
        static PatchInfo commonPatch;
        static PatchInfo serverPatch;
        static PatchInfo remoteClientPatch;
        static PatchInfo dalibPatch;

        inline static FARPROC oldUpdateLoop;
        static void Shutdown() { delete instance; }
        static void Startup() { instance = new FLHook(); }

        static void LoadSettings();
        static void ClearClientInfo(ClientId client);
        void LoadUserSettings(uint client);
        static void ProcessPendingCommands();

        // Timers
        static void PublishServerStats();
        static void TimerTempBanCheck();
        static void TimerCheckKick();
        static void TimerNpcAndF1Check();

        static bool ApplyPatch(PatchInfo& pi);
        static bool RevertPatch(PatchInfo& pi);

        // Non-Static things

        StartupCache* startupCache;
        ClientList* clientList;
        InfocardManager* infocardManager;
        Logger* logger;
        TempBanManager* tempbanManager;

        bool OnServerStart();
        void InitHookExports();
        void PatchClientImpl();
        void UnloadHookExports();

        uint damageToClientId;
        uint damageToSpaceId;

        bool messagePrivate;
        bool messageSystem;
        bool messageUniverse;

        std::wstring accPath;

        uint serverLoadInMs;
        uint playerCount;
        bool disableNpcs;

        std::list<BaseInfo> allBases;

        bool flhookReady;

    public:
        FLHook();
        ~FLHook();
        FLHook(const FLHook&) = delete;
        FLHook& operator=(FLHook) = delete;
        FLHook(FLHook&&) = delete;
        FLHook& operator=(FLHook&&) = delete;

        static void SetupEventLoop();

        enum class BinaryType
        {
            Exe,
            Server,
            Common,
            DaLib,
            Content,
            RemoteClient,
        };

        static DWORD __stdcall Offset(BinaryType type, AddressList address);

        static bool IsReady() { return instance != nullptr && instance->flhookReady; }
        static const std::list<BaseInfo>& GetBases() { return instance->allBases; }
        static std::wstring_view GetAccountPath() { return instance->accPath; }
        static bool GetShipInspect(uint& ship, IObjInspectImpl*& inspect, uint& dunno) { return getShipInspect(ship, inspect, dunno); }

        static ClientList& Clients() { return *instance->clientList; }
        static InfocardManager& GetInfocardManager() { return *instance->infocardManager; }
        static Logger& GetLogger() { return *instance->logger; }
        static TempBanManager& GetTempBanManager() { return *instance->tempbanManager; }

        static Action<void, Error> MessageUniverse(std::wstring_view message);
};
