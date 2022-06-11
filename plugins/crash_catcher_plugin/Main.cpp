// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
// Ported 2022 by Laz
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::CrashCatcher
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void AddLogInternal(const char* szString, ...)
	{
		char szBufString[1024];
		va_list marker;
		va_start(marker, szString);
		_vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

		FILE* log;
		fopen_s(&log, "flhook_logs\\flhook_crashes.log", "a");

		char szBuf[64];
		time_t tNow = time(nullptr);
		tm t;
		localtime_s(&t, &tNow);
		strftime(szBuf, sizeof(szBuf), "%d.%m.%Y %H:%M:%S", &t);
		fprintf(log, "[%s] %s\n", szBuf, szBufString);
		fflush(log);

		if (IsDebuggerPresent())
		{
			OutputDebugString(("[LOG] " + std::string(szBufString) + "\n").c_str());
		}

		fclose(log);
	}

	// Save after a tractor to prevent cargo duplication loss on crash
	void __stdcall TractorObjects(uint& iClientID, struct XTractorObjects const& objs)
	{
		if (global->mapSaveTimes[iClientID] == 0)
		{
			global->mapSaveTimes[iClientID] = GetTimeInMS() + 60000;
		}
	}

	// Save after jettison to reduce chance of duplication on crash
	void __stdcall JettisonCargo(uint& iClientID, struct XJettisonCargo const& objs)
	{
		if (global->mapSaveTimes[iClientID] == 0)
		{
			global->mapSaveTimes[iClientID] = GetTimeInMS() + 60000;
		}
	}

	// Action the save times recorded in the above two functions
	void Timer()
	{
		mstime currTime = GetTimeInMS();
		for (auto& t : global->mapSaveTimes)
		{
			uint iClientID = t.first;
			if (t.second != 0 && t.second < currTime)
			{
				if (HkIsValidClientID(iClientID) && !HkIsInCharSelectMenu(iClientID))
					HkSaveChar(t.first);
				t.second = 0;
			}
		}
	}

	/** Originally in Main.cpp of PlayerControl */
	void __stdcall RequestBestPath(unsigned int p1, DWORD* p2, int p3)
	{
		global->returncode = ReturnCode::SkipFunctionCall;
		try
		{
			Server.RequestBestPath(p1, (unsigned char*)p2, p3);
		}
		catch (...)
		{
			AddLog(Normal, LogLevel::Err,
			    L"Exception in RequestBestPath p1=%d p2=%08x %08x %08x %08x "
			    "%08x %08x %08x %08x %08x p3=%08x",
			    p1, p2[0], p2[7], p2[3], p2[4], p2[5], p2[8], p2[9], p2[10], p2[12]);
		}
	}

	/** GetRoot hook to stop crashes at engbase.dll offset 0x000124bd */
	static FARPROC fpOldGetRootProc = 0;
	struct CObject* __cdecl HkCb_GetRoot(struct CObject* child)
	{
		try
		{
			return GetRoot(child);
		}
		catch (...)
		{
			AddLog(Normal, LogLevel::Err, L"Crash suppression in GetRoot(child=%08x)", child);
			Console::ConErr(L"Crash suppression in GetRoot(child=%08x)", child);
			return child;
		}
	}

	// This crash will happen if you have broken path finding or more than 10
	// connections per system.
	static FARPROC fpCrashProc1b221Old = 0;
	int __cdecl HkCb_CrashProc1b221(unsigned int const& system, struct pub::System::ConnectionEnumerator& conn, int type)
	{
		__try
		{
			return pub::System::EnumerateConnections(system, conn, (enum ConnectionType)type);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			AddLogInternal("Crash suppression in pub::System::EnumerateConnections(system=%08x,type=%d)", system, type);
			LOG_EXCEPTION_INTERNAL()
			return -2;
		}
	}

	static FARPROC fpCrashProc1b113470Old = 0;
	uint __cdecl HkCb_PubZoneSystem(uint system)
	{
		int res = pub::Zone::GetSystem(system);
		// AddLog(Normal, LogLevel::Info,L"pub::Zone::GetSystem %u %u", system, res);
		return res;
	}

	static void* dwSavedECX = 0;

	static FARPROC fpCrashProc6F8B330Old = 0;
	char __stdcall HkCb_CrashProc6F8B330(int arg1)
	{
		int res = 0;
		try
		{
			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(L"HkCb_CrashProc6F8B330(arg1=%08x)", arg1);
			__asm {
            pushad
            push arg1
            mov ecx, dwSavedECX
            call [fpCrashProc6F8B330Old]
            mov [res], eax
            popad
			}
		}
		catch (...)
		{
			LOG_EXCEPTION_INTERNAL()
		}

		return res;
	}

	__declspec(naked) void HkCb_CrashProc6F8B330Naked()
	{
		__asm {
        mov dwSavedECX, ecx
        jmp HkCb_CrashProc6F8B330
		}
	}

	static FARPROC fpCrashProc6F78DD0Old = 0;
	void __stdcall HkCb_CrashProc6F78DD0(int arg1, int arg2)
	{
		try
		{
			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(L"HkCb_CrashProc6F78DD0(arg1=%08x,arg2=%08x)", arg1, arg2);
			__asm {
            pushad
            push arg2
            push arg1
            mov ecx, dwSavedECX
            call [fpCrashProc6F78DD0Old]
            popad
			}
		}
		catch (...)
		{
			LOG_EXCEPTION_INTERNAL()
		}
	}
	__declspec(naked) void HkCb_CrashProc6F78DD0Naked()
	{
		__asm {
        mov dwSavedECX, ecx
        jmp HkCb_CrashProc6F78DD0
		}
	}

	static FARPROC fpCrashProc6F671A0Old = 0;
	void __cdecl HkCb_CrashProc6F671A0(int arg1)
	{
		try
		{
			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(L"HkCb_CrashProc6F671A0(arg1=%08x)", arg1);
			__asm {
            pushad
            push arg1
            call [fpCrashProc6F671A0Old]
            add esp, 4
            popad
			}
		}
		catch (...)
		{
			LOG_EXCEPTION_INTERNAL()
		}
	}

	static char cmp[256], part[256];

	const BYTE* __stdcall EngBase124BD_Log(const BYTE* data)
	{
		__try
		{
			DWORD addr = *(DWORD*)(data + 12);
			if (addr)
				addr = *(DWORD*)(addr + 4);
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
				addr = *(DWORD*)(addr + 4);
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
			AddLogInternal("Exception/Crash suppression engbase.dll:0x124BD");
			AddLogInternal("Cmp=%s Part=%s", cmp, part);
			data = 0;
		}

		return data;
	}

	__declspec(naked) void HkCb_EngBase124BDNaked()
	{
		__asm {
        push eax
        call EngBase124BD_Log
        test eax, eax
        ret
		}
	}

	const DWORD __stdcall HkCb_EngBase11a6dNaked_Log(const BYTE* data)
	{
		__try
		{
			return *(DWORD*)(data + 0x28);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			AddLogInternal("Exception/Crash suppression engbase.dll:0x11A6D");
			return 0;
		}
	}

	__declspec(naked) void HkCb_EngBase11a6dNaked()
	{
		__asm {
        push eax
        call HkCb_EngBase11a6dNaked_Log
        ret 8
		}
	}

	void __stdcall HkCb_47bc4Naked_Log()
	{
		AddLog(Normal, LogLevel::Err,
		    L"Exception/Crash in content.dll:0x47bc4 - probably missing "
		    "formation in faction_props.ini/formations.ini - exiting");
		exit(-1);
	}

	__declspec(naked) void HkCb_47bc4Naked()
	{
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

	static HMODULE modContentAc = 0; // or whatever value that is
	void __stdcall HkCb_C4800HookNaked()
	{
		modContentAc = global->hModContentAC;
	}

	int HkCb_C4800Hook(int* a1, int* a2, int* zone, double* a4, int a5, int a6)
	{
		__try
		{
			int res;

			__asm {
            pushad
            push a6
            push a5
            push a4
            push zone
            push a2
            push a1
			mov eax, [HkCb_C4800HookNaked]
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
			AddLogInternal("Exception/Crash suppression content.dll:0xC608D(zone=%08x)", (zone ? *zone : 0));
			LOG_EXCEPTION_INTERNAL()
			return 0;
		}
	}

	static double __cdecl HkCb_TimingSeconds(__int64& ticks_delta)
	{
		double seconds = Timing::seconds(ticks_delta);
		if (seconds < 0 || seconds > 10.0)
		{
			AddLog(Normal, LogLevel::Err, L"Time delta invalid seconds=%f ticks_delta=%I64i", seconds, ticks_delta);
			Console::ConErr(L"Time delta invalid seconds=%f ticks_delta=%I64i", seconds, ticks_delta);
			ticks_delta = 1000000;
			seconds = Timing::seconds(ticks_delta);
		}
		else if (seconds > 1.0)
		{
			AddLog(Normal, LogLevel::Err, L"Time lag detected seconds=%f ticks_delta=%I64i", seconds, ticks_delta);
			Console::ConErr(L"Time lag detected seconds=%f ticks_delta=%I64i", seconds, ticks_delta);
		}
		return seconds;
	}

	// Load configuration file
	void Init()
	{
		try
		{
			if (!global->bPatchInstalled)
			{
				global->bPatchInstalled = true;

				global->hModServerAC = GetModuleHandle("server.dll");
				if (global->hModServerAC)
				{
					// Patch the NPC visibility distance in MP to 6.5km (default
					// is 2.5km)
					float fDistance = 6500.0f * 6500.0f;
					WriteProcMem((char*)global->hModServerAC + 0x86AEC, &fDistance, 4);

					FARPROC fpHook = (FARPROC)HkCb_GetRoot;
					ReadProcMem((char*)global->hModServerAC + 0x84018, &fpOldGetRootProc, 4);
					WriteProcMem((char*)global->hModServerAC + 0x84018, &fpHook, 4);
				}

				// Patch the time functions to work around bugs on multiprocessor
				// and virtual machines.
				FARPROC fpTimingSeconds = (FARPROC)HkCb_TimingSeconds;
				ReadProcMem((char*)GetModuleHandle(0) + 0x1B0A0, &global->fpOldTimingSeconds, 4);
				WriteProcMem((char*)GetModuleHandle(0) + 0x1B0A0, &fpTimingSeconds, 4);

				global->hEngBase = GetModuleHandle("engbase.dll");
				global->hModContentAC = GetModuleHandle("content.dll");
				if (global->hModContentAC)
				{
					if (FLHookConfig::i()->general.debugMode)
						Console::ConInfo(L"NOTICE: Installing patches into content.dll");

					// Patch for crash at content.dll + blarg
					{
						PatchCallAddr((char*)global->hModContentAC, 0xC608D, (char*)HkCb_C4800Hook);
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
						WriteProcMem((char*)global->hModContentAC + 0xC457F, patch, 4);
					}

					// Patch for crash at content.dll + 47bc4
					// This appears to be related to NPCs and/or their chatter.
					// What's missing contains the from, to and cargo entries
					// (amongst other stuff). Original Bytes: 8B F8 8B 17 8B CF
					{
						uchar patch[] = {0x90, 0xe8}; // nop call
						WriteProcMem((char*)global->hModContentAC + 0x47bc2, patch, 2);
						PatchCallAddr((char*)global->hModContentAC, 0x47bc2 + 1, (char*)HkCb_47bc4Naked);
					}

					// Patch for crash at engbase.dll + 0x0124BD ~ adoxa (thanks
					// man) This is caused by a bad cmp.
					{
						uchar patch[] = {0xe8};
						WriteProcMem((char*)global->hEngBase + 0x0124BD, patch, 1);
						PatchCallAddr((char*)global->hEngBase, 0x0124BD, (char*)HkCb_EngBase124BDNaked);
					}

					// Patch for crash at engbase.dll + 0x011a6d
					// This is caused by a bad cmp I suspect
					{
						uchar patch[] = {0x90, 0xe9}; // nop jmpr
						WriteProcMem((char*)global->hEngBase + 0x011a6d, patch, 2);
						PatchCallAddr((char*)global->hEngBase, 0x011a6d + 1, (char*)HkCb_EngBase11a6dNaked);
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
					ReadProcMem((char*)global->hModContentAC + 0x11C970, &fpCrashProc6F8B330Old, 4);
					WriteProcMem((char*)global->hModContentAC + 0x11C970, &fpHook, 4);
					WriteProcMem((char*)global->hModContentAC + 0x11CA00, &fpHook, 4);

					// Hook for crash at 0xD8E14 (confirmed)
					fpCrashProc6F78DD0Old = PatchCallAddr((char*)global->hModContentAC, 0x5ED4B, (char*)HkCb_CrashProc6F78DD0Naked);
					PatchCallAddr((char*)global->hModContentAC, 0xBD96A, (char*)HkCb_CrashProc6F78DD0Naked);

					// Hook for crash at 0xC71AE (confirmed)
					fpCrashProc6F671A0Old = PatchCallAddr((char*)global->hModContentAC, 0xBDC80, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xBDCF9, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xBE41C, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC67E2, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC6AA5, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC6BE8, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC6F71, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC702A, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC713B, (char*)HkCb_CrashProc6F671A0);
					PatchCallAddr((char*)global->hModContentAC, 0xC7180, (char*)HkCb_CrashProc6F671A0);

					// Patch the NPC persist distance in MP to 6.5km and patch the
					// max spawn distance to 6.5km
					float fDistance = 6500;
					WriteProcMem((char*)global->hModContentAC + 0xD3D6E, &fDistance, 4);
					WriteProcMem((char*)global->hModContentAC + 0x58F46, &fDistance, 4);
				}
			}
		}
		catch (...)
		{
			LOG_EXCEPTION
		}
	}

	void Shutdown()
	{
		if (global->bPatchInstalled)
		{
			// Unhook getroot
			if (global->hModServerAC)
			{
				WriteProcMem((char*)global->hModServerAC + 0x84018, &fpOldGetRootProc, 4);
			}

			// Unload the timing patches.
			WriteProcMem((char*)GetModuleHandle(0) + 0x1B0A0, &global->fpOldTimingSeconds, 4);

			if (global->hModContentAC)
			{
				if (FLHookConfig::i()->general.debugMode)
					Console::ConInfo(L"NOTICE: Uninstalling patches from content.dll");

				{
					uchar patch[] = {0xe8, 0x6e, 0xe7, 0xff, 0xff};
					WriteProcMem((char*)global->hModContentAC + 0xC608D, patch, 5);
				}

				{
					uchar patch[] = {0x8B, 0xF8, 0x8B, 0x17, 0x8B, 0xCF};
					WriteProcMem((char*)global->hModContentAC + 0x47bc2, patch, 6);
				}

				{
					uchar patch[] = {0x8B, 0x40, 0x10, 0x85, 0xc0};
					WriteProcMem((char*)global->hEngBase + 0x0124BD, patch, 5);
				}

				{
					uchar patch[] = {0x8B, 0x40, 0x28, 0xC2, 0x08, 0x00};
					WriteProcMem((char*)global->hEngBase + 0x011a6d, patch, 6);
				}

				// WriteProcMem((char*)hModContentAC + 0x1134f4,
				// &fpCrashProc1b221Old, 4); WriteProcMem((char*)hModContentAC +
				// 0x113470, &fpCrashProc1b113470Old, 4);

				WriteProcMem((char*)global->hModContentAC + 0x11C970, &fpCrashProc6F8B330Old, 4);
				WriteProcMem((char*)global->hModContentAC + 0x11CA00, &fpCrashProc6F8B330Old, 4);

				PatchCallAddr((char*)global->hModContentAC, 0x5ED4B, (char*)fpCrashProc6F78DD0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xBD96A, (char*)fpCrashProc6F78DD0Old);

				PatchCallAddr((char*)global->hModContentAC, 0xBDC80, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xBDCF9, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xBE41C, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC67E2, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC6AA5, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC6BE8, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC6F71, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC702A, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC713B, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->hModContentAC, 0xC7180, (char*)fpCrashProc6F671A0Old);
			}
		}
	}
} // namespace Plugins::CrashCatcher

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CrashCatcher;

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		Init();

	else if (fdwReason == DLL_PROCESS_DETACH)
		Shutdown();

	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Crash Catcher");
	pi->shortName("CrashCatcher");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &Init);
	pi->emplaceHook(HookedCall::IServerImpl__RequestBestPath, &RequestBestPath);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &Timer);
	pi->emplaceHook(HookedCall::IServerImpl__TractorObjects, &TractorObjects);
	pi->emplaceHook(HookedCall::IServerImpl__JettisonCargo, &JettisonCargo);
}