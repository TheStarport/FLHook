#pragma once
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::MiningControl
{
	//! A struct that defines a mining bonus to a player if they meet certain criteria
	struct PlayerBonus : Reflectable
	{
		PlayerBonus() = default;

		//! The loot commodity id this configuration applies to.
		std::string Loot = "commodity_gold";
		uint LootId = 0;

		//! The loot bonus multiplier.
		float Bonus = 0.0f;

		//! The affiliation/reputation of the player
		std::string Rep = "li_n_grp";
		uint RepId = UINT_MAX;

		//! The list of ships that this bonus applies to
		std::vector<std::string> Ships = {"ge_fighter"};
		std::vector<uint> ShipIds;

		//! The list of equipment items that the ship must carry
		std::vector<std::string> Items = {"ge_s_battery_01"};
		std::vector<uint> ItemIds;

		//! The list of ammo arch ids for mining guns
		std::vector<std::string> Ammo = {"missile01_mark01"};
		std::vector<uint> AmmoIds;
	};
	
	//! A struct that defines a mining bonus for a certain zone in space
	struct ZoneBonus : Reflectable
	{
		ZoneBonus() = default;

		std::string Zone = "ExampleZone";

		//! The loot bonus multiplier.
		float Bonus = 0.0f;

		//! The hash of the item to replace the dropped
		std::string ReplacementLoot = "commodity_gold";
		uint ReplacementLootId = 0;

		//! The recharge rate of the zone. This is the number of units of ore added to the reserve per minute.
		float RechargeRate = 0;

		//! The current amount of ore in the zone. When this gets low, ore gets harder to mine. When it gets to 0, ore is impossible to mine.
		float CurrentReserve = 100000;

		//! The maximum limit for the amount of ore in the field
		float MaxReserve = 50000;

		//! The amount of ore that has been mined.
		float Mined = 0;
	};

	//! A struct to represent each client
	struct ClientData
	{
		ClientData() = default;

		bool Setup = false;
		std::map<uint, float> LootBonus;
		std::map<uint, std::vector<uint>> LootAmmo;
		std::map<uint, std::vector<uint>> LootShip;
		int Debug = 0;

		int MineAsteroidEvents = 0;
		time_t MineAsteroidSampleStart = 0;
	};



	//! A struct to hold the current status of a zone so their progress persists across restarts
	struct ZoneStats : Reflectable
	{
		std::string Zone;
		float CurrentReserve = 0.0f;
		float Mined = 0.0f;
	};

	//! A struct to load in the stats config file across restarts
	struct MiningStats : Reflectable
	{
		std::string File() override
		{
			char path[MAX_PATH];
			GetUserDataPath(path);
			return std::string(path) + "\\MiningStats.json";
		}

		std::vector<ZoneStats> Stats;
	};

	//! Config data for this plugin
	struct Config : Reflectable
	{
		//! The json file we load out of
		std::string File() override { return "config/mine_control.json"; }

		PlayerBonus playerBonusExample;
		ZoneBonus zoneBonusExample;

		std::vector<PlayerBonus> PlayerBonus = {playerBonusExample};
		std::vector<ZoneBonus> ZoneBonus = {zoneBonusExample};
		float GenericFactor = 1.0f;
		int PluginDebug = 0;
	};

	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returnCode = ReturnCode::Default;
		std::map<uint, ClientData> Clients;
		std::multimap<uint, PlayerBonus> PlayerBonus;
		std::map<uint, ZoneBonus> ZoneBonus;
		std::unique_ptr<Config> config = nullptr;
	};
} // namespace Plugins::MiningControl
