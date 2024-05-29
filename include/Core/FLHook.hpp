#pragma once

#include "API/Utils/Logger.hpp"

#include "AddressList.hpp"

#include <mongocxx/pool.hpp>

class InternalApi;
class IServerImplHook;
class IEngineHook;
class StartupCache;
class ClientList;
class InfocardManager;
class Logger;
class FlPacket;
class PersonalityHelper;
class Database;
class AccountManager;
class CrashCatcher;

class DLL FLHook final
{
        friend InternalApi;
        friend IEngineHook;
        friend IServerImplHook;
        friend IClientImpl;
        friend InternalApi;
        friend ClientId;
        friend ShipId;
        friend FlPacket;

        // Static things

        inline static FLHook* instance;

        inline static HMODULE moduleFLHook;
        inline static HMODULE serverDll;
        inline static HMODULE commonDll;
        inline static HMODULE contentDll;
        inline static HMODULE dalibDll;
        inline static HMODULE remoteClient;
        inline static HMODULE flProc;

        inline static RCSendChatMsgT rcSendChatMsg;
        inline static CRCAntiCheatT crcAntiCheat;
        inline static GetShipInspectT getShipInspect;

        inline static bool nonGunHitsBase;
        inline static float lastHitPts;
        inline static ClientId dmgToClient;
        inline static ObjectId dmgToSpaceId;

        inline static CDPServer* cdpServer;
        inline static CDPClientProxy** clientProxyArray;
        inline static IClientImpl* fakeClientImpl;
        inline static IClientImpl* hookClientImpl;
        inline static char* oldClientImpl;

        inline static FARPROC oldUpdateLoop;
        static void Shutdown() { delete instance; }
        static void Startup() { instance = new FLHook(); }

        static void LoadSettings();
        static void ClearClientInfo(ClientId client);
        static void ProcessPendingCommands();
        static void ProcessPendingAsyncTasks();

        // Timers
        static void PublishServerStats();
        static void TimerCheckKick();
        static void OneSecondTimer();

        // Non-Static things

        ClientList* clientList;
        Database* database;
        InfocardManager* infocardManager;
        PersonalityHelper* personalityHelper;
        AccountManager* accountManager;
        FLHookConfig* flhookConfig;
        CrashCatcher* crashCatcher;

        bool OnServerStart();
        void InitHookExports();
        static void PatchClientImpl();
        void UnloadHookExports();

        void LoadUserSettings(ClientId client);

        uint damageToClientId;

        bool messagePrivate;
        bool messageSystem;
        bool messageUniverse;

        std::wstring accPath;

        uint serverLoadInMs;
        uint playerCount;
        bool disableNpcs;

        bool flhookReady;

        std::unordered_map<std::wstring, std::vector<std::wstring_view>> credentialsMap = {
            { L"console", { L"SuperAdmin" } }
        };

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

        struct LastHitInformation
        {
                bool nonGunHitsBase;
                float lastHitPts;
                ClientId lastHitClient;
                ObjectId lastHitObject;
        };

        static DWORD __stdcall Offset(BinaryType type, AddressList address);

        static bool IsReady();
        static std::wstring_view GetAccountPath();
        static bool GetObjInspect(uint& ship, IObjInspectImpl*& inspect);

        static AccountManager& GetAccountManager();
        static const std::unordered_map<std::wstring, std::vector<std::wstring_view>>& GetAdmins();
        static ClientList& Clients();
        static ClientData& GetClient(ClientId client);
        static FLHookConfig& GetConfig();
        static Database& GetDatabase();
        static mongocxx::pool::entry GetDbClient();
        static InfocardManager& GetInfocardManager();
        static LastHitInformation GetLastHitInformation();
        static IClientImpl* GetPacketInterface();
        static Action<pub::AI::Personality*, Error> GetPersonality(const std::wstring& pilotNickname);

        static Action<void, Error> MessageUniverse(std::wstring_view message);
};
