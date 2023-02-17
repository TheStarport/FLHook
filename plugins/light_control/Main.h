#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::LightControl
{
	//! A hardpoint for a single client.
	struct EquipmentHardpoint
	{
		EquipmentHardpoint() = default;

		uint id = 0;
		uint archId = 0;
		uint originalArchId = 0;
		std::wstring hardPoint;
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/light_control.json"; }

		//! Vector of Available equipment
		std::vector<uint> lightsHashed;
		std::vector<std::wstring> lights;

		//! cost per changed item.
		uint cost = 0;

		//For display lights, how many to display.
		uint itemsPerPage = 25;

		//! Map of bases who offer LightControl
		std::vector<std::string> bases;
		std::vector<uint> baseIdHashes;

		//! Intro messages when entering the base.
		std::wstring introMessage1 = L"Light customization facilities are available here.";
		std::wstring introMessage2 = L"Type /lights on your console to see options.";
		bool notifyAvailabilityOnEnter = false;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		ReturnCode returnCode = ReturnCode::Default;

		jpWide::Regex regex = jpWide::Regex(L"(?<=.{2})([A-Z])", "g");
	};
} // namespace Plugins::LightControl