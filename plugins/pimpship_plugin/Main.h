#pragma once

#include <FLHook.h>
#include <plugin.h>

namespace Plugins::Pimpship
{
	IMPORT unsigned int MakeLocationID(unsigned int, char const*);

	// Map of item id to ITEM INFO
	struct ITEM_INFO : Reflectable
	{
		ITEM_INFO() : ArchID(0) {}

		uint ArchID;
		std::string Nickname;
		std::string Description;
	};

	// Item of equipment for a single client.
	struct EQ_HARDPOINT
	{
		EQ_HARDPOINT() : ID(0), ArchID(0), OriginalArchID(0) {}

		uint ID;
		uint ArchID;
		uint OriginalArchID;
		std::wstring HardPoint;
	};

	// List of connected clients.
	struct INFO
	{
		INFO() : InPimpDealer(false) {}

		// Map of hard point ID to equip.
		std::map<uint, EQ_HARDPOINT> CurrentEquipment;

		// Are they in the pimpship dealer?
		bool InPimpDealer;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/pimpship.json"; }

		// Map of Available equipment
		std::vector<ITEM_INFO> AvailableItems;

		// Cost per changed item.
		int Cost = 0;

		// Map of Equipment dealer rooms who offer pimpship
		std::vector<std::string> Dealers;

		// Intro messages when entering the room.
		std::wstring IntroMsg1 = L"Pimp-my-ship facilities are available here.";
		std::wstring IntroMsg2 = L"Type /pimpship on your console to see options.";
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		ReturnCode returncode = ReturnCode::Default;

		// Map of dealer rooms
		std::map<uint, std::wstring> Dealers;

		// Map of available equipment
		std::map<uint, ITEM_INFO> AvailableItems;

		// Map of clients and if they're in the pimpship and their hardpoints
		std::map<uint, INFO> Info;
	};
} // namespace Plugins::Pimpship