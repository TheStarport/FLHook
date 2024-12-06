#include "PCH.hpp"

#include "Core/FLHook.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/MessageInterface.hpp"
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
#include "Core/DebugTools.hpp"
#include "Exceptions/InvalidParameterException.hpp"

#include "API/FLHook/Plugin.hpp"
#include "API/Utils/ZoneUtilities.hpp"

#include "API/Exceptions/InvalidClientException.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "Core/CrashCatcher.hpp"
#include "Core/MessageHandler.hpp"

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
    Logger::Init();
    // Set module references
    commonDll = GetModuleHandle(L"common.dll");
    serverDll = GetModuleHandle(L"server.dll");
    dalibDll = GetModuleHandle(L"dalib.dll");

    instance = this;

    // Load our settings before anything that might need access to debug mode
    LoadSettings();

    Logger::Debug(L"Creating InfocardManager");
    infocardManager = std::make_unique<InfocardManager>();

    Logger::Debug(L"Creating ClientList");
    clientList = std::make_unique<ClientList>();

    Logger::Debug(L"Creating PersonalityHelper");
    personalityHelper = std::make_unique<PersonalityHelper>();

    Logger::Debug(std::format(L"Connecting to database @ \"{}\"", StringUtils::stows(flhookConfig->database.uri)));
    database = std::make_unique<Database>(flhookConfig->database.uri);

    Logger::Debug(L"Creating AccountManager");
    accountManager = std::make_unique<AccountManager>();

    Logger::Debug(L"Creating CrashCatcher");
    crashCatcher = std::make_unique<CrashCatcher>();

    Logger::Debug(L"Creating ResourceManager");
    resourceManager = std::make_unique<ResourceManager>();

    Logger::Debug(L"Creating TaskScheduler");
    taskScheduler = std::make_unique<TaskScheduler>();

    flProc = GetModuleHandle(nullptr);

    // Replace the global exception filter with our own so we can write exception dumps
    ExceptionHandler::SetupExceptionHandling();

    // Setup needed debug tools
    DebugTools::Init();

    // Initialize the Database before everything as other systems rely on it
    // database = new Database(FLHook::GetConfig().databaseConfig.uri);

    const auto config = GetConfig();

    // Init our message service, this is a blocking call and some plugins might want to setup their own queues,
    // so we want to make sure the service is up at startup time
    if (config->messageQueue.enableQueues)
    {
        messageInterface = std::make_unique<MessageInterface>();
        messageHandler = std::make_unique<MessageHandler>();
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
            Logger::Err(StringUtils::stows(std::format("Failed to create directory {}\n{}", error.path1().generic_string(), error.what())));
        }
    }

    Timer::Add(std::bind_front(&TaskScheduler::ProcessTasks, taskScheduler, std::ref(taskScheduler->mainTasks)), 25);
    Timer::Add(ProcessPendingAsyncTasks, 250);

    PatchClientImpl();
}

void FLHook::ProcessPendingAsyncTasks()
{
    std::optional<TaskScheduler::CallbackTask> task;
    while ((task = TaskScheduler::GetCompletedTask()).has_value())
    {
        auto& t = task.value();
        if (!t.callback.has_value())
        {
            continue;
        }

        if (t.callback.value().index() == 0)
        {
            std::get<0>(t.callback.value())(t.taskData);
        }
        else
        {
            std::get<1>(t.callback.value())();
        }
    }
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

bool FLHook::GetObjInspect(uint& ship, IObjInspectImpl*& inspect)
{
    uint dunno; // Something related to watchables
    return getShipInspect(ship, inspect, dunno);
}

const std::unordered_map<std::wstring, std::vector<std::wstring>>& FLHook::GetAdmins() { return instance->credentialsMap; }
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
std::shared_ptr<InfocardManager> FLHook::GetInfocardManager() { return instance->infocardManager; }
FLHook::LastHitInformation FLHook::GetLastHitInformation() { return { nonGunHitsBase, lastHitPts, dmgToClient, dmgToSpaceId }; }
std::shared_ptr<MessageInterface> FLHook::GetMessageInterface() { return instance->messageInterface; }
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

    const std::wstring xml = std::format(L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>", StringUtils::XmlText(message));

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

        Logger::Err(StringUtils::stows(err.what()));
        return false;
    }

    flhookReady = true;
    return true;
}

void FLHook::LoadSettings()
{
    auto* config = &instance->flhookConfig;
    std::ifstream stream("flhook.json");
    if (!stream.is_open())
    {
        *config = std::make_shared<FLHookConfig>();
    }
    else
    {
        *config = std::make_shared<FLHookConfig>();
        auto configResult = rfl::json::read<FLHookConfig>(stream);
        if (auto err = configResult.error(); err.has_value())
        {
            Logger::Warn(std::format(L"Error while trying to read FLHook.json. Writing new config.\nErrors: {}", StringUtils::stows(err.value().what())));
        }
        else
        {
            rfl::internal::wrap_in_rfl_array_t<FLHookConfig> value = configResult.value();
            **config = value;
        }
    }

    // Resave to add any missing properties that have been added
    Json::Save(*config, "flhook.json");
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
        try
        {
            constexpr auto consoleId = L"0"sv;
            if (const auto response = AdminCommandProcessor::i()->ProcessCommand(ClientId(), AllowedContext::ConsoleOnly, consoleId, cmd.value());
                response.has_value())
            {
                GetTaskScheduler()->AddTask(std::make_shared<Task>(*response));
            }
        }
        catch (InvalidParameterException& ex)
        {
            Logger::Warn(ex.Msg());
        }
        catch (GameException& ex)
        {
            Logger::Warn(ex.Msg());
        }
        catch (StopProcessingException&)
        {
            // Continue processing
        }
        catch (std::exception& ex)
        {
            // Anything else critically log
            Logger::Err(StringUtils::stows(ex.what()));
        }

        cmd = Logger::GetCommand();
    }
}
