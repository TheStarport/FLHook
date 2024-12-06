#pragma once

#include "API/Utils/Logger.hpp"

#include "AddressList.hpp"

#include <mongocxx/pool.hpp>

class TaskScheduler;
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
class HttpServer;
class ResourceManager;

static constexpr std::wstring_view ConsoleName = L"CONSOLE";

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
        friend AdminCommandProcessor;

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
        static void TimerCheckKick();
        static void OneSecondTimer();

        // Non-Static things

        std::shared_ptr<ClientList> clientList;
        std::shared_ptr<Database> database;
        std::shared_ptr<InfocardManager> infocardManager;
        std::shared_ptr<PersonalityHelper> personalityHelper;
        std::shared_ptr<TaskScheduler> taskScheduler;
        std::shared_ptr<AccountManager> accountManager;
        std::shared_ptr<ResourceManager> resourceManager;
        std::shared_ptr<FLHookConfig> flhookConfig;
        std::shared_ptr<CrashCatcher> crashCatcher;
        std::shared_ptr<HttpServer> httpServer;

        bool OnServerStart();
        void InitHookExports();
        static void PatchClientImpl();
        void UnloadHookExports();

        uint damageToClientId;

        bool messagePrivate;
        bool messageSystem;
        bool messageUniverse;

        std::wstring accPath;

        uint serverLoadInMs;
        uint playerCount;
        bool disableNpcs;

        bool flhookReady;

        std::unordered_map<ClientId, std::unordered_set<std::wstring>> credentialsMap = {
            { ClientId(), { L"superadmin" } }  // Console is always super admin
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

        static std::shared_ptr<AccountManager> GetAccountManager();
        static const std::unordered_map<ClientId, std::unordered_set<std::wstring>>& GetAdmins();
        static ClientList& Clients();
        static ClientData& GetClient(ClientId client);
        static ClientData* GetClientByName(std::wstring_view characterName);
        static std::shared_ptr<FLHookConfig> GetConfig();
        static std::shared_ptr<Database> GetDatabase();
        static mongocxx::pool::entry GetDbClient();
        static std::shared_ptr<InfocardManager> GetInfocardManager();
        static LastHitInformation GetLastHitInformation();
        static IClientImpl* GetPacketInterface();
        static std::shared_ptr<ResourceManager> GetResourceManager();
        static Action<pub::AI::Personality*> GetPersonality(const std::wstring& pilotNickname);
        static uint GetServerLoadInMs();
        static CDPClientProxy** GetClientProxyArray();
        static std::shared_ptr<TaskScheduler> GetTaskScheduler();

        static Action<void> MessageUniverse(std::wstring_view message);
};
