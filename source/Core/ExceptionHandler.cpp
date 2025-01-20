// ReSharper disable CppClangTidyCertErr33C
#include "PCH.hpp"

#include "Core/ExceptionHandler.hpp"

#include "dbghelp.h"

#include <Psapi.h>
#include <csignal>

#pragma pack(push, ehdata, 4)

struct msvc__PMD // NOLINT(*-reserved-identifier)
{
        int mdisp;
        int pdisp;
        int vdisp;
};

typedef void (*msvc__PMFN)(void); // NOLINT(*-reserved-identifier)

#pragma warning(disable : 4200)
#pragma pack(push, _TypeDescriptor, 8)
struct msvc__TypeDescriptor // NOLINT(*-reserved-identifier)
{
        const void* pVFTable;
        void* spare;
        char name[];
};
#pragma pack(pop, _TypeDescriptor)
#pragma warning(default : 4200)

struct msvc__CatchableType // NOLINT(*-reserved-identifier)
{
        unsigned int properties;
        msvc__TypeDescriptor* pType;
        msvc__PMD thisDisplacement;
        int sizeOrOffset;
        msvc__PMFN copyFunction;
};

#pragma warning(disable : 4200)
struct msvc__CatchableTypeArray // NOLINT(*-reserved-identifier)
{
        int nCatchableTypes;
        msvc__CatchableType* arrayOfCatchableTypes[];
};
#pragma warning(default : 4200)

struct msvc__ThrowInfo // NOLINT(*-reserved-identifier)
{
        unsigned int attributes;
        msvc__PMFN pmfnUnwind;
        int(__cdecl* pForwardCompat)(...);
        msvc__CatchableTypeArray* pCatchableTypeArray;
};

#pragma pack(pop, ehdata)

void ExceptionHandler::GlobalHandler(EXCEPTION_POINTERS* ex)
{
    try
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

        if (const auto file = CreateFileA(std::format("dumps/{:%d-%m-%Y %H.%M.%OS}.dmp", now).c_str(),
                                          GENERIC_WRITE,
                                          FILE_SHARE_WRITE,
                                          nullptr,
                                          CREATE_ALWAYS,
                                          FILE_ATTRIBUTE_NORMAL,
                                          nullptr);
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
                     static_cast<MINIDUMP_TYPE>(MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory),
                     &exInfo,
                     nullptr,
                     nullptr);
            }
            else
            {
                dump(GetCurrentProcess(),
                     GetCurrentProcessId(),
                     file,
                     static_cast<MINIDUMP_TYPE>(MiniDumpScanMemory | MiniDumpWithIndirectlyReferencedMemory),
                     nullptr,
                     nullptr,
                     nullptr);
            }
            CloseHandle(file);
        }
    }
    catch (...)
    {
        MessageBoxA(nullptr,
                    "Unable to write dump file after an exception. Ensure the 'dumps' directory exists and "
                    "FLHook has permission to write there.",
                    "Couldn't write minidump",
                    MB_OK);
    }
}

LONG WINAPI ExceptionHandler::UnhandledException(EXCEPTION_POINTERS* excpInfo)
{
    exceptionMutex.lock();
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

    exceptionMutex.unlock();
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

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI SetUnhandledExceptionFilterOld([[maybe_unused]]
                                                                   LPTOP_LEVEL_EXCEPTION_FILTER)
{
    return nullptr;
}

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

HMODULE ExceptionHandler::GetModuleFromAddress(uint address)
{
    HMODULE modArr[1024];
    DWORD sizeNeeded;
    HANDLE process = GetCurrentProcess();
    if (EnumProcessModules(process, modArr, sizeof(modArr), &sizeNeeded))
    {
        if (sizeNeeded > sizeof(modArr))
        {
            sizeNeeded = sizeof(modArr);
        }
        sizeNeeded /= sizeof(HMODULE);
        for (uint i = 0; i < sizeNeeded; i++)
        {
            MODULEINFO mi;
            if (GetModuleInformation(process, modArr[i], &mi, sizeof(mi)))
            {
                if (reinterpret_cast<uint>(mi.lpBaseOfDll) < address && reinterpret_cast<uint>(mi.lpBaseOfDll) + static_cast<uint>(mi.SizeOfImage) > address)
                {
                    return modArr[i];
                }
            }
        }
    }
    return 0;
}

void ExceptionHandler::LogException(const SehException& ex)
{
    auto scopedLock = std::scoped_lock(exceptionMutex);
    try
    {
        EXCEPTION_RECORD const* exception = &ex.record;
        _CONTEXT const* reg = &ex.context;

        if (exception)
        {
            DWORD code = exception->ExceptionCode;
            uint address = reinterpret_cast<uint>(exception->ExceptionAddress);
            uint offset = 0;

            HMODULE module = NULL;
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)address, &module);
            WCHAR moduleName[MAX_PATH];
            GetModuleBaseNameW(GetCurrentProcess(), module, moduleName, MAX_PATH);

            ERROR(L"{0}{1}{2}", { L"code", std::to_wstring(code) }, { L"offset", std::to_wstring(offset) }, { L"moduleName", moduleName })

            if (code == 0xE06D7363 && exception->NumberParameters == 3) // C++ exception
            {
                const auto* info = reinterpret_cast<const msvc__ThrowInfo*>(exception->ExceptionInformation[2]);
                const auto* typeArr = &info->pCatchableTypeArray->arrayOfCatchableTypes;
                const auto* obj = reinterpret_cast<const std::exception*>(exception->ExceptionInformation[1]);
                const char* szName = info && info->pCatchableTypeArray && info->pCatchableTypeArray->nCatchableTypes ? (*typeArr)[0]->pType->name : "";
                int i = 0;
                for (; i < info->pCatchableTypeArray->nCatchableTypes; i++)
                {
                    if (!strcmp(".?AVexception@@", (*typeArr)[i]->pType->name))
                    {
                        break;
                    }
                }
                const char* szMessage = i != info->pCatchableTypeArray->nCatchableTypes ? obj->what() : "";
                // C++ exceptions are triggered by RaiseException in kernel32,
                // so use ebp to get the return address
                offset = 0;
                moduleName[0] = 0;
                if (reg)
                {
                    address = *((*((uint**)reg->Ebp)) + 1);
                    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)address, &module);
                    GetModuleBaseNameW(GetCurrentProcess(), module, moduleName, MAX_PATH);
                }
                ERROR(L"{0}{1}{2}{3}",
                      { L"name", StringUtils::stows(szName) },
                      { L"message", StringUtils::stows(szMessage) },
                      { L"offset", std::to_wstring(offset) },
                      { L"moduleName", wcsrchr(moduleName, L'\\') + 1 })
            }

            void* callers[62];
            int count = CaptureStackBackTrace(0, 62, callers, nullptr);
            for (int i = 0; i < count; i++)
            {
                // TODO: Log stack trace string
                // Logger::Log(LogFile::Default, LogLevel::Err, std::vformat(L"{} called from {:#X}", std::make_wformat_args(i, callers[i])));
            }
        }
        else
        {
            ERROR(L"No exception information available");
        }
        if (reg)
        {
            ERROR(L"{0}{1}{2}{3}{4}{5}{6}{7}{8}",
                  { L"eax", std::to_wstring(reg->Eax) },
                  { L"ebx", std::to_wstring(reg->Ebx) },
                  { L"ecx", std::to_wstring(reg->Ecx) },
                  { L"edx", std::to_wstring(reg->Edx) },
                  { L"edi", std::to_wstring(reg->Edi) },
                  { L"esi", std::to_wstring(reg->Esi) },
                  { L"ebp", std::to_wstring(reg->Ebp) },
                  { L"eip", std::to_wstring(reg->Eip) },
                  { L"esp", std::to_wstring(reg->Esp) })
        }
        else
        {
           ERROR(L"No register information available");
        }
    }
    catch (...)
    {
        ERROR(L"Exception in AddExceptionInfoLog!");
    }
}
