// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
// Ported 2022 by Laz
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode;

IMPORT struct CObject *__cdecl GetRoot(struct CObject const *);

/** GetRoot hook to stop crashes at engbase.dll offset 0x000124bd */
static FARPROC fpOldGetRootProc = 0;
struct CObject *__cdecl HkCb_GetRoot(struct CObject *child) {
    try {
        return GetRoot(child);
    } catch (...) {
        AddLog("ERROR: Crash suppression in GetRoot(child=%08x)", child);
        ConPrint(L"ERROR: Crash suppression in GetRoot(child=%08x)\n", child);
        return child;
    }
}

// This crash will happen if you have broken path finding or more than 10
// connections per system.
static FARPROC fpCrashProc1b221Old = 0;
int __cdecl HkCb_CrashProc1b221(unsigned int const &system,
                                struct pub::System::ConnectionEnumerator &conn,
                                int type) {
    __try {
        return pub::System::EnumerateConnections(system, conn,
                                                 (enum ConnectionType)type);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        AddLog("ERROR: Crash suppression in "
               "pub::System::EnumerateConnections(system=%08x,type=%d)",
               system, type);
        LOG_EXCEPTION;
        return -2;
    }
}

static FARPROC fpCrashProc1b113470Old = 0;
uint __cdecl HkCb_PubZoneSystem(uint system) {
    int res = pub::Zone::GetSystem(system);
    // AddLog("pub::Zone::GetSystem %u %u", system, res);
    return res;
}

static void *dwSavedECX = 0;

static FARPROC fpCrashProc6F8B330Old = 0;
char __stdcall HkCb_CrashProc6F8B330(int arg1) {
    int res = 0;
    try {
        if (set_bDebug > 2)
            ConPrint(L"HkCb_CrashProc6F8B330(arg1=%08x)\n", arg1);
        __asm {
            pushad
            push arg1
            mov ecx, dwSavedECX
            call [fpCrashProc6F8B330Old]
            mov [res], eax
            popad
        }
    } catch (...) {
        LOG_EXCEPTION
    }

    return res;
}

__declspec(naked) void HkCb_CrashProc6F8B330Naked() {
    __asm {
        mov dwSavedECX, ecx
        jmp HkCb_CrashProc6F8B330
    }
}

static FARPROC fpCrashProc6F78DD0Old = 0;
void __stdcall HkCb_CrashProc6F78DD0(int arg1, int arg2) {
    try {
        if (set_bDebug > 2)
            ConPrint(L"HkCb_CrashProc6F78DD0(arg1=%08x,arg2=%08x)\n", arg1,
                     arg2);
        __asm {
            pushad
            push arg2
            push arg1
            mov ecx, dwSavedECX
            call [fpCrashProc6F78DD0Old]
            popad
        }
    } catch (...) {
        LOG_EXCEPTION
    }
}
__declspec(naked) void HkCb_CrashProc6F78DD0Naked() {
    __asm {
        mov dwSavedECX, ecx
        jmp HkCb_CrashProc6F78DD0
    }
}

static FARPROC fpCrashProc6F671A0Old = 0;
void __cdecl HkCb_CrashProc6F671A0(int arg1) {
    try {
        if (set_bDebug > 2)
            ConPrint(L"HkCb_CrashProc6F671A0(arg1=%08x)\n", arg1);
        __asm {
            pushad
            push arg1
            call [fpCrashProc6F671A0Old]
            add esp, 4
            popad
        }
    } catch (...) {
        LOG_EXCEPTION
    }
}

static char cmp[256], part[256];

const BYTE *__stdcall EngBase124BD_Log(const BYTE *data) {
    __try {
        DWORD addr = *(DWORD *)(data + 12);
        if (addr)
            addr = *(DWORD *)(addr + 4);
        if (addr) {
            strncpy_s(cmp, (LPCSTR)addr, sizeof(cmp));
            cmp[sizeof(cmp) - 1] = '\0';
        } else {
            *cmp = '\0';
        }
        addr = *(DWORD *)(data + 8);
        if (addr)
            addr = *(DWORD *)(addr + 4);
        if (addr) {
            strncpy_s(part, (LPCSTR)addr, sizeof(part));
            part[sizeof(part) - 1] = '\0';
        } else {
            *part = '\0';
        }
        data = *(PBYTE *)(data + 16);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        AddLog("ERROR: Exception/Crash suppression engbase.dll:0x124BD");
        AddLog("Cmp=%s Part=%s", cmp, part);
        data = 0;
    }

    return data;
}

__declspec(naked) void HkCb_EngBase124BDNaked() {
    __asm {
        push eax
        call EngBase124BD_Log
        test eax, eax
        ret
    }
}

const DWORD __stdcall HkCb_EngBase11a6dNaked_Log(const BYTE *data) {
    __try {
        return *(DWORD *)(data + 0x28);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        AddLog("ERROR: Exception/Crash suppression engbase.dll:0x11A6D");
        return 0;
    }
}

__declspec(naked) void HkCb_EngBase11a6dNaked() {
    __asm {
        push eax
        call HkCb_EngBase11a6dNaked_Log
        ret 8
    }
}

void __stdcall HkCb_47bc4Naked_Log() {
    AddLog("ERROR: Exception/Crash in content.dll:0x47bc4 - probably missing "
           "formation in faction_props.ini/formations.ini - exiting");
    exit(-1);
}

__declspec(naked) void HkCb_47bc4Naked() {
    __asm {
        test eax, eax
        jz will_crash
        mov edi, eax
        mov edx, [edi]
        mov ecx, edi
        ret
will_crash:
        call HkCb_47bc4Naked_Log
        xor ecx, ecx
        ret
    }
}

int HkCb_C4800Hook(int *a1, int *a2, int *zone, double *a4, int a5, int a6) {
    __try {
        int res;

        __asm {
            pushad
            push a6
            push a5
            push a4
            push zone
            push a2
            push a1
            mov eax, [hModContentAC]
            add eax, 0xC4800
            call eax
            add esp, 24
            mov [res], eax
            popad
        }

        return res;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        AddLog(
            "ERROR: Exception/Crash suppression content.dll:0xC608D(zone=%08x)",
            (zone ? *zone : 0));
        LOG_EXCEPTION
        return 0;
    }
}

static double __cdecl HkCb_TimingSeconds(__int64 &ticks_delta) {
    double seconds = Timing::seconds(ticks_delta);
    if (seconds < 0 || seconds > 10.0) {
        AddLog("ERROR: Time delta invalid seconds=%f ticks_delta=%I64i",
               seconds, ticks_delta);
        ConPrint(L"ERROR: Time delta invalid seconds=%f ticks_delta=%I64i\n",
                 seconds, ticks_delta);
        ticks_delta = 1000000;
        seconds = Timing::seconds(ticks_delta);
    } else if (seconds > 1.0) {
        AddLog("ERROR: Time lag detected seconds=%f ticks_delta=%I64i", seconds,
               ticks_delta);
        ConPrint(L"ERROR: Time lag detected seconds=%f ticks_delta=%I64i\n",
                 seconds, ticks_delta);
    }
    return seconds;
}

static bool bPatchInstalled = false;
static FARPROC fpOldTimingSeconds = 0;

static HMODULE hModServerAC;
static HMODULE hEngBase;
static HMODULE hModContentAC;

// Load configuration file
void Init()
{
    try {
        if (!bPatchInstalled) {
            bPatchInstalled = true;

            hModServerAC = GetModuleHandle("server.dll");
            if (hModServerAC) {
                // Patch the NPC visibility distance in MP to 6.5km (default
                // is 2.5km)
                float fDistance = 6500.0f * 6500.0f;
                WriteProcMem((char *)hModServerAC + 0x86AEC, &fDistance, 4);

                FARPROC fpHook = (FARPROC)HkCb_GetRoot;
                ReadProcMem((char *)hModServerAC + 0x84018, &fpOldGetRootProc,
                            4);
                WriteProcMem((char *)hModServerAC + 0x84018, &fpHook, 4);
            }

            // Patch the time functions to work around bugs on multiprocessor
            // and virtual machines.
            FARPROC fpTimingSeconds = (FARPROC)HkCb_TimingSeconds;
            ReadProcMem((char *)GetModuleHandle(0) + 0x1B0A0,
                        &fpOldTimingSeconds, 4);
            WriteProcMem((char *)GetModuleHandle(0) + 0x1B0A0, &fpTimingSeconds,
                         4);

            hEngBase = GetModuleHandle("engbase.dll");
            hModContentAC = GetModuleHandle("content.dll");
            if (hModContentAC) {
                if (set_bDebug)
                    ConPrint(L"NOTICE: Installing patches into content.dll\n");

                // Patch for crash at content.dll + blarg
                {
                    PatchCallAddr((char *)hModContentAC, 0xC608D,
                                  (char *)HkCb_C4800Hook);
                }

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
                    uchar patch[] = {0x74, 0x11, 0xeb, 0x05};
                    WriteProcMem((char *)hModContentAC + 0xC457F, patch, 4);
                }

                // Patch for crash at content.dll + 47bc4
                // This appears to be related to NPCs and/or their chatter.
                // What's missing contains the from, to and cargo entries
                // (amongst other stuff). Original Bytes: 8B F8 8B 17 8B CF
                {
                    uchar patch[] = {0x90, 0xe8}; // nop call
                    WriteProcMem((char *)hModContentAC + 0x47bc2, patch, 2);
                    PatchCallAddr((char *)hModContentAC, 0x47bc2 + 1,
                                  (char *)HkCb_47bc4Naked);
                }

                // Patch for crash at engbase.dll + 0x0124BD ~ adoxa (thanks
                // man) This is caused by a bad cmp.
                {
                    uchar patch[] = {0xe8};
                    WriteProcMem((char *)hEngBase + 0x0124BD, patch, 1);
                    PatchCallAddr((char *)hEngBase, 0x0124BD,
                                  (char *)HkCb_EngBase124BDNaked);
                }

                // Patch for crash at engbase.dll + 0x011a6d
                // This is caused by a bad cmp I suspect
                {
                    uchar patch[] = {0x90, 0xe9}; // nop jmpr
                    WriteProcMem((char *)hEngBase + 0x011a6d, patch, 2);
                    PatchCallAddr((char *)hEngBase, 0x011a6d + 1,
                                  (char *)HkCb_EngBase11a6dNaked);
                }

                // Hook for crash at 1b221 in server.dll
                //{
                //	FARPROC fpHook = (FARPROC)HkCb_CrashProc1b221;
                //	ReadProcMem((char*)hModContentAC + 0x1134f4,
                //&fpCrashProc1b221Old, 4); WriteProcMem((char*)hModContentAC
                //+
                // 0x1134f4, &fpHook, 4);
                //}

                //{
                //	FARPROC fpHook = (FARPROC)HkCb_PubZoneSystem;
                //	ReadProcMem((char*)hModContentAC + 0x113470,
                //&fpCrashProc1b113470Old, 4); WriteProcMem((char*)hModContentAC
                //+
                // 0x113470, &fpHook, 4);
                //}

                // Hook for crash at 0xEB4B5 (confirmed)
                FARPROC fpHook = (FARPROC)HkCb_CrashProc6F8B330Naked;
                ReadProcMem((char *)hModContentAC + 0x11C970,
                            &fpCrashProc6F8B330Old, 4);
                WriteProcMem((char *)hModContentAC + 0x11C970, &fpHook, 4);
                WriteProcMem((char *)hModContentAC + 0x11CA00, &fpHook, 4);

                // Hook for crash at 0xD8E14 (confirmed)
                fpCrashProc6F78DD0Old =
                    PatchCallAddr((char *)hModContentAC, 0x5ED4B,
                                  (char *)HkCb_CrashProc6F78DD0Naked);
                PatchCallAddr((char *)hModContentAC, 0xBD96A,
                              (char *)HkCb_CrashProc6F78DD0Naked);

                // Hook for crash at 0xC71AE (confirmed)
                fpCrashProc6F671A0Old =
                    PatchCallAddr((char *)hModContentAC, 0xBDC80,
                                  (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xBDCF9,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xBE41C,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC67E2,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC6AA5,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC6BE8,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC6F71,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC702A,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC713B,
                              (char *)HkCb_CrashProc6F671A0);
                PatchCallAddr((char *)hModContentAC, 0xC7180,
                              (char *)HkCb_CrashProc6F671A0);

                // Patch the NPC persist distance in MP to 6.5km and patch the
                // max spawn distance to 6.5km
                float fDistance = 6500;
                WriteProcMem((char *)hModContentAC + 0xD3D6E, &fDistance, 4);
                WriteProcMem((char *)hModContentAC + 0x58F46, &fDistance, 4);
            }
        }
    } catch (...) {
        LOG_EXCEPTION
    }
}

void Shutdown() {
    if (bPatchInstalled) {
        // Unhook getroot
        if (hModServerAC) {
            WriteProcMem((char *)hModServerAC + 0x84018, &fpOldGetRootProc, 4);
        }

        // Unload the timing patches.
        WriteProcMem((char *)GetModuleHandle(0) + 0x1B0A0, &fpOldTimingSeconds,
                     4);

        if (hModContentAC) {
            if (set_bDebug)
                ConPrint(L"NOTICE: Uninstalling patches from content.dll\n");

            {
                uchar patch[] = {0xe8, 0x6e, 0xe7, 0xff, 0xff};
                WriteProcMem((char *)hModContentAC + 0xC608D, patch, 5);
            }

            {
                uchar patch[] = {0x8B, 0xF8, 0x8B, 0x17, 0x8B, 0xCF};
                WriteProcMem((char *)hModContentAC + 0x47bc2, patch, 6);
            }

            {
                uchar patch[] = {0x8B, 0x40, 0x10, 0x85, 0xc0};
                WriteProcMem((char *)hEngBase + 0x0124BD, patch, 5);
            }

            {
                uchar patch[] = {0x8B, 0x40, 0x28, 0xC2, 0x08, 0x00};
                WriteProcMem((char *)hEngBase + 0x011a6d, patch, 6);
            }

            // WriteProcMem((char*)hModContentAC + 0x1134f4,
            // &fpCrashProc1b221Old, 4); WriteProcMem((char*)hModContentAC +
            // 0x113470, &fpCrashProc1b113470Old, 4);

            WriteProcMem((char *)hModContentAC + 0x11C970,
                         &fpCrashProc6F8B330Old, 4);
            WriteProcMem((char *)hModContentAC + 0x11CA00,
                         &fpCrashProc6F8B330Old, 4);

            PatchCallAddr((char *)hModContentAC, 0x5ED4B,
                          (char *)fpCrashProc6F78DD0Old);
            PatchCallAddr((char *)hModContentAC, 0xBD96A,
                          (char *)fpCrashProc6F78DD0Old);

            PatchCallAddr((char *)hModContentAC, 0xBDC80,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xBDCF9,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xBE41C,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC67E2,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC6AA5,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC6BE8,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC6F71,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC702A,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC713B,
                          (char *)fpCrashProc6F671A0Old);
            PatchCallAddr((char *)hModContentAC, 0xC7180,
                          (char *)fpCrashProc6F671A0Old);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));

	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        Init();

    else if (fdwReason == DLL_PROCESS_DETACH)
        Shutdown();

	return true;
}

// Functions to hook
EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Crash Catcher");
    pi->shortName("CrashCatcher");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &Init);
}