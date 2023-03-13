#include "Global.hpp"
#include "CConsole.h"

#include "Memory/MemoryManager.hpp"
#include <Features/MessageHandler.hpp>
#include <Features/Logger.hpp>

HANDLE hProcFL = nullptr;
HMODULE server = nullptr;
HMODULE common = nullptr;
HMODULE remoteClient = nullptr;
HMODULE hMe = nullptr;
HMODULE hModDPNet = nullptr;
HMODULE hModDaLib = nullptr;
HMODULE content = nullptr;

bool bExecuted = false;

CConsole AdminConsole;

st6_malloc_t st6_malloc;
st6_free_t st6_free;

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

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (bExecuted)
		return TRUE;

	char File[MAX_PATH];
	GetModuleFileName(nullptr, File, sizeof(File));

	if (const std::wstring FileName = ToLower(stows(File)); FileName.find(L"flserver.exe") != -1)
	{
		// We need to init our memory hooks before anything is loaded!
		MemoryManager::i()->AddHooks();

		bExecuted = true;

		// redirect IServerImpl::Update
		const auto fpLoop = IServerImplHook::Update;
		auto address = (char*)GetModuleHandle(nullptr) + ADDR_UPDATE;
		ReadProcMem(address, &fpOldUpdate, 4);
		WriteProcMem(address, &fpLoop, 4);

		// install startup hook
		const auto fpStartup = (FARPROC)IServerImplHook::Startup;
		address = static_cast<void*>((char*)GetModuleHandle(0) + ADDR_STARTUP);
		WriteProcMem(address, &fpStartup, 4);

		// install shutdown hook
		const auto fpShutdown = (FARPROC)IServerImplHook::Shutdown;
		address = static_cast<void*>((char*)GetModuleHandle(0) + ADDR_SHUTDOWN);
		WriteProcMem(address, &fpShutdown, 4);

		// create log dirs
		CreateDirectoryA("./logs/", nullptr);
		CreateDirectoryA("./logs/debug", nullptr);
	}
	return TRUE;
}

/**************************************************************************************************************
Replace FLServer's exception handler with our own.
**************************************************************************************************************/
// TODO: Move exception to exception handler class
BYTE oldSetUnhandledExceptionFilter[5];

LONG WINAPI FLHookTopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	AddLog(LogType::Normal, LogLevel::Critical, "!!TOP LEVEL EXCEPTION!!");
	SEHException ex(0, pExceptionInfo);
	WriteMiniDump(&ex);
	return EXCEPTION_EXECUTE_HANDLER; // EXCEPTION_CONTINUE_SEARCH;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return nullptr;
}

/**************************************************************************************************************
init
**************************************************************************************************************/

const std::array<const char*, 6> PluginLibs = {
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

	// Get direct pointers to malloc and free for st6 to prevent debug heap
	// issues
	{
		const auto dll = GetModuleHandle(TEXT("msvcrt.dll"));
		if (!dll)
			throw std::runtime_error("msvcrt.dll");

		st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(dll, "malloc"));
		st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(dll, "free"));
	}

	try
	{
		// Load our settings before anything that might need access to debug mode
		LoadSettings();

		// Initialize the log files and throw exception if there is a problem
		if (!InitLogs())
			throw std::runtime_error("Log files cannot be created.");

		// Setup needed debug tools
		DebugTools::i()->Init();

		// get module handles
		if (!(server = GetModuleHandle("server")))
			throw std::runtime_error("server.dll not loaded");

		// Init our message service, this is a blocking call and some plugins might want to setup their own queues, 
		// so we want to make sure the service is up
		MessageHandler::i();

		if (const auto config = FLHookConfig::c(); config->plugins.loadAllPlugins)
		{
			PluginManager::i()->loadAll(true, &AdminConsole);
		}
		else
		{
			for (auto& i : config->plugins.plugins)
			{
				PluginManager::i()->load(stows(i), &AdminConsole, true);
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

		Console::ConInfo("Loading Freelancer INIs");
		const auto dataManager = DataManager::i();

		dataManager->LoadLights();

		Hk::Ini::CharacterInit();
		Hk::Personalities::LoadPersonalities();

		PatchClientImpl();

		// Install our own exception handler to automatically log minidumps.
		SetUnhandledExceptionFilter(FLHookTopLevelFilter);

		// Hook the kernel SetUnhandledExceptionFilter function to prevent
		// newer versions of the crt disabling our filter function if a buffer
		// overrun is detected.
		const HMODULE ernel32 = LoadLibrary("kernel32.dll");
		if (ernel32)
		{
			void* orgEntry = GetProcAddress(ernel32, "SetUnhandledExceptionFilter");
			if (orgEntry)
			{
				const DWORD offset = static_cast<char*>(orgEntry) - (char*)ernel32;

				ReadProcMem((char*)ernel32 + offset, oldSetUnhandledExceptionFilter, 5);

				const BYTE patch[] = {0xE9};
				WriteProcMem((char*)ernel32 + offset, patch, 1);
				PatchCallAddr((char*)ernel32, offset, (char*)SetUnhandledExceptionFilter);
			}
		}

		CallPluginsAfter(HookedCall::FLHook__LoadSettings);
	}
	catch (char* Error)
	{
		Console::ConErr(std::format("CRITICAL! {}\n", Error));
		exit(EXIT_FAILURE);
	}
	catch (std::filesystem::filesystem_error error)
	{
		Console::ConErr(std::format("Failed to create directory {}\n{}", error.path1().generic_string(), error.what()));
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
			throw "InitHookExports failed";

		// Force the singleton to be created
		FLHookConfig::c();
	}
	catch (char* Error)
	{
		UnloadHookExports();

		Console::ConErr(Error);
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
	WriteProcMem(address, &fpOldUpdate, 4);

	// unload hooks
	UnloadHookExports();

	// If extended exception logging is in use, restore patched functions
	if (const HMODULE ernel32 = GetModuleHandle("kernel32.dll"))
	{
		void* orgEntry = GetProcAddress(ernel32, "SetUnhandledExceptionFilter");
		if (orgEntry)
		{
			const DWORD offset = static_cast<char*>(orgEntry) - (char*)ernel32;
			WriteProcMem((char*)ernel32 + offset, oldSetUnhandledExceptionFilter, 5);
		}
	}
	// And restore the default exception filter.
	SetUnhandledExceptionFilter(nullptr);

	// unload rest
	DWORD id;
	DWORD dwParam;
	CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)FLHookUnload, &dwParam, 0, &id);
}

void ProcessPendingCommands()
{
	const auto logger = Logger::i();
	auto cmd = logger->GetCommand();
	while (cmd.has_value())
	{
		AdminConsole.ExecuteCommandString(cmd);
		cmd = logger->GetCommand();
	}
}
