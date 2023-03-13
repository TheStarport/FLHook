#include "Global.hpp"
#include "ExceptionInfo.h"

#ifndef DISABLE_EXTENDED_EXCEPTION_LOGGING
#include <psapi.h>

HMODULE GetModuleAddr(uint iAddr)
{
	HMODULE hModArr[1024];
	DWORD iArrSizeNeeded;
	const HANDLE hProcess = GetCurrentProcess();
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
				if (((uint)mi.lpBaseOfDll) < iAddr && (uint)mi.lpBaseOfDll + static_cast<uint>(mi.SizeOfImage) > iAddr)
				{
					return hModArr[i];
				}
			}
		}
	}
	return nullptr;
}

#include "dbghelp.h"

#include <string.h>

// based on dbghelp.h
using MINIdUMPWRITEDUMP = BOOL(WINAPI*)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

void WriteMiniDump(SEHException* ex)
{
	Logger::i()->Log(LogLevel::Err, "Attempting to write minidump...");
	const HMODULE hDll = ::LoadLibrary("DBGHELP.DLL");
	if (hDll)
	{
		const auto pDump = (MINIdUMPWRITEDUMP)GetProcAddress(hDll, "MiniDumpWriteDump");
		if (pDump)
		{
			// put the dump file in the flhook logs/debug directory
			char DumpPath[_MAX_PATH];
			char DumpPathFirst[_MAX_PATH];

			const time_t tNow = time(nullptr);
			tm t;
			localtime_s(&t, &tNow);
			strftime(DumpPathFirst, sizeof(DumpPathFirst), "./logs/debug/flserver_%d.%m.%Y_%H.%M.%S", &t);

			int n = 1;
			do
			{
				sprintf_s(DumpPath, "%s-%d.dmp", DumpPathFirst, n);
				n++;
			} while (std::filesystem::exists(DumpPath));

			// create the file
			const HANDLE hFile = ::CreateFile(DumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

			if (hFile != INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				if (ex)
				{
					ExInfo.ThreadId = GetCurrentThreadId();
					EXCEPTION_POINTERS ep;
					ep.ContextRecord = &ex->context;
					ep.ExceptionRecord = &ex->record;
					ExInfo.ExceptionPointers = &ep;
					ExInfo.ClientPointers = NULL;
					pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr);
				}
				else
				{
					pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, nullptr, nullptr, nullptr);
				}
				CloseHandle(hFile);

				Logger::i()->Log(LogLevel::Err, std::format("Minidump '{}' written.", DumpPath));
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
		const EXCEPTION_RECORD* exception = &ex->record;
		const _CONTEXT* reg = &ex->context;

		if (exception)
		{
			DWORD iCode = exception->ExceptionCode;
			uint iAddr = (uint)exception->ExceptionAddress;
			uint iOffset = 0;
			HMODULE hModExc = GetModuleAddr(iAddr);
			char ModName[MAX_PATH] = "";
			if (hModExc)
			{
				iOffset = iAddr - (uint)hModExc;
				GetModuleFileName(hModExc, ModName, sizeof(ModName));
			}
			Logger::i()->Log(LogLevel::Debug, std::format("Code={:X} Offset={:X} Module=\"{}\"", iCode, iOffset, ModName));
			if (iCode == 0xE06D7363 && exception->NumberParameters == 3) // C++ exception
			{
				const auto* info = reinterpret_cast<const msvc__ThrowInfo*>(exception->ExceptionInformation[2]);
				const auto* typeArr = &info->pCatchableTypeArray->arrayOfCatchableTypes;
				const auto* obj = reinterpret_cast<const std::exception*>(exception->ExceptionInformation[1]);
				const char* Name = info && info->pCatchableTypeArray && info->pCatchableTypeArray->nCatchableTypes ? (*typeArr)[0]->pType->name : "";
				int i = 0;
				for (; i < info->pCatchableTypeArray->nCatchableTypes; i++)
				{
					if (!strcmp(".?AVexception@@", (*typeArr)[i]->pType->name))
						break;
				}
				const char* Message = i != info->pCatchableTypeArray->nCatchableTypes ? obj->what() : "";
				// C++ exceptions are triggered by RaiseException in kernel32,
				// so use ebp to get the return address
				iOffset = 0;
				ModName[0] = 0;
				if (reg)
				{
					iAddr = *((*((uint**)reg->Ebp)) + 1);
					hModExc = GetModuleAddr(iAddr);
					if (hModExc)
					{
						iOffset = iAddr - (uint)hModExc;
						GetModuleFileName(hModExc, ModName, sizeof(ModName));
					}
				}
				Logger::i()->Log(LogLevel::Debug, std::format("Name=\"{}\" Message=\"{}\" Offset=0x{:08X} Module=\"{}\"", Name, Message, iOffset, strrchr(ModName, '\\') + 1));
			}

			void* callers[62];
			const int count = CaptureStackBackTrace(0, 62, callers, nullptr);
			for (int i = 0; i < count; i++)
				Logger::i()->Log(LogLevel::Debug, std::vformat("{} called from {:#X}", std::make_format_args(i, callers[i])));
		}
		else
			Logger::i()->Log(LogLevel::Debug, "No exception information available");
		if (reg)
		{
			Logger::i()->Log(LogLevel::Debug,
				std::format("eax={:X} ebx={:X} ecx={:X} edx={:X} edi={:X} esi={:X} ebp={:X} eip={:X} esp={:X}",
					reg->Eax,
					reg->Ebx,
					reg->Ecx,
					reg->Edx,
					reg->Edi,
					reg->Esi,
					reg->Ebp,
					reg->Eip,
					reg->Esp));
		}
		else
		{
			Logger::i()->Log(LogLevel::Trace, "No register information available");
		}
	}
	catch (...)
	{
		Logger::i()->Log(LogLevel::Trace, "Exception in AddExceptionInfoLog!");
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
