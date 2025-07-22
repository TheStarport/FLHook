#include "PCH.hpp"

#include "Core/FLHook.hpp"
#include "Core/IEngineHook.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/PersonalityHelper.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "API/InternalApi.hpp"
#include "API/Utils/Logger.hpp"
#include "Core/MemoryManager.hpp"

#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Defs/FLHookConfig.hpp"

#include "Core/AddressList.hpp"
#include "Core/IpResolver.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Exceptions/InvalidParameterException.hpp"

#include "API/FLHook/Plugin.hpp"
#include "API/Utils/ZoneUtilities.hpp"

#include "API/Exceptions/InvalidClientException.hpp"
#include "API/FLHook/HttpServer.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "Core/CrashCatcher.hpp"

// ReSharper disable CppClangTidyClangDiagnosticCastFunctionTypeStrict
const st6_malloc_t st6_malloc = reinterpret_cast<const st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<const st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

// ReSharper disable once CppDFAConstantFunctionResult
BOOL WINAPI DllMain([[maybe_unused]] const HINSTANCE& hinstDLL, [[maybe_unused]] DWORD fdwReason, [[maybe_unused]] const LPVOID& lpvReserved)
{
    static bool executed = false;
    if (executed)
    {
        return TRUE;
    }

    wchar_t file[MAX_PATH];
    GetModuleFileName(nullptr, file, sizeof file);

    if (const std::wstring fileName = StringUtils::ToLower(std::wstring(file)); fileName.find(L"flserver.exe") == std::wstring::npos)
    {
        return TRUE;
    }

    executed = true;

    // We need to init our memory hooks before anything is loaded!
    MemoryManager::i()->AddHooks();
    FLHook::SetupEventLoop();

    return TRUE;
}

FLHook::FLHook()
    : damageToClientId(0), messagePrivate(false), messageSystem(false), messageUniverse(false), serverLoadInMs(0), playerCount(0), disableNpcs(false),
      flhookReady(false)
{
    // Set module references
    commonDll = GetModuleHandle(L"common.dll");
    serverDll = GetModuleHandle(L"server.dll");
    dalibDll = GetModuleHandle(L"dalib.dll");

    instance = this;

    // Load our settings before anything that might need access to debug mode
    LoadSettings();

    Logger::Init();

    DEBUG("Creating TaskScheduler");
    taskScheduler = std::make_shared<TaskScheduler>();

    DEBUG("Connecting to database at {{uri}}", { "uri", flhookConfig->database.uri });
    database = std::make_shared<Database>(flhookConfig->database.uri);

    DEBUG("Creating InfocardManager");
    infocardManager = std::make_unique<InfocardManager>();

    DEBUG("Creating ClientList");
    clientList = std::make_shared<ClientList>();

    DEBUG("Creating PersonalityHelper");
    personalityHelper = std::make_shared<PersonalityHelper>();

    DEBUG("Creating AccountManager");
    accountManager = std::make_shared<AccountManager>();

    DEBUG("Creating ResourceManager");
    resourceManager = std::make_shared<ResourceManager>();

    flProc = GetModuleHandle(nullptr);

    // Replace the global exception filter with our own so we can write exception dumps
    ExceptionHandler::SetupExceptionHandling();

    // Setup needed debug tools
    InternalApi::Init();

    // Initialize the Database before everything as other systems rely on it
    // database = new Database(FLHook::GetConfig().databaseConfig.uri);

    const auto config = GetConfig();

    if (config->httpSettings.enableHttpServer)
    {
        DEBUG("Creating HttpServer");
        httpServer = std::make_shared<HttpServer>();
    }

    if (config->plugins.loadAllPlugins)
    {
        PluginManager::i()->LoadAll(true);
    }
    else
    {
        for (auto& plugin : config->plugins.plugins)
        {
            PluginManager::i()->Load(plugin, true);
        }
    }

    if (!std::filesystem::exists(L"config"))
    {
        try
        {
            std::filesystem::create_directory(L"config");
        }
        catch (std::filesystem::filesystem_error& error)
        {
            ERROR("Failed to create directory {{path}} {{error}}", { "path", error.path1().generic_string() }, { "error", error.what() });
        }
    }

    Timer::Add(std::bind_front(&TaskScheduler::ProcessTasks, taskScheduler), 1);

    PatchClientImpl();
}

void FLHook::SetupEventLoop()
{
    if (instance)
    {
        return;
    }

    auto [startup, shutdown, update] = std::make_tuple(reinterpret_cast<DWORD>(&IServerImplHook::Startup),
                                                       reinterpret_cast<DWORD>(&IServerImplHook::Shutdown),
                                                       reinterpret_cast<DWORD>(&IServerImplHook::Update));

    flProc = GetModuleHandle(nullptr);
    auto address = Offset(BinaryType::Exe, AddressList::Update);

    MemUtils::ReadProcMem(address, &oldUpdateLoop, 4);
    MemUtils::WriteProcMem(address, &update, 4);

    // install startup hook
    address = Offset(BinaryType::Exe, AddressList::Startup);
    MemUtils::WriteProcMem(address, &startup, 4);

    // install shutdown hook
    address = Offset(BinaryType::Exe, AddressList::Shutdown);
    MemUtils::WriteProcMem(address, &shutdown, 4);
}

DWORD __stdcall FLHook::Offset(const BinaryType type, AddressList address)
{
    const auto offset = static_cast<DWORD>(address);

    switch (type)
    {
        case BinaryType::Exe: return reinterpret_cast<DWORD>(flProc) + offset;
        case BinaryType::Server: return reinterpret_cast<DWORD>(serverDll) + offset;
        case BinaryType::Common: return reinterpret_cast<DWORD>(commonDll) + offset;
        case BinaryType::Content: return reinterpret_cast<DWORD>(contentDll) + offset;
        case BinaryType::DaLib: return reinterpret_cast<DWORD>(dalibDll) + offset;
        case BinaryType::RemoteClient: return reinterpret_cast<DWORD>(remoteClient) + offset;
        default: throw std::runtime_error("Provided BinaryType is not loaded."); // NOLINT(clang-diagnostic-covered-switch-default)
    }
}

bool FLHook::IsReady() { return instance != nullptr && instance->flhookReady; }
std::wstring_view FLHook::GetAccountPath() { return instance->accPath; }

GameObject* FLHook::GetObjInspect(const uint& objId)
{
    StarSystem* starSystem;
    GameObject* inspect;
    getShipInspect(objId, reinterpret_cast<IObjInspectImpl*&>(inspect), starSystem);
    return inspect;
}
GameObject* FLHook::GetObjInspect(Id objId) { return GetObjInspect(objId.GetValue()); }

bool FLHook::GetObjInspect(const uint& objId, GameObject*& inspect, StarSystem*& starSystem)
{
    return getShipInspect(objId, reinterpret_cast<IObjInspectImpl*&>(inspect), starSystem);
}

bool FLHook::GetObjInspect(Id objId, GameObject*& inspect, StarSystem*& starSystem) { return GetObjInspect(objId.GetValue(), inspect, starSystem); }

const std::unordered_map<ClientId, std::unordered_set<std::wstring>>& FLHook::GetAdmins() { return instance->credentialsMap; }
ClientList& FLHook::Clients() { return *instance->clientList; }

ClientData& FLHook::GetClient(ClientId client)
{
    if (!client.IsValidClientId())
    {
        throw InvalidClientException(client);
    }

    return Clients()[client];
}

ClientData* FLHook::GetClientByName(std::wstring_view characterName)
{
    for (auto& client : Clients())
    {
        if (StringUtils::CompareCaseInsensitive(client.characterName, characterName))
        {
            return &client;
        }
    }

    return nullptr;
}

std::shared_ptr<Database> FLHook::GetDatabase() { return instance->database; }
mongocxx::pool::entry FLHook::GetDbClient() { return instance->database->AcquireClient(); }
std::shared_ptr<HttpServer> FLHook::GetHttpServer() { return instance->httpServer; }
std::shared_ptr<InfocardManager> FLHook::GetInfocardManager() { return instance->infocardManager; }
Action<pub::AI::Personality*> FLHook::GetPersonality(const std::wstring& pilotNickname) { return instance->personalityHelper->GetPersonality(pilotNickname); }
uint FLHook::GetServerLoadInMs() { return instance->serverLoadInMs; }
CDPClientProxy** FLHook::GetClientProxyArray() { return instance->clientProxyArray; }
std::shared_ptr<TaskScheduler> FLHook::GetTaskScheduler() { return instance->taskScheduler; }
std::shared_ptr<AccountManager> FLHook::GetAccountManager() { return instance->accountManager; }
std::shared_ptr<FLHookConfig> FLHook::GetConfig() { return instance->flhookConfig; }
IClientImpl* FLHook::GetPacketInterface() { return hookClientImpl; }
std::shared_ptr<ResourceManager> FLHook::GetResourceManager()
{
    if (!instance || !instance->flhookReady)
    {
        throw std::logic_error("Attempting to access resource manager before server has started.");
    }

    return instance->resourceManager;
}

Action<void> FLHook::MessageUniverse(std::wstring_view message)
{
    if (message.empty())
    {
        return { {} };
    }

    constexpr CHAT_ID ci = { 0 };
    constexpr CHAT_ID ciClient = { 0x00010000 };

    const std::wstring xml = std::format(L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>{}</TEXT>", StringUtils::XmlText(message));

    uint retVal;
    static std::array<char, 1024> buffer;
    std::fill_n(buffer.begin(), buffer.size(), '\0');

    if (const auto err = InternalApi::FMsgEncodeXml(xml, buffer.data(), buffer.size(), retVal).Raw(); err.has_error())
    {
        return { cpp::fail(err.error()) };
    }

    Server.SubmitChat(ci, retVal, buffer.data(), ciClient, -1);
    return { {} };
}

bool FLHook::OnServerStart()
{
    uint index = 0;
    for (auto& client : clientList->clients)
    {
        client.id = ClientId(index++);
        client.playerData = &Players[client.id.GetValue()];
    }

    try
    {
        // Init Hooks
        InitHookExports();

        // Setup timers
        Timer::Add(ProcessPendingCommands, 50);
        Timer::Add(TimerCheckKick, 50);
        Timer::Add(OneSecondTimer, 1000);
        Timer::Add(IpResolver::TimerCheckResolveResults, 100);

        ZoneUtilities::Init();
    }
    catch (std::runtime_error& err)
    {
        UnloadHookExports();

        ERROR("{{ex}}", { "ex", err.what() });
        return false;
    }

    flhookReady = true;
    return true;
}

void FLHook::LoadSettings()
{
    auto* config = &instance->flhookConfig;
    *config = std::make_shared<FLHookConfig>();

    auto configResult = Json::Load<FLHookConfig>("flhook.json", false);
    if (configResult.first == Json::LoadState::Success)
    {
        **config = configResult.second.value();
    }
    else
    {
        // clang-format off
        if (configResult.first == Json::LoadState::DoesNotExist)
        {
            Json::Save<FLHookConfig>(**config, "flhook.json");
            MessageBoxA(nullptr, "This is your first time opening FLServer with FLHook enabled."
                "A file called flhook.json has been created in the Freelancer directory.\n"
                "Please open and configure it as per documentation.\n\n"
                "FLServer will now terminate.", "flhook.json Created", MB_OK);
            std::exit(0);

        }

        const auto boxResult = MessageBoxA(nullptr, "Failed to read/validate flhook.json.\nPlease ensure that the JSON within the file is"
            "not malformed and all parameters are as expected (types match, for instance).\n\n"
            "If you would like to generate a fresh config, press OK, otherwise press cancel and try again.",
            "flhook.json failed to load", MB_OKCANCEL);
        // clang-format on
        if (boxResult == IDOK)
        {
            Json::Save<FLHookConfig>(**config, "flhook.json");
        }
        else
        {
            std::abort();
        }
    }
}

DWORD __stdcall Unload(const LPVOID module)
{
    Sleep(1000);

    FreeLibraryAndExitThread(static_cast<HMODULE>(module), 0);
}

FLHook::~FLHook()
{
    // Force thread to terminate
    TerminateThread(IpResolver::resolveThread.native_handle(), 0);

    const auto address = Offset(BinaryType::Exe, AddressList::Update);
    MemUtils::WriteProcMem(address, &oldUpdateLoop, 4);

    // unload hooks
    UnloadHookExports();

    // Once FLServer has finished cleanup, let FLHook properly terminate
    CreateThread(nullptr, 0, Unload, moduleFLHook, 0, nullptr);
}

void FLHook::ProcessPendingCommands()
{
    auto cmd = Logger::GetCommand();
    while (cmd.has_value())
    {
        std::wstring& cmdStr = cmd.value();
        if (cmdStr.empty())
        {
            continue;
        }

        if (cmdStr[0] != L'.')
        {
            cmdStr = std::format(L".{}", cmdStr);
        }

        constexpr auto consoleId = L"0"sv;
        if (auto task = AdminCommandProcessor::i()->ProcessCommand(ClientId(), AllowedContext::ConsoleOnly, consoleId, cmdStr))
        {
            GetTaskScheduler()->StoreTaskHandle(std::make_shared<Task>(std::move(*task), ClientId()));
        }

        cmd = Logger::GetCommand();
    }
}

void FLHook::LoadZoneDamageData(const char* path)
{
    std::string pathStr = "..\\data\\universe\\" + std::string(path);

    INI_Reader ini;
    if (!ini.open(pathStr.c_str(), false))
    {
        return;
    }

    while (ini.read_header())
    {
        if (!ini.is_header("zone"))
        {
            continue;
        }

        Id nickname;
        IEngineHook::ZoneSpecialData data;

        while (ini.read_value())
        {
            if (ini.is_value("nickname"))
            {
                nickname = Id(ini.get_value_string());
            }
            else if (ini.is_value("damage"))
            {
                data.flatDamage = ini.get_value_float(0);
                data.percentageDamage = ini.get_value_float(1) * 0.01f;
                data.distanceScaling = ini.get_value_float(2);
                data.logScale = ini.get_value_float(3);
                data.dmgType = IEngineHook::ZoneDamageType::ZONEDMG_HULL;
                std::string propertiesStr = ini.get_value_string(4);
                if (propertiesStr.find("shield") != std::string::npos)
                {
                    data.dmgType |= IEngineHook::ZoneDamageType::ZONEDMG_SHIELD;
                }
                if (propertiesStr.find("cruise") != std::string::npos)
                {
                    data.dmgType |= IEngineHook::ZoneDamageType::ZONEDMG_CRUISE;
                }
                if (propertiesStr.find("energy") != std::string::npos)
                {
                    data.dmgType |= IEngineHook::ZoneDamageType::ZONEDMG_ENERGY;
                }
                if (propertiesStr.find("nohull") != std::string::npos)
                {
                    data.dmgType -= IEngineHook::ZoneDamageType::ZONEDMG_HULL;
                }
                if (data.logScale == 0)
                {
                    data.logScale = 1;
                }

                data.shieldMult = ini.get_value_float(5);
                data.energyMult = ini.get_value_float(6);

                IEngineHook::zoneSpecialData[nickname] = data;
                break;
            }
        }
    }

    ini.close();
}
