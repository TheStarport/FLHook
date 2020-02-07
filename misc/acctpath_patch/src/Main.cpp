/**
 A patch for flserver to change the account directory path.
 By cannon 15 July 2009.
*/

// includes 
#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <float.h>

/// Current configuration.
std::string set_scDataPath = "";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Write iSize bytes from pMem into pAddress
static void WriteProcMem(void *pAddress, void *pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	BOOL res = VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	res = WriteProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string IniGetS(std::string scFile, std::string scApp, std::string scKey, std::string scDefault)
{
	char szRet[2048];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), scDefault.c_str(), szRet, sizeof(szRet), scFile.c_str());
	return szRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string Trim(std::string scIn)
{
	while(scIn.length() && (scIn[0]==' ' || scIn[0]=='	' || scIn[0]=='\n' || scIn[0]=='\r') )
	{
		scIn = scIn.substr(1);
	}
	while(scIn.length() && (scIn[scIn.length()-1]==L' ' || scIn[scIn.length()-1]=='	' || scIn[scIn.length()-1]=='\n' || scIn[scIn.length()-1]=='\r') )
	{
		scIn = scIn.substr(0, scIn.length()-1);
	}
	return scIn;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static FARPROC fpOldGetUserDataPath = 0;
char __cdecl HkCb_GetUserDataPath(LPSTR pszPath)
{
	strncpy(pszPath, set_scDataPath.c_str(), MAX_PATH);
	return 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	static bool bPatchInstalled = false;

	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
	if (fdwReason == DLL_PROCESS_ATTACH && !bPatchInstalled)
	{
		bPatchInstalled = true;

		// The path to the configuration file.
		char szCurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(szCurDir), szCurDir);
		std::string scServerCfgFile = std::string(szCurDir) + "\\acctpath_patch.cfg";
		set_scDataPath = Trim(IniGetS(scServerCfgFile, "General", "DataPath", ""));

		// Install the patch
		HMODULE hModCommon = GetModuleHandleA("common");
		if (hModCommon && set_scDataPath.length())
		{
			char fpPatch[] = { '\xE9' };
			WriteProcMem((char*)hModCommon + 0xA2FA0, &fpPatch, 1);
			DWORD dwJmpOffset = (DWORD)((char*)HkCb_GetUserDataPath) - (DWORD)hModCommon - 0xA2FA0 - 5;
			WriteProcMem((char*)hModCommon + 0xA2FA1, &dwJmpOffset, 4);
		}
	}
	else if (fdwReason == DLL_PROCESS_DETACH && bPatchInstalled)
	{
		bPatchInstalled = false;

		// Remove the patch
		HMODULE hModCommon = GetModuleHandleA("common");
		if (hModCommon && set_scDataPath.length())
		{
			char fpPatch[] = { '\x81', '\xEC', '\x10', '\x01', '\x00'};
			WriteProcMem((char*)hModCommon + 0xA2FA0, &fpPatch, 5);
		}
	}
	return true;
}
