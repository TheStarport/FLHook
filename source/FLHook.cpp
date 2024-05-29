#include "PCH.hpp"

#include "Core/FLHook.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/PersonalityHelper.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "API/InternalApi.hpp"
#include "Core/MemoryManager.hpp"
#include <API/Utils/Logger.hpp>
#include <Core/MessageHandler.hpp>

#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Defs/FLHookConfig.hpp"

#include "Core/AddressList.hpp"
#include "Core/IpResolver.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Core/DebugTools.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Exceptions/InvalidParameterException.hpp"

#include "API/FLHook/Plugin.hpp"
#include "API/Utils/ZoneUtilities.hpp"

#include "API/Exceptions/InvalidClientException.hpp"
#include "Core/CrashCatcher.hpp"
#include "Defs/BsonWrapper.hpp"

#include <mongocxx/exception/exception.hpp>

// ReSharper disable CppClangTidyClangDiagnosticCastFunctionTypeStrict
const st6_malloc_t st6_malloc = reinterpret_cast<const st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

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

    infocardManager = new InfocardManager();
    clientList = new ClientList();
    personalityHelper = new PersonalityHelper();
    database = new Database(flhookConfig->databaseConfig.uri);
    accountManager = new AccountManager();
    crashCatcher = new CrashCatcher();

    flProc = GetModuleHandle(nullptr);

    // Replace the global exception filter with our own so we can write exception dumps
    // ExceptionHandler::SetupExceptionHandling();

    // Setup needed debug tools
    DebugTools::Init();

    // Initialize the Database before everything as other systems rely on it
    // database = new Database(FLHook::GetConfig().databaseConfig.uri);

    const auto& config = GetConfig();

    try
    {
        Timer::AddCron(PublishServerStats, L"0 0 0 * * *");
    } catch (std::exception& w)
    {
        const std::string_view e = w.what();
        Logger::Err(StringUtils::stows(e));
    }

    // Init our message service, this is a blocking call and some plugins might want to setup their own queues,
    // so we want to make sure the service is up at startup time
    if (config.messageQueue.enableQueues)
    {
        const auto msg = MessageHandler::i();

        // TODO: Move logQueue initialization to separate function
        msg->DeclareExchange(std::wstring(MessageHandler::QueueToStr(MessageHandler::Queue::ServerStats)), AMQP::fanout, AMQP::durable);
        msg->DeclareQueue(std::wstring(MessageHandler::QueueToStr(MessageHandler::Queue::ExternalCommands)), AMQP::durable);

        msg->Subscribe(std::wstring(MessageHandler::QueueToStr(MessageHandler::Queue::ExternalCommands)),
                       [](const AMQP::Message& message, std::shared_ptr<BsonWrapper>& response)
                       {
                           // TODO: ensure this runs on the main thread and therefore is safe to manipulate players
                           const std::string_view body = { message.body(), message.body() + message.bodySize() };

                           try
                           {
                               const BsonWrapper bsonWrapper(body);
                               const auto bson = bsonWrapper.GetValue();
                               if (!bson.has_value())
                               {
                                   return false;
                               }

                               if (const auto [successful, responseDoc] = ExternalCommandProcessor::i()->ProcessCommand(bson.value());
                                   successful || responseDoc)
                               {
                                   response = responseDoc;
                                   return true;
                               }

                               return false;
                           }
                           catch (GameException& ex)
                           {
                               // TODO: Log but no reply?
                               return true;
                           }
                           catch (std::exception& ex)
                           {
                               return true;
                           }
                       });

        Timer::Add(PublishServerStats, config.messageQueue.timeBetweenServerUpdates);
    }

    if (config.plugins.loadAllPlugins)
    {
        PluginManager::i()->LoadAll(true);
    }
    else
    {
        for (auto& plugin : config.plugins.plugins)
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

    Timer::Add(ProcessPendingAsyncTasks, 250);

    PatchClientImpl();

    CallPlugins(&Plugin::OnLoadSettings);
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

bool FLHook::GetObjInspect(uint& ship, IObjInspectImpl*& inspect)
{
    uint dunno; // Something related to watchables
    return getShipInspect(ship, inspect, dunno);
}

ClientData& FLHook::GetClient(ClientId client)
{
    if (!client.IsValidClientId())
    {
        throw InvalidClientException(client);
    }

    return Clients()[client];
}

mongocxx::pool::entry FLHook::GetDbClient() { return instance->database->AcquireClient(); }

Action<pub::AI::Personality*, Error> FLHook::GetPersonality(const std::wstring& pilotNickname)
{
    return instance->personalityHelper->GetPersonality(pilotNickname);
}

Action<void, Error> FLHook::MessageUniverse(std::wstring_view message)
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
        *config = new FLHookConfig();
    }
    else
    {
        *config = new FLHookConfig();
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

    // NoPVP
    (*config)->general.noPVPSystemsHashed.clear();
    for (const auto& system : (*config)->general.noPVPSystems)
    {
        uint systemId;
        pub::GetSystemID(systemId, StringUtils::wstos(system).c_str());
        (*config)->general.noPVPSystemsHashed.emplace_back(systemId);
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
    delete crashCatcher;

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
        const auto processor = AdminCommandProcessor::i();
        processor->SetCurrentUser(L"console", AdminCommandProcessor::AllowedContext::ConsoleOnly);

        try
        {
            const auto response = AdminCommandProcessor::i()->ProcessCommand(cmd.value());
            Logger::Info(response);
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
