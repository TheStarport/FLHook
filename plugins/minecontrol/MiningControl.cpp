/**
 * @date Feb, 2010
 * @author Cannon (Ported by Raikkonen)
 * @defgroup MiningControl Mining Control
 * @brief
 * Adds bonuses to mining.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * There are no players commands in this plugin.
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - printminezones - Prints all the configured mining zones.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "GenericFactor": 1.0,
 *     "PlayerBonus": [
 *         {
 *             "Ammo": [
 *                 "missile01_mark01"
 *             ],
 *             "Bonus": 0.0,
 *             "Items": [
 *                 "ge_s_battery_01"
 *             ],
 *             "Loot": "commodity_gold",
 *             "Rep": "",
 *             "Ships": [
 *                 "ge_fighter"
 *             ]
 *         }
 *     ],
 *     "PluginDebug": 0,
 *     "ZoneBonus": [
 *         {
 *             "Bonus": 0.0,
 *             "CurrentReserve": 100000.0,
 *             "MaxReserve": 50000.0,
 *             "Mined": 0.0,
 *             "RechargeRate": 0.0,
 *             "ReplacementLoot": "commodity_gold",
 *             "Zone": "ExampleZone"
 *         }
 *     ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */
#include "MiningControl.h"

namespace Plugins::MiningControl
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup MiningControl
	 * @brief Return true if the cargo list contains the specified good.
	 */
	static bool ContainsEquipment(const std::list<CargoInfo>& cargoList, uint archId)
	{
		return std::ranges::any_of(cargoList, [archId](const CargoInfo& c) { return c.mounted && c.archId == archId; });
	}

	/** @ingroup MiningControl
	 * @brief Return the factor to modify a mining loot drop by.
	 */
	static float GetBonus(uint reputation, uint shipId, const std::list<CargoInfo>& cargoList, uint lootId)
	{
		if (!global->PlayerBonus.size())
			return 0.0f;

		// Get all player bonuses for this commodity.
		auto start = global->PlayerBonus.lower_bound(lootId);
		const auto end = global->PlayerBonus.upper_bound(lootId);
		for (; start != end; start++)
		{
			// Check for matching reputation if reputation is required.
			if (start->second.RepId != -1 && reputation != start->second.RepId)
				continue;

			// Check for matching ship.
			if (std::ranges::find(start->second.ShipIds, shipId) == start->second.ShipIds.end())
				continue;

			// Check that every simple item in the equipment list is present and
			// mounted.
			bool bEquipMatch = true;
			for (const auto item : start->second.ItemIds)
			{
				if (!ContainsEquipment(cargoList, item))
				{
					bEquipMatch = false;
					break;
				}
			}

			// This is a match.
			if (bEquipMatch)
				return start->second.Bonus;
		}

		return 0.0f;
	}

	/** @ingroup MiningControl
	 * @brief Check if the client qualifies for bonuses
	 */
	void CheckClientSetup(ClientId client)
	{
		if (!global->Clients[client].Setup)
		{
			if (global->config->PluginDebug > 1)
				Logger::i()->Log(LogLevel::Info, std::format("client={} setup bonuses", client));
			global->Clients[client].Setup = true;

			// Get the player affiliation
			uint repGroupId = -1;
			if (const auto shipObj = Hk::Client::GetInspect(client); shipObj.has_value())
				shipObj.value()->get_affiliation(repGroupId);

			// Get the ship type
			uint shipId = Hk::Player::GetShipID(client).value();

			// Get the ship cargo so that we can check ids, guns, etc.
			int remainingHoldSize = 0;
			const auto Cargo = Hk::Player::EnumCargo((const wchar_t*)Players.GetActiveCharacterName(client), remainingHoldSize);
			if (Cargo.has_error())
			{
				return;
			}
			if (global->config->PluginDebug > 1)
			{
				Logger::i()->Log(LogLevel::Info, std::format("client={} iRepGroupId={} shipId={} Cargo=", client, repGroupId, shipId));
				for (auto& ci : Cargo.value())
				{
					Logger::i()->Log(LogLevel::Info, std::format("{} ", ci.archId));
				}
			}

			// Check the player bonus list and if this player has the right ship and
			// equipment then record the bonus and the weapon types that can be used
			// to gather the ore.
			global->Clients[client].LootBonus.clear();
			global->Clients[client].LootAmmo.clear();
			global->Clients[client].LootShip.clear();
			for (const auto& [lootId, playerBonus] : global->PlayerBonus)
			{
				float bonus = GetBonus(repGroupId, shipId, Cargo.value(), lootId);
				if (bonus > 0.0f)
				{
					global->Clients[client].LootBonus[lootId] = bonus;
					global->Clients[client].LootAmmo[lootId] = playerBonus.AmmoIds;
					global->Clients[client].LootShip[lootId] = playerBonus.ShipIds;
					if (global->config->PluginDebug > 1)
					{
						Logger::i()->Log(LogLevel::Info, std::format("client={} LootId={} Bonus={}\n", client, lootId, bonus));
					}
				}
			}

			const auto rights = Hk::Admin::GetAdmin((const wchar_t*)Players.GetActiveCharacterName(client));
			if (rights.has_value())
				global->Clients[client].Debug = global->config->PluginDebug;
		}
	}

	/** @ingroup MiningControl
	 * @brief Timer hook to update mining stats to file
	 */
	void UpdateStatsFile()
	{
		MiningStats stats;
		// Recharge the fields
		for (auto& [_, zoneBonus] : global->ZoneBonus)
		{
			zoneBonus.CurrentReserve += zoneBonus.RechargeRate;
			if (zoneBonus.CurrentReserve > zoneBonus.MaxReserve)
				zoneBonus.CurrentReserve = zoneBonus.MaxReserve;

			ZoneStats zs;
			zs.CurrentReserve = zoneBonus.CurrentReserve;
			zs.Mined = zoneBonus.Mined;
			zs.Zone = zoneBonus.Zone;
			stats.Stats.emplace_back(zs);
		}
		Serializer::SaveToJson(stats);
	}

	const std::vector<Timer> timers = {{UpdateStatsFile, 60}};

	/** @ingroup MiningControl
	 * @brief Clear client info when a client connects.
	 */
	void ClearClientInfo(const uint& client)
	{
		global->Clients[client].Setup = false;
		global->Clients[client].LootBonus.clear();
		global->Clients[client].LootAmmo.clear();
		global->Clients[client].Debug = 0;
		global->Clients[client].MineAsteroidEvents = 0;
		global->Clients[client].MineAsteroidSampleStart = 0;
	}

	/** @ingroup MiningControl
	 * @brief Load the configuration
	 */
	void LoadSettingsAfterStartup()
	{
		global->ZoneBonus.clear();
		global->PlayerBonus.clear();

		// Patch Archetype::GetEquipment & Archetype::GetShip to suppress annoying
		// warnings flserver-errors.log
		const unsigned char patch1[] = {0x90, 0x90};
		WriteProcMem((char*)0x62F327E, &patch1, 2);
		WriteProcMem((char*)0x62F944E, &patch1, 2);
		WriteProcMem((char*)0x62F123E, &patch1, 2);

		auto config = Serializer::JsonToObject<Config>();

		if (config.PluginDebug)
			Logger::i()->Log(LogLevel::Info, std::format("generic_factor={:.2f} debug={}", config.GenericFactor, config.PluginDebug));

		for (auto& pb : config.PlayerBonus)
		{
			pb.LootId = CreateID(pb.Loot.c_str());
			if (!Archetype::GetEquipment(pb.LootId) && !Archetype::GetSimple(pb.LootId))
			{
				Logger::i()->Log(LogLevel::Err, std::format("Item '{}' not valid", pb.Loot));
				continue;
			}

			if (pb.Bonus <= 0.0f)
			{
				Logger::i()->Log(LogLevel::Err, std::format("{}:{:5f}: bonus not valid", pb.Loot, pb.Bonus));
				continue;
			}

			pb.RepId = -1;
			pub::Reputation::GetReputationGroup(pb.RepId, pb.Rep.c_str());
			if (pb.RepId == UINT_MAX)
			{
				Logger::i()->Log(LogLevel::Err, std::format("{}: reputation not valid", pb.Rep));
				continue;
			}

			for (const auto& ship : pb.Ships)
			{
				uint ShipId = CreateID(ship.c_str());
				if (!Archetype::GetShip(ShipId))
				{
					Logger::i()->Log(LogLevel::Err, std::format("{}: ship not valid", ship));
					continue;
				}
				pb.ShipIds.push_back(ShipId);
			}

			for (const auto& item : pb.Items)
			{
				uint ItemId = CreateID(item.c_str());
				if (const auto equipment = Archetype::GetEquipment(ItemId); equipment && equipment->get_class_type() != Archetype::GUN)
					pb.ItemIds.push_back(ItemId);
				else
				{
					Logger::i()->Log(LogLevel::Err, std::format("{}: item not valid", item));
					continue;
				}
			}

			for (const auto& ammo : pb.Ammo)
			{
				const uint ItemId = CreateID(ammo.c_str());
				if (const auto equipment = Archetype::GetEquipment(ItemId); equipment && equipment->get_class_type() == Archetype::GUN)
				{
					const Archetype::Gun* gun = static_cast<Archetype::Gun*>(equipment);
					if (gun->iProjectileArchId && gun->iProjectileArchId != 0xBAADF00D && gun->iProjectileArchId != 0x3E07E70)
					{
						pb.AmmoIds.push_back(gun->iProjectileArchId);
						continue;
					}
				}
				Logger::i()->Log(LogLevel::Err, std::format("{}: ammo not valid", ammo));
			}

			global->PlayerBonus.insert(std::multimap<uint, PlayerBonus>::value_type(pb.LootId, pb));

			if (config.PluginDebug)
			{
				Logger::i()->Log(LogLevel::Info, std::format("mining player bonus LootId: {} Bonus: {:.2f} RepId: {}\n", pb.LootId, pb.Bonus, pb.Rep));
			}
		}

		for (auto& zb : config.ZoneBonus)
		{
			if (zb.Zone.empty())
			{
				Logger::i()->Log(LogLevel::Err, std::format("{}: zone not valid", zb.Zone));
				continue;
			}

			if (zb.Bonus <= 0.0f)
			{
				Logger::i()->Log(LogLevel::Err, std::format("{}:{:.2f}: bonus not valid", zb.Zone, zb.Bonus));
				continue;
			}

			uint iReplacementLootId = 0;
			if (!zb.ReplacementLoot.empty())
				zb.ReplacementLootId = CreateID(zb.ReplacementLoot.c_str());

			if (zb.RechargeRate <= 0.0f)
				zb.RechargeRate = 50;

			if (zb.MaxReserve <= 0.0f)
				zb.MaxReserve = 100000;

			global->ZoneBonus[CreateID(zb.Zone.c_str())] = zb;

			if (config.PluginDebug)
			{
				Logger::i()->Log(LogLevel::Info, std::format("zone bonus {} Bonus={:5f} ReplacementLootId={}({}) RechargeRate={:.2f} MaxReserve={:.2f}\n",
				    zb.Zone,
				    zb.Bonus,
				    zb.ReplacementLoot,
				    iReplacementLootId,
				    zb.RechargeRate,
				    zb.MaxReserve));
			}
		}

		for (const auto miningStats = Serializer::JsonToObject<MiningStats>(); auto& zone : miningStats.Stats)
		{
			uint ZoneId = CreateID(zone.Zone.c_str());
			if (global->ZoneBonus.contains(ZoneId))
			{
				global->ZoneBonus[ZoneId].CurrentReserve = zone.CurrentReserve;
				global->ZoneBonus[ZoneId].Mined = zone.Mined;
			}
		}

		global->config = std::make_unique<Config>(config);

		// Remove patch now that we've finished loading.
		const unsigned char patch2[] = {0xFF, 0x12};
		WriteProcMem((char*)0x62F327E, &patch2, 2);
		WriteProcMem((char*)0x62F944E, &patch2, 2);
		WriteProcMem((char*)0x62F123E, &patch2, 2);

		PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			uint client = playerData->iOnlineId;
			ClearClientInfo(client);
		}
	}

	/** @ingroup MiningControl
	 * @brief PlayerLaunch hook. Calls ClearClientInfo.
	 */
	void PlayerLaunch([[maybe_unused]] ShipId& ship, ClientId& client)
	{
		ClearClientInfo(client);
	}

	/** @ingroup MiningControl
	 * @brief Called when a gun hits something.
	 */
	void SPMunitionCollision(SSPMunitionCollisionInfo const& ci, ClientId& client)
	{
		// If this is not a lootable rock, do no other processing.
		if (ci.dwTargetShip != 0)
			return;

		global->returnCode = ReturnCode::SkipAll;

		// Initialise the mining setup for this client if it hasn't been done
		// already.
		CheckClientSetup(client);

		uint ship = Hk::Player::GetShip(client).value();

		auto [shipPosition, _] = Hk::Solar::GetLocation(ship, IdType::Ship).value();

		SystemId iClientSystemId = Hk::Player::GetSystem(client).value();
		CmnAsteroid::CAsteroidSystem* csys = CmnAsteroid::Find(iClientSystemId);
		if (csys)
		{
			// Find asteroid field that matches the best.
			for (CmnAsteroid::CAsteroidField* cfield = csys->FindFirst(); cfield; cfield = csys->FindNext())
			{
				try
				{
					const Universe::IZone* zone = cfield->get_lootable_zone(shipPosition);
					if (cfield->near_field(shipPosition) && zone && zone->lootableZone)
					{
						ClientData& cd = global->Clients[client];

						// Adjust the bonus based on the zone.
						float zoneBonus = 0.25f;
						if (global->ZoneBonus[zone->iZoneId].Bonus != 0.0f)
							zoneBonus = global->ZoneBonus[zone->iZoneId].Bonus;

						// If the field is getting mined out, reduce the bonus
						zoneBonus *= global->ZoneBonus[zone->iZoneId].CurrentReserve / global->ZoneBonus[zone->iZoneId].MaxReserve;

						uint lootId = zone->lootableZone->dynamicLootCommodity;
						uint crateId = zone->lootableZone->dynamicLootContainer;

						// Change the commodity if appropriate.
						if (global->ZoneBonus[zone->iZoneId].ReplacementLootId)
							lootId = global->ZoneBonus[zone->iZoneId].ReplacementLootId;

						// If no mining bonus entry for this commodity is found,
						// flag as no bonus
						auto ammo = cd.LootAmmo.find(lootId);
						bool miningBonusEligible = true;
						if (ammo == cd.LootAmmo.end())
						{
							miningBonusEligible = false;
							if (cd.Debug)
								PrintUserCmdText(client, L"* Wrong ship/equip/rep");
						}
						// If this minable commodity was not hit by the right type
						// of gun, flag as no bonus
						else if (std::ranges::find(ammo->second, ci.iProjectileArchId) == ammo->second.end())
						{
							miningBonusEligible = false;
							if (cd.Debug)
								PrintUserCmdText(client, L"* Wrong gun");
						}

						// If either no mining gun was used in the shot, or the
						// character isn't using a valid mining combo for this
						// commodity, set bonus to *0.5
						float fPlayerBonus = 0.5f;
						if (miningBonusEligible)
							fPlayerBonus = cd.LootBonus[lootId];

						// If this ship is has another ship targetted then send the
						// ore into the cargo hold of the other ship.
						uint sendToClientId = client;
						if (!miningBonusEligible)
						{
							auto targetShip = Hk::Player::GetTarget(ship);
							if (targetShip.has_value())
							{
								const auto targetClientId = Hk::Client::GetClientIdByShip(targetShip.value());
								if (targetClientId.value() && Hk::Math::Distance3DByShip(ship, targetShip.value()) < 1000.0f)
								{
									sendToClientId = targetClientId.value();
								}
							}
						}

						// Calculate the loot drop count
						const float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						// Calculate the loot drop and drop it.
						auto lootCount = static_cast<int>(
						    random * global->config->GenericFactor * zoneBonus * fPlayerBonus * static_cast<float>(zone->lootableZone->dynamicLootCount2));

						// Remove this lootCount from the field
						global->ZoneBonus[zone->iZoneId].CurrentReserve -= static_cast<float>(lootCount);
						global->ZoneBonus[zone->iZoneId].Mined += static_cast<float>(lootCount);
						if (global->ZoneBonus[zone->iZoneId].CurrentReserve <= 0)
						{
							global->ZoneBonus[zone->iZoneId].CurrentReserve = 0;
							lootCount = 0;
						}

						if (global->Clients[client].Debug)
						{
							PrintUserCmdText(client,
							    std::format(L"* fRand={} fGenericBonus={} fPlayerBonus={} fZoneBonus{} iLootCount={} LootId={}/{} CurrentReserve={:.1f}",
							        random,
							        global->config->GenericFactor,
							        fPlayerBonus,
							        zoneBonus,
							        lootCount,
							        lootId,
							        crateId,
							        global->ZoneBonus[zone->iZoneId].CurrentReserve));
						}

						global->Clients[client].MineAsteroidEvents++;
						if (global->Clients[client].MineAsteroidSampleStart < time(0))
						{
							if (float average = static_cast<float>(global->Clients[client].MineAsteroidEvents) / 30.0f; average > 2.0f)
							{
								std::wstring CharName = (const wchar_t*)Players.GetActiveCharacterName(client);
								AddLog(LogType::Normal,
								    LogLevel::Info,
								    std::format("High mining rate charname={} rate={:.1f}/sec location={:.1f},{:.1f},{:.1f} system={} zone={}",
								        wstos(CharName.c_str()),
								        average,
								        shipPosition.x,
								        shipPosition.y,
								        shipPosition.z,
								        zone->systemId,
								        zone->iZoneId));
							}

							global->Clients[client].MineAsteroidSampleStart = time(0) + 30;
							global->Clients[client].MineAsteroidEvents = 0;
						}

						if (lootCount)
						{
							float fHoldRemaining;
							pub::Player::GetRemainingHoldSize(sendToClientId, fHoldRemaining);
							if (fHoldRemaining < static_cast<float>(lootCount))
							{
								lootCount = (int)fHoldRemaining;
							}
							if (lootCount == 0)
							{
								pub::Player::SendNNMessage(client, CreateID("insufficient_cargo_space"));
								return;
							}
							Hk::Player::AddCargo(sendToClientId, lootId, lootCount, false);
						}
						return;
					}
				}
				catch (...)
				{
				}
			}
		}
	}

	/** @ingroup MiningControl
	 * @brief Called when an asteriod is mined. We ignore all of the parameters from the client.
	 */
	void MineAsteroid([[maybe_unused]] SystemId& clientSystemId, [[maybe_unused]] const Vector& pos, [[maybe_unused]] const uint& crateId,
	    [[maybe_unused]] const uint& lootId, [[maybe_unused]] const uint& count, [[maybe_unused]] ClientId& client)
	{
		global->returnCode = ReturnCode::SkipAll;
		return;
	}
} // namespace Plugins::MiningControl

using namespace Plugins::MiningControl;

REFL_AUTO(type(PlayerBonus), field(Loot), field(Bonus), field(Rep), field(Ships), field(Items), field(Ammo))
REFL_AUTO(type(ZoneBonus), field(Zone), field(Bonus), field(ReplacementLoot), field(RechargeRate), field(CurrentReserve), field(MaxReserve), field(Mined))
REFL_AUTO(type(ZoneStats), field(Zone), field(CurrentReserve), field(Mined))
REFL_AUTO(type(MiningStats), field(Stats))
REFL_AUTO(type(Config), field(PlayerBonus), field(ZoneBonus), field(GenericFactor), field(PluginDebug));

DefaultDllMainSettings(LoadSettingsAfterStartup);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Mine Control");
	pi->shortName("minecontrol");
	pi->mayUnload(true);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Startup, &LoadSettingsAfterStartup, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__MineAsteroid, &MineAsteroid);
	pi->emplaceHook(HookedCall::IServerImpl__SPMunitionCollision, &SPMunitionCollision);
}