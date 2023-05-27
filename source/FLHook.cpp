#include "PCH.hpp"
#include "Global.hpp"

#include "Memory/MemoryManager.hpp"
#include <Features/MessageHandler.hpp>
#include <Features/Logger.hpp>

#include "Defs/FLHookConfig.hpp"
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

const st6_malloc_t st6_malloc = reinterpret_cast<const st6_malloc_t>(GetProcAddress(GetModuleHandle("msvcrt.dll"), "malloc"));
const st6_free_t st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(GetModuleHandle("msvcrt.dll"), "free"));

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
		return TRUE;

	char file[MAX_PATH];
	GetModuleFileName(nullptr, file, sizeof file);

	if (const std::wstring fileName = StringUtils::ToLower(StringUtils::stows(file)); fileName.find(L"flserver.exe") != -1)
	{
		// We need to init our memory hooks before anything is loaded!
		MemoryManager::i()->AddHooks();

		executed = true;

		// redirect IServerImpl::Update
		const auto fpLoop = IServerImplHook::Update;
		auto address = reinterpret_cast<char*>(GetModuleHandle(nullptr)) + ADDR_UPDATE;
		MemUtils::MemUtils::ReadProcMem(address, &fpOldUpdate, 4);
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
		CreateDirectoryA("./logs/debug", nullptr);
	}
	return TRUE;
}

// TODO: Reimplement exception handling

/**************************************************************************************************************
init
**************************************************************************************************************/

const std::array PluginLibs = {
	"pcre2-posix.dll",
	"pcre2-8.dll",
	"pcre2-16.dll",
	"pcre2-32.dll",
	"libcrypto-3.dll",
	"libssl-3.dll",
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
		if (!(server = GetModuleHandle("server")))
			throw std::runtime_error("server.dll not loaded");

		// Init our message service, this is a blocking call and some plugins might want to setup their own queues, 
		// so we want to make sure the service is up at startup time
		if (FLHookConfig::i()->messageQueue.enableQueues)
		{
			MessageHandler::i()->DeclareExchange(MessageHandler::QueueToStr(MessageHandler::Queue::ServerStats), AMQP::fanout, AMQP::durable);
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

		if (!std::filesystem::exists("config"))
		{
			std::filesystem::create_directory("config");
		}

		// Load required libs that plugins might leverage
		for (const auto& lib : PluginLibs)
		{
			LoadLibrary(lib);
		}

		Logger::i()->Log(LogLevel::Info, "Loading Freelancer INIs");
		const auto dataManager = DataManager::i();

		dataManager->LoadLights();

		Hk::Ini::CharacterInit();
		Hk::Personalities::LoadPersonalities();

		PatchClientImpl();

		CallPluginsAfter(HookedCall::FLHook__LoadSettings);
	}
	catch (char* error)
	{
		Logger::i()->Log(LogLevel::Err, std::format("CRITICAL! {}\n", error));
		std::quick_exit(EXIT_FAILURE);
	}
	catch (std::filesystem::filesystem_error& error)
	{
		Logger::i()->Log(LogLevel::Err, std::format("Failed to create directory {}\n{}", error.path1().generic_string(), error.what()));
	}
}

bool FLHookInit()
{
	try
	{
		// get module handles
		server = GetModuleHandle("server");
		remoteClient = GetModuleHandle("remoteclient");
		common = GetModuleHandle("common");
		hModDPNet = GetModuleHandle("dpnet");
		hModDaLib = GetModuleHandle("dalib");
		content = GetModuleHandle("content");
		hMe = GetModuleHandle("FLHook");

		// Init Hooks
		if (!InitHookExports())
			throw std::runtime_error("InitHookExports failed");

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

		Logger::i()->Log(LogLevel::Err, err.what());
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
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)FLHookUnload, &param, 0, &id);
}

void ProcessPendingCommands()
{
	const auto logger = Logger::i();
	auto cmd = logger->GetCommand();
	while (cmd.has_value())
	{
		// TODO: Reimplement admin command
		//AdminConsole.ExecuteCommandString(cmd);
		cmd = logger->GetCommand();
	}
}

