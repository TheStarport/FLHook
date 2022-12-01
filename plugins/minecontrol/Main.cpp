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
	static bool ContainsEquipment(std::list<CARGO_INFO> & lstCargo, uint iArchID)
	{
		for (auto& c : lstCargo)
			if (c.bMounted && c.iArchID == iArchID)
				return true;
		return false;
	}

	/** @ingroup MiningControl
	 * @brief Return the factor to modify a mining loot drop by.
	 */
	static float GetBonus(uint iRep, uint iShipID, std::list<CARGO_INFO> lstCargo, uint iLootID)
	{
		if (!global->PlayerBonus.size())
			return 0.0f;

		// Get all player bonuses for this commodity.
		auto start = global->PlayerBonus.lower_bound(iLootID);
		auto end = global->PlayerBonus.upper_bound(iLootID);
		for (; start != end; start++)
		{
			// Check for matching reputation if reputation is required.
			if (start->second.RepID != -1 && iRep != start->second.RepID)
				continue;

			// Check for matching ship.
			if (find(start->second.ShipIDs.begin(), start->second.ShipIDs.end(), iShipID) == start->second.ShipIDs.end())
				continue;

			// Check that every simple item in the equipment list is present and
			// mounted.
			bool bEquipMatch = true;
			for (auto item : start->second.ItemIDs)
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
	void CheckClientSetup(uint iClientID)
	{
		if (!Clients[iClientID].Setup)
		{
			if (global->config->PluginDebug > 1)
				Console::ConInfo(L"NOTICE: iClientID=%d setup bonuses", iClientID);
			Clients[iClientID].Setup = true;

			// Get the player affiliation
			uint iRepGroupID = -1;
			IObjInspectImpl* oship = GetInspect(iClientID);
			if (oship)
				oship->get_affiliation(iRepGroupID);

			// Get the ship type
			uint iShipID;
			pub::Player::GetShipID(iClientID, iShipID);

			// Get the ship cargo so that we can check ids, guns, etc.
			std::list<CARGO_INFO> lstCargo;
			int remainingHoldSize = 0;
			EnumCargo((const wchar_t*)Players.GetActiveCharacterName(iClientID), lstCargo, remainingHoldSize);
			if (global->config->PluginDebug > 1)
			{
				Console::ConInfo(L"NOTICE: iClientID=%d iRepGroupID=%u iShipID=%u lstCargo=", iClientID, iRepGroupID, iShipID);
				for (auto& ci : lstCargo)
				{
					Console::ConInfo(L"%u ", ci.iArchID);
				}
				Console::ConPrint(L"");
			}

			// Check the player bonus list and if this player has the right ship and
			// equipment then record the bonus and the weapon types that can be used
			// to gather the ore.
			Clients[iClientID].LootBonus.clear();
			Clients[iClientID].LootAmmo.clear();
			Clients[iClientID].LootShip.clear();
			for (auto& i : global->PlayerBonus)
			{
				uint iLootID = i.first;
				float fBonus = GetBonus(iRepGroupID, iShipID, lstCargo, iLootID);
				if (fBonus > 0.0f)
				{
					Clients[iClientID].LootBonus[iLootID] = fBonus;
					Clients[iClientID].LootAmmo[iLootID] = i.second.AmmoIDs;
					Clients[iClientID].LootShip[iLootID] = i.second.ShipIDs;
					if (global->config->PluginDebug > 1)
					{
						Console::ConInfo(L"NOTICE: iClientID=%d LootID=%08x Bonus=%2.2f\n", iClientID, iLootID, fBonus);
					}
				}
			}

			std::wstring wscRights;
			GetAdmin((const wchar_t*)Players.GetActiveCharacterName(iClientID), wscRights);
			if (wscRights.size())
				Clients[iClientID].Debug = global->config->PluginDebug;
		}
	}

	/** @ingroup MiningControl
	 * @brief Timer hook to update mining stats to file
	 */
	void TimerCheckKick()
	{
		// Perform 60 second tasks.
		if ((time(0) % 60) == 0)
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
	}

	/** @ingroup MiningControl
	 * @brief Clear client info when a client connects.
	 */
	void ClearClientInfo(uint & iClientID)
	{
		Clients[iClientID].Setup = false;
		Clients[iClientID].LootBonus.clear();
		Clients[iClientID].LootAmmo.clear();
		Clients[iClientID].Debug = 0;
		Clients[iClientID].PendingMineAsteroidEvents = 0;
		Clients[iClientID].MineAsteroidEvents = 0;
		Clients[iClientID].MineAsteroidSampleStart = 0;
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
			Console::ConInfo(L"NOTICE: generic_factor=%0.0f debug=%d", config.GenericFactor, config.PluginDebug);

		for (auto& pb : config.PlayerBonus)
		{
			pb.LootID = CreateID(pb.Loot.c_str());
			if (!Archetype::GetEquipment(pb.LootID) && !Archetype::GetSimple(pb.LootID))
			{
				Console::ConErr(
				    L"Item '%s' not valid", stows(pb.Loot).c_str());
				continue;
			}

			if (pb.Bonus <= 0.0f)
			{
				Console::ConErr(L"ERROR: %s:%0.0f: bonus not valid", stows(pb.Loot).c_str(), pb.Bonus);
				continue;
			}

			pb.RepID = -1;
			pub::Reputation::GetReputationGroup(pb.RepID, pb.Rep.c_str());
			if (pb.RepID == -1)
			{
				Console::ConErr(L"ERROR: %s: reputation not valid", stows(pb.Rep).c_str());
				continue;
			}

			for (auto& ship : pb.Ships)
			{
				uint ShipID = CreateID(ship.c_str());
				if (!Archetype::GetShip(ShipID))
				{
					Console::ConErr(L"ERROR: %s: ship not valid", stows(ship).c_str());
					continue;
				}
				pb.ShipIDs.push_back(ShipID);
			}
				

			for (auto& item : pb.Items)
			{
				uint ItemID = CreateID(item.c_str());
				Archetype::Equipment* eq = Archetype::GetEquipment(ItemID);
				if (Archetype::GetSimple(ItemID) && eq->get_class_type() != Archetype::GUN)
					pb.ItemIDs.push_back(ItemID);
				else
				{
					Console::ConErr(L"ERROR: %s: item not valid", stows(item).c_str());
					continue;
				}
			}
				

			for (auto& ammo : pb.Ammo)
			{
				uint ItemID = CreateID(ammo.c_str());
				Archetype::Equipment* eq = Archetype::GetEquipment(ItemID);
				if (eq->get_class_type() == Archetype::GUN)
				{
					Archetype::Gun* gun = (Archetype::Gun*)eq;
					if (gun->iProjectileArchID && gun->iProjectileArchID != 0xBAADF00D && gun->iProjectileArchID != 0x3E07E70)
					{
						pb.AmmoIDs.push_back(gun->iProjectileArchID);
						continue;
					}
				}
				Console::ConErr(L"ERROR: %s: ammo not valid", stows(ammo).c_str());
			}
				
			global->PlayerBonus.insert(std::multimap<uint, PlayerBonus>::value_type(pb.LootID, pb));
			if (global->config->PluginDebug)
			{
				Console::ConInfo(L"NOTICE: mining player bonus LootID: %u Bonus: %2.2f RepID: %u\n",
				    pb.LootID, pb.Bonus, pb.Rep);
			}
		}

		for (auto& zb : config.ZoneBonus)
		{
			if (zb.Zone.empty())
			{
				Console::ConErr(L"ERROR: %s: zone not valid", stows(zb.Zone).c_str());
				continue;
			}

			if (zb.Bonus <= 0.0f)
			{
				Console::ConErr(L"ERROR: %s:%0.0f: bonus not valid", stows(zb.Zone).c_str(), zb.Bonus);
				continue;
			}

			uint iReplacementLootID = 0;
			if (!zb.ReplacementLoot.empty())
				zb.ReplacementLootID = CreateID(zb.ReplacementLoot.c_str());

			if (zb.RechargeRate <= 0.0f)
				zb.RechargeRate = 50;

			if (zb.MaxReserve <= 0.0f)
				zb.MaxReserve = 100000;

			global->ZoneBonus[CreateID(zb.Zone.c_str())] = zb;

			if (config.PluginDebug)
			{
				Console::ConInfo(L"NOTICE: zone bonus %s Bonus=%2.2f "
				                 L"ReplacementLootID=%s(%u) "
				                 L"RechargeRate=%0.0f MaxReserve=%0.0f\n",
				    stows(zb.Zone).c_str(), zb.Bonus, stows(zb.ReplacementLoot).c_str(), iReplacementLootID, zb.RechargeRate, zb.MaxReserve);
			}
		}

		const auto miningStats = Serializer::JsonToObject<MiningStats>();
		for (auto& zone : miningStats.Stats)
		{
			uint ZoneID = CreateID(zone.Zone.c_str());
			if (global->ZoneBonus.find(ZoneID) != global->ZoneBonus.end())
			{
				global->ZoneBonus[ZoneID].CurrentReserve = zone.CurrentReserve;
				global->ZoneBonus[ZoneID].Mined = zone.Mined;
			}
		}

		global->config = std::make_unique<Config>(config);

		// Remove patch now that we've finished loading.
		unsigned char patch2[] = {0xFF, 0x12};
		WriteProcMem((char*)0x62F327E, &patch2, 2);
		WriteProcMem((char*)0x62F944E, &patch2, 2);
		WriteProcMem((char*)0x62F123E, &patch2, 2);

		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			uint iClientID = GetClientIdFromPD(pPD);
			ClearClientInfo(iClientID);
		}
	}

	/** @ingroup MiningControl
	 * @brief PlayerLaunch hook. Calls ClearClientInfo.
	 */
	void __stdcall PlayerLaunch(uint & iShip, uint & iClientID) { ClearClientInfo(iClientID); }

	/** @ingroup MiningControl
	 * @brief Called when a gun hits something.
	 */
	void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const& ci, uint& iClientID)
	{
		// If this is not a lootable rock, do no other processing.
		if (ci.dwTargetShip != 0)
			return;

		global->returnCode = ReturnCode::SkipAll;

		// Initialise the mining setup for this client if it hasn't been done
		// already.
		CheckClientSetup(iClientID);

		uint iShip;
		pub::Player::GetShip(iClientID, iShip);

		Vector vPos;
		Matrix mRot;
		pub::SpaceObj::GetLocation(iShip, vPos, mRot);

		uint iClientSystemID;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		CmnAsteroid::CAsteroidSystem* csys = CmnAsteroid::Find(iClientSystemID);
		if (csys)
		{
			// Find asteroid field that matches the best.
			for (CmnAsteroid::CAsteroidField* cfield = csys->FindFirst(); cfield; cfield = csys->FindNext())
			{
				try
				{
					const Universe::IZone* zone = cfield->get_lootable_zone(vPos);
					if (cfield->near_field(vPos) && zone && zone->lootableZone)
					{
						ClientData& cd = Clients[iClientID];

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
						if (global->ZoneBonus[zone->iZoneID].Bonus)
							fZoneBonus = global->ZoneBonus[zone->iZoneID].Bonus;

						// If the field is getting mined out, reduce the bonus
						fZoneBonus *= global->ZoneBonus[zone->iZoneID].CurrentReserve / global->ZoneBonus[zone->iZoneID].MaxReserve;

						uint iLootID = zone->lootableZone->dynamic_loot_commodity;
						uint iCrateID = zone->lootableZone->dynamic_loot_container;

						// Change the commodity if appropriate.
						if (global->ZoneBonus[zone->iZoneID].ReplacementLootID)
							iLootID = global->ZoneBonus[zone->iZoneID].ReplacementLootID;

						// If no mining bonus entry for this commodity is found,
						// flag as no bonus
						auto ammolst = cd.LootAmmo.find(iLootID);
						bool bNoMiningCombo = false;
						if (ammolst == cd.LootAmmo.end())
						{
							bNoMiningCombo = true;
							if (cd.Debug)
								PrintUserCmdText(iClientID, L"* Wrong ship/equip/rep");
						}
						// If this minable commodity was not hit by the right type
						// of gun, flag as no bonus
						else if (find(ammolst->second.begin(), ammolst->second.end(), ci.iProjectileArchID) == ammolst->second.end())
						{
							bNoMiningCombo = true;
							if (cd.Debug)
								PrintUserCmdText(iClientID, L"* Wrong gun");
						}

						// If either no mining gun was used in the shot, or the
						// character isn't using a valid mining combo for this
						// commodity, set bonus to *0.5
						float fPlayerBonus = 0.5f;
						if (bNoMiningCombo)
							fPlayerBonus = 0.5f;
						else
							fPlayerBonus = cd.LootBonus[iLootID];

						// If this ship is has another ship targetted then send the
						// ore into the cargo hold of the other ship.
						uint iSendToClientID = iClientID;
						if (!bNoMiningCombo)
						{
							uint iTargetShip;
							pub::SpaceObj::GetTarget(iShip, iTargetShip);
							if (iTargetShip)
							{
								uint iTargetClientID = GetClientIDByShip(iTargetShip);
								if (iTargetClientID)
								{
									if (Distance3DByShip(iShip, iTargetShip) < 1000.0f)
									{
										iSendToClientID = iTargetClientID;
									}
								}
							}
						}

						// Calculate the loot drop count
						float fRand = (float)rand() / (float)RAND_MAX;

						// Calculate the loot drop and drop it.
						int iLootCount = (int)(fRand * global->config->GenericFactor * fZoneBonus * fPlayerBonus * zone->lootableZone->dynamic_loot_count2);

						// Remove this lootCount from the field
						global->ZoneBonus[zone->iZoneID].CurrentReserve -= iLootCount;
						global->ZoneBonus[zone->iZoneID].Mined += iLootCount;
						if (global->ZoneBonus[zone->iZoneID].CurrentReserve <= 0)
						{
							global->ZoneBonus[zone->iZoneID].CurrentReserve = 0;
							iLootCount = 0;
						}

						if (Clients[iClientID].Debug)
						{
							PrintUserCmdText(iClientID,
							    L"* fRand=%2.2f fGenericBonus=%2.2f "
							    L"fPlayerBonus=%2.2f fZoneBonus=%2.2f "
							    L"iLootCount=%d LootID=%u/%u CurrentReserve=%0.0f",
							    fRand, global->config->GenericFactor, fPlayerBonus, fZoneBonus, iLootCount, iLootID, iCrateID,
							    global->ZoneBonus[zone->iZoneID].CurrentReserve);
						}

						Clients[iClientID].MineAsteroidEvents++;
						if (Clients[iClientID].MineAsteroidSampleStart < time(0))
						{
							float average = Clients[iClientID].MineAsteroidEvents / 30.0f;
							if (average > 2.0f)
							{
								std::wstring CharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
								AddLog(LogType::Normal, LogLevel::Info,
								    L"High mining rate charname=%s "
								    "rate=%0.1f/sec "
								    "location=%0.0f,%0.0f,%0.0f system=%08x "
								    "zone=%08x",
								    CharName.c_str(), average, vPos.x, vPos.y, vPos.z, zone->iSystemID, zone->iZoneID);
							}

							Clients[iClientID].MineAsteroidSampleStart = time(0) + 30;
							Clients[iClientID].MineAsteroidEvents = 0;
						}

						if (iLootCount)
						{
							float fHoldRemaining;
							pub::Player::GetRemainingHoldSize(iSendToClientID, fHoldRemaining);
							if (fHoldRemaining < iLootCount)
							{
								iLootCount = (int)fHoldRemaining;
							}
							if (iLootCount == 0)
							{
								pub::Player::SendNNMessage(iClientID, CreateID("insufficient_cargo_space"));
								return;
							}
							pub::Player::AddCargo(iSendToClientID, iLootID, iLootCount, 1.0, false);
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
	void __stdcall MineAsteroid(uint & iClientSystemID, class Vector const& vPos, uint& iCrateID, uint& iLootID, uint& iCount, uint& iClientID)
	{
		Clients[iClientID].PendingMineAsteroidEvents += 4;
		return;
	}

	/** @ingroup MiningControl
	 * @brief Admin command processing.
	 */
	bool ExecuteCommandString(CCmds * cmd, const std::wstring& wscCmd)
	{
		if (wscCmd == L"printminezones")
		{
			global->returnCode = ReturnCode::SkipAll;
			ZoneUtilities::PrintZones();
			return true;
		}

		return false;
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
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Startup, &LoadSettingsAfterStartup, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__MineAsteroid, &MineAsteroid);
	pi->emplaceHook(HookedCall::IServerImpl__SPMunitionCollision, &SPMunitionCollision);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &TimerCheckKick);
}