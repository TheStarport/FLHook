#pragma once

#ifndef DLL
	#ifndef FLHOOK
		#define DLL __declspec(dllimport)
	#else
		#define DLL __declspec(dllexport)
	#endif
#endif

#ifndef FLHOOK
	#define EXTERN extern
#else
	#define EXTERN
#endif

#define SRV_ADDR(a) ((char*)hModServer + (a))
#define DALIB_ADDR(a) ((char*)hModDaLib + (a))
#define FLSERVER_ADDR(a) ((char*)hProcFL + (a))
#define CONTENT_ADDR(a) ((char*)hModContent + (a))

#ifndef DISABLE_EXTENDED_EXCEPTION_LOGGING
	#pragma warning(disable : 4091)
	#include <DbgHelp.h>

struct SEHException
{
	SEHException(uint code, EXCEPTION_POINTERS* ep) : code(code), record(*ep->ExceptionRecord), context(*ep->ContextRecord) {}

	SEHException() = default;

	uint code;
	EXCEPTION_RECORD record;
	CONTEXT context;

	static void Translator(uint code, EXCEPTION_POINTERS* ep) { throw SEHException(code, ep); }
};

DLL void WriteMiniDump(SEHException* ex);
DLL void AddExceptionInfoLog();
DLL void AddExceptionInfoLog(SEHException* ex);
	#define TRY_HOOK \
		try          \
		{            \
			_set_se_translator(SEHException::Translator);
	#define CATCH_HOOK(e)                                                                                                                \
		}                                                                                                                                \
		catch (SEHException & ex)                                                                                                        \
		{                                                                                                                                \
			e;                                                                                                                           \
			AddLog(LogType::Normal, LogLevel::Err, std::format("ERROR: SEH Exception in {} on line {}; minidump may contain more information.", __FUNCTION__, __LINE__)); \
			AddExceptionInfoLog(&ex);                                                                                                    \
		}                                                                                                                                \
		catch (std::exception & ex)                                                                                                      \
		{                                                                                                                                \
			e;                                                                                                                           \
			AddLog(LogType::Normal, LogLevel::Err, std::format("ERROR: STL Exception in {} on line {}: {}.", __FUNCTION__, __LINE__, ex.what()));                  \
			AddExceptionInfoLog(0);                                                                                                      \
		}                                                                                                                                \
		catch (...)                                                                                                                      \
		{                                                                                                                                \
			e;                                                                                                                           \
			AddLog(LogType::Normal, LogLevel::Err, std::format("ERROR: Exception in {} on line {}.", __FUNCTION__, __LINE__));                                            \
			AddExceptionInfoLog(0);                                                                                                      \
		}

	#define LOG_EXCEPTION                                                                                  \
		{                                                                                                  \
			AddLog(LogType::Normal, LogLevel::Err, std::format("ERROR Exception in {}", __FUNCTION__)); \
			AddExceptionInfoLog();                                                                         \
		}
#else
	#define TRY_HOOK try
	#define CATCH_HOOK(e)                                                                            \
		catch (...)                                                                                  \
		{                                                                                            \
			e;                                                                                       \
			AddLog(LogType::Normal, LogLevel::Err, std::format("ERROR: Exception in {}", __FUNCTION__)); \
		}
#endif

#define GetPluginClientData(id, info) ((&ClientInfo[(id)])->mapPluginData[(info)].data())

#define DefaultDllMain(x)                                                                                                   \
	BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE dll, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved)    \
	{                                                                                                                       \
		if (CoreGlobals::c()->flhookReady && reason == DLL_PROCESS_ATTACH)                                                  \
		{                                                                                                                   \
			x;                                                                                                              \
		}                                                                                                                   \
		return true;                                                                                                        \
	}
#define DefaultDllMainSettings(loadSettings) DefaultDllMain(loadSettings())