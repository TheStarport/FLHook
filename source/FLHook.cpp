#include "Global.hpp"
#include "CConsole.h"

#include "Memory/MemoryManager.hpp"

HANDLE hProcFL = 0;
HMODULE server = 0;
HMODULE common = 0;
HMODULE remoteClient = 0;
HMODULE hMe = 0;
HMODULE hModDPNet = 0;
HMODULE hModDaLib = 0;
HMODULE content = 0;
HANDLE hConsoleThread;

HANDLE hConsoleIn;
HANDLE hConsoleOut;
HANDLE hConsoleErr;

std::list<std::wstring*> lstConsoleCmds;

CRITICAL_SECTION cs;

bool bExecuted = false;

CConsole AdminConsole;

st6_malloc_t st6_malloc;
st6_free_t st6_free;

bool flhookReady;

bool Init();

/**************************************************************************************************************
DllMain
**************************************************************************************************************/

FARPROC fpOldUpdate;

namespace IServerImplHook
{
	bool __stdcall Startup(SStartupInfo const& si);
	void __stdcall Shutdown();
	int __stdcall Update();
} // namespace IServerImplHook

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (bExecuted)
		return TRUE;

	char szFile[MAX_PATH];
	GetModuleFileName(0, szFile, sizeof(szFile));

	if (std::wstring wscFileName = ToLower(stows(szFile)); wscFileName.find(L"flserver.exe") != -1)
	{ 
		// We need to init our memory hooks before anything is loaded!
		MemoryManager::i()->AddHooks();

		bExecuted = true;

		// redirect IServerImpl::Update
		FARPROC fpLoop = (FARPROC)IServerImplHook::Update;
		void* pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_UPDATE);
		ReadProcMem(pAddress, &fpOldUpdate, 4);
		WriteProcMem(pAddress, &fpLoop, 4);

		// install startup hook
		FARPROC fpStartup = (FARPROC)IServerImplHook::Startup;
		pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_STARTUP);
		WriteProcMem(pAddress, &fpStartup, 4);

		// install shutdown hook
		FARPROC fpShutdown = (FARPROC)IServerImplHook::Shutdown;
		pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_SHUTDOWN);
		WriteProcMem(pAddress, &fpShutdown, 4);

		// create log dirs
		CreateDirectoryA("./logs/", NULL);
		CreateDirectoryA("./logs/debug", NULL);
	}
	return TRUE;
}

/**************************************************************************************************************
Replace FLServer's exception handler with our own.
**************************************************************************************************************/
#ifndef DISABLE_EXTENDED_EXCEPTION_LOGGING
BYTE oldSetUnhandledExceptionFilter[5];

LONG WINAPI FLHookTopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
	AddLog(LogType::Normal, LogLevel::Critical, "!!TOP LEVEL EXCEPTION!!");
	SEHException ex(0, pExceptionInfo);
	WriteMiniDump(&ex);
	return EXCEPTION_EXECUTE_HANDLER; // EXCEPTION_CONTINUE_SEARCH;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI Cb_SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}
#endif

/**************************************************************************************************************
thread that reads console input
**************************************************************************************************************/

void __stdcall ReadConsoleEvents()
{
	while (true)
	{
		DWORD dwBytesRead;
		std::string cmd;
		cmd.resize(1024);
		if (ReadConsole(hConsoleIn, cmd.data(), cmd.size(), &dwBytesRead, nullptr))
		{
			if (cmd[cmd.length() - 1] == '\n')
				cmd = cmd.substr(0, cmd.length() - 1);
			if (cmd[cmd.length() - 1] == '\r')
				cmd = cmd.substr(0, cmd.length() - 1);

			std::wstring wscCmd = stows(cmd);
			EnterCriticalSection(&cs);
			auto pwscCmd = new std::wstring;
			*pwscCmd = wscCmd;
			lstConsoleCmds.push_back(pwscCmd);
			LeaveCriticalSection(&cs);
		}
	}
}

/**************************************************************************************************************
handles console events
**************************************************************************************************************/

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
		case CTRL_CLOSE_EVENT: {
			return TRUE;
		}
		break;
	}

	return FALSE;
}

/**************************************************************************************************************
init
**************************************************************************************************************/

const std::array<const char*, 4> PluginLibs = {
    "pcre2-posix.dll",
    "pcre2-8.dll",
    "pcre2-16.dll",
    "pcre2-32.dll",
};

void FLHookInit_Pre()
{
	InitializeCriticalSection(&cs);
	hProcFL = GetModuleHandle(0);

	// Get direct pointers to malloc and free for st6 to prevent debug heap
	// issues
	{
		const auto dll = GetModuleHandle(TEXT("msvcrt.dll"));
		if (!dll)
			throw std::runtime_error("msvcrt.dll");

		st6_malloc = reinterpret_cast<st6_malloc_t>(GetProcAddress(dll, "malloc"));
		st6_free = reinterpret_cast<st6_free_t>(GetProcAddress(dll, "free"));
	}

	// start console
	AllocConsole();
	SetConsoleTitle("FLHook");

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);

	MoveWindow(console, r.left, r.top, 1366, 768, TRUE);

	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);

	// change version number here:
	// https://patorjk.com/software/taag/#p=display&f=Big&t=FLHook%204.0
	std::string welcomeText = R"(
  ______ _      _    _             _      _  _  __                 _ _           
 |  ____| |    | |  | |           | |    | || |/_ |               | | |          
 | |__  | |    | |__| | ___   ___ | | __ | || |_| |    _ __   __ _| | | __ _ ___ 
 |  __| | |    |  __  |/ _ \ / _ \| |/ / |__   _| |   | '_ \ / _` | | |/ _` / __|
 | |    | |____| |  | | (_) | (_) |   <     | |_| |   | |_) | (_| | | | (_| \__ \
 |_|    |______|_|  |_|\___/ \___/|_|\_\    |_(_)_|   | .__/ \__,_|_|_|\__,_|___/
                                                      | |                        
                                                      |_|                        
                                                                       )";
	welcomeText += "\n\n";
	DWORD _;
	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), welcomeText.c_str(), DWORD(welcomeText.length()), &_, nullptr);
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

		DWORD id;
		hConsoleThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(ReadConsoleEvents), nullptr, 0, &id);
		PluginManager::i();

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

#ifndef DISABLE_EXTENDED_EXCEPTION_LOGGING
		// Install our own exception handler to automatically log minidumps.
		::SetUnhandledExceptionFilter(FLHookTopLevelFilter);

		// Hook the kernel SetUnhandledExceptionFilter function to prevent
		// newer versions of the crt disabling our filter function if a buffer
		// overrun is detected.
		HMODULE ernel32 = LoadLibrary("kernel32.dll");
		if (ernel32)
		{
			void* dwOrgEntry = GetProcAddress(ernel32, "SetUnhandledExceptionFilter");
			if (dwOrgEntry)
			{
				DWORD offset = (char*)dwOrgEntry - (char*)ernel32;

				ReadProcMem((char*)ernel32 + offset, oldSetUnhandledExceptionFilter, 5);

				BYTE patch[] = {0xE9};
				WriteProcMem((char*)ernel32 + offset, patch, 1);
				PatchCallAddr((char*)ernel32, offset, (char*)Cb_SetUnhandledExceptionFilter);
			}
		}
#endif

		CallPluginsAfter(HookedCall::FLHook__LoadSettings);
	}
	catch (char* szError)
	{
		Console::ConErr(std::format("CRITICAL! {}\n", szError));
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
	catch (char* szError)
	{
		UnloadHookExports();

		Console::ConErr(szError);
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

	// unload console
	TerminateThread(hConsoleThread, 0);
	SetConsoleCtrlHandler(ConsoleHandler, FALSE);
	FreeConsole();

	// misc
	DeleteCriticalSection(&cs);

	// finish
	FreeLibraryAndExitThread(hMe, 0);
}

void FLHookShutdown()
{
	TerminateThread(hThreadResolver, 0);

	// unload update hook
	void* pAddress = (void*)((char*)hProcFL + ADDR_UPDATE);
	WriteProcMem(pAddress, &fpOldUpdate, 4);

	// unload hooks
	UnloadHookExports();

#ifndef DISABLE_EXTENDED_EXCEPTION_LOGGING
	// If extended exception logging is in use, restore patched functions
	if (HMODULE ernel32 = GetModuleHandle("kernel32.dll"))
	{
		void* dwOrgEntry = GetProcAddress(ernel32, "SetUnhandledExceptionFilter");
		if (dwOrgEntry)
		{
			DWORD offset = (char*)dwOrgEntry - (char*)ernel32;
			WriteProcMem((char*)ernel32 + offset, oldSetUnhandledExceptionFilter, 5);
		}
	}
	// And restore the default exception filter.
	SetUnhandledExceptionFilter(0);
#endif

	AddLog(LogType::Normal, LogLevel::Err, "-------------------");

	// unload rest
	DWORD id;
	DWORD dwParam;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FLHookUnload, &dwParam, 0, &id);
}

void ProcessPendingCommands()
{
	TRY_HOOK
	{
		// check for new console commands
		EnterCriticalSection(&cs);
		while (!lstConsoleCmds.empty())
		{
			std::wstring* pwscCmd = lstConsoleCmds.front();
			lstConsoleCmds.pop_front();
			if (pwscCmd)
			{
				AdminConsole.ExecuteCommandString(Trim(*pwscCmd));
				delete pwscCmd;
			}
		}
		LeaveCriticalSection(&cs);
	}
	CATCH_HOOK({})
}
