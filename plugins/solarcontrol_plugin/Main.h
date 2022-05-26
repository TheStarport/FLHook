#pragma once

#include <FLHook.h>
#include <plugin.h>

namespace Plugins::SolarControl
{
	struct SOLAR
	{
		std::wstring name;
		Vector pos;
		uint system;
		Matrix rot;
	};

	struct SOLAR_ARCHTYPE_STRUCT
	{
		uint Shiparch;
		uint Loadout;
		uint IFF;
		uint Infocard;
		uint Base;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::map<std::wstring, SOLAR_ARCHTYPE_STRUCT> mapSolarArchtypes;
		std::map<int, SOLAR> startupSOLARs;
		std::map<uint, std::wstring> spawnedSOLARs;

		bool FirstRun = true;

		ReturnCode returncode = ReturnCode::Default;
	};
}
