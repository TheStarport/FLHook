#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CrashCatcher
{

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/crashcatcher.json"; }

		//! Sets the maximum distance at which NPCs become visible as per scanner settings, a value of 0 will disable this override.
		float npcVisibilityDistance = 6500.f;
		//! Sets the distnace at which NPCs will despawn, a value of 0 will disable this override.
		float npcPersistDistance = 6500.f;
		//! Sets the distance at which NPCs will spawn, a value of 0 will disable this override.
		float npcSpawnDistance = 6500.f;
	};

	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		bool bPatchInstalled = false;
		FARPROC fpOldTimingSeconds = 0;

		HMODULE hModServerAC;
		HMODULE hEngBase;
		HMODULE hModContentAC;

		std::map<uint, mstime> mapSaveTimes;
	};

#define LOG_EXCEPTION_INTERNAL()                               \
	{                                                          \
		AddLogInternal("ERROR Exception in %s", __FUNCTION__); \
		AddExceptionInfoLog();                                 \
	};
} // namespace Plugins::CrashCatcher

IMPORT struct CObject* __cdecl GetRoot(struct CObject const*);