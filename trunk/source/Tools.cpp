#include <time.h>
#include "global.h"
#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring stows(const string &scText)
{
	int iSize = MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, 0, 0);
	wchar_t *wszText = new wchar_t[iSize];
	MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, wszText, iSize);
	wstring wscRet = wszText;
	delete[] wszText;
	return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string wstos(const wstring &wscText)
{
	uint iLen = (uint)wscText.length() + 1;
	char *szBuf = new char[iLen];
	WideCharToMultiByte(CP_ACP, 0, wscText.c_str(), -1, szBuf, iLen, 0, 0);
	string scRet = szBuf;
	delete[] szBuf;
	return scRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string itos(int i)
{
	char szBuf[16];
	sprintf(szBuf, "%d", i);
	return szBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring ToLower(const wstring &wscStr)
{
	wstring wscResult;
	for(uint i = 0; (i < wscStr.length()); i++)
		wscResult += towlower(wscStr[i]);

	return wscResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string ToLower(const string &scStr)
{
	string scResult;
	for(uint i = 0; (i < scStr.length()); i++)
		scResult += tolower(scStr[i]);

	return scResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ToInt(const wstring &wscStr)
{
	return wcstoul(wscStr.c_str(), 0, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint ToUInt(const wstring &wscStr)
{
	return wcstoul(wscStr.c_str(), 0, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ToFloat(const wstring &wscStr)
{
	return (float)atof(wstos(wscStr).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 999.999.999

wstring ToMoneyStr(int iCash)
{
	int iMillions = iCash / 1000000;
	int iThousands = (iCash % 1000000) / 1000;
	int iRest = (iCash % 1000);
	wchar_t wszBuf[32];

	if(iMillions)
		swprintf(wszBuf, L"%d.%.3d.%.3d", iMillions, abs(iThousands), abs(iRest));
	else if(iThousands)
		swprintf(wszBuf, L"%d.%.3d", iThousands, abs(iRest));
	else
		swprintf(wszBuf, L"%d", iRest);

	return wszBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

string IniGetS(const string &scFile, const string &scApp, const string &scKey, const string &scDefault)
{
	char szRet[2048*2];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), scDefault.c_str(), szRet, sizeof(szRet), scFile.c_str());
	return szRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const string &scFile, const string &scApp, const string &scKey, int iDefault)
{
	return GetPrivateProfileInt(scApp.c_str(), scKey.c_str(), iDefault, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const string &scFile, const string &scApp, const string &scKey, float fDefault)
{
	char szRet[2048*2];
	char szDefault[16];
	sprintf(szDefault, "%f", fDefault);
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), szDefault, szRet, sizeof(szRet), scFile.c_str());
	return (float)atof(szRet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const string &scFile, const string &scApp, const string &scKey, bool bDefault)
{
	string val = ToLower(IniGetS(scFile, scApp, scKey, bDefault ? "true" : "false"));
	return val.compare("yes") == 0 || val.compare("true") == 0 ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const string &scFile, const string &scApp, const string &scKey, const string &scValue)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const string &scFile, const string &scApp, const string &scKey, const wstring &wscValue)
{
	string scValue = "";
	for(uint i = 0; (i < wscValue.length()); i++)
	{
		char cHiByte = wscValue[i] >> 8;
		char cLoByte = wscValue[i] & 0xFF;
		char szBuf[8];
		sprintf(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		scValue += szBuf;
	}
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring IniGetWS(const string &scFile, const string &scApp, const string &scKey, const wstring &wscDefault)
{
	char szRet[2048*2];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet, sizeof(szRet), scFile.c_str());
	string scValue = szRet;
	if(!scValue.length())
		return wscDefault;

	wstring wscValue = L"";
	long lHiByte;
	long lLoByte;
	while(sscanf(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}

	return wscValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelete(const string &scFile, const string &scApp, const string &scKey)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const string &scFile, const string &scApp)
{
	WritePrivateProfileString(scApp.c_str(), NULL, NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniGetSection(const string &scFile, const string &scApp, list<INISECTIONVALUE> &lstValues)
{
	lstValues.clear();
	char szBuf[0xFFFF];
	GetPrivateProfileSection(scApp.c_str(), szBuf, sizeof(szBuf), scFile.c_str());
	char *szNext = szBuf;
	while(strlen(szNext) > 0)
	{
		INISECTIONVALUE isv;
		char szKey[0xFFFF] = "";
		char szValue[0xFFFF] = "";
		sscanf(szNext, "%[^=]=%[^\n]", szKey, szValue);
		isv.scKey = szKey;
		isv.scValue = szValue;
		lstValues.push_back(isv);

		szNext += strlen(szNext) + 1;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring XMLText(const wstring &wscText)
{
	wstring wscRet;
	for(uint i = 0; (i < wscText.length()); i++)
	{
		if(wscText[i] == '<')
			wscRet.append(L"&#60;");
		else if(wscText[i] == '>')
			wscRet.append(L"&#62;");
		else if(wscText[i] == '&')
			wscRet.append(L"&#38;");
		else
			wscRet.append(1, wscText[i]);
	}

	return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteProcMem(void *pAddress, void *pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	WriteProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadProcMem(void *pAddress, void *pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION|PROCESS_VM_WRITE|PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	ReadProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring GetParam(const wstring &wscLine, wchar_t wcSplitChar, uint iPos)
{
	uint i = 0, j = 0;

	wstring wscResult = L"";
	for(i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++)
	{
		if(wscLine[j] == wcSplitChar)
		{
			while(((j + 1) < wscLine.length()) && (wscLine[j+1] == wcSplitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}

		if(i == iPos)
			wscResult += wscLine[j];
	}

	return wscResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring ReplaceStr(const wstring &wscSource, const wstring &wscSearchFor, const wstring &wscReplaceWith)
{
	uint lPos, sPos = 0;

	wstring wscResult = wscSource;
	while((lPos = (uint)wscResult.find(wscSearchFor, sPos)) != -1)
	{
		wscResult.replace(lPos, wscSearchFor.length(), wscReplaceWith);
		sPos = lPos + wscReplaceWith.length();
	}

	return wscResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

mstime timeInMS()
{
	mstime iCount;
	QueryPerformanceCounter((LARGE_INTEGER*)&iCount);
	mstime iFreq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&iFreq);
	return 1000 * iCount / iFreq;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SwapBytes(void *ptr, uint iLen)
{
	if(iLen % 4)
		return;

	for(uint i=0; i<iLen; i+=4)
	{
		char *ptr1 = (char*)ptr + i;
		unsigned long temp;
		memcpy(&temp, ptr1, 4);
		char *ptr2 = (char*)&temp;
		memcpy(ptr1, ptr2+3, 1);
		memcpy(ptr1+1, ptr2+2, 1);
		memcpy(ptr1+2, ptr2+1, 1);
		memcpy(ptr1+3, ptr2, 1);
	}
}

FARPROC PatchCallAddr(char *hMod, DWORD dwInstallAddress, char *dwHookFunction)
{
	DWORD dwRelAddr;
	ReadProcMem(hMod + dwInstallAddress + 1, &dwRelAddr, 4);

	DWORD dwOffset = (DWORD)dwHookFunction - (DWORD)(hMod + dwInstallAddress + 5);
	WriteProcMem(hMod + dwInstallAddress + 1, &dwOffset, 4);

	return (FARPROC)(hMod + dwRelAddr + dwInstallAddress + 5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef EXTENDED_EXCEPTION_LOGGING
#include <psapi.h>

HMODULE GetModuleAddr(uint iAddr)
{
	HMODULE hModArr[1024];
	DWORD iArrSizeNeeded;
	HANDLE hProcess = GetCurrentProcess();
	if(EnumProcessModules(hProcess, hModArr, sizeof(hModArr), &iArrSizeNeeded))
	{
		if(iArrSizeNeeded > sizeof(hModArr))
			iArrSizeNeeded = sizeof(hModArr);
		iArrSizeNeeded /= sizeof(HMODULE);
		for(uint i = 0; i < iArrSizeNeeded; i++)
		{
			MODULEINFO mi;
			if(GetModuleInformation(hProcess, hModArr[i], &mi, sizeof(mi)))
			{
				if(((uint)mi.lpBaseOfDll) < iAddr && (uint)mi.lpBaseOfDll + (uint)mi.SizeOfImage > iAddr)
				{
					return hModArr[i];
				}
			}
		}
	}
	return 0;
}

#include <string.h>
#include "dbghelp.h"

// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

void WriteMiniDump(struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	AddLog("Attempting to write minidump...");
	AddDebugLog("Attempting to write minidump...");
	HMODULE hDll = ::LoadLibrary( "DBGHELP.DLL" );
	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			// put the dump file in the flhook logs/debug directory
			char szDumpPath[_MAX_PATH];
			char szDumpPathFirst[_MAX_PATH];

			time_t tNow = time(0);
			struct tm *t = localtime(&tNow);
			strftime(szDumpPathFirst, sizeof(szDumpPathFirst), "./flhook_logs/debug/flserver_%d.%m.%Y_%H.%M.%S", t);

			int n = 1;
			do
			{
				sprintf(szDumpPath, "%s-%d.dmp", szDumpPathFirst, n);
				n++;
			} while (FileExists(szDumpPath));

			// create the file
			HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL, NULL );

			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				if (pExceptionInfo)
				{
					ExInfo.ThreadId = ::GetCurrentThreadId();
					ExInfo.ExceptionPointers = pExceptionInfo;
					ExInfo.ClientPointers = NULL;
					pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
				}
				else
				{
					pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, NULL, NULL, NULL );
				}
				::CloseHandle(hFile);

				AddLog("Minidump '%s' written.", szDumpPath);
			}
		}
	}
}

void AddExceptionInfoLog(LPEXCEPTION_POINTERS pep)
{
	try
	{
		_EXCEPTION_RECORD *exception = NULL;
		_CONTEXT *reg = NULL;
		if (pep)
		{
			exception = pep->ExceptionRecord;
			reg = pep->ContextRecord;
		}

		if (exception)
		{
			DWORD iCode = exception->ExceptionCode;
			uint iAddr = (uint)exception->ExceptionAddress;
			uint iOffset = 0;
			HMODULE hModExc = GetModuleAddr(iAddr);
			char szModName[MAX_PATH] = "";
			if (hModExc)
			{
				iOffset = iAddr - (uint)hModExc;
				GetModuleFileName(hModExc, szModName, sizeof(szModName));
			}
			AddLog("Code=%x Offset=%x Module=\"%s\"", iCode, iOffset, szModName);
		}
		else
		{
			AddLog("No exception information available");
		}

		if (reg)
		{
			AddLog("eax=%x ebx=%x ecx=%x edx=%x edi=%x esi=%x ebp=%x eip=%x esp=%x",
				reg->Eax, reg->Ebx, reg->Ecx, reg->Edx, reg->Edi, reg->Esi, reg->Ebp, reg->Eip, reg->Esp);
		}
		else
		{
			AddLog("No register information available");
		}

		WriteMiniDump(pep);

	} catch(...) { AddLog("Exception in AddExceptionInfoLog!"); }
}

#endif
