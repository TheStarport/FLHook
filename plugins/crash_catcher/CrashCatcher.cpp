/**
 * @date Feb, 2010
 * @author Cannon (Ported by Laz 2022)
 * @defgroup CrashCatcher Crash Catcher
 * @brief
 * This plugin is used to catch and handle known crashes in FLServer
 *
 * @paragraph cmds Player Commands
 * There are no player commands in this plugin.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "CrashCatcher.h"

namespace Plugins::CrashCatcher
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup CrashCatcher
	 * @brief Need to use our own logging functions since the nature of this plugin isn't compatible with FLHook's standard logging functionality.
	 */
	void AddLogInternal(const char* String, ...)
	{
		char BufString[1024];
		va_list marker;
		va_start(marker, String);
		_vsnprintf_s(BufString, sizeof(BufString) - 1, String, marker);

		FILE* log;
		fopen_s(&log, "logs\\crashes.log", "a");

		char Buf[64];
		time_t tNow = time(nullptr);
		tm t;
		localtime_s(&t, &tNow);
		strftime(Buf, sizeof(Buf), "%d.%m.%Y %H:%M:%S", &t);
		fprintf(log, "[%s] %s\n", Buf, BufString);
		fflush(log);

		if (IsDebuggerPresent())
		{
			OutputDebugString(("[LOG] " + std::string(BufString) + "\n").c_str());
		}

		fclose(log);
	}

	/** @ingroup CrashCatcher
	 * @brief Save after a tractor to prevent cargo duplication loss on crash
	 */
	void TractorObjects(ClientId& client, [[maybe_unused]] struct XTractorObjects const& objs)
	{
		if (global->mapSaveTimes[client] == 0)
		{
			global->mapSaveTimes[client] = Hk::Time::GetUnixMiliseconds() + 60000;
		}
	}

	/** @ingroup CrashCatcher
	 * @brief Save after jettison to reduce chance of duplication on crash
	 */
	void JettisonCargo(ClientId& client, [[maybe_unused]] struct XJettisonCargo const& objs)
	{
		if (global->mapSaveTimes[client] == 0)
		{
			global->mapSaveTimes[client] = Hk::Time::GetUnixMiliseconds() + 60000;
		}
	}

	/** @ingroup CrashCatcher
	 * @brief Action the save times recorded in the above two functions
	 */
	void SaveCrashingCharacter()
	{
		mstime currTime = Hk::Time::GetUnixMiliseconds();
		for (auto& [client, saveTime] : global->mapSaveTimes)
		{
			if (saveTime != 0 && saveTime < currTime)
			{
				if (Hk::Client::IsValidClientID(client) && !Hk::Client::IsInCharSelectMenu(client))
					Hk::Player::SaveChar(client);
				saveTime = 0;
			}
		}
	}

	const std::vector<Timer> timers = {{SaveCrashingCharacter, 1}};

	/** @ingroup CrashCatcher
	 * @brief Originally in Main.cpp of PlayerControl
	 */
	void RequestBestPath(ClientId& p1, const uint& p2, const int& p3)
	{
		global->returncode = ReturnCode::SkipFunctionCall;
		try
		{
			Server.RequestBestPath(p1, (unsigned char*)p2, p3);
		}
		catch (...)
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("Exception in RequestBestPath p1={}", p1));
		}
	}

	static FARPROC fpOldGetRootProc = nullptr;
	/** @ingroup CrashCatcher
	 * @brief GetRoot hook to stop crashes at engbase.dll offset 0x000124bd
	 */
	struct CObject* __cdecl Cb_GetRoot(struct CObject* child)
	{
		try
		{
			return GetRoot(child);
		}
		catch (...)
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("Crash suppression in GetRoot(child={})", child->get_archetype()->iArchId));
			Console::ConErr(std::format("Crash suppression in GetRoot(child={})", child->get_archetype()->iArchId));
			return child;
		}
	}

	static FARPROC fpCrashProc1b221Old = nullptr;
	/** @ingroup CrashCatcher
	 * @brief This crash will happen if you have broken path finding or more than 10 connections per system.
	 */
	int __cdecl Cb_CrashProc1b221(unsigned int const& system, struct pub::System::ConnectionEnumerator& conn, int type)
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

	static FARPROC fpCrashProc1b113470Old = nullptr;
	uint __cdecl Cb_PubZoneSystem(uint system)
	{
		int res = pub::Zone::GetSystem(system);
		// AddLog(LogType::Normal, LogLevel::Info,L"pub::Zone::GetSystem %u %u", system, res);
		return res;
	}

	static void* dwSavedECX = nullptr;

	static FARPROC fpCrashProc6F8B330Old = nullptr;
	char __stdcall Cb_CrashProc6F8B330(int arg1)
	{
		int res = 0;
		try
		{
			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(std::format("Cb_CrashProc6F8B330(arg1={:#X})", arg1));
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

	__declspec(naked) void Cb_CrashProc6F8B330Naked()
	{
		__asm {
        mov dwSavedECX, ecx
        jmp Cb_CrashProc6F8B330
		}
	}

	static FARPROC fpCrashProc6F78DD0Old = nullptr;
	void __stdcall Cb_CrashProc6F78DD0(int arg1, int arg2)
	{
		try
		{
			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(std::format("Cb_CrashProc6F78DD0(arg1={:#X},arg2={:#X})", arg1, arg2));
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
	__declspec(naked) void Cb_CrashProc6F78DD0Naked()
	{
		__asm {
        mov dwSavedECX, ecx
        jmp Cb_CrashProc6F78DD0
		}
	}

	static FARPROC fpCrashProc6F671A0Old = nullptr;
	void __cdecl Cb_CrashProc6F671A0(int arg1)
	{
		try
		{
			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(std::format("Cb_CrashProc6F671A0(arg1={:#X})", arg1));
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

	__declspec(naked) void Cb_EngBase124BDNaked()
	{
		__asm {
        push eax
        call EngBase124BD_Log
        test eax, eax
        ret
		}
	}

	const DWORD __stdcall Cb_EngBase11a6dNaked_Log(const BYTE* data)
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

	__declspec(naked) void Cb_EngBase11a6dNaked()
	{
		__asm {
        push eax
        call Cb_EngBase11a6dNaked_Log
        ret 8
		}
	}

	void __stdcall Cb_47bc4Naked_Log()
	{
		AddLog(LogType::Normal,
		    LogLevel::Err,
		    "Exception/Crash in content.dll:0x47bc4 - probably missing formation in faction_props.ini/formations.ini - exiting");
		exit(-1);
	}

	__declspec(naked) void Cb_47bc4Naked()
	{
		__asm {
        test eax, eax
        jz will_crash
        mov edi, eax
        mov edx, [edi]
        mov ecx, edi
        ret
will_crash:
        call Cb_47bc4Naked_Log
        xor ecx, ecx
        ret
		}
	}

	static HMODULE modContentAc = nullptr; // or whatever value that is
	void __stdcall Cb_C4800HookNaked()
	{
		modContentAc = global->contentAC;
	}

	int Cb_C4800Hook(int* a1, int* a2, int* zone, double* a4, int a5, int a6)
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
			mov eax, [Cb_C4800HookNaked]
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

	static double __cdecl Cb_TimingSeconds(int64& ticks_delta)
	{
		double seconds = Timing::seconds(ticks_delta);
		if (seconds < 0 || seconds > 10.0)
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("Time delta invalid seconds={:.2f} ticks_delta={}", seconds, ticks_delta));
			Console::ConErr(std::format("Time delta invalid seconds={:.2f} ticks_delta={}", seconds, ticks_delta));
			ticks_delta = 1000000;
			seconds = Timing::seconds(ticks_delta);
		}
		else if (seconds > 1.0)
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("Time lag detected seconds={:.2f} ticks_delta={}", seconds, ticks_delta));
			Console::ConErr(std::format("Time lag detected seconds={:.2f} ticks_delta={}", seconds, ticks_delta));
		}
		return seconds;
	}

	/** @ingroup CrashCatcher
	 * @brief Install hooks
	 */
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

					FARPROC fpHook = (FARPROC)Cb_GetRoot;
					ReadProcMem((char*)global->hModServerAC + 0x84018, &fpOldGetRootProc, 4);
					WriteProcMem((char*)global->hModServerAC + 0x84018, &fpHook, 4);
				}

				// Patch the time functions to work around bugs on multiprocessor
				// and virtual machines.
				FARPROC fpTimingSeconds = (FARPROC)Cb_TimingSeconds;
				ReadProcMem((char*)GetModuleHandle(nullptr) + 0x1B0A0, &global->fpOldTimingSeconds, 4);
				WriteProcMem((char*)GetModuleHandle(nullptr) + 0x1B0A0, &fpTimingSeconds, 4);

				global->hEngBase = GetModuleHandle("engbase.dll");
				global->contentAC = GetModuleHandle("content.dll");
				if (global->contentAC)
				{
					if (FLHookConfig::i()->general.debugMode)
						Console::ConInfo("Installing patches into content.dll");

					// Patch for crash at content.dll + blarg
					{
						PatchCallAddr((char*)global->contentAC, 0xC608D, (char*)Cb_C4800Hook);
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
						WriteProcMem((char*)global->contentAC + 0xC457F, patch, 4);
					}

					// Patch for crash at content.dll + 47bc4
					// This appears to be related to NPCs and/or their chatter.
					// What's missing contains the from, to and cargo entries
					// (amongst other stuff). Original Bytes: 8B F8 8B 17 8B CF
					{
						uchar patch[] = {0x90, 0xe8}; // nop call
						WriteProcMem((char*)global->contentAC + 0x47bc2, patch, 2);
						PatchCallAddr((char*)global->contentAC, 0x47bc2 + 1, (char*)Cb_47bc4Naked);
					}

					// Patch for crash at engbase.dll + 0x0124BD ~ adoxa (thanks
					// man) This is caused by a bad cmp.
					{
						uchar patch[] = {0xe8};
						WriteProcMem((char*)global->hEngBase + 0x0124BD, patch, 1);
						PatchCallAddr((char*)global->hEngBase, 0x0124BD, (char*)Cb_EngBase124BDNaked);
					}

					// Patch for crash at engbase.dll + 0x011a6d
					// This is caused by a bad cmp I suspect
					{
						uchar patch[] = {0x90, 0xe9}; // nop jmpr
						WriteProcMem((char*)global->hEngBase + 0x011a6d, patch, 2);
						PatchCallAddr((char*)global->hEngBase, 0x011a6d + 1, (char*)Cb_EngBase11a6dNaked);
					}

					// Hook for crash at 1b221 in server.dll
					//{
					//	FARPROC fpHook = (FARPROC)Cb_CrashProc1b221;
					//	ReadProcMem((char*)contentAC + 0x1134f4,
					//&fpCrashProc1b221Old, 4); WriteProcMem((char*)contentAC
					//+
					// 0x1134f4, &fpHook, 4);
					//}

					//{
					//	FARPROC fpHook = (FARPROC)Cb_PubZoneSystem;
					//	ReadProcMem((char*)contentAC + 0x113470,
					//&fpCrashProc1b113470Old, 4); WriteProcMem((char*)contentAC
					//+
					// 0x113470, &fpHook, 4);
					//}

					// Hook for crash at 0xEB4B5 (confirmed)
					auto fpHook = (FARPROC)Cb_CrashProc6F8B330Naked;
					ReadProcMem((char*)global->contentAC + 0x11C970, &fpCrashProc6F8B330Old, 4);
					WriteProcMem((char*)global->contentAC + 0x11C970, &fpHook, 4);
					WriteProcMem((char*)global->contentAC + 0x11CA00, &fpHook, 4);

					// Hook for crash at 0xD8E14 (confirmed)
					fpCrashProc6F78DD0Old = PatchCallAddr((char*)global->contentAC, 0x5ED4B, (char*)Cb_CrashProc6F78DD0Naked);
					PatchCallAddr((char*)global->contentAC, 0xBD96A, (char*)Cb_CrashProc6F78DD0Naked);

					// Hook for crash at 0xC71AE (confirmed)
					fpCrashProc6F671A0Old = PatchCallAddr((char*)global->contentAC, 0xBDC80, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xBDCF9, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xBE41C, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC67E2, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC6AA5, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC6BE8, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC6F71, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC702A, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC713B, (char*)Cb_CrashProc6F671A0);
					PatchCallAddr((char*)global->contentAC, 0xC7180, (char*)Cb_CrashProc6F671A0);

					// Patch the NPC persist distance in MP to 6.5km and patch the
					// max spawn distance to 6.5km
					float fDistance = 6500;
					WriteProcMem((char*)global->contentAC + 0xD3D6E, &fDistance, 4);
					WriteProcMem((char*)global->contentAC + 0x58F46, &fDistance, 4);
				}
			}
		}
		catch (...)
		{
			LOG_EXCEPTION
		}
	}

	/** @ingroup CrashCatcher
	 * @brief Uninstall hooks
	 */
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
			WriteProcMem((char*)GetModuleHandle(nullptr) + 0x1B0A0, &global->fpOldTimingSeconds, 4);

			if (global->contentAC)
			{
				if (FLHookConfig::i()->general.debugMode)
					Console::ConInfo("Uninstalling patches from content.dll");

				{
					uchar patch[] = {0xe8, 0x6e, 0xe7, 0xff, 0xff};
					WriteProcMem((char*)global->contentAC + 0xC608D, patch, 5);
				}

				{
					uchar patch[] = {0x8B, 0xF8, 0x8B, 0x17, 0x8B, 0xCF};
					WriteProcMem((char*)global->contentAC + 0x47bc2, patch, 6);
				}

				{
					uchar patch[] = {0x8B, 0x40, 0x10, 0x85, 0xc0};
					WriteProcMem((char*)global->hEngBase + 0x0124BD, patch, 5);
				}

				{
					uchar patch[] = {0x8B, 0x40, 0x28, 0xC2, 0x08, 0x00};
					WriteProcMem((char*)global->hEngBase + 0x011a6d, patch, 6);
				}

				// WriteProcMem((char*)contentAC + 0x1134f4,
				// &fpCrashProc1b221Old, 4); WriteProcMem((char*)contentAC +
				// 0x113470, &fpCrashProc1b113470Old, 4);

				WriteProcMem((char*)global->contentAC + 0x11C970, &fpCrashProc6F8B330Old, 4);
				WriteProcMem((char*)global->contentAC + 0x11CA00, &fpCrashProc6F8B330Old, 4);

				PatchCallAddr((char*)global->contentAC, 0x5ED4B, (char*)fpCrashProc6F78DD0Old);
				PatchCallAddr((char*)global->contentAC, 0xBD96A, (char*)fpCrashProc6F78DD0Old);

				PatchCallAddr((char*)global->contentAC, 0xBDC80, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xBDCF9, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xBE41C, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC67E2, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC6AA5, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC6BE8, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC6F71, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC702A, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC713B, (char*)fpCrashProc6F671A0Old);
				PatchCallAddr((char*)global->contentAC, 0xC7180, (char*)fpCrashProc6F671A0Old);
			}
		}
	}
} // namespace Plugins::CrashCatcher

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CrashCatcher;

// Do things when the dll is loaded
BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE hinstDLL, DWORD fdwReason, [[maybe_unused]] LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH && CoreGlobals::c()->flhookReady)
		Init();

	if (fdwReason == DLL_PROCESS_DETACH)
		Shutdown();

	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Crash Catcher");
	pi->shortName("CrashCatcher");
	pi->mayUnload(true);
	pi->timers(&timers);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &Init, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__RequestBestPath, &RequestBestPath);
	pi->emplaceHook(HookedCall::IServerImpl__TractorObjects, &TractorObjects);
	pi->emplaceHook(HookedCall::IServerImpl__JettisonCargo, &JettisonCargo);
}