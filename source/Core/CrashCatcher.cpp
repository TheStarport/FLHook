#include "PCH.hpp"

#include "Core/CrashCatcher.hpp"

// Inline-ASM isn't read and it often thinks this file is full of 1-line statements
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppDFAConstantFunctionResult
// ReSharper disable CppRedundantQualifier
// ReSharper disable CppCStyleCast

template <class... Args>
void LogInternal(const wchar_t* templateStr, Args&&... args)
{
    ERROR("{{ex}}", { "ex", std::vformat(templateStr, std::make_wformat_args(std::forward<Args&>(args)...)) });
}

template <class... Args>
void LogInternal(const char* templateStr, Args&&... args)
{
    ERROR("{{ex}}", { "ex", std::vformat(templateStr, std::make_format_args(std::forward<Args&>(args)...)) });
}

void __stdcall CrashCatcher::LogContent47bc4()
{
    ERROR("Exception/Crash in content.dll:0xCb_47bc4 - "
          "probably missing formation in faction_props.ini/formations.ini - exiting");
    Sleep(1000);
    std::exit(-1);
}

CrashCatcher::FixContent47bc4::FixContent47bc4()
{
    test(eax, eax);
    jz("willCrash");
    mov(edi, eax);
    mov(edx, dword[edi]);
    mov(ecx, edi);
    ret();
    L("willCrash");
    xor_(ecx, ecx);
    ret();
}

DWORD __stdcall CrashCatcher::EngBase11A6DCatch(const BYTE* data)
{
    __try
    {
        return *reinterpret_cast<const DWORD*>(data + 0x28);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LogInternal(L"Exception/Crash suppression engbase.dll:0x11A6D");
        return 0;
    }
}

const BYTE* __stdcall CrashCatcher::EngBase124BDCatch(const BYTE* data)
{
    static char cmp[256], part[256];

    __try
    {
        DWORD addr = *(DWORD*)(data + 12);
        if (addr)
        {
            addr = *(DWORD*)(addr + 4);
        }

        if (addr)
        {
            strncpy_s(cmp, (LPCSTR)addr, sizeof(cmp));
            cmp[sizeof(cmp) - 1] = '\0';
        }
        else
        {
            *cmp = '\0';
        }

        addr = *(DWORD*)(data + 8);
        if (addr)
        {
            addr = *(DWORD*)(addr + 4);
        }

        if (addr)
        {
            strncpy_s(part, (LPCSTR)addr, sizeof(part));
            part[sizeof(part) - 1] = '\0';
        }
        else
        {
            *part = '\0';
        }

        data = *(PBYTE*)(data + 16);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LogInternal(L"Exception/Crash suppression engbase.dll:0x124BD");
        LogInternal("Cmp={} Part={}", cmp, part);
        data = nullptr;
    }

    return data;
}

double CrashCatcher::TimingSeconds(int64& delta)
{
    double seconds = Timing::seconds(delta);
    if (seconds < 0 || seconds > 10.0)
    {
        ERROR("Time delta invalid {{seconds}} {{delta}}", { "seconds", seconds }, { "delta", delta });
        delta = 1000000;
        seconds = Timing::seconds(delta);
    }
    else if (seconds > 1.0)
    {

        ERROR("Time lag detected {{seconds}} {{delta}}", { "seconds", seconds }, { "delta", delta });
    }
    return seconds;
}

CrashCatcher::FixEngBase11A6D::FixEngBase11A6D()
{
    push(eax);
    call(CrashCatcher::EngBase11A6DCatch);
    ret(8);
}

CrashCatcher::FixEngBase124BD::FixEngBase124BD()
{
    push(eax);
    call(CrashCatcher::EngBase124BDCatch);
    test(eax, eax);
    ret();
}

CrashCatcher::FixContent6F8B330::FixContent6F8B330()
{
    mov(ptr[&CrashCatcher::savedEcx], ecx);
    jmp(CrashCatcher::FixCommon6F8B330Detour);
}

CrashCatcher::FixContent6F78DD0::FixContent6F78DD0()
{
    mov(ptr[&CrashCatcher::savedEcx], ecx);
    jmp(CrashCatcher::FixCommon6F78DD0Detour);
}

void CrashCatcher::CrashProc6F671A0(int arg1)
{
    try
    {
        DEBUG("{{arg1}}", { "arg1", arg1 });

        __asm {
            pushad
            push arg1
            call [crashProc6F671A0Old]
            add esp, 4
            popad
        }
    }
    catch (...)
    {
        ERROR("Crash suppression {{arg1}}", { "arg1", arg1 });
    }
}

CObject* CrashCatcher::GetRoot(CObject* child)
{
    try
    {
        return ::GetRoot(child);
    }
    catch (...)
    {
        ERROR("Crash suppression {{archId}}", { "archId", child->get_archetype()->archId });
        return child;
    }
}

DWORD __stdcall CrashCatcher::C4800HookNaked() { return contentModule; }

int CrashCatcher::C4800Hook(int* a1, int* a2, int* zone, double* a4, int a5, int a6)
{
    __try
    {
        int res = 0;

        __asm {
            pushad
            push a6
            push a5
            push a4
            push zone
            push a2
            push a1
            call C4800HookNaked
            add eax, 0xC4800
            call eax
            add esp, 24
            mov [res], eax
            popad
        }

        return res;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        LogInternal(L"Exception/Crash suppression content.dll:0xC608D(zone={:#8X})", zone ? *zone : 0);
        return 0;
    }
}

char __stdcall CrashCatcher::FixCommon6F8B330Detour(int arg1)
{
    int res = 0;
    try
    {
        DEBUG("{{arg1}}", { "arg1", arg1 });
        __asm
        {
            pushad
            push arg1
            mov ecx, savedEcx
            call [crashProc6F8B330Old]
            mov [res], eax
            popad
        }
    }
    catch (...)
    {
        ERROR("Crash suppression {{arg1}}", { "arg1", arg1 });
    }

    return static_cast<char>(res);
}

void __stdcall CrashCatcher::FixCommon6F78DD0Detour(int arg1, int arg2)
{
    try
    {
        DEBUG("{{arg1}} {{arg2}}", { "arg1", arg1 }, { "arg2", arg2 });

        __asm
        {
            pushad
            push arg2
            push arg1
            mov ecx, savedEcx
            call [crashProc6F78DD0Old]
            popad
        }
    }
    catch (...)
    {
        ERROR("Crash suppression {{arg1}} {{arg2}}", { "arg1", arg1 }, { "arg2", arg2 });
    }
}

CrashCatcher::CrashCatcher()
{
    if (instance != nullptr)
    {
        throw std::logic_error("Crash catcher cannot be instantiated multiple times.");
    }

    instance = this;

    static FixContent47bc4 fixContent47Bc4;
    static FixEngBase11A6D fixEngBase11A6D;
    static FixEngBase124BD fixEngBase124BD;
    static FixContent6F8B330 fixContent6F8B330;
    static FixContent6F78DD0 fixContent6F78DD0;

    const auto config = FLHook::GetConfig();

    const auto flServerModule = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    const auto serverModule = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));

    const float npcVisibility = std::powf(config->npc.npcVisibilityDistance, 2);
    MemUtils::WriteProcMem(serverModule + 0x86AEC, &npcVisibility, 4);

    auto hook = reinterpret_cast<FARPROC>(CrashCatcher::GetRoot);
    MemUtils::ReadProcMem(serverModule + 0x84018, &oldGetRootProc, 4);
    MemUtils::WriteProcMem(serverModule + 0x84018, &hook, 4);

    // Patch the time functions to work around bugs on multiprocessor
    // and virtual machines.
    const auto timingSeconds = reinterpret_cast<FARPROC>(TimingSeconds);
    MemUtils::ReadProcMem(flServerModule + 0x1B0A0, &oldTimingSeconds, 4);
    MemUtils::WriteProcMem(flServerModule + 0x1B0A0, &timingSeconds, 4);

    const auto engBaseModule = reinterpret_cast<DWORD>(GetModuleHandleA("engbase.dll"));
    contentModule = reinterpret_cast<DWORD>(GetModuleHandleA("content.dll"));

    DEBUG("Installing patches into content.dll");

    // Patch for crash at content.dll + blarg
    MemUtils::PatchCallAddr(contentModule, 0xC608D, C4800Hook);

    // Patch for crash at content.dll + c458f ~ adoxa (thanks man)
    // Crash if solar has wrong destructible archetype (NewArk for
    // example is fuchu_core with hit_pts = 0 - different from
    // client and server in my case) and player taken off from
    // nearest base of this archetype (Manhattan) This is caused by
    // multiple players dying in the same planet death zone. Also
    // 000c458f error arises when nearby stations within a zone )
    // are reputed not coinciding with reputation on the
    // client-side.
    {
        // alternative: 0C458F, 8B0482->33C090
        constexpr uchar patch[] = { 0x74, 0x11, 0xeb, 0x05 };
        MemUtils::WriteProcMem(contentModule + 0xC457F, patch, 4);
    }

    // Patch for crash at content.dll + Cb_47bc4
    // This appears to be related to NPCs and/or their chatter.
    // What's missing contains the from, to and cargo entries
    // (amongst other stuff). Original Bytes: 8B F8 8B 17 8B CF
    {
        constexpr uchar patch[] = { 0x90, 0xe8 }; // nop call
        MemUtils::WriteProcMem(contentModule + 0x47bc2, patch, 2);
        MemUtils::PatchCallAddr(contentModule, 0x47bc2 + 1, (void*)fixContent47Bc4.getCode());
    }

    // Patch for crash at engbase.dll + 0x0124BD ~ adoxa (thanks
    // man) This is caused by a bad cmp.
    {
        constexpr uchar patch[] = { 0xe8 };
        MemUtils::WriteProcMem(engBaseModule + 0x0124BD, patch, 1);
        MemUtils::PatchCallAddr(engBaseModule, 0x0124BD, (void*)fixEngBase124BD.getCode());
    }

    // Patch for crash at engbase.dll + 0x011a6d
    // This is caused by a bad cmp I suspect
    {
        constexpr uchar patch[] = { 0x90, 0xe9 }; // nop jmpr
        MemUtils::WriteProcMem(engBaseModule + 0x011a6d, patch, 2);
        MemUtils::PatchCallAddr(engBaseModule, 0x011a6d + 1, (void*)fixEngBase11A6D.getCode());
    }

    // Hook for crash at 0xEB4B5 (confirmed)
    hook = (FARPROC)fixContent6F8B330.getCode();
    MemUtils::ReadProcMem(contentModule + 0x11C970, &crashProc6F8B330Old, 4);
    MemUtils::WriteProcMem(contentModule + 0x11C970, &hook, 4);
    MemUtils::WriteProcMem(contentModule + 0x11CA00, &hook, 4);

    // Hook for crash at 0xD8E14 (confirmed)
    crashProc6F78DD0Old = MemUtils::PatchCallAddr(contentModule, 0x5ED4B, (void*)fixContent6F78DD0.getCode());
    MemUtils::PatchCallAddr(contentModule, 0xBD96A, (void*)fixContent6F78DD0.getCode());

    // Hook for crash at 0xC71AE (confirmed)
    crashProc6F671A0Old = MemUtils::PatchCallAddr(contentModule, 0xBDC80, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xBDCF9, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xBE41C, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC67E2, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC6AA5, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC6BE8, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC6F71, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC702A, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC713B, CrashProc6F671A0);
    MemUtils::PatchCallAddr(contentModule, 0xC7180, CrashProc6F671A0);

    const float distance = config->npc.npcPersistDistance;
    MemUtils::WriteProcMem(contentModule + 0xD3D6E, &distance, 4);
    MemUtils::WriteProcMem(contentModule + 0x58F46, &distance, 4);
}

CrashCatcher::~CrashCatcher()
{
    if (instance != this)
    {
        return;
    }

    instance = nullptr;

    const auto flServerModule = reinterpret_cast<DWORD>(GetModuleHandleA(nullptr));
    const auto serverModule = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));
    const auto contentModule = reinterpret_cast<DWORD>(GetModuleHandleA("content.dll"));
    const auto engBaseModule = reinterpret_cast<DWORD>(GetModuleHandleA("engbase.dll"));
    MemUtils::WriteProcMem(serverModule + 0x84018, &oldGetRootProc, 4);

    // Unload the timing patches.
    MemUtils::WriteProcMem(flServerModule + 0x1B0A0, &oldTimingSeconds, 4);

    DEBUG("Uninstalling patches from content.dll");

    {
        const uchar patch[] = { 0xe8, 0x6e, 0xe7, 0xff, 0xff };
        MemUtils::WriteProcMem(contentModule + 0xC608D, patch, 5);
    }

    {
        const uchar patch[] = { 0x8B, 0xF8, 0x8B, 0x17, 0x8B, 0xCF };
        MemUtils::WriteProcMem(contentModule + 0x47bc2, patch, 6);
    }

    {
        const uchar patch[] = { 0x8B, 0x40, 0x10, 0x85, 0xc0 };
        MemUtils::WriteProcMem(engBaseModule + 0x0124BD, patch, 5);
    }

    {
        const uchar patch[] = { 0x8B, 0x40, 0x28, 0xC2, 0x08, 0x00 };
        MemUtils::WriteProcMem(engBaseModule + 0x011a6d, patch, 6);
    }

    MemUtils::WriteProcMem(contentModule + 0x11C970, &crashProc6F8B330Old, 4);
    MemUtils::WriteProcMem(contentModule + 0x11CA00, &crashProc6F8B330Old, 4);

    MemUtils::PatchCallAddr(contentModule, 0x5ED4B, crashProc6F78DD0Old);
    MemUtils::PatchCallAddr(contentModule, 0xBD96A, crashProc6F78DD0Old);

    MemUtils::PatchCallAddr(contentModule, 0xBDC80, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xBDCF9, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xBE41C, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC67E2, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC6AA5, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC6BE8, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC6F71, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC702A, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC713B, crashProc6F671A0Old);
    MemUtils::PatchCallAddr(contentModule, 0xC7180, crashProc6F671A0Old);
}
