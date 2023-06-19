#pragma once

#ifndef DLL
    #ifndef FLHOOK
        #define DLL __declspec(dllimport)
    #else
        #define DLL __declspec(dllexport)
    #endif
#endif

#define SRV_ADDR(a)      ((char*)server + (a))
#define DALIB_ADDR(a)    ((char*)hModDaLib + (a))
#define FLSERVER_ADDR(a) ((char*)hProcFL + (a))
#define CONTENT_ADDR(a)  ((char*)content + (a))

#pragma warning(disable : 4091)
#include <DbgHelp.h>

struct SEHException
{
        SEHException(unsigned code, EXCEPTION_POINTERS* ep) : code(code), record(*ep->ExceptionRecord), context(*ep->ContextRecord) {}

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
#define CATCH_HOOK(e)                                                 \
    }                                                                 \
    catch ([[maybe_unused]] SEHException & exc) { e; }                \
    catch ([[maybe_unused]] const StopProcessingException& ex) { e; } \
    catch (const GameException& ex)                                   \
    {                                                                 \
        Logger::i()->Log(LogLevel::Info, ex.Msg());                   \
        e;                                                            \
    }                                                                 \
    catch ([[maybe_unused]] std::exception & exc) { e; }              \
    catch (...) { e; }

#define LOG_EXCEPTION \
    {}

#define DefaultDllMain(x, xx)                                                                                            \
    BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE dll, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved) \
    {                                                                                                                    \
        if (xx reason == DLL_PROCESS_ATTACH)                                                                             \
        {                                                                                                                \
            x;                                                                                                           \
        }                                                                                                                \
        return true;                                                                                                     \
    }
#define DefaultDllMainSettings(loadSettings) DefaultDllMain(loadSettings(), CoreGlobals::c()->flhookReady&&)

#define DeduceClassType(variableName, type, value)                         \
private:                                                                   \
    static auto DeduceType##variableName() { return value; }               \
    using type = std::invoke_result_t<decltype(DeduceType##variableName)>; \
    type variableName = DeduceType##variableName()

#define AddCommand(class, str, func, usage, description)                                                                       \
    {                                                                                                                          \
        std::wstring_view(str), ClassFunctionWrapper<decltype(&class ::func), &class ::func>::ProcessParam, usage, description \
    }

#define SetupUserCommandHandler(class, commandArray)                                                                                          \
    template <int N>                                                                                                                          \
    bool MatchCommand(class* processor, ClientId triggeringClient, const std::wstring_view cmd, const std::vector<std::wstring>& paramVector) \
    {                                                                                                                                         \
        if (const CommandInfo<class> command = std::get<N - 1>(class ::commandArray); command.cmd == cmd)                                     \
        {                                                                                                                                     \
            command.func(*processor, paramVector);                                                                                            \
            return true;                                                                                                                      \
        }                                                                                                                                     \
                                                                                                                                              \
        return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);                                                            \
    }                                                                                                                                         \
                                                                                                                                              \
    template <>                                                                                                                               \
    bool MatchCommand<0>(class * processor, ClientId triggeringClient, std::wstring_view cmd, const std::vector<std::wstring>& paramVector)   \
    {                                                                                                                                         \
        return false;                                                                                                                         \
    }                                                                                                                                         \
                                                                                                                                              \
    bool ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, const std::vector<std::wstring>& paramVector) override              \
    {                                                                                                                                         \
        client = triggeringClient;                                                                                                            \
        return MatchCommand<commandArray.size()>(this, client, cmd, paramVector);                                                             \
    }
