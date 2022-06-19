#include "Global.hpp"
#include "ExceptionInfo.h"

#ifndef DISABLE_EXTENDED_EXCEPTION_LOGGING
	#include <psapi.h>

HMODULE GetModuleAddr(uint iAddr)
{
	HMODULE hModArr[1024];
	DWORD iArrSizeNeeded;
	HANDLE hProcess = GetCurrentProcess();
	if (EnumProcessModules(hProcess, hModArr, sizeof(hModArr), &iArrSizeNeeded))
	{
		if (iArrSizeNeeded > sizeof(hModArr))
			iArrSizeNeeded = sizeof(hModArr);
		iArrSizeNeeded /= sizeof(HMODULE);
		for (uint i = 0; i < iArrSizeNeeded; i++)
		{
			MODULEINFO mi;
			if (GetModuleInformation(hProcess, hModArr[i], &mi, sizeof(mi)))
			{
				if (((uint)mi.lpBaseOfDll) < iAddr && (uint)mi.lpBaseOfDll + (uint)mi.SizeOfImage > iAddr)
				{
					return hModArr[i];
				}
			}
		}
	}
	return 0;
}

	#include "dbghelp.h"

	#include <string.h>

// based on dbghelp.h
typedef BOOL(WINAPI* MINIDUMPWRITEDUMP)(
    HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

void WriteMiniDump(SEHException* ex)
{
	AddBothLog(false, L"Attempting to write minidump...");
	HMODULE hDll = ::LoadLibrary("DBGHELP.DLL");
	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			// put the dump file in the flhook logs/debug directory
			char szDumpPath[_MAX_PATH];
			char szDumpPathFirst[_MAX_PATH];

			time_t tNow = time(0);
			tm t;
			localtime_s(&t, &tNow);
			strftime(szDumpPathFirst, sizeof(szDumpPathFirst), "./flhook_logs/debug/flserver_%d.%m.%Y_%H.%M.%S", &t);

			int n = 1;
			do
			{
				sprintf_s(szDumpPath, "%s-%d.dmp", szDumpPathFirst, n);
				n++;
			} while (FileExists(szDumpPath));

			// create the file
			HANDLE hFile = ::CreateFile(
			    szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

			if (hFile != INVALID_HANDLE_VALUE)
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
					pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL);
				}
				else
				{
					pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, NULL, NULL, NULL);
				}
				::CloseHandle(hFile);

				AddBothLog(false, L"Minidump '%s' written.", szDumpPath);
			}
		}
	}
}

	#include "ExceptionInfo.h"
	#include "dbghelp.h"

	#include <Psapi.h>
	#include <io.h>
	#include <shlwapi.h>
	#include <string.h>

void AddExceptionInfoLog(SEHException* ex)
{
	if (!ex)
	{
		AddExceptionInfoLog();
		return;
	}
	try
	{
		EXCEPTION_RECORD const* exception = &ex->record;
		_CONTEXT const* reg = &ex->context;

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
			AddBothLog(false, L"Code=%x Offset=%x Module=\"%s\"", iCode, iOffset, szModName);
			if (iCode == 0xE06D7363 && exception->NumberParameters == 3) // C++ exception
			{
				const auto* info = reinterpret_cast<const msvc__ThrowInfo*>(exception->ExceptionInformation[2]);
				const auto* typeArr = &info->pCatchableTypeArray->arrayOfCatchableTypes;
				const auto* obj = reinterpret_cast<const std::exception*>(exception->ExceptionInformation[1]);
				const char* szName = info && info->pCatchableTypeArray && info->pCatchableTypeArray->nCatchableTypes
				    ? (*typeArr)[0]->pType->name
				    : "";
				int i = 0;
				for (; i < info->pCatchableTypeArray->nCatchableTypes; i++)
				{
					if (!strcmp(".?AVexception@@", (*typeArr)[i]->pType->name))
						break;
				}
				const char* szMessage = i != info->pCatchableTypeArray->nCatchableTypes ? obj->what() : "";
				// C++ exceptions are triggered by RaiseException in kernel32,
				// so use ebp to get the return address
				iOffset = 0;
				szModName[0] = 0;
				if (reg)
				{
					iAddr = *((*((uint**)reg->Ebp)) + 1);
					hModExc = GetModuleAddr(iAddr);
					if (hModExc)
					{
						iOffset = iAddr - (uint)hModExc;
						GetModuleFileName(hModExc, szModName, sizeof(szModName));
					}
				}
				AddBothLog(
				    false, L"Name=\"%s\" Message=\"%s\" Offset=%x Module=\"%s\"", szName, szMessage, iOffset,
				    strrchr(szModName, '\\') + 1);
			}

			void* callers[62];
			int count = CaptureStackBackTrace(0, 62, callers, NULL);
			for (int i = 0; i < count; i++)
				AddBothLog(false, L"%08x called from %08X", i, callers[i]);
		}
		else
			AddBothLog(true, L"No exception information available");
		if (reg)
		{
			AddBothLog(
			    false,
			    L"eax=%x ebx=%x ecx=%x edx=%x edi=%x esi=%x ebp=%x "
			    "eip=%x esp=%x",
			    reg->Eax, reg->Ebx, reg->Ecx, reg->Edx, reg->Edi, reg->Esi, reg->Ebp, reg->Eip, reg->Esp);
		}
		else
		{
			AddBothLog(true, L"No register information available");
		}
	}
	catch (...)
	{
		AddBothLog(true, L"Exception in AddExceptionInfoLog!");
	}
}

void AddExceptionInfoLog()
{
	SEHException ex;
	ex.context = *GetCurrentExceptionContext();
	ex.record = *GetCurrentExceptionRecord();
	ex.code = ex.record.ExceptionCode;

	AddExceptionInfoLog(&ex);
}

#endif
