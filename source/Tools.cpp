#include <time.h>
#include "global.h"
#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring stows(const std::string &scText)
{
	int iSize = MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, 0, 0);
	wchar_t *wszText = new wchar_t[iSize];
	MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, wszText, iSize);
	std::wstring wscRet = wszText;
	delete[] wszText;
	return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string wstos(const std::wstring &wscText)
{
	uint iLen = (uint)wscText.length() + 1;
	char *szBuf = new char[iLen];
	WideCharToMultiByte(CP_ACP, 0, wscText.c_str(), -1, szBuf, iLen, 0, 0);
	std::string scRet = szBuf;
	delete[] szBuf;
	return scRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string itos(int i)
{
	char szBuf[16];
	sprintf(szBuf, "%d", i);
	return szBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring ToLower(const std::wstring &wscStr)
{
	std::wstring wscResult;
	for(uint i = 0; (i < wscStr.length()); i++)
		wscResult += towlower(wscStr[i]);

	return wscResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ToLower(const std::string &scStr)
{
	std::string scResult;
	for(uint i = 0; (i < scStr.length()); i++)
		scResult += tolower(scStr[i]);

	return scResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ToInt(const std::wstring &wscStr)
{
	return wcstoul(wscStr.c_str(), 0, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint ToUInt(const std::wstring &wscStr)
{
	return wcstoul(wscStr.c_str(), 0, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ToFloat(const std::wstring &wscStr)
{
	return (float)atof(wstos(wscStr).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string itohexs(uint value)
{
	char buf[16];
	sprintf_s(buf, "%08X", value);
	return buf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 999.999.999

std::wstring ToMoneyStr(int iCash)
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

std::string IniGetS(const std::string &scFile, const std::string &scApp, const std::string &scKey, const std::string &scDefault)
{
	char szRet[2048*2];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), scDefault.c_str(), szRet, sizeof(szRet), scFile.c_str());
	return szRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const std::string &scFile, const std::string &scApp, const std::string &scKey, int iDefault)
{
	return GetPrivateProfileInt(scApp.c_str(), scKey.c_str(), iDefault, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const std::string &scFile, const std::string &scApp, const std::string &scKey, float fDefault)
{
	char szRet[2048*2];
	char szDefault[16];
	sprintf(szDefault, "%f", fDefault);
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), szDefault, szRet, sizeof(szRet), scFile.c_str());
	return (float)atof(szRet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const std::string &scFile, const std::string &scApp, const std::string &scKey, bool bDefault)
{
	std::string val = ToLower(IniGetS(scFile, scApp, scKey, bDefault ? "true" : "false"));
	return val.compare("yes") == 0 || val.compare("true") == 0 ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const std::string &scFile, const std::string &scApp, const std::string &scKey, const std::string &scValue)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const std::string &scFile, const std::string &scApp, const std::string &scKey, const std::wstring &wscValue)
{
	std::string scValue = "";
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

std::wstring IniGetWS(const std::string &scFile, const std::string &scApp, const std::string &scKey, const std::wstring &wscDefault)
{
	char szRet[2048*2];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet, sizeof(szRet), scFile.c_str());
	std::string scValue = szRet;
	if(!scValue.length())
		return wscDefault;

	std::wstring wscValue = L"";
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

void IniDelete(const std::string &scFile, const std::string &scApp, const std::string &scKey)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const std::string &scFile, const std::string &scApp)
{
	WritePrivateProfileString(scApp.c_str(), NULL, NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniGetSection(const std::string &scFile, const std::string &scApp, std::list<INISECTIONVALUE> &lstValues)
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

/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::string GetUserFilePath(const std::wstring &wscCharname, const std::string &scExtension)
{
	// init variables
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

	std::wstring wscDir;
	std::wstring wscFile;
	if (HkGetAccountDirName(wscCharname, wscDir)!=HKE_OK)
		return "";
	if (HkGetCharFileName(wscCharname, wscFile)!=HKE_OK)
		return "";

	return scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + scExtension;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring XMLText(const std::wstring &wscText)
{
	std::wstring wscRet;
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

std::wstring GetParam(const std::wstring &wscLine, wchar_t wcSplitChar, uint iPos)
{
	uint i = 0, j = 0;

	std::wstring wscResult = L"";
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


std::string GetParam(std::string scLine, char cSplitChar, uint iPos)
{
	uint i = 0, j = 0;

	std::string scResult = "";
	for(i = 0, j = 0; (i <= iPos) && (j < scLine.length()); j++)
	{
		if(scLine[j] == cSplitChar)
		{
			while(((j + 1) < scLine.length()) && (scLine[j+1] == cSplitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}

		if(i == iPos)
			scResult += scLine[j];
	}

	return scResult;
}

/**
This function is similar to GetParam but instead returns everything
from the parameter specified by iPos to the end of wscLine.

wscLine - the std::string to get parameters from
wcSplitChar - the seperator character
iPos - the parameter number to start from.
*/
std::wstring GetParamToEnd(const std::wstring &wscLine, wchar_t wcSplitChar, uint iPos)
{
	for(uint i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++)
	{
		if(wscLine[j] == wcSplitChar)
		{
			while(((j + 1) < wscLine.length()) && (wscLine[j+1] == wcSplitChar))
				j++; // skip "whitechar"
			i++;
			continue;
		}
		if	(i == iPos)
		{
			return wscLine.substr(j);
		}
	}
	return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring ReplaceStr(const std::wstring &wscSource, const std::wstring &wscSearchFor, const std::wstring &wscReplaceWith)
{
	uint lPos, sPos = 0;

	std::wstring wscResult = wscSource;
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


/// Use this function to get the ticks since system startup. The FLHook timeInMS()
/// function seems to report inaccurate time when the FLServer.exe process freezes
/// (which happens due to other bugs).
mstime GetTimeInMS()
{
	static mstime msBaseTime = 0;
	static mstime msLastTickCount = 0;

	mstime msCurTime = GetTickCount();
	// GetTickCount is 32 bits and so wraps around ever 49.5 days
	// If a wrap around has occurred then
	if (msCurTime < msLastTickCount)
		msBaseTime += (2^32);
	msLastTickCount = msCurTime;
	return msBaseTime + msLastTickCount;
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

void WriteMiniDump(SEHException* ex)
{
	AddBothLog("Attempting to write minidump...");
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

				if (ex)
				{
					ExInfo.ThreadId = ::GetCurrentThreadId();
					EXCEPTION_POINTERS ep;
					ep.ContextRecord = &ex->context;
					ep.ExceptionRecord = &ex->record;
					ExInfo.ExceptionPointers = &ep;
					ExInfo.ClientPointers = NULL;
					pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
				}
				else
				{
					pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, NULL, NULL, NULL );
				}
				::CloseHandle(hFile);

				AddBothLog("Minidump '%s' written.", szDumpPath);
			}
		}
	}
}

#include <string.h>
#include "dbghelp.h"
#include "exceptioninfo.h"
#include <Psapi.h>
#include <io.h>
#include <shlwapi.h>

void AddExceptionInfoLog()
{
	try
	{
		EXCEPTION_RECORD const *exception = GetCurrentExceptionRecord();
		_CONTEXT const *reg = GetCurrentExceptionContext();

		if (exception)
		{
			DWORD iCode = exception->ExceptionCode;
			uint iAddr = (uint)exception->ExceptionAddress;
			uint iOffset = 0;
			HMODULE hModExc = GetModuleAddr(iAddr);
			char szModName[MAX_PATH] = "";
			if(hModExc)
			{
				iOffset = iAddr - (uint)hModExc;
				GetModuleFileName(hModExc, szModName, sizeof(szModName));
			}
			AddBothLog("Code=%x Offset=%x Module=\"%s\"", iCode, iOffset, szModName);
			if (iCode == 0xE06D7363 && exception->NumberParameters == 3) //C++ exception
			{
				_s__ThrowInfo *info = (_s__ThrowInfo*)exception->ExceptionInformation[2];
				const _s__CatchableType *const (*typeArr)[] = &info->pCatchableTypeArray->arrayOfCatchableTypes;
				std::exception *obj = (std::exception*)exception->ExceptionInformation[1];
				const char *szName = info && info->pCatchableTypeArray && info->pCatchableTypeArray->nCatchableTypes ? (*typeArr)[0]->pType->name : "";
				int i = 0;
				for(; i < info->pCatchableTypeArray->nCatchableTypes; i++)
				{
					if(!strcmp(".?AVexception@@", (*typeArr)[i]->pType->name))
						break;
				}
				const char *szMessage = i != info->pCatchableTypeArray->nCatchableTypes ? obj->what() : "";
				//C++ exceptions are triggered by RaiseException in kernel32, so use ebp to get the return address
				iOffset = 0;
				szModName[0] = 0;
				if(reg)
				{
					iAddr = *((*((uint**)reg->Ebp)) + 1);
					hModExc = GetModuleAddr(iAddr);
					if(hModExc)
					{
						iOffset = iAddr - (uint)hModExc;
						GetModuleFileName(hModExc, szModName, sizeof(szModName));
					}
				}
				AddBothLog("Name=\"%s\" Message=\"%s\" Offset=%x Module=\"%s\"", szName, szMessage, iOffset, strrchr(szModName, '\\')+1);
			}

			void* callers[62];
			int count = CaptureStackBackTrace(0, 62, callers, NULL);
			for (int i = 0; i < count; i++)
				AddBothLog("%08x called from %08X", i, callers[i]);
		}
		else
			AddBothLog("No exception information available");
		if(reg)
			AddBothLog("eax=%x ebx=%x ecx=%x edx=%x edi=%x esi=%x ebp=%x eip=%x esp=%x",
				reg->Eax, reg->Ebx, reg->Ecx, reg->Edx, reg->Edi, reg->Esi, reg->Ebp, reg->Eip, reg->Esp);
		else
			AddBothLog("No register information available");
	} catch(...) { AddBothLog("Exception in AddExceptionInfoLog!"); }
}

#endif


/**
Remove leading and trailing spaces from the std::string ~FlakCommon by Motah.
*/
std::wstring Trim(std::wstring wscIn)
{
	while(wscIn.length() && (wscIn[0]==L' ' || wscIn[0]==L'	' || wscIn[0]==L'\n' || wscIn[0]==L'\r') )
	{
		wscIn = wscIn.substr(1);
	}
	while(wscIn.length() && (wscIn[wscIn.length()-1]==L' ' || wscIn[wscIn.length()-1]==L'	' || wscIn[wscIn.length()-1]==L'\n' || wscIn[wscIn.length()-1]==L'\r') )
	{
		wscIn = wscIn.substr(0, wscIn.length()-1);
	}
	return wscIn;
}

/**
Remove leading and trailing spaces from the std::string  ~FlakCommon by Motah.
*/
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

std::wstring GetTimeString(bool bLocalTime)
{
	SYSTEMTIME st;
	if (bLocalTime)
		GetLocalTime(&st);
	else
		GetSystemTime(&st);

	wchar_t wszBuf[100];
	_snwprintf_s(wszBuf, sizeof(wszBuf), L"%04d-%02d-%02d %02d:%02d:%02d SMT ", st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond);
	return wszBuf;
}


void ini_write_wstring(FILE *file, const std::string &parmname, const std::wstring &in)
{
	fprintf(file, "%s=", parmname.c_str());
	for (int i = 0; i < (int)in.size(); i++)
	{
		UINT v1 = in[i] >> 8;
		UINT v2 = in[i] & 0xFF;
		fprintf(file, "%02x%02x", v1, v2);
	}
	fprintf(file, "\n");
}


void ini_get_wstring(INI_Reader &ini, std::wstring &wscValue)
{
	std::string scValue = ini.get_value_string();
	wscValue = L"";
	long lHiByte;
	long lLoByte;
	while(sscanf(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}
}