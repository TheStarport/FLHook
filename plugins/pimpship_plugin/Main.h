#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Pimpship
{
	IMPORT unsigned int MakeLocationID(unsigned int, char const*);

	//! This struct stores a piece of equipment available for purchase an a user friendly name/description
	struct ItemInfo : Reflectable
	{
		ItemInfo() : archId(0) {}

		uint archId;
		std::string nickname;
		std::string description;
	};

	//! A hardpoint for a single client.
	struct EquipmentHardpoint
	{
		EquipmentHardpoint() : ID(0), ArchID(0), OriginalArchID(0) {}

		uint ID;
		uint ArchID;
		uint OriginalArchID;
		std::wstring HardPoint;
	};

	//! A struct that represents a client
	struct ClientInfo
	{
		ClientInfo() : InPimpDealer(false) {}

		//! Map of hard point ID to equip.
		std::map<uint, EquipmentHardpoint> CurrentEquipment;

		//! Are they in the pimpship dealer?
		bool InPimpDealer;
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/pimpship.json"; }

		//! Vector of Available equipment
		std::vector<ItemInfo> AvailableItems;

		//! Cost per changed item.
		int Cost = 0;

		//! Map of Equipment dealer rooms who offer pimpship
		std::vector<std::string> bases;
		std::vector<uint> baseIdHashes;

		//! Intro messages when entering the room.
		std::wstring IntroMsg1 = L"Pimp-my-ship facilities are available here.";
		std::wstring IntroMsg2 = L"Type /pimpship on your console to see options.";
		bool notifyAvailabilityOnEnter = false;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		ReturnCode returncode = ReturnCode::Default;

		//! Map of dealer rooms
		std::map<uint, std::wstring> Dealers;

		//! Map of available equipment
		std::map<uint, ItemInfo> AvailableItems;

		//! Map of clients and if they're in the pimpship and their hardpoints
		std::map<uint, ClientInfo> Info;
	};
} // namespace Plugins::Pimpship