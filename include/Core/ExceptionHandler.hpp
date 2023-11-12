#pragma once

class ExceptionHandler final
{
        friend FLHook;
        inline static std::array<byte, 5> oldSetUnhandledFilter;
        inline static std::mutex exceptionMutex;
        static void GlobalHandler(EXCEPTION_POINTERS* excpInfo);
        static LONG WINAPI UnhandledException(EXCEPTION_POINTERS* excpInfo = NULL);
        static void InvalidParameter(const wchar_t* expr, const wchar_t* func, const wchar_t* file, unsigned int line, uintptr_t reserved);
        static void PureVirtualCall();
        static void SigAbortHandler(int sig);
        static void SetupExceptionHandling();
};
