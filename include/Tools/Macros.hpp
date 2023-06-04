#pragma once

#ifndef DLL
#ifndef FLHOOK
		#define DLL __declspec(dllimport)
#else
#define DLL __declspec(dllexport)
#endif
#endif

#define SRV_ADDR(a) ((char*)server + (a))
#define DALIB_ADDR(a) ((char*)hModDaLib + (a))
#define FLSERVER_ADDR(a) ((char*)hProcFL + (a))
#define CONTENT_ADDR(a) ((char*)content + (a))

#pragma warning(disable : 4091)
#include <DbgHelp.h>

struct SEHException
{
	SEHException(unsigned code, EXCEPTION_POINTERS* ep)
		: code(code), record(*ep->ExceptionRecord), context(*ep->ContextRecord)
	{
	}

	SEHException() = default;

	unsigned code;
	EXCEPTION_RECORD record;
	CONTEXT context;

	static void Translator(unsigned code, EXCEPTION_POINTERS* ep) { throw SEHException(code, ep); }
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
		catch ([[maybe_unused]] SEHException & exc)                                                                                      \
		{                                                                                                                                \
			e;                                                                                                                           \
		}                                                                                                                                \
		catch ([[maybe_unused]] std::exception & exc)                                                                                    \
		{                                                                                                                                \
			e;                                                                                                                           \
		}                                                                                                                                \
		catch (...)                                                                                                                      \
		{                                                                                                                                \
			e;                                                                                                                           \
		}

#define LOG_EXCEPTION                                                                                  \
		{                                                                                                  \
		}

#define DefaultDllMain(x, xx)                                                                                               \
	BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE dll, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved)    \
	{                                                                                                                       \
		if (xx reason == DLL_PROCESS_ATTACH)                                                  \
		{                                                                                                                   \
			x;                                                                                                              \
		}                                                                                                                   \
		return true;                                                                                                        \
	}
#define DefaultDllMainSettings(loadSettings) DefaultDllMain(loadSettings(), CoreGlobals::c()->flhookReady &&)

#define DeduceClassType(variableName, type, value)   \
  private:                                           \
	static auto DeduceType##variableName()           \
	{                                                \
		return value;                                \
	}                                                \
	using type = std::invoke_result_t<decltype(DeduceType##variableName)>; \
	type variableName = DeduceType##variableName()