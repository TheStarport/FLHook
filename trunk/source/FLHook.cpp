#include "global.h"
#include <Psapi.h>
#include "hook.h"
#include "CConsole.h"
#include "CSocket.h"

// structs
struct SOCKET_CONNECTION
{
	wstring wscPending;
	CSocket	csock;
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
HMODULE hWString;

HANDLE hConsoleIn;
HANDLE hConsoleOut;
HANDLE hConsoleErr;

SOCKET sListen = INVALID_SOCKET;
SOCKET sWListen = INVALID_SOCKET;
SOCKET sEListen = INVALID_SOCKET;
SOCKET sEWListen = INVALID_SOCKET;

list<wstring*> lstConsoleCmds;
list<SOCKET_CONNECTION*> lstSockets;
list<SOCKET_CONNECTION*> lstDelete;

CRITICAL_SECTION cs;

FILE *fLog = 0;
FILE *fLogDebug = 0;

bool bExecuted = false;

CConsole AdminConsole;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

_CreateWString CreateWString;
_FreeWString FreeWString;
_CreateString CreateString;
_FreeString FreeString;
_GetCString GetCString;
_GetWCString GetWCString;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Init();

/**************************************************************************************************************
DllMain
**************************************************************************************************************/

FARPROC fpOldUpdate;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if(bExecuted)
		return TRUE;

	char szFile[MAX_PATH];
	GetModuleFileName(0, szFile, sizeof(szFile));
	wstring wscFileName = ToLower(stows(szFile));

	if(wscFileName.find(L"flserver.exe") != -1)
	{ // start FLHook
		bExecuted = true;

		// redirect IServerImpl::Update
		FARPROC fpLoop = (FARPROC)HkIServerImpl::Update;
		void *pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_UPDATE);
		ReadProcMem(pAddress, &fpOldUpdate, 4);
		WriteProcMem(pAddress, &fpLoop, 4);

		// install startup hook
		FARPROC fpStartup = (FARPROC)HkIServerImpl::Startup;
		pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_STARTUP);
		WriteProcMem(pAddress, &fpStartup, 4);

		// install shutdown hook
		FARPROC fpShutdown = (FARPROC)HkIServerImpl::Shutdown;
		pAddress = (void*)((char*)GetModuleHandle(0) + ADDR_SHUTDOWN);
		WriteProcMem(pAddress, &fpShutdown, 4);

		// create log dirs
		CreateDirectoryA("./flhook_logs/",NULL);
		CreateDirectoryA("./flhook_logs/debug",NULL);
	}
	return TRUE;
}

/**************************************************************************************************************
Replace FLServer's exception handler with our own.
**************************************************************************************************************/
#ifdef EXTENDED_EXCEPTION_LOGGING
LONG WINAPI FLHookTopLevelFilter(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	WriteMiniDump(pExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER; 	// EXCEPTION_CONTINUE_SEARCH;
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI HkCb_SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}
#endif

/**************************************************************************************************************
thread that reads console input
**************************************************************************************************************/

void ReadConsoleEvents()
{
	while(1)
	{
		DWORD dwBytesRead;
		char szCmd[1024];
		memset(szCmd, 0, sizeof(szCmd));
		if(ReadConsole(hConsoleIn, szCmd, sizeof(szCmd), &dwBytesRead, 0))
		{
			string scCmd = szCmd;
			if(scCmd[scCmd.length()-1] == '\n')
				scCmd = scCmd.substr(0, scCmd.length()-1);
			if(scCmd[scCmd.length()-1] == '\r')
				scCmd = scCmd.substr(0, scCmd.length()-1);

			wstring wscCmd = stows(scCmd);
			EnterCriticalSection(&cs);
			wstring *pwscCmd = new wstring;
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
	switch(dwCtrlType)
	{
	case CTRL_CLOSE_EVENT:
		{
			return TRUE;
		} break;
	}

	return FALSE;
}

/**************************************************************************************************************
init
**************************************************************************************************************/
string sDebugLog;

void FLHookInit_Pre()
{

	InitializeCriticalSection(&cs);
	hProcFL = GetModuleHandle(0);

	try {

		// get module handles
		if(!(hModServer = GetModuleHandle("server")))
			throw "server.dll not loaded";

		// create log dirs
		CreateDirectoryA("./flhook_logs/",NULL);
		CreateDirectoryA("./flhook_logs/debug",NULL);

		// start console
		AllocConsole();
		SetConsoleTitle("FLHook");
		SetConsoleCtrlHandler(ConsoleHandler, TRUE);
		hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);
		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		hConsoleErr = GetStdHandle(STD_ERROR_HANDLE);

		ConPrint(L"Welcome to FLHook Console (" VERSION L")\n");

		DWORD id;
		DWORD dwParam;
		hConsoleThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ReadConsoleEvents, &dwParam, 0, &id);

		// logs
		fLog = fopen("./flhook_logs/FLHook.log", "at");
		char szDate[64];
		time_t tNow = time(0);
		struct tm *t = localtime(&tNow);
		strftime(szDate, sizeof(szDate), "%d.%m.%Y_%H.%M", t);
		sDebugLog = "./flhook_logs/debug/FLHookDebug_"+(string)szDate;
		sDebugLog += ".log";

		// plugins
		mpPluginHooks.clear();
		lstPlugins.clear();

		//check what plugins should be loaded; we need to read out the settings ourselves cause LoadSettings() wasn't called yet
		char szCurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(szCurDir), szCurDir);
		string scCfgFile = string(szCurDir) + "\\FLHook.ini";
		
		if(IniGetB(scCfgFile, "Plugins", "LoadAllPlugins", true))
			PluginManager::LoadPlugins(true, &AdminConsole);
		else
		{
			//LoadAllPlugins = false, check what plugins should be loaded
			list<INISECTIONVALUE> lstIniPlugins;
			IniGetSection(scCfgFile, "Plugins", lstIniPlugins);
			foreach(lstIniPlugins, INISECTIONVALUE, it)
			{
				if(it->scKey != "plugin")
					continue;
				PluginManager::LoadPlugin(it->scValue, &AdminConsole);
			}
		}

		PatchClientImpl();

#ifdef EXTENDED_EXCEPTION_LOGGING
		// Install our own exception handler to automatically log minidumps.
		::SetUnhandledExceptionFilter(FLHookTopLevelFilter);

		// Hook the kernel SetUnhandledExceptionFilter function to prevent
		// newer versions of the crt disabling our filter function if a buffer
		// overrun is detected.
		HMODULE hKernel32 = LoadLibrary("kernel32.dll");
		if (hKernel32)
		{
			void *dwOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
			if (dwOrgEntry)
			{
				void *HookFunc = &HkCb_SetUnhandledExceptionFilter;

				DWORD dwRelativeAddr = (DWORD)HookFunc;
				dwRelativeAddr -= (DWORD)dwOrgEntry;
				dwRelativeAddr -= 5;
				BYTE patch[] = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
				memcpy(patch + 1, &dwRelativeAddr, 4);
				WriteProcMem(dwOrgEntry, patch, 5);
			}
		}
#endif

	} catch(char *szError) {
		ConPrint(L"ERROR: %s\n", stows(szError).c_str());
	}
}


bool FLHookInit()
{
	bool bInitHookExports = false;

	try {

		// get module handles
		if(!(hModServer = GetModuleHandle("server")))
			throw "server.dll not loaded";
		if(!(hModRemoteClient = GetModuleHandle("remoteclient")))
			throw "remoteclient.dll not loaded";
		if(!(hModCommon = GetModuleHandle("common")))
			throw "common.dll not loaded";
		if(!(hModDPNet = GetModuleHandle("dpnet")))
			throw "dpnet.dll not loaded";
		if(!(hModDaLib = GetModuleHandle("dalib")))
			throw "dalib.dll not loaded";
		if(!(hModContent = GetModuleHandle("content")))
			throw "content.dll not loaded";
		if(!(hMe = GetModuleHandle("FLHook")))
			throw "FLHook.dll not loaded";
		if(!(hWString = LoadLibrary("FLHookVC6Strings.dll")))
			throw "FLHookVC6Strings.dll not found";
		if(!(CreateWString = (_CreateWString)GetProcAddress(hWString, "CreateWString")))
			throw "FLHookVC6Strings.dll: CreateWString not found";
		if(!(FreeWString = (_FreeWString)GetProcAddress(hWString, "FreeWString")))
			throw "FLHookVC6Strings.dll: FreeWString not found";
		if(!(CreateString = (_CreateString)GetProcAddress(hWString, "CreateString")))
			throw "FLHookVC6Strings.dll: CreateString not found";
		if(!(FreeString = (_FreeString)GetProcAddress(hWString, "FreeString")))
			throw "FLHookVC6Strings.dll: FreeString not found";
		if(!(GetCString = (_GetCString)GetProcAddress(hWString, "GetCString")))
			throw "FLHookVC6Strings.dll: GetCString not found";
		if(!(GetWCString = (_GetWCString)GetProcAddress(hWString, "GetWCString")))
			throw "FLHookVC6Strings.dll: GetWCString not found";


		// load settings
		LoadSettings();

		// init hooks
		if(!InitHookExports())
			throw "InitHookExports failed";

		bInitHookExports = true;

		if(set_bDebug && !fLogDebug)
			fLogDebug = fopen(sDebugLog.c_str(), "at");

		if(set_bSocketActivated)
		{ // listen to socket
			WSADATA wsa;
			WSAStartup(MAKEWORD(2, 0), &wsa);
			if(set_bSocketActivated)
			{
				if(set_iPort)
				{
					sListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iPort);
					if(bind(sListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "ascii: socket-bind failed, port already in use?";

					if(listen(sListen, SOMAXCONN) != 0)
						throw "ascii: socket-listen failed";

					ConPrint(L"socket(ascii): socket connection listening\n");
				}

				if(set_iWPort)
				{
					sWListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iWPort);
					if(bind(sWListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "unicode: socket-bind failed, port already in use?";

					if(listen(sWListen, SOMAXCONN) != 0)
						throw "unicode: socket-listen failed";

					ConPrint(L"socket(unicode): socket connection listening\n");
				}

				if(set_iEPort)
				{
					sEListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iEPort);
					if(bind(sEListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "encrypted: socket-bind failed, port already in use?";

					if(listen(sEListen, SOMAXCONN) != 0)
						throw "encrypted: socket-listen failed";

					ConPrint(L"socket(encrypted-ascii): socket connection listening\n");
				}

				if(set_iEWPort)
				{
					sEWListen = socket(AF_INET, SOCK_STREAM, 0);
					sockaddr_in adr;
					memset(&adr, 0, sizeof(adr));
					adr.sin_family = AF_INET;
					adr.sin_port = htons(set_iEWPort);
					if(bind(sEWListen, (sockaddr*)&adr, sizeof(adr)) != 0)
						throw "encrypted-unicode: socket-bind failed, port already in use?";

					if(listen(sEWListen, SOMAXCONN) != 0)
						throw "encrypted-unicode: socket-listen failed";

					ConPrint(L"socket(encrypted-unicode): socket connection listening\n");
				}
			}
		}

		
	} catch(char *szError) {
		if(bInitHookExports)
			UnloadHookExports();

		if(sListen != INVALID_SOCKET)
		{
			closesocket(sListen);
			sListen = INVALID_SOCKET;
		}

		if(sWListen != INVALID_SOCKET)
		{
			closesocket(sWListen);
			sWListen = INVALID_SOCKET;
		}

		if(sEListen != INVALID_SOCKET)
		{
			closesocket(sEListen);
			sEListen = INVALID_SOCKET;
		}

		if(sEWListen != INVALID_SOCKET)
		{
			closesocket(sEWListen);
			sEWListen = INVALID_SOCKET;
		}

		ConPrint(L"ERROR: %s\n", stows(szError).c_str());
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
	if(sListen != INVALID_SOCKET)
		closesocket(sListen);
	if(sWListen != INVALID_SOCKET)
		closesocket(sWListen);
	if(sEListen != INVALID_SOCKET)
		closesocket(sEListen);
	if(sEWListen != INVALID_SOCKET)
		closesocket(sEWListen);

	for(list<SOCKET_CONNECTION*>::iterator i = lstSockets.begin(); (i != lstSockets.end()); i++)
	{
		closesocket((*i)->csock.s);
		delete *i;
	}
	lstSockets.clear();

	// free blowfish encryption data
	if(set_BF_CTX)
	{
		ZeroMemory(set_BF_CTX, sizeof(set_BF_CTX));
		free(set_BF_CTX);
	}

	// misc
	DeleteCriticalSection(&cs);

	// finish
	FreeLibrary(hWString);
	FreeLibraryAndExitThread(hMe, 0); 
}

void FLHookShutdown()
{

	// unload update hook
	void *pAddress = (void*)((char*)hProcFL + ADDR_UPDATE);
	WriteProcMem(pAddress, &fpOldUpdate, 4);

	// unload hooks
	UnloadHookExports();

	// close log
	fclose(fLog);
	if(set_bDebug)
		fclose(fLogDebug);

	// unload rest
	DWORD id;
	DWORD dwParam;
	CreateThread(0, 0, (LPTHREAD_START_ROUTINE)FLHookUnload, &dwParam, 0, &id);

}

/**************************************************************************************************************
process a socket command
return true -> close socket connection
**************************************************************************************************************/

bool ProcessSocketCmd(SOCKET_CONNECTION *sc, wstring wscCmd)
{
	if(!ToLower(wscCmd).find(L"quit")) { // quit connection
		sc->csock.DoPrint(L"Goodbye.\r\n");
		ConPrint(L"socket: connection closed\n");
		return true;
	} else if(!(sc->csock.bAuthed)) { // not authenticated yet
		wstring wscLwr = ToLower(wscCmd);
		if(wscLwr.find(L"pass") != 0)
		{
			sc->csock.Print(L"ERR Please authenticate first\n");
			return false;
		}

		wchar_t wszPass[256];
		if(wscCmd.length() >= 256)
		{
			sc->csock.DoPrint(L"ERR Wrong password\n");
//			ConPrint(L"socket: socket authentication failed (invalid pass)\n");
			sc->csock.DoPrint(L"Goodbye.\r\n");
			ConPrint(L"socket: connection closed (invalid pass)\n");
			AddLog("socket: socket connection from %s:%d closed (invalid pass)", sc->csock.sIP.c_str(), sc->csock.iPort);
			return true;
		}
		swscanf(wscCmd.c_str(), L"pass %s", wszPass);

		// read passes from ini
		for(uint i = 0;; i++)
		{
			char szBuf[64];
			sprintf(szBuf, "pass%u", i);
			string scPass = IniGetS(set_scCfgFile, "Socket", szBuf, "");
			sprintf(szBuf, "rights%u", i);
			string scRights = IniGetS(set_scCfgFile, "Socket", szBuf, "");

			if(!scPass.length()) {
				sc->csock.DoPrint(L"ERR Wrong password\n");
//				ConPrint(L"socket: socket authentication failed (invalid pass)\n");
				sc->csock.DoPrint(L"Goodbye.\r\n");
				ConPrint(L"socket: connection closed (invalid pass)\n");
				AddLog("socket: socket connection from %s:%d closed (invalid pass)", sc->csock.sIP.c_str(), sc->csock.iPort);
				return true;
			} else if(!scPass.compare(wstos(wszPass))) {
				sc->csock.bAuthed = true;
				sc->csock.SetRightsByString(scRights);
				sc->csock.Print(L"OK\n");
				ConPrint(L"socket: socket authentication successful\n");
				return false;
			}
		}
	} else { // execute admin command
		if(wscCmd[wscCmd.length()-1] == '\n')
			wscCmd = wscCmd.substr(0, wscCmd.length()-1);
		if(wscCmd[wscCmd.length()-1] == '\r')
			wscCmd = wscCmd.substr(0, wscCmd.length()-1);

		if(!wscCmd.compare(L"eventmode")) {
			if(sc->csock.rights & RIGHT_EVENTMODE) {
				sc->csock.Print(L"OK\n");
				sc->csock.bEventMode = true;
			} else {
				sc->csock.Print(L"ERR No permission\n");
			}		
		} else
			sc->csock.ExecuteCommandString(wscCmd);

		return false;
	}
}

/**************************************************************************************************************
write text to console
**************************************************************************************************************/

void ConPrint(wstring wscText, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscText);

	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscText.c_str(), marker);

	DWORD iCharsWritten;
	string scText = wstos(wszBuf);
	WriteConsole(hConsoleOut, scText.c_str(), (DWORD)scText.length(), &iCharsWritten, 0);
}

/**************************************************************************************************************
send event to all sockets which are in eventmode
**************************************************************************************************************/

void ProcessEvent(wstring wscText, ...)
{
	wchar_t wszBuf[1024] = L"";
	va_list marker;
	va_start(marker, wscText);
	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscText.c_str(), marker);

	foreach(lstSockets, SOCKET_CONNECTION*, i)
	{
		if((*i)->csock.bEventMode)
			(*i)->csock.Print(L"%s\n", wszBuf);
	}
}

/**************************************************************************************************************
check for pending admin commands in console or socket and execute them
**************************************************************************************************************/

struct timeval tv = {0, 0};

void ProcessPendingCommands()
{
	try {
		// check for new console commands
		EnterCriticalSection(&cs);
		while(lstConsoleCmds.size())
		{
			wstring *pwscCmd = lstConsoleCmds.front();
			lstConsoleCmds.pop_front();
			AdminConsole.ExecuteCommandString(*pwscCmd);
			delete pwscCmd;
		}
		LeaveCriticalSection(&cs);

		if(sListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = false;
				sc->csock.bEncrypted = false;
				sc->wscPending = L"";
				lstSockets.push_back(sc);
				ConPrint(L"socket(ascii): new socket connection from %s:%d\n", stows(sc->csock.sIP).c_str(), sc->csock.iPort);
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		if(sWListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sWListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sWListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = true;
				sc->wscPending = L"";
				sc->csock.bEncrypted = false;
				lstSockets.push_back(sc);
				ConPrint(L"socket(unicode): new socket connection from %s:%d\n", stows(sc->csock.sIP).c_str(), sc->csock.iPort);
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		if(sEListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sEListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sEListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = false;
				sc->wscPending = L"";
				sc->csock.bEncrypted = true;
				sc->csock.bfc = set_BF_CTX;
				lstSockets.push_back(sc);
				ConPrint(L"socket(encrypted-ascii): new socket connection from %s:%d\n", stows(sc->csock.sIP).c_str(), sc->csock.iPort);
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		if(sEWListen != INVALID_SOCKET)
		{ // check for new ascii socket connections
			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sEWListen, &fds);
			if(select(0, &fds, 0, 0, &tv))
			{ // accept new connection 
				sockaddr_in adr;

				int iLen = sizeof(adr);
				SOCKET s = accept(sEWListen, (sockaddr*)&adr, &iLen);
				ulong lNB = 1;
				ioctlsocket(s, FIONBIO, &lNB);
				SOCKET_CONNECTION *sc = new SOCKET_CONNECTION;
				sc->csock.s = s;
				sc->csock.sIP = (string)inet_ntoa(adr.sin_addr);
				sc->csock.iPort = adr.sin_port;
				sc->csock.bUnicode = true;
				sc->wscPending = L"";
				sc->csock.bEncrypted = true;
				sc->csock.bfc = set_BF_CTX;
				lstSockets.push_back(sc);
				ConPrint(L"socket(encrypted-unicode): new socket connection from %s:%d\n", stows(sc->csock.sIP).c_str(), sc->csock.iPort);
				sc->csock.Print(L"Welcome to FLHack, please authenticate\n");
			}
		}

		// check for pending socket-commands
		foreach(lstSockets, SOCKET_CONNECTION*, i)
		{
			SOCKET_CONNECTION *sc = *i;

			FD_SET fds;
			FD_ZERO(&fds);
			FD_SET(sc->csock.s, &fds);
			struct timeval tv = {0, 0};
			if(select(0, &fds, 0, 0, &tv))
			{ // data to be read
				ulong lSize;
				ioctlsocket(sc->csock.s, FIONREAD, &lSize);
				char *szData = new char[lSize + 1];
				memset(szData, 0, lSize + 1);
				if(recv(sc->csock.s, szData, lSize, 0) <= 0)
				{
					ConPrint(L"socket: socket connection closed\n");
					delete[] szData;
					lstDelete.push_back(sc);
					continue;
				}

				// enqueue commands (terminated by \n)
				wstring wscData;
				if(sc->csock.bEncrypted)
				{
					SwapBytes(szData, lSize);
					Blowfish_Decrypt(sc->csock.bfc, szData, lSize); 
					SwapBytes(szData, lSize);
				}
				if(sc->csock.bUnicode)
					wscData = wstring((wchar_t*)szData,lSize/2);
				else
					wscData = stows(szData);

				wstring wscTmp = sc->wscPending + wscData;
				wscData = wscTmp;

				// check for memory overflow ddos attack
				uint iMaxKB = 1;
				if((sc->csock.bAuthed))  // not authenticated yet
					iMaxKB = 500;
				if(wscData.length() > (1024*iMaxKB)) {
					ConPrint(L"socket: socket connection closed (possible ddos attempt)\n");
					AddLog("socket: socket connection from %s:%d closed (possible ddos attempt)", sc->csock.sIP.c_str(), sc->csock.iPort);
					delete[] szData;
					lstDelete.push_back(sc);
					continue;
				}

				list<wstring> lstCmds;
				wstring wscCmd;
				for(uint i = 0; (i < wscData.length()); i++)
				{
					if(!wscData.substr(i, 2).compare(L"\r\n")) {
						lstCmds.push_back(wscCmd);
						wscCmd = L"";
						i++;
					} else if(wscData[i] == '\n') {
						lstCmds.push_back(wscCmd);
						wscCmd = L"";
					} else 
						wscCmd.append(1, wscData[i]);
				}

				sc->wscPending = wscCmd;

				// process cmds
				foreach(lstCmds, wstring, it)
				{
					if(ProcessSocketCmd(sc, (*it)))
					{
						lstDelete.push_back(sc);
						break;
					}
				}

				delete[] szData;
			}
		}

		// delete closed connections
		foreach(lstDelete, SOCKET_CONNECTION*, it)
		{
			closesocket((*it)->csock.s);
			lstSockets.remove(*it);
			delete (*it);
		}

		lstDelete.clear();
	} catch(...) { 
		LOG_EXCEPTION
		throw "exception"; 
	}
}