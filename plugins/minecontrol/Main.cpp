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
#include "Main.h"

namespace Plugins::MiningControl
{
	std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup MiningControl
	 * @brief Return true if the cargo list contains the specified good.
	 */
	static bool ContainsEquipment(std::list<CARGO_INFO> & lstCargo, uint iArchId)
	{
		for (auto& c : lstCargo)
			if (c.bMounted && c.iArchId == iArchId)
				return true;
		return false;
	}

	/** @ingroup MiningControl
	 * @brief Return the factor to modify a mining loot drop by.
	 */
	static float GetBonus(uint iRep, uint shipId, std::list<CARGO_INFO> lstCargo, uint iLootId)
	{
		if (!global->PlayerBonus.size())
			return 0.0f;

		// Get all player bonuses for this commodity.
		auto start = global->PlayerBonus.lower_bound(iLootId);
		auto end = global->PlayerBonus.upper_bound(iLootId);
		for (; start != end; start++)
		{
			// Check for matching reputation if reputation is required.
			if (start->second.RepId != -1 && iRep != start->second.RepId)
				continue;

			// Check for matching ship.
			if (find(start->second.ShipIds.begin(), start->second.ShipIds.end(), shipId) == start->second.ShipIds.end())
				continue;

			// Check that every simple item in the equipment list is present and
			// mounted.
			bool bEquipMatch = true;
			for (auto item : start->second.ItemIds)
			{
				if (!ContainsEquipment(lstCargo, item))
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
		if (!Clients[client].Setup)
		{
			if (global->config->PluginDebug > 1)
				Console::ConInfo(L"client=%d setup bonuses", client);
			Clients[client].Setup = true;

			// Get the player affiliation
			uint iRepGroupId = -1;
			if (const auto shipObj = Hk::Client::GetInspect(client); shipObj.has_value())
				shipObj.value()->get_affiliation(iRepGroupId);

			// Get the ship type
			uint shipId = Hk::Player::GetShipID(client).value();

			// Get the ship cargo so that we can check ids, guns, etc.
			int remainingHoldSize = 0;
			const auto lstCargo = Hk::Player::EnumCargo((const wchar_t*)Players.GetActiveCharacterName(client), remainingHoldSize);
			if (global->config->PluginDebug > 1)
			{
				Console::ConInfo(L"client=%d iRepGroupId=%u shipId=%u lstCargo=", client, iRepGroupId, shipId);
				for (auto& ci : lstCargo.value())
				{
					Console::ConInfo(L"%u ", ci.iArchId);
				}
				Console::ConPrint(L"");
			}

			// Check the player bonus list and if this player has the right ship and
			// equipment then record the bonus and the weapon types that can be used
			// to gather the ore.
			Clients[client].LootBonus.clear();
			Clients[client].LootAmmo.clear();
			Clients[client].LootShip.clear();
			for (auto& i : global->PlayerBonus)
			{
				uint iLootId = i.first;
				float fBonus = GetBonus(iRepGroupId, shipId, lstCargo.value(), iLootId);
				if (fBonus > 0.0f)
				{
					Clients[client].LootBonus[iLootId] = fBonus;
					Clients[client].LootAmmo[iLootId] = i.second.AmmoIds;
					Clients[client].LootShip[iLootId] = i.second.ShipIds;
					if (global->config->PluginDebug > 1)
					{
						Console::ConInfo(L"client=%d LootId=%08x Bonus=%2.2f\n", client, iLootId, fBonus);
					}
				}
			}

			const auto rights = Hk::Admin::GetAdmin((const wchar_t*)Players.GetActiveCharacterName(client));
			if (rights.has_value())
				Clients[client].Debug = global->config->PluginDebug;
		}
	}

	/** @ingroup MiningControl
	 * @brief Timer hook to update mining stats to file
	 */
	void UpdateStatsFile()
	{
		
		MiningStats stats;
		// Recharge the fields
		for (auto& i : global->ZoneBonus)
		{
			i.second.CurrentReserve += i.second.RechargeRate;
			if (i.second.CurrentReserve > i.second.MaxReserve)
				i.second.CurrentReserve = i.second.MaxReserve;

			ZoneStats zs;
			zs.CurrentReserve = i.second.CurrentReserve;
			zs.Mined = i.second.Mined;
			zs.Zone = i.second.Zone;
			stats.Stats.emplace_back(zs);
		}
		Serializer::SaveToJson(stats);
	}

	const std::vector<Timer> timers = {
		{UpdateStatsFile, 60}
	};

	/** @ingroup MiningControl
	 * @brief Clear client info when a client connects.
	 */
	void ClearClientInfo(uint & client)
	{
		Clients[client].Setup = false;
		Clients[client].LootBonus.clear();
		Clients[client].LootAmmo.clear();
		Clients[client].Debug = 0;
		Clients[client].PendingMineAsteroidEvents = 0;
		Clients[client].MineAsteroidEvents = 0;
		Clients[client].MineAsteroidSampleStart = 0;
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
		unsigned char patch1[] = {0x90, 0x90};
		WriteProcMem((char*)0x62F327E, &patch1, 2);
		WriteProcMem((char*)0x62F944E, &patch1, 2);
		WriteProcMem((char*)0x62F123E, &patch1, 2);

		auto config = Serializer::JsonToObject<Config>();

		if (config.PluginDebug)
			Console::ConInfo(L"generic_factor=%0.0f debug=%d", config.GenericFactor, config.PluginDebug);

		for (auto& pb : config.PlayerBonus)
		{
			pb.LootId = CreateID(pb.Loot.c_str());
			if (!Archetype::GetEquipment(pb.LootId) && !Archetype::GetSimple(pb.LootId))
			{
				Console::ConErr(
				    L"Item '%s' not valid", stows(pb.Loot).c_str());
				continue;
			}

			if (pb.Bonus <= 0.0f)
			{
				Console::ConErr(L"%s:%0.0f: bonus not valid", stows(pb.Loot).c_str(), pb.Bonus);
				continue;
			}

			pb.RepId = -1;
			pub::Reputation::GetReputationGroup(pb.RepId, pb.Rep.c_str());
			if (pb.RepId == -1)
			{
				Console::ConErr(L"%s: reputation not valid", stows(pb.Rep).c_str());
				continue;
			}

			for (const auto& ship : pb.Ships)
			{
				uint ShipId = CreateID(ship.c_str());
				if (!Archetype::GetShip(ShipId))
				{
					Console::ConErr(L"%s: ship not valid", stows(ship).c_str());
					continue;
				}
				pb.ShipIds.push_back(ShipId);
			}
				

			for (const auto& item : pb.Items)
			{
				uint ItemId = CreateID(item.c_str());
				if (auto equipment = Archetype::GetEquipment(ItemId);
					equipment && equipment->get_class_type() != Archetype::GUN)
					pb.ItemIds.push_back(ItemId);
				else
				{
					Console::ConErr(L"%s: item not valid", stows(item).c_str());
					continue;
				}
			}
				
			for (const auto& ammo : pb.Ammo)
			{
				uint ItemId = CreateID(ammo.c_str());
				if (auto equipment = Archetype::GetEquipment(ItemId);
					equipment && equipment->get_class_type() == Archetype::GUN)
				{
					const Archetype::Gun* gun = static_cast<Archetype::Gun*>(equipment);
					if (gun->iProjectileArchId && gun->iProjectileArchId != 0xBAADF00D && gun->iProjectileArchId != 0x3E07E70)
					{
						pb.AmmoIds.push_back(gun->iProjectileArchId);
						continue;
					}
				}
				Console::ConErr(L"%s: ammo not valid", stows(ammo).c_str());
			}
				
			global->PlayerBonus.insert(std::multimap<uint, PlayerBonus>::value_type(pb.LootId, pb));

			if (config.PluginDebug)
			{
				Console::ConInfo(L"mining player bonus LootId: %u Bonus: %2.2f RepId: %u\n",
				    pb.LootId, pb.Bonus, pb.Rep);
			}
		}

		for (auto& zb : config.ZoneBonus)
		{
			if (zb.Zone.empty())
			{
				Console::ConErr(L"%s: zone not valid", stows(zb.Zone).c_str());
				continue;
			}

			if (zb.Bonus <= 0.0f)
			{
				Console::ConErr(L"%s:%0.0f: bonus not valid", stows(zb.Zone).c_str(), zb.Bonus);
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
				Console::ConInfo(L"zone bonus %s Bonus=%2.2f "
				                 L"ReplacementLootId=%s(%u) "
				                 L"RechargeRate=%0.0f MaxReserve=%0.0f\n",
				    stows(zb.Zone).c_str(), zb.Bonus, stows(zb.ReplacementLoot).c_str(), iReplacementLootId, zb.RechargeRate, zb.MaxReserve);
			}
		}

		const auto miningStats = Serializer::JsonToObject<MiningStats>();
		for (auto& zone : miningStats.Stats)
		{
			uint ZoneId = CreateID(zone.Zone.c_str());
			if (global->ZoneBonus.find(ZoneId) != global->ZoneBonus.end())
			{
				global->ZoneBonus[ZoneId].CurrentReserve = zone.CurrentReserve;
				global->ZoneBonus[ZoneId].Mined = zone.Mined;
			}
		}

		global->config = std::make_unique<Config>(config);

		// Remove patch now that we've finished loading.
		unsigned char patch2[] = {0xFF, 0x12};
		WriteProcMem((char*)0x62F327E, &patch2, 2);
		WriteProcMem((char*)0x62F944E, &patch2, 2);
		WriteProcMem((char*)0x62F123E, &patch2, 2);

		struct PlayerData* playerData = 0;
		while (playerData = Players.traverse_active(playerData))
		{
			uint client = playerData->iOnlineId;
			ClearClientInfo(client);
		}
	}

	/** @ingroup MiningControl
	 * @brief PlayerLaunch hook. Calls ClearClientInfo.
	 */
	void PlayerLaunch(uint & ship, uint & client) { ClearClientInfo(client); }

	/** @ingroup MiningControl
	 * @brief Called when a gun hits something.
	 */
	void SPMunitionCollision(struct SSPMunitionCollisionInfo const& ci, ClientId& client)
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
						ClientData& cd = Clients[client];

						// If a non-rock is being shot we won't have an associated
						// mining event so ignore this.
						cd.PendingMineAsteroidEvents--;
						if (cd.PendingMineAsteroidEvents < 0)
						{
							cd.PendingMineAsteroidEvents = 0;
							return;
						}

						// Adjust the bonus based on the zone.
						float fZoneBonus = 0.25f;
						if (global->ZoneBonus[zone->iZoneId].Bonus != 0.0f)
							fZoneBonus = global->ZoneBonus[zone->iZoneId].Bonus;

						// If the field is getting mined out, reduce the bonus
						fZoneBonus *= global->ZoneBonus[zone->iZoneId].CurrentReserve / global->ZoneBonus[zone->iZoneId].MaxReserve;

						uint iLootId = zone->lootableZone->dynamic_loot_commodity;
						uint iCrateId = zone->lootableZone->dynamic_loot_container;

						// Change the commodity if appropriate.
						if (global->ZoneBonus[zone->iZoneId].ReplacementLootId)
							iLootId = global->ZoneBonus[zone->iZoneId].ReplacementLootId;

						// If no mining bonus entry for this commodity is found,
						// flag as no bonus
						auto ammolst = cd.LootAmmo.find(iLootId);
						bool bNoMiningCombo = false;
						if (ammolst == cd.LootAmmo.end())
						{
							bNoMiningCombo = true;
							if (cd.Debug)
								PrintUserCmdText(client, L"* Wrong ship/equip/rep");
						}
						// If this minable commodity was not hit by the right type
						// of gun, flag as no bonus
						else if (find(ammolst->second.begin(), ammolst->second.end(), ci.iProjectileArchId) == ammolst->second.end())
						{
							bNoMiningCombo = true;
							if (cd.Debug)
								PrintUserCmdText(client, L"* Wrong gun");
						}

						// If either no mining gun was used in the shot, or the
						// character isn't using a valid mining combo for this
						// commodity, set bonus to *0.5
						float fPlayerBonus = 0.5f;
						if (bNoMiningCombo)
							fPlayerBonus = 0.5f;
						else
							fPlayerBonus = cd.LootBonus[iLootId];

						// If this ship is has another ship targetted then send the
						// ore into the cargo hold of the other ship.
						uint iSendToClientId = client;
						if (!bNoMiningCombo)
						{
							auto iTargetShip = Hk::Player::GetTarget(ship);
							if (iTargetShip.has_value())
							{
								const auto iTargetClientId = Hk::Client::GetClientIdByShip(iTargetShip.value());
								if (iTargetClientId.value() && Hk::Math::Distance3DByShip(ship, iTargetShip.value()) < 1000.0f)
								{
									iSendToClientId = iTargetClientId.value();
								}
							}
						}

						// Calculate the loot drop count
						const float random = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

						// Calculate the loot drop and drop it.
						int iLootCount = (int)(random * global->config->GenericFactor * fZoneBonus * fPlayerBonus * zone->lootableZone->dynamic_loot_count2);

						// Remove this lootCount from the field
						global->ZoneBonus[zone->iZoneId].CurrentReserve -= iLootCount;
						global->ZoneBonus[zone->iZoneId].Mined += iLootCount;
						if (global->ZoneBonus[zone->iZoneId].CurrentReserve <= 0)
						{
							global->ZoneBonus[zone->iZoneId].CurrentReserve = 0;
							iLootCount = 0;
						}

						if (Clients[client].Debug)
						{
							PrintUserCmdText(client,
							    fmt::format(L"* fRand={} fGenericBonus={} fPlayerBonus={} fZoneBonus{} iLootCount={} LootId={}/{} CurrentReserve={:.1f}"), 
							    random, global->config->GenericFactor, fPlayerBonus, fZoneBonus, iLootCount, iLootId, iCrateId,
							    global->ZoneBonus[zone->iZoneId].CurrentReserve);
						}

						Clients[client].MineAsteroidEvents++;
						if (Clients[client].MineAsteroidSampleStart < time(0))
						{
							float average = Clients[client].MineAsteroidEvents / 30.0f;
							if (average > 2.0f)
							{
								std::wstring CharName = (const wchar_t*)Players.GetActiveCharacterName(client);
								AddLog(LogType::Normal, LogLevel::Info, fmt::format("High mining rate charname={} rate={:.1f}/sec location={:.1f},{:.1f},{:.1f} system={} zone={}",
								        wstos(CharName.c_str()),
								        average,
								        shipPosition.x,
								        shipPosition.y,
								        shipPosition.z,
								        zone->systemId,
								        zone->iZoneId));
							}

							Clients[client].MineAsteroidSampleStart = time(0) + 30;
							Clients[client].MineAsteroidEvents = 0;
						}

						if (iLootCount)
						{
							float fHoldRemaining;
							pub::Player::GetRemainingHoldSize(iSendToClientId, fHoldRemaining);
							if (fHoldRemaining < iLootCount)
							{
								iLootCount = (int)fHoldRemaining;
							}
							if (iLootCount == 0)
							{
								pub::Player::SendNNMessage(client, CreateID("insufficient_cargo_space"));
								return;
							}
							Hk::Player::AddCargo(iSendToClientId, iLootId, iLootCount, false);
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
	void MineAsteroid(uint & iClientSystemId, class Vector const& vPos, uint& iCrateId, uint& iLootId, uint& iCount, ClientId& client)
	{
		Clients[client].PendingMineAsteroidEvents += 4;
		global->returnCode = ReturnCode::SkipAll;
		return;
	}
}

using namespace Plugins::MiningControl;

REFL_AUTO(type(PlayerBonus), field(Loot), field(Bonus), field(Rep), field(Ships), field(Items), field(Ammo))
REFL_AUTO(type(ZoneBonus), field(Zone), field(Bonus), field(ReplacementLoot), field(RechargeRate), field(CurrentReserve), field(MaxReserve), field(Mined))
REFL_AUTO(type(ZoneStats), field(Zone), field(CurrentReserve), field(Mined))
REFL_AUTO(type(MiningStats), field(Stats))
REFL_AUTO(type(Config), field(PlayerBonus), field(ZoneBonus), field(GenericFactor), field(PluginDebug));

DefaultDllMainSettings(LoadSettingsAfterStartup)

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