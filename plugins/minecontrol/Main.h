#pragma once
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::MiningControl
{
	//! A struct that defines a mining bonus to a player if they meet certain criteria
	struct PlayerBonus : Reflectable
	{
		PlayerBonus() : LootID(0), Bonus(0.0f), RepID(-1) {}

		//! The loot commodity id this configuration applies to.
		std::string Loot = "commodity_gold";
		uint LootID;

		//! The loot bonus multiplier.
		float Bonus;

		//! The affiliation/reputation of the player
		std::string Rep = "li_n_grp";
		uint RepID;

		//! The list of ships that this bonus applies to
		std::vector<std::string> Ships = {"ge_fighter"};
		std::vector<uint> ShipIDs;

		//! The list of equipment items that the ship must carry
		std::vector<std::string> Items = {"ge_s_battery_01"};
		std::vector<uint> ItemIDs;

		//! The list of ammo arch ids for mining guns
		std::vector<std::string> Ammo = {"missile01_mark01"};
		std::vector<uint> AmmoIDs;
	};
	
	//! A struct that defines a mining bonus for a certain zone in space
	struct ZoneBonus : Reflectable
	{
		ZoneBonus() : Bonus(0.0f), ReplacementLootID(0), RechargeRate(0), CurrentReserve(100000), MaxReserve(50000), Mined(0) {}

		std::string Zone = "ExampleZone";

		//! The loot bonus multiplier.
		float Bonus;

		//! The hash of the item to replace the dropped
		std::string ReplacementLoot = "commodity_gold";
		uint ReplacementLootID;

		//! The recharge rate of the zone. This is the number of units of ore added to the reserve per minute.
		float RechargeRate;

		//! The current amount of ore in the zone. When this gets low, ore gets harder to mine. When it gets to 0, ore is impossible to mine.
		float CurrentReserve;

		//! The maximum limit for the amount of ore in the field
		float MaxReserve;

		//! The amount of ore that has been mined.
		float Mined;
	};

	//! A struct to represent each client
	struct ClientData
	{
		ClientData() : Setup(false), Debug(0), PendingMineAsteroidEvents(0), MineAsteroidEvents(0) {}

		bool Setup;
		std::map<uint, float> LootBonus;
		std::map<uint, std::vector<uint>> LootAmmo;
		std::map<uint, std::vector<uint>> LootShip;
		int Debug;

		int PendingMineAsteroidEvents;
		int MineAsteroidEvents;
		time_t MineAsteroidSampleStart;
	};

	//! A map to hold all the clients
	std::map<uint, ClientData> Clients;


	//! A struct to hold the current status of a zone so their progress persists across restarts
	struct ZoneStats : Reflectable
	{
		std::string Zone;
		float CurrentReserve;
		float Mined;
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
		std::string File() override { return "flhook_plugins/MineControl.json"; }

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
		std::multimap<uint, PlayerBonus> PlayerBonus;
		std::map<uint, ZoneBonus> ZoneBonus;
		std::unique_ptr<Config> config = nullptr;
	};
} // namespace Plugins::MiningControl
