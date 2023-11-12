// ReSharper disable CppClangTidyCertErr33C
#include "PCH.hpp"

#include "Core/ExceptionHandler.hpp"

#include "dbghelp.h"
#include <csignal>

void ExceptionHandler::GlobalHandler(EXCEPTION_POINTERS* ex)
{
    if (!std::filesystem::exists("dumps"))
    {
        std::filesystem::create_directory("dumps");
    }

    const HMODULE dll = LoadLibraryA("DBGHELP.DLL");
    if (!dll)
    {
        return;
    }

    using WriteMiniDump = BOOL(WINAPI*)(HANDLE hProcess,
                                        DWORD dwPid,
                                        HANDLE hFile,
                                        MINIDUMP_TYPE dumpType,
                                        PMINIDUMP_EXCEPTION_INFORMATION exceptionParam,
                                        PMINIDUMP_USER_STREAM_INFORMATION userStreamParam,
                                        PMINIDUMP_CALLBACK_INFORMATION callbackParam);

    const auto dump = reinterpret_cast<WriteMiniDump>(GetProcAddress(dll, "MiniDumpWriteDump")); // NOLINT(clang-diagnostic-cast-function-type-strict)
    if (!dump)
    {
        return;
    }

    const auto now = std::chrono::system_clock::now();

    // create the file

    if (const auto file = CreateFileA(
            std::format("dumps/{:%d-%m-%Y %H.%M.%OS}", now).c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        file != INVALID_HANDLE_VALUE)
    {

        // TODO: allow mini dump levels to be configurable
        _MINIDUMP_EXCEPTION_INFORMATION exInfo;
        if (ex)
        {
            exInfo.ThreadId = GetCurrentThreadId();
            EXCEPTION_POINTERS ep;
            ep.ContextRecord = ex->ContextRecord;
            ep.ExceptionRecord = ex->ExceptionRecord;
            exInfo.ExceptionPointers = &ep;
            exInfo.ClientPointers = NULL;

            dump(GetCurrentProcess(),
                 GetCurrentProcessId(),
                 file,
                 static_cast<MINIDUMP_TYPE>(MiniDumpWithFullMemory | MiniDumpWithIndirectlyReferencedMemory),
                 &exInfo,
                 nullptr,
                 nullptr);
        }
        else
        {
            dump(GetCurrentProcess(),
                 GetCurrentProcessId(),
                 file,
                 static_cast<MINIDUMP_TYPE>(MiniDumpWithFullMemory | MiniDumpWithIndirectlyReferencedMemory),
                 nullptr,
                 nullptr,
                 nullptr);
        }
        CloseHandle(file);
    }
}

LONG WINAPI ExceptionHandler::UnhandledException(EXCEPTION_POINTERS* excpInfo)
{
    std::scoped_lock lock(exceptionMutex);
    if (excpInfo != nullptr)
    {
        __try // Generate exception to get proper context in dump
        {
            RaiseException(EXCEPTION_BREAKPOINT, 0, 0, nullptr);
        }
        __except (GlobalHandler(GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
        {
            // Handled
        }
    }
    else
    {
        GlobalHandler(excpInfo);
    }

    return 0;
}

void ExceptionHandler::InvalidParameter(const wchar_t* expr, const wchar_t* func, const wchar_t* file, unsigned int line, uintptr_t reserved)
{
    UnhandledException();
}

void ExceptionHandler::PureVirtualCall() { UnhandledException(); }

void ExceptionHandler::SigAbortHandler(int sig)
{
    // this is required, otherwise if there is another thread
    // simultaneously tries to abort process will be terminated
    signal(SIGABRT, SigAbortHandler);
    UnhandledException();
}

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterOld([[maybe_unused]] LPTOP_LEVEL_EXCEPTION_FILTER) { return nullptr; }

void ExceptionHandler::SetupExceptionHandling()
{
    // Hook the kernel SetUnhandledExceptionFilter function to prevent
    // newer versions of the crt disabling our filter function if a buffer
    // overrun is detected.
    if (auto kernel = LoadLibrary(L"kernel32.dll"))
    {
        if (void* entry = GetProcAddress(kernel, "SetUnhandledExceptionFilter"))
        {
            const DWORD offset = static_cast<char*>(entry) - reinterpret_cast<char*>(kernel);

            MemUtils::ReadProcMem(reinterpret_cast<DWORD>(kernel) + offset, oldSetUnhandledFilter.data(), 5);

            BYTE patch[] = { 0xE9 };
            MemUtils::WriteProcMem(reinterpret_cast<DWORD>(kernel) + offset, patch, 1);
            MemUtils::PatchCallAddr(reinterpret_cast<DWORD>(kernel), offset, SetUnhandledExceptionFilterOld);
        }
    }

    SetUnhandledExceptionFilter(UnhandledException);
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
    _set_invalid_parameter_handler(InvalidParameter);
    _set_purecall_handler(PureVirtualCall);
    signal(SIGABRT, SigAbortHandler);
    _set_abort_behavior(0, 0);
}
