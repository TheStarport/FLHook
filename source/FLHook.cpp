#include "Global.hpp"
#include "CConsole.h"

#include "Memory/MemoryManager.hpp"

// structs
struct SOCKET_CONNECTION
{
	std::wstring wscPending;
	CSocket csock;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HANDLE hProcFL = 0;
HMODULE hModServer = 0;
HMODULE hModCommon = 0;
HMODULE hModRemoteClient = 0;
HMODULE hMe = 0;
HMODULE hModDPNet = 0;
HMODULE hModDaLib = 0;
HMODULE hModContent = 0;
HANDLE hConsoleThread;

HANDLE hConsoleIn;
HANDLE hConsoleOut;
HANDLE hConsoleErr;

SOCKET sListen = INVALID_SOCKET;
SOCKET sWListen = INVALID_SOCKET;
SOCKET sEListen = INVALID_SOCKET;
SOCKET sEWListen = INVALID_SOCKET;

std::list<std::wstring*> lstConsoleCmds;
std::list<SOCKET_CONNECTION*> lstSockets;
std::list<SOCKET_CONNECTION*> lstDelete;

CRITICAL_SECTION cs;

bool bExecuted = false;

CConsole AdminConsole;

st6_malloc_t st6_malloc;
st6_free_t st6_free;

bool flhookReady;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  ______ _      _    _             _      _  _    ___  
 |  ____| |    | |  | |           | |    | || |  / _ \ 
 | |__  | |    | |__| | ___   ___ | | __ | || |_| | | |
 |  __| | |    |  __  |/ _ \ / _ \| |/ / |__   _| | | |
 | |    | |____| |  | | (_) | (_) |   <     | |_| |_| |
 |_|    |______|_|  |_|\___/ \___/|_|\_\    |_(_)\___/                              
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
		if (!(hModServer = GetModuleHandle("server")))
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
	catch (std::filesystem::filesystem_error& error)
	{
		Console::ConErr(std::format("Failed to create directory {}\n{}", error.path1().generic_string(), error.what()));
	}
}

bool FLHookInit()
{
	bool bInitHookExports = false;

	try
	{
		// get module handles
		if (!(hModServer = GetModuleHandle("server")))
			throw std::runtime_error("server.dll not loaded");
		if (!(hModRemoteClient = GetModuleHandle("remoteclient")))
			throw std::runtime_error("remoteclient.dll not loaded");
		if (!(hModCommon = GetModuleHandle("common")))
			throw std::runtime_error("common.dll not loaded");
		if (!(hModDPNet = GetModuleHandle("dpnet")))
			throw std::runtime_error("dpnet.dll not loaded");
		if (!(hModDaLib = GetModuleHandle("dalib")))
			throw std::runtime_error("dalib.dll not loaded");
		if (!(hModContent = GetModuleHandle("content")))
			throw std::runtime_error("content.dll not loaded");
		if (!(hMe = GetModuleHandle("FLHook")))
			throw std::runtime_error("FLHook.dll not loaded");

		// init hooks
		if (!InitHookExports())
			throw std::runtime_error("InitHookExports failed");

		bInitHookExports = true;
		const auto* config = FLHookConfig::c();

		if (config->socket.activated)
		{ // listen to socket
			WSADATA wsa;
			WSAStartup(MAKEWORD(2, 0), &wsa);
			if (config->socket.port > 0)
			{
				sListen = socket(AF_INET, SOCK_STREAM, 0);
				sockaddr_in adr;
				memset(&adr, 0, sizeof(adr));
				adr.sin_family = AF_INET;
				adr.sin_port = htons(config->socket.port);
				if (::bind(sListen, (sockaddr*)&adr, sizeof(adr)) != 0)
					throw std::runtime_error("ascii: socket-bind failed, port already in use?");

				if (listen(sListen, SOMAXCONN) != 0)
					throw std::runtime_error("ascii: socket-listen failed");

				Console::ConInfo("socket(ascii): socket connection listening");
			}

			if (config->socket.wPort > 0)
			{
				sWListen = socket(AF_INET, SOCK_STREAM, 0);
				sockaddr_in adr;
				memset(&adr, 0, sizeof(adr));
				adr.sin_family = AF_INET;
				adr.sin_port = htons(config->socket.wPort);
				if (::bind(sWListen, (sockaddr*)&adr, sizeof(adr)) != 0)
					throw std::runtime_error("unicode: socket-bind failed, port already in use?");

				if (listen(sWListen, SOMAXCONN) != 0)
					throw std::runtime_error("unicode: socket-listen failed");

				Console::ConInfo("socket(unicode): socket connection listening");
			}

			if (config->socket.ePort > 0)
			{
				sEListen = socket(AF_INET, SOCK_STREAM, 0);
				sockaddr_in adr;
				memset(&adr, 0, sizeof(adr));
				adr.sin_family = AF_INET;
				adr.sin_port = htons(config->socket.ePort);
				if (::bind(sEListen, (sockaddr*)&adr, sizeof(adr)) != 0)
					throw std::runtime_error("encrypted: socket-bind failed, port already in use?");

				if (listen(sEListen, SOMAXCONN) != 0)
					throw std::runtime_error("encrypted: socket-listen failed");

				Console::ConInfo("socket(encrypted-ascii): socket connection listening");
			}

			if (config->socket.eWPort > 0)
			{
				sEWListen = socket(AF_INET, SOCK_STREAM, 0);
				sockaddr_in adr;
				memset(&adr, 0, sizeof(adr));
				adr.sin_family = AF_INET;
				adr.sin_port = htons(config->socket.eWPort);
				if (::bind(sEWListen, (sockaddr*)&adr, sizeof(adr)) != 0)
					throw std::runtime_error("encrypted-unicode: socket-bind failed, port already in use?");

				if (listen(sEWListen, SOMAXCONN) != 0)
					throw std::runtime_error("encrypted-unicode: socket-listen failed");

				Console::ConInfo("socket(encrypted-unicode): socket connection listening");
			}
		}
	}
	catch (char* szError)
	{
		if (bInitHookExports)
			UnloadHookExports();

		if (sListen != INVALID_SOCKET)
		{
			closesocket(sListen);
			sListen = INVALID_SOCKET;
		}

		if (sWListen != INVALID_SOCKET)
		{
			closesocket(sWListen);
			sWListen = INVALID_SOCKET;
		}

		if (sEListen != INVALID_SOCKET)
		{
			closesocket(sEListen);
			sEListen = INVALID_SOCKET;
		}

		if (sEWListen != INVALID_SOCKET)
		{
			closesocket(sEWListen);
			sEWListen = INVALID_SOCKET;
		}

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

	// quit network sockets
	if (sListen != INVALID_SOCKET)
		closesocket(sListen);
	if (sWListen != INVALID_SOCKET)
		closesocket(sWListen);
	if (sEListen != INVALID_SOCKET)
		closesocket(sEListen);
	if (sEWListen != INVALID_SOCKET)
		closesocket(sEWListen);

	for (auto i : lstSockets)
	{
		closesocket(i->csock.s);
		delete i;
	}
	lstSockets.clear();

	// free blowfish encryption data
	if (const FLHookConfig* config = FLHookConfig::i(); config->socket.bfCTX)
	{
		ZeroMemory(config->socket.bfCTX, sizeof(config->socket.bfCTX));
		free(config->socket.bfCTX);
	}

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
	HMODULE ernel32 = GetModuleHandle("kernel32.dll");
	if (ernel32)
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

/**************************************************************************************************************
process a socket command
return true -> close socket connection
**************************************************************************************************************/

bool ProcessSocketCmd(SOCKET_CONNECTION* sc, std::wstring wscCmd)
{
	if (!ToLower(wscCmd).find(L"quit"))
	{ // quit connection
		sc->csock.DoPrint("Goodbye.\r");
		Console::ConInfo("socket: connection closed");
		return true;
	}
	else if (!(sc->csock.bAuthed))
	{
		const std::wstring wscLwr = ToLower(wscCmd);
		auto passFind = wscLwr.find(L"pass ");
		if (passFind == std::wstring::npos)
		{
			sc->csock.Print("ERR Please authenticate first");
			return false;
		}

		if (wscCmd.length() >= 256)
		{
			sc->csock.DoPrint("ERR Wrong password");
			sc->csock.DoPrint("Goodbye.\r");
			Console::ConInfo("socket: connection closed (invalid pass)");
			AddLog(LogType::Normal, LogLevel::Info, std::format("socket: socket connection from {}:{} closed (invalid pass)", sc->csock.sIP, sc->csock.iPort));
			return true;
		}

		const auto* config = FLHookConfig::c();

		// Remove the string "pass " from the string
		auto pass = wscCmd.substr(passFind + 5);
		pass = Trim(pass);

		const auto auth = config->socket.passRightsMap.find(pass);
		if (auth == config->socket.passRightsMap.end())
		{
			sc->csock.DoPrint("ERR Wrong password");
			sc->csock.DoPrint("Goodbye.\r");
			Console::ConInfo("socket: connection closed (invalid pass)");
			AddLog(LogType::Normal,
			    LogLevel::Info,
			    std::format("socket: socket connection from {}:{} closed (invalid pass)", sc->csock.sIP.c_str(), sc->csock.iPort));
			return true;
		}

		sc->csock.bAuthed = true;
		sc->csock.SetRightsByString(auth->second);
		sc->csock.Print("OK");
		Console::ConInfo("socket: socket authentication successful");
		return false;
	}
	else
	{
		if (const auto cmd = Trim(wscCmd); cmd == L"eventmode")
		{
			if (sc->csock.rights & RIGHT_EVENTMODE)
			{
				sc->csock.Print("OK");
				sc->csock.bEventMode = true;
			}
			else
			{
				sc->csock.Print("ERR No permission");
			}
		}
		else
			sc->csock.ExecuteCommandString(cmd);

		return false;
	}
}

/**************************************************************************************************************
send event to all sockets which are in eventmode
**************************************************************************************************************/

void ProcessEvent(std::wstring text, ...)
{
	wchar_t wszBuf[1024] = L"";
	va_list marker;
	va_start(marker, text);
	_vsnwprintf_s(wszBuf, (sizeof(wszBuf) / 2) - 1, text.c_str(), marker);

	text = wszBuf;

	CallPluginsBefore(HookedCall::FLHook__ProcessEvent, static_cast<std::wstring&>(text));

	for (auto& socket : lstSockets)
	{
		if (socket->csock.bEventMode)
			socket->csock.Print(wstos(text));
	}
}

/**************************************************************************************************************
check for pending admin commands in console or socket and execute them
**************************************************************************************************************/

struct timeval tv = {0, 0};

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

		if (sListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sListen, &fds);
			if (select(0, &fds, 0, 0, &tv))
			{ // accept new connection
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION* sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (std::string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = false;
				sc->csock.bEncrypted = false;
				sc->wscPending = L"";
				lstSockets.push_back(sc);
				Console::ConInfo(std::format("socket(ascii): new socket connection from {}:{}", sc->csock.sIP, sc->csock.iPort));
				sc->csock.Print("Welcome to FLHack, please authenticate");
			}
		}

		if (sWListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sWListen, &fds);
			if (select(0, &fds, 0, 0, &tv))
			{ // accept new connection
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sWListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION* sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (std::string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = true;
				sc->wscPending = L"";
				sc->csock.bEncrypted = false;
				lstSockets.push_back(sc);
				Console::ConInfo(std::format("socket(unicode): new socket connection from {}:{}", sc->csock.sIP, sc->csock.iPort));
				sc->csock.Print("Welcome to FLHack, please authenticate");
			}
		}

		if (sEListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sEListen, &fds);
			if (select(0, &fds, nullptr, 0, &tv))
			{ // accept new connection
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sEListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION* sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (std::string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = false;
				sc->wscPending = L"";
				sc->csock.bEncrypted = true;
				sc->csock.bfc = static_cast<BLOWFISH_CTX*>(FLHookConfig::c()->socket.bfCTX);
				lstSockets.push_back(sc);
				Console::ConInfo(std::format("socket(encrypted-ascii): new socket connection from {}:{}", sc->csock.sIP, sc->csock.iPort));
				sc->csock.Print("Welcome to FLHack, please authenticate");
			}
		}

		if (sEWListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sEWListen, &fds);
			if (select(0, &fds, 0, 0, &tv))
			{ // accept new connection
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sEWListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION* sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (std::string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = true;
				sc->wscPending = L"";
				sc->csock.bEncrypted = true;
				sc->csock.bfc = static_cast<BLOWFISH_CTX*>(FLHookConfig::c()->socket.bfCTX);
				lstSockets.push_back(sc);
				Console::ConInfo(std::format("socket(encrypted-unicode): new socket connection from {}:{}", sc->csock.sIP, sc->csock.iPort));
				sc->csock.Print("Welcome to FLHack, please authenticate");
			}
		}

		// check for pending socket-commands
		for (auto& sc : lstSockets)
		{
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sc->csock.s, &fds);
			struct timeval tv = {0, 0};
			if (select(0, &fds, 0, 0, &tv))
			{ // data to be read
				ulong lSize;
				ioctlsocket(sc->csock.s, FIONREAD, &lSize);
				char* szData = new char[lSize + 1];
				memset(szData, 0, lSize + 1);
				if (int err = recv(sc->csock.s, szData, lSize, 0); err <= 0)
				{
					if (FLHookConfig::c()->general.debugMode)
					{
						int wsaLastErr = WSAGetLastError();
						Console::ConWarn(std::format("Socket Error - recv: {} - WSAGetLastError: {}", err, wsaLastErr));
					}
					Console::ConWarn("socket: socket connection closed");
					delete[] szData;
					lstDelete.push_back(sc);
					continue;
				}

				// enqueue commands (terminated by \n)
				std::wstring wscData;
				if (sc->csock.bEncrypted)
				{
					SwapBytes(szData, lSize);
					Blowfish_Decrypt(sc->csock.bfc, szData, lSize);
					SwapBytes(szData, lSize);
				}
				if (sc->csock.bUnicode)
					wscData = std::wstring((wchar_t*)szData, lSize / 2);
				else
					wscData = stows(szData);

				std::wstring wscTmp = sc->wscPending + wscData;
				wscData = wscTmp;

				// check for memory overflow ddos attack
				uint iMaxKB = 1;
				if ((sc->csock.bAuthed)) // not authenticated yet
					iMaxKB = 500;
				if (wscData.length() > (1024 * iMaxKB))
				{
					Console::ConWarn("socket: socket connection closed (possible ddos attempt)");
					AddLog(LogType::Normal,
					    LogLevel::Info,
					    std::format("socket: socket connection from {}:{} closed (possible ddos attempt)", sc->csock.sIP, sc->csock.iPort));
					delete[] szData;
					lstDelete.push_back(sc);
					continue;
				}

				std::list<std::wstring> lstCmds;
				std::wstring wscCmd;
				for (uint i = 0; (i < wscData.length()); i++)
				{
					if (!wscData.substr(i, 2).compare(L"\r\n"))
					{
						lstCmds.push_back(wscCmd);
						wscCmd = L"";
						i++;
					}
					else if (wscData[i] == '\n')
					{
						lstCmds.push_back(wscCmd);
						wscCmd = L"";
					}
					else
						wscCmd.append(1, wscData[i]);
				}

				sc->wscPending = wscCmd;

				// process cmds
				for (auto& cmd : lstCmds)
				{
					if (ProcessSocketCmd(sc, cmd))
					{
						lstDelete.push_back(sc);
						break;
					}
				}

				delete[] szData;
			}
		}

		// delete closed connections
		for (auto it = lstDelete.begin(); it != lstDelete.end(); ++it)
		{
			closesocket((*it)->csock.s);
			lstSockets.remove(*it);
			delete (*it);
		}

		lstDelete.clear();
	}
	CATCH_HOOK({})
}
