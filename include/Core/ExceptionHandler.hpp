#pragma once

class IpResolver;
class ExceptionHandler final
{
        friend FLHook;
        friend IServerImplHook;
        friend PluginManager;
        friend IEngineHook;
        friend IClientImpl;
        friend IpResolver;

        inline static std::array<byte, 5> oldSetUnhandledFilter;
        inline static std::mutex exceptionMutex;
        static void GlobalHandler(EXCEPTION_POINTERS* excpInfo);
        static LONG WINAPI UnhandledException(EXCEPTION_POINTERS* excpInfo = NULL);
        static void InvalidParameter(const wchar_t* expr, const wchar_t* func, const wchar_t* file, unsigned int line, uintptr_t reserved);
        static void PureVirtualCall();
        static void SigAbortHandler(int sig);
        static void SetupExceptionHandling();
        static HMODULE GetModuleFromAddress(uint address);
        static void LogException(const SehException& ex);
};
