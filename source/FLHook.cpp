#include "PCH.hpp"

#include "Global.hpp"
#include "Memory/MemoryManager.hpp"
#include <Features/Logger.hpp>
#include <Features/MessageHandler.hpp>

#include "Defs/FLHookConfig.hpp"
#include "Features/CommandProcessors/AdminCommandProcessor.hpp"
#include "Features/DataManager.hpp"

HANDLE hProcFL = nullptr;
HMODULE server = nullptr;
HMODULE common = nullptr;
HMODULE remoteClient = nullptr;
HMODULE hMe = nullptr;
HMODULE hModDPNet = nullptr;
HMODULE hModDaLib = nullptr;
HMODULE content = nullptr;

bool executed = false;

const st6_malloc_t st6_malloc = reinterpret_cast<const st6_malloc_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandleA("msvcrt.dll"), "free"));

bool flhookReady;

/**************************************************************************************************************
DllMain
**************************************************************************************************************/

FARPROC fpOldUpdate;

namespace IServerImplHook
{
    bool __stdcall Startup(const SStartupInfo& si);
    void __stdcall Shutdown();
    int __stdcall Update();
} // namespace IServerImplHook

BOOL WINAPI DllMain([[maybe_unused]] const HINSTANCE& hinstDLL, [[maybe_unused]] DWORD fdwReason, [[maybe_unused]] const LPVOID& lpvReserved)
{
    if (executed)
    {
        return TRUE;
    }

    wchar_t file[MAX_PATH];
    GetModuleFileName(nullptr, file, sizeof file);

    if (const std::wstring fileName = StringUtils::ToLower(std::wstring(file)); fileName.find(L"flserver.exe") != std::wstring::npos)
    {
        // We need to init our memory hooks before anything is loaded!
        MemoryManager::i()->AddHooks();

        executed = true;

        // redirect IServerImpl::Update
        const auto fpLoop = IServerImplHook::Update;
        auto address = reinterpret_cast<char*>(GetModuleHandle(nullptr)) + ADDR_UPDATE;
        MemUtils::ReadProcMem(address, &fpOldUpdate, 4);
        MemUtils::WriteProcMem(address, &fpLoop, 4);

        // install startup hook
        const auto fpStartup = reinterpret_cast<FARPROC>(IServerImplHook::Startup);
        address = reinterpret_cast<char*>(GetModuleHandle(nullptr)) + ADDR_STARTUP;
        MemUtils::WriteProcMem(address, &fpStartup, 4);

        // install shutdown hook
        const auto fpShutdown = reinterpret_cast<FARPROC>(IServerImplHook::Shutdown);
        address = reinterpret_cast<char*>(GetModuleHandle(nullptr)) + ADDR_SHUTDOWN;
        MemUtils::WriteProcMem(address, &fpShutdown, 4);

        // create log dirs
        CreateDirectoryA("./logs/", nullptr);
    }
    return TRUE;
}

// TODO: Reimplement exception handling

/**************************************************************************************************************
init
**************************************************************************************************************/

const std::array PluginLibs = {
    "pcre2-posix.dll", "pcre2-8.dll", "pcre2-16.dll", "pcre2-32.dll", "libcrypto-3.dll", "libssl-3.dll",
};

void FLHookInit_Pre()
{
    hProcFL = GetModuleHandle(nullptr);

    try
    {
        // Load our settings before anything that might need access to debug mode
        LoadSettings();

        // Setup needed debug tools
        DebugTools::i()->Init();

        // TODO: Move module handles to FLCoreGlobals
        if (!(server = GetModuleHandle(L"server")))
        {
            throw std::runtime_error("server.dll not loaded");
        }

        // Init our message service, this is a blocking call and some plugins might want to setup their own queues,
        // so we want to make sure the service is up at startup time
        if (FLHookConfig::i()->messageQueue.enableQueues)
        {
            MessageHandler::i()->DeclareExchange(std::wstring(MessageHandler::QueueToStr(MessageHandler::Queue::ServerStats)), AMQP::fanout, AMQP::durable);
            Timer::Add(PublishServerStats, 30000);
        }

        if (const auto config = FLHookConfig::c(); config->plugins.loadAllPlugins)
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
            std::filesystem::create_directory(L"config");
        }

        // Load required libs that plugins might leverage
        for (const auto& lib : PluginLibs)
        {
            LoadLibraryA(lib);
        }

        Logger::i()->Log(LogLevel::Info, L"Loading Freelancer INIs");
        const auto dataManager = DataManager::i();

        dataManager->LoadLights();

        // Force constructor to run
        Hk::IniUtils::i();
        Hk::Personalities::LoadPersonalities();

        PatchClientImpl();

        CallPluginsAfter(HookedCall::FLHook__LoadSettings);
    }
    catch (char* error)
    {
        Logger::i()->Log(LogLevel::Err, StringUtils::stows(std::format("CRITICAL! {}\n", std::string(error))));
        std::quick_exit(EXIT_FAILURE);
    }
    catch (std::filesystem::filesystem_error& error)
    {
        Logger::i()->Log(LogLevel::Err, StringUtils::stows(std::format("Failed to create directory {}\n{}", error.path1().generic_string(), error.what())));
    }
}

bool FLHookInit()
{
    try
    {
        // get module handles
        server = GetModuleHandle(L"server");
        remoteClient = GetModuleHandle(L"remoteclient");
        common = GetModuleHandle(L"common");
        hModDPNet = GetModuleHandle(L"dpnet");
        hModDaLib = GetModuleHandle(L"dalib");
        content = GetModuleHandle(L"content");
        hMe = GetModuleHandle(L"FLHook");

        // Init Hooks
        if (!InitHookExports())
        {
            throw std::runtime_error("InitHookExports failed");
        }

        // Setup timers
        Timer::Add(ProcessPendingCommands, 50);
        Timer::Add(TimerCheckKick, 50);
        Timer::Add(TimerNPCAndF1Check, 1000);
        Timer::Add(TimerCheckResolveResults, 0);
        Timer::Add(TimerTempBanCheck, 15000);
    }
    catch (std::runtime_error& err)
    {
        UnloadHookExports();

        Logger::i()->Log(LogLevel::Err, StringUtils::stows(err.what()));
        return false;
    }

    return true;
}

/**************************************************************************************************************
shutdown
**************************************************************************************************************/

void FLHookUnload()
{
    // bad but working..
    Sleep(1000);

    Logger::del();

    // finish
    FreeLibraryAndExitThread(hMe, 0);
}

void FLHookShutdown()
{
    TerminateThread(hThreadResolver, 0);

    // unload update hook
    auto* address = static_cast<void*>((char*)hProcFL + ADDR_UPDATE);
    MemUtils::WriteProcMem(address, &fpOldUpdate, 4);

    // unload hooks
    UnloadHookExports();

    // unload rest
    DWORD id;
    DWORD param;
    CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(FLHookUnload), &param, 0, &id);
}

void ProcessPendingCommands()
{
    const auto logger = Logger::i();
    auto cmd = logger->GetCommand();
    while (cmd.has_value())
    {
        const auto processor = AdminCommandProcessor::i();
        processor->SetCurrentUser(L"console", AdminCommandProcessor::AllowedContext::ConsoleOnly);

        try
        {
            const auto response = AdminCommandProcessor::i()->ProcessCommand(cmd.value());
            Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Info, response);
        }
        catch (GameException& ex)
        {
            // TODO: Log to admin command file
            Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Warn, ex.Msg());
        }
        catch (StopProcessingException&)
        {
            // Continue processing
        }
        catch (std::exception& ex)
        {
            // Anything else critically log
            // TODO: Log to error log file
            Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Err, StringUtils::stows(ex.what()));
        }

        cmd = logger->GetCommand();
    }
}

void PrintUserCmdText(ClientId client, std::wstring_view text)
{
    if (const auto newLineChar = text.find(L'\n'); newLineChar == std::wstring::npos)
    {
        const std::wstring xml =
            std::format(L"<TRA data=\"{}\" mask=\"-1\"/><TEXT>{}</TEXT>", FLHookConfig::i()->chatConfig.msgStyle.userCmdStyle, StringUtils::XmlText(text));
        Hk::Chat::FMsg(client, xml);
    }
    else
    {
        // Split text into two strings, one from the beginning to the character before newLineChar, and one after newLineChar till the end.
        // It will then recursively call itself for each new line char until the original text is all displayed.
        PrintUserCmdText(client, text.substr(0, newLineChar));
        PrintUserCmdText(client, text.substr(newLineChar + 1, std::wstring::npos));
    }
}

// Print message to all ships within the specific number of meters of the player.
void PrintLocalUserCmdText(ClientId client, const std::wstring_view msg, float distance)
{
    uint ship;
    pub::Player::GetShip(client, ship);

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(ship, pos, rot);

    uint system;
    pub::Player::GetSystem(client, system);

    // For all players in system...
    PlayerData* playerDb = nullptr;
    while ((playerDb = Players.traverse_active(playerDb)))
    {
        // Get the this player's current system and location in the system.
        ClientId client2 = playerDb->onlineId;
        uint system2 = 0;
        pub::Player::GetSystem(client2, system2);
        if (system != system2)
        {
            continue;
        }

        uint ship2;
        pub::Player::GetShip(client2, ship2);

        Vector pos2;
        Matrix rot2;
        pub::SpaceObj::GetLocation(ship2, pos2, rot2);

        // Is player within the specified range of the sending char.
        if (Hk::Math::Distance3D(pos, pos2) > distance)
        {
            continue;
        }

        PrintUserCmdText(client2, msg);
    }
}
