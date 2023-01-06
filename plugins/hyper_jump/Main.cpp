// Hyperjump - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
// WARNING: The player "Hyperjump" aspects of this plugin only work with the Discovery Client Hook. The admin beam/pull
// etc. commands work fine on Vanilla though.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "main.h"

// Check that the item is a ship, cargo or equipment item is valid
static uint CreateValidID(const char* nickname)
{
	uint item = CreateID(nickname);

	if (!Archetype::GetEquipment(item) && !Archetype::GetSimple(item) && !Archetype::GetShip(item))
	{
		Console::ConErr(L"ERROR: item '%s' is not valid", stows(nickname).c_str());
	}

	return item;
}

static std::wstring FormatCoords(char* ibuf)
{
	std::wstring sbuf;
	wchar_t buf[100];
	for (int i = 0; i < HCOORD_SIZE; i++)
	{
		if (i != 0 && (i % 4) == 0)
			sbuf += L"-";
		_snwprintf(buf, sizeof(buf), L"%02X", (uchar)ibuf[i]);
		sbuf += buf;
	}
	return sbuf;
}

static void EncryptDecrypt(char* ibuf, char* obuf)
{
	obuf[0] = ibuf[0];
	obuf[1] = ibuf[1];
	for (uint i = 2, p = ibuf[0] % set_scEncryptKey.length(); i < HCOORD_SIZE; i++, p++)
	{
		if (p > set_scEncryptKey.length())
			p = 0;
		obuf[i] = ibuf[i] ^ set_scEncryptKey[p];
	}
}

void SwitchSystem(ClientId client, uint system, Vector pos, Matrix ornt)
{
	mapDeferredJumps[client].system = system;
	mapDeferredJumps[client].pos = pos;
	mapDeferredJumps[client].ornt = ornt;

	// Force a launch to put the ship in the right location in the current
	// system so that when the change system command arrives (hopefully) a
	// fraction of a second later the ship will appear at the right location.
	RelocateClient(client, pos, ornt);
	// Send the jump command to the client. The client will send a system switch
	// out complete event which we intercept to set the new starting positions.
	PrintUserCmdText(client, L" ChangeSys %u", system);
}

void LoadSettings(const std::string& scPluginCfgFile)
{
	// Patch Archetype::GetEquipment & Archetype::GetShip to suppress annoying
	// warnings flserver-errors.log
	unsigned char patch1[] = { 0x90, 0x90 };
	WriteProcMem((char*)0x62F327E, &patch1, 2);
	WriteProcMem((char*)0x62F944E, &patch1, 2);
	WriteProcMem((char*)0x62F123E, &patch1, 2);

	set_scEncryptKey = Players.GetServerSig();

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scCfgFile = std::string(szCurDir) + "\\flhook_plugins\\jump.cfg";

	INI_Reader ini;
	if (ini.open(scCfgFile.c_str(), false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("general"))
			{
				JUMPDRIVE_ARCH jd;
				while (ini.read_value())
				{
					if (ini.is_value("death_system"))
					{
						set_death_systems[CreateID(ini.get_value_string(0))] = true;
					}
				}
			}
			else if (ini.is_header("jumpdrive"))
			{
				JUMPDRIVE_ARCH jd;
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
					{
						jd.nickname = CreateValidID(ini.get_value_string(0));
					}
					else if (ini.is_value("can_jump_charge"))
					{
						jd.can_jump_charge = ini.get_value_float(0);
					}
					else if (ini.is_value("charge_rate"))
					{
						jd.charge_rate = ini.get_value_float(0);
					}
					else if (ini.is_value("discharge_rate"))
					{
						jd.discharge_rate = ini.get_value_float(0);
					}
					else if (ini.is_value("charge_fuse"))
					{
						jd.charge_fuse.push_back(CreateID(ini.get_value_string(0)));
					}
					else if (ini.is_value("jump_fuse"))
					{
						jd.jump_fuse = CreateID(ini.get_value_string(0));
					}
					else if (ini.is_value("fuel"))
					{
						uint fuel = CreateValidID(ini.get_value_string(0));
						int rate = ini.get_value_int(1);
						jd.mapFuelToUsage[fuel] = rate;
					}
					else if (ini.is_value("power"))
					{
						jd.power = ini.get_value_float(0);
					}
					else if (ini.is_value("field_range"))
					{
						jd.field_range = ini.get_value_float(0);
					}
				}
				mapJumpDriveArch[jd.nickname] = jd;
			}
			else if (ini.is_header("survey"))
			{
				SURVEY_ARCH hs;
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
					{
						hs.nickname = CreateValidID(ini.get_value_string(0));
					}
					else if (ini.is_value("survey_complete_charge"))
					{
						hs.survey_complete_charge = ini.get_value_float(0);
					}
					else if (ini.is_value("charge_rate"))
					{
						hs.charge_rate = ini.get_value_float(0);
					}
					else if (ini.is_value("fuel"))
					{
						uint fuel = CreateValidID(ini.get_value_string(0));
						int rate = ini.get_value_int(1);
						hs.mapFuelToUsage[fuel] = rate;
					}
					else if (ini.is_value("power"))
					{
						hs.power = ini.get_value_float(0);
					}
					else if (ini.is_value("coord_accuracy"))
					{
						hs.coord_accuracy = ini.get_value_float(0);
					}
				}
				mapSurveyArch[hs.nickname] = hs;
			}
		}
		ini.close();
	}

	// Remove patch now that we've finished loading.
	unsigned char patch2[] = { 0xFF, 0x12 };
	WriteProcMem((char*)0x62F327E, &patch2, 2);
	WriteProcMem((char*)0x62F944E, &patch2, 2);
	WriteProcMem((char*)0x62F123E, &patch2, 2);

	Console::ConInfo(L"Jumpdrive [%d]", mapJumpDriveArch.size());
}

void SetFuse(ClientId client, uint fuse)
{
	JUMPDRIVE& jd = mapJumpDrives[client];
	if (jd.active_fuse)
	{
		IObjInspectImpl* obj = GetInspect(client);
		if (obj)
		{
			UnLightFuse((IObjRW*)obj, jd.active_fuse);
		}
		jd.active_fuse = 0;
	}

	if (fuse)
	{
		IObjInspectImpl* obj = GetInspect(client);
		if (obj)
		{
			jd.active_fuse = fuse;
			LightFuse((IObjRW*)obj, jd.active_fuse, 0.0f, 0.0f, 0.0f);
		}
	}
}

void AddChargeFuse(ClientId client, uint fuse)
{
	IObjInspectImpl* obj = GetInspect(client);
	if (obj)
	{
		mapJumpDrives[client].active_charge_fuse.push_back(fuse);
		LightFuse((IObjRW*)obj, fuse, 0, 0, 0);
	}
}

void StopChargeFuses(ClientId client)
{
	IObjInspectImpl* obj = GetInspect(client);
	if (obj)
	{
		for (auto& fuse : mapJumpDrives[client].active_charge_fuse)
			UnLightFuse((IObjRW*)obj, fuse);
		mapJumpDrives[client].active_charge_fuse.clear();
	}
}

void Timer()
{
	std::list<uint> lstOldClients;

	// Handle survey charging
	for (auto& iter : mapSurvey)
	{
		ClientId client = iter.first;

		uint ship;
		pub::Player::GetShip(client, ship);
		if (ship == 0)
		{
			lstOldClients.push_back(client);
		}
		else
		{
			SURVEY& sm = iter.second;
			if (sm.charging_on)
			{
				// Use fuel to charge the jump drive's storage capacitors
				sm.charging_on = false;

				for (auto item = Players[client].equipDescList.equip.begin();
				     item != Players[client].equipDescList.equip.end(); item++)
				{
					if (sm.arch.mapFuelToUsage.find(item->iArchId) != sm.arch.mapFuelToUsage.end())
					{
						uint fuel_usage = sm.arch.mapFuelToUsage[item->iArchId];
						if (item->iCount >= fuel_usage)
						{
							pub::Player::RemoveCargo(client, item->sId, fuel_usage);
							sm.curr_charge += sm.arch.charge_rate;
							sm.charging_on = true;
							break;
						}
					}
				}

				Vector dir1;
				Vector dir2;
				pub::SpaceObj::GetMotion(ship, dir1, dir2);
				if (dir1.x > 5 || dir1.y > 5 || dir1.z > 5)
				{
					sm.charging_on = false;
				}

				// Turn off the charging effect if the charging has failed due
				// to lack of fuel and skip to the next player.
				if (!sm.charging_on)
				{
					sm.curr_charge = 0;
					PrintUserCmdText(client, L"Survey failed");
					continue;
				}

				if (sm.curr_charge >= sm.arch.survey_complete_charge)
				{
					// We're done.
					Vector v;
					Matrix m;
					pub::SpaceObj::GetLocation(ship, v, m);

					// Fill out the coordinate structure
					HYPERSPACE_COORDS coords;
					char* ibuf = (char*)&coords;
					memset(&coords, 0, sizeof(coords));
					coords.seed = rand();
					coords.system = Players[client].iSystemId;
					coords.x = v.x;
					coords.y = v.y;
					coords.z = v.z;
					coords.time = (uint)time(0) + (35 * 24 * 3600);
					coords.accuracy = sm.arch.coord_accuracy;

					// Calculate a simple parity check
					WORD parity = 0;
					for (int i = 2; i < HCOORD_SIZE; i++)
						parity += ibuf[i];
					coords.parity = parity;

					// Encrypt it
					char obuf[HCOORD_SIZE];
					EncryptDecrypt(ibuf, obuf);
					PrintUserCmdText(client, L"Hyperspace survey complete");
					// PrintUserCmdText(client, L"Raw: %s",
					// FormatCoords(ibuf).c_str());
					PrintUserCmdText(client, L"Coords: %s", FormatCoords(obuf).c_str());

					lstOldClients.push_back(client);
				}
				else if (time(0) % 10 == 0)
				{
					PrintUserCmdText(
					    client, L"Survey %0.0f%% complete",
					    (sm.curr_charge / sm.arch.survey_complete_charge) * 100.0f);
				}
			}
		}
	}

	for (auto& client : lstOldClients)
		mapSurvey.erase(client);

	lstOldClients.clear();

	// Handle jump drive charging
	for (auto& iter : mapJumpDrives)
	{
		ClientId client = iter.first;

		uint ship;
		pub::Player::GetShip(client, ship);
		if (ship == 0)
		{
			lstOldClients.push_back(client);
		}
		else
		{
			JUMPDRIVE& jd = iter.second;
			if (jd.jump_timer > 0)
			{
				jd.jump_timer--;
				// Turn on the jumpdrive flash
				if (jd.jump_timer == 7)
				{
					jd.charging_complete = false;
					jd.curr_charge = 0.0;
					jd.charging_on = false;
					SetFuse(client, jd.arch.jump_fuse);
					pub::Audio::PlaySoundEffect(client, CreateID("dsy_jumpdrive_activate"));
				}
				// Execute the jump and do the pop sound
				else if (jd.jump_timer == 2)
				{
					// Stop the charging fuses
					StopChargeFuses(client);

					// Jump the player's ship
					Vector vPosition;
					Matrix mShipDir;
					pub::SpaceObj::GetLocation(ship, vPosition, mShipDir);

					SystemId iSystemId = Hk::Solar::GetSystemBySpaceId(ship).value();

					pub::SpaceObj::DrainShields(ship);
					SwitchSystem(client, jd.iTargetSystem, jd.vTargetPosition, mShipDir);

					// Find all ships within the jump field including the one
					// with the jump engine.
					if (jd.arch.field_range > 0)
					{
						struct PlayerData* playerData = 0;
						while (playerData = Players.traverse_active(playerData))
						{
							SystemId iSystemId2 = Hk::Solar::GetSystemBySpaceId(playerData->shipId).value();

							Vector vPosition2;
							Matrix mShipDir2;
							pub::SpaceObj::GetLocation(playerData->shipId, vPosition2, mShipDir2);

							if (playerData->iOnlineId != client && iSystemId2 == iSystemId &&
							    Distance3D(vPosition, vPosition2) <= jd.arch.field_range)
							{
								PrintUserCmdText(playerData->iOnlineId, L"Jumping...");

								Vector vNewTargetPosition;
								vNewTargetPosition.x = jd.vTargetPosition.x + (vPosition.x - vPosition2.x);
								vNewTargetPosition.y = jd.vTargetPosition.y + (vPosition.y - vPosition2.y);
								vNewTargetPosition.z = jd.vTargetPosition.z + (vPosition.z - vPosition2.z);
								pub::Audio::PlaySoundEffect(playerData->iOnlineId, CreateID("dsy_jumpdrive_activate"));
								pub::SpaceObj::DrainShields(playerData->shipId);
								SwitchSystem(playerData->iOnlineId, jd.iTargetSystem, vNewTargetPosition, mShipDir2);
							}
						}
					}
				}
				// Wait until the ship is in the target system before turning
				// off the fuse by holding the timer.
				else if (jd.jump_timer == 1)
				{
					SystemId iSystem = Hk::Player::GetSystem(client).value();
					if (iSystem != jd.iTargetSystem)
						jd.jump_timer = 2;
				}
				// Finally turn off the fuse and make sure the ship is damagable
				// (the switch out causes the ship to be invincible
				else if (jd.jump_timer == 0)
				{
					// If this system is a system that kills on jump in then
					// kill the ship
					if (set_death_systems.find(jd.iTargetSystem) != set_death_systems.end())
					{
						IObjInspectImpl* obj = GetInspect(client);
						if (obj)
						{
							LightFuse((IObjRW*)obj, CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
						}
					}

					jd.iTargetSystem = 0;
					jd.vTargetPosition.x = 0;
					jd.vTargetPosition.y = 0;
					jd.vTargetPosition.z = 0;
					pub::SpaceObj::SetInvincible(ship, false, false, 0);
					SetFuse(client, 0);
					StopChargeFuses(client);
				}

				// Proceed to the next ship.
				continue;
			}

			if (jd.charging_on)
			{
				// Use fuel to charge the jump drive's storage capacitors
				jd.charging_on = false;

				for (auto item = Players[client].equipDescList.equip.begin();
				     item != Players[client].equipDescList.equip.end(); item++)
				{
					if (jd.arch.mapFuelToUsage.find(item->iArchId) != jd.arch.mapFuelToUsage.end())
					{
						uint fuel_usage = jd.arch.mapFuelToUsage[item->iArchId];
						if (item->iCount >= fuel_usage)
						{
							pub::Player::RemoveCargo(client, item->sId, fuel_usage);
							jd.curr_charge += jd.arch.charge_rate;
							jd.charging_on = true;
							break;
						}
					}
				}

				// Turn off the charging effect if the charging has failed due
				// to lack of fuel and skip to the next player.
				if (!jd.charging_on)
				{
					PrintUserCmdText(client, L"Jump drive charging failed");
					pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_failed"));
					SetFuse(client, 0);
					StopChargeFuses(client);
					continue;
				}

				pub::Audio::PlaySoundEffect(client, CreateID("dsy_jumpdrive_charge"));

				if (jd.curr_charge >= jd.arch.can_jump_charge)
				{
					jd.curr_charge = jd.arch.can_jump_charge;
					if (!jd.charging_complete)
					{
						PrintUserCmdText(client, L"Jump drive charging complete, ready to jump");
						pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_complete"));
						jd.charging_complete = true;
					}
				}
				else
				{
					jd.charging_complete = false;
				}

				uint expected_charge_status = (uint)(jd.curr_charge / jd.arch.can_jump_charge * 10);
				if (jd.charge_status != expected_charge_status)
				{
					jd.charge_status = expected_charge_status;
					PrintUserCmdText(
					    client, L"Jump drive charge %0.0f%%", (jd.curr_charge / jd.arch.can_jump_charge) * 100.0f);

					// Find the currently expected charge fuse
					uint charge_fuse_idx =
					    (uint)((jd.curr_charge / jd.arch.can_jump_charge) * (jd.arch.charge_fuse.size() - 1));
					if (charge_fuse_idx >= jd.arch.charge_fuse.size())
						charge_fuse_idx = jd.arch.charge_fuse.size() - 1;

					// If the fuse is not present then activate it.
					uint charge_fuse = jd.arch.charge_fuse[charge_fuse_idx];
					if (find(jd.active_charge_fuse.begin(), jd.active_charge_fuse.end(), charge_fuse) ==
					    jd.active_charge_fuse.end())
					{
						AddChargeFuse(client, charge_fuse);
					}
				}
			}
			else
			{
				// The drive is inactive, discharge the jump capacitors.
				jd.curr_charge -= jd.arch.discharge_rate;
				if (jd.curr_charge < 0.0f)
				{
					jd.curr_charge = 0.0;
				}

				jd.charging_complete = false;
				jd.charge_status = -1;
				StopChargeFuses(client);
				SetFuse(client, 0);
			}
		}
	}

	// If the ship has docked or died remove the client.
	for (auto& client : lstOldClients)
		mapJumpDrives.erase(client);

	// Handle testbot jumping.
	for (auto& iter : mapTestBots)
	{
		ClientId client = iter.first;
		TESTBOT& tb = iter.second;

		if (tb.bBaseTest)
		{
			if (tb.iCheckZonesTimer > 0)
			{
				tb.iCheckZonesTimer--;
			}
			else if (tb.iCheckZonesTimer == 0)
			{
				tb.iCheckZonesTimer = tb.iCheckZoneTime;
				tb.iCheckTestedZones++;
				SystemId iSystem = Hk::Player::GetSystem(client).value();

				uint ship;
				pub::Player::GetShip(client, ship);

				// if ship is in base undock it
				auto base = Hk::Player::GetCurrentBase(client);
				if (base.value())
				{
					PrintUserCmdText(client, L"Launching iteration %d", tb.iCheckTestedZones);

					Vector pos = { 0, 300000, 0 };
					Matrix ornt = { 0 };
					SwitchSystem(client, iSystem, pos, ornt);
				}
				// if in space dock with specified base.
				else if (ship)
				{
					PrintUserCmdText(client, L"Docking iteration %d", tb.iCheckTestedZones);

					const struct Universe::IBase* baseinfo = Universe::get_base(tb.iCheckSystemOrBase);
					pub::Player::ForceLand(client, baseinfo->iBaseId);
					if (iSystem != baseinfo->iSystemId)
					{
						Server.BaseEnter(baseinfo->iBaseId, client);
						Server.BaseExit(baseinfo->iBaseId, client);
						std::wstring wscCharFileName;
						GetCharFileName((const wchar_t*)Players.GetActiveCharacterName(client), wscCharFileName);
						wscCharFileName += L".fl";
						CHARACTER_ID cId;
						strcpy(cId.szCharFilename, wstos(wscCharFileName.substr(0, 14)).c_str());
						Server.CharacterSelect(cId, client);
					}
				}
			}
		}
		else
		{
			SystemId iSystem = Hk::Player::GetSystem(iter.first).value();
			if (tb.iCheckSystemOrBase == iSystem)
			{
				if (tb.iCheckZonesTimer > 0)
				{
					if ((tb.iCheckZonesTimer % 10) == 0)
						PrintUserCmdText(client, L"Jump to next zone in %d seconds", tb.iCheckZonesTimer);
					tb.iCheckZonesTimer--;
				}
				else if (tb.iCheckZonesTimer == 0)
				{
					if (tb.lstCheckZones.size())
					{
						tb.iCheckZonesTimer = tb.iCheckZoneTime;
						tb.iCheckTestedZones++;

						const ZONE& lz = tb.lstCheckZones.front();
						PrintUserCmdText(
						    client, L"Testing zone %s (%0.0f %0.0f %0.0f) iteration %d", stows(lz.zoneNick).c_str(),
						    lz.pos.x, lz.pos.y, lz.pos.z, tb.iCheckTestedZones);

						uint ship;
						pub::Player::GetShip(client, ship);

						Vector myLocation;
						Matrix myRot;
						pub::SpaceObj::GetLocation(ship, myLocation, myRot);

						RelocateClient(client, lz.pos, myRot);
						tb.lstCheckZones.pop_front();
						tb.lstCheckZones.push_back(lz);
					}
				}
			}
		}
	}
}

void SendDeathMsg(const std::wstring& wscMsg, uint& iSystem, ClientId& clientVictim, ClientId& clientKiller)
{
	// If someone killed a bot then take revenge
	auto iter = mapTestBots.find(clientVictim);
	if (iter != mapTestBots.end())
	{
		PrintUserCmdText(clientKiller, L"Err 0101010001110 Does not compute");
		Kill((const wchar_t*)Players.GetActiveCharacterName(clientKiller));
		return;
	}
}

bool SystemSwitchOutComplete(uint& ship, ClientId& client)
{
	static PBYTE SwitchOut = 0;
	if (!SwitchOut)
	{
		SwitchOut = (PBYTE)hModServer + 0xf600;

		DWORD dummy;
		VirtualProtect(SwitchOut + 0xd7, 200, PAGE_EXECUTE_READWRITE, &dummy);
	}

	// Patch the system switch out routine to put the ship in a
	// system of our choosing.
	if (mapDeferredJumps.find(client) != mapDeferredJumps.end())
	{
		SwitchOut[0x0d7] = 0xeb; // ignore exit object
		SwitchOut[0x0d8] = 0x40;
		SwitchOut[0x119] = 0xbb; // set the destination system
		*(PDWORD)(SwitchOut + 0x11a) = mapDeferredJumps[client].system;
		SwitchOut[0x266] = 0x45;                                          // don't generate warning
		*(float*)(SwitchOut + 0x2b0) = mapDeferredJumps[client].pos.z; // set entry location
		*(float*)(SwitchOut + 0x2b8) = mapDeferredJumps[client].pos.y;
		*(float*)(SwitchOut + 0x2c0) = mapDeferredJumps[client].pos.x;
		*(float*)(SwitchOut + 0x2c8) = mapDeferredJumps[client].ornt.data[2][2];
		*(float*)(SwitchOut + 0x2d0) = mapDeferredJumps[client].ornt.data[1][1];
		*(float*)(SwitchOut + 0x2d8) = mapDeferredJumps[client].ornt.data[0][0];
		*(float*)(SwitchOut + 0x2e0) = mapDeferredJumps[client].ornt.data[2][1];
		*(float*)(SwitchOut + 0x2e8) = mapDeferredJumps[client].ornt.data[2][0];
		*(float*)(SwitchOut + 0x2f0) = mapDeferredJumps[client].ornt.data[1][2];
		*(float*)(SwitchOut + 0x2f8) = mapDeferredJumps[client].ornt.data[1][0];
		*(float*)(SwitchOut + 0x300) = mapDeferredJumps[client].ornt.data[0][2];
		*(float*)(SwitchOut + 0x308) = mapDeferredJumps[client].ornt.data[0][1];
		*(PDWORD)(SwitchOut + 0x388) = 0x03ebc031; // ignore entry object
		mapDeferredJumps.erase(client);
		pub::SpaceObj::SetInvincible(ship, false, false, 0);
		Server.SystemSwitchOutComplete(ship, client);
		SwitchOut[0x0d7] = 0x0f;
		SwitchOut[0x0d8] = 0x84;
		SwitchOut[0x119] = 0x87;
		*(PDWORD)(SwitchOut + 0x11a) = 0x1b8;
		*(PDWORD)(SwitchOut + 0x25d) = 0x1cf7f;
		SwitchOut[0x266] = 0x1a;
		*(float*)(SwitchOut + 0x2b0) = *(float*)(SwitchOut + 0x2b8) = *(float*)(SwitchOut + 0x2c0) = 0;
		*(float*)(SwitchOut + 0x2c8) = *(float*)(SwitchOut + 0x2d0) = *(float*)(SwitchOut + 0x2d8) = 1;
		*(float*)(SwitchOut + 0x2e0) = *(float*)(SwitchOut + 0x2e8) = *(float*)(SwitchOut + 0x2f0) =
		    *(float*)(SwitchOut + 0x2f8) = *(float*)(SwitchOut + 0x300) = *(float*)(SwitchOut + 0x308) = 0;
		*(PDWORD)(SwitchOut + 0x388) = 0xcf8b178b;

		return true;
	}
	return false;
}

void ClearClientInfo(ClientId& client)
{
	mapTestBots.erase(client);
	mapDeferredJumps.erase(client);
	mapJumpDrives.erase(client);
	mapSurvey.erase(client);
}

bool InitJumpDriveInfo(ClientId client)
{
	// Initialise the drive parameters for this ship
	if (mapJumpDrives.find(client) == mapJumpDrives.end())
	{
		mapJumpDrives[client].arch.nickname = 0;
		mapJumpDrives[client].arch.can_jump_charge = 0;
		mapJumpDrives[client].arch.charge_fuse.clear();
		mapJumpDrives[client].arch.charge_rate = 0;
		mapJumpDrives[client].arch.discharge_rate = 0;
		mapJumpDrives[client].arch.jump_fuse = 0;
		mapJumpDrives[client].arch.mapFuelToUsage.clear();

		mapJumpDrives[client].charging_on = false;
		mapJumpDrives[client].curr_charge = 0;
		mapJumpDrives[client].active_fuse = 0;
		mapJumpDrives[client].active_charge_fuse.clear();
		mapJumpDrives[client].charging_complete = false;
		mapJumpDrives[client].charge_status = -1;

		mapJumpDrives[client].jump_timer = 0;
		mapJumpDrives[client].iTargetSystem = 0;
		mapJumpDrives[client].vTargetPosition.x = 0;
		mapJumpDrives[client].vTargetPosition.y = 0;
		mapJumpDrives[client].vTargetPosition.z = 0;

		// Check that the player has a jump drive and initialise the infomation
		// about it - otherwise return false.
		for (auto item = Players[client].equipDescList.equip.begin();
		     item != Players[client].equipDescList.equip.end(); item++)
		{
			if (mapJumpDriveArch.find(item->iArchId) != mapJumpDriveArch.end())
			{
				if (item->bMounted)
				{
					mapJumpDrives[client].arch = mapJumpDriveArch[item->iArchId];
					return true;
				}
			}
		}

		return false;
	}

	return true;
}

time_t filetime_to_timet(const FILETIME& ft)
{
	ULARGE_INTEGER ull;
	ull.LowPart = ft.dwLowDateTime;
	ull.HighPart = ft.dwHighDateTime;
	return ull.QuadPart / 10000000ULL - 11644473600ULL;
}

// Move the ship's starting position randomly if it has been logged out in
// space.
void PlayerLaunch(uint& ship, ClientId& client)
{
	static const uint MAX_DRIFT = 50000;

	// Find the time this ship was last online.
	std::wstring wscTimeStamp = L"";
	if (FLIniGet((const wchar_t*)Players.GetActiveCharacterName(client), L"tstamp", wscTimeStamp) != E_OK)
		return;

	FILETIME ft;
	ft.dwHighDateTime = strtoul(GetParam(wstos(wscTimeStamp), ',', 0).c_str(), 0, 10);
	ft.dwLowDateTime = strtoul(GetParam(wstos(wscTimeStamp), ',', 1).c_str(), 0, 10);
	time_t lastTime = filetime_to_timet(ft);

	// Get the current time; note FL stores the FILETIME in local time not UTC.
	SYSTEMTIME st;
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &ft);
	time_t currTime = filetime_to_timet(ft);

	// Calculate the expected drift.
	float drift = (float)(currTime - lastTime);

	if (drift > MAX_DRIFT)
		drift = MAX_DRIFT;

	drift *= ((2.0f * rand() / (float)RAND_MAX) - 1.0f);

	// Adjust the ship's position.
	Vector pos = { Players[client].vPosition.x, Players[client].vPosition.y, Players[client].vPosition.z };
	pos.x += drift / 10;
	pos.y += drift;
	pos.z += drift / 10;
	pub::Player::SetInitialPos(client, pos);
}

void GuidedHit(ClientId& client, DamageList** dmg)
{
	if (mapSurvey.find(client) != mapSurvey.find(client))
	{
		DamageList* dmg2 = *dmg;
		if (dmg2->get_cause() == 6 || dmg2->get_cause() == 0x15)
		{
			mapSurvey[client].curr_charge = 0;
			mapSurvey[client].charging_on = false;
			PrintUserCmdText(client, L"Hyperspace survey disrupted, restart required");
		}
	}
}

bool InitSurveyInfo(ClientId client)
{
	// Initialise the drive parameters for this ship
	if (mapSurvey.find(client) == mapSurvey.end())
	{
		mapSurvey[client].arch.nickname = 0;
		mapSurvey[client].arch.survey_complete_charge = 0;
		mapSurvey[client].arch.charge_rate = 0;
		mapSurvey[client].arch.coord_accuracy = 0;
		mapSurvey[client].arch.power = 0;
		mapSurvey[client].arch.mapFuelToUsage.clear();

		mapSurvey[client].charging_on = false;
		mapSurvey[client].curr_charge = 0;

		// Check that the player has a jump drive and initialise the infomation
		// about it - otherwise return false.
		for (auto item = Players[client].equipDescList.equip.begin();
		     item != Players[client].equipDescList.equip.end(); item++)
		{
			if (mapSurveyArch.find(item->iArchId) != mapSurveyArch.end())
			{
				mapSurvey[client].arch = mapSurveyArch[item->iArchId];
				return true;
			}
		}

		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// User commands
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Survey(ClientId& client, const std::wstring& wscParam)
{
	// If no ship, report a warning
	IObjInspectImpl* obj = GetInspect(client);
	if (!obj)
	{
		PrintUserCmdText(client, L"Survey module not available");
		return;
	}

	if (!InitSurveyInfo(client))
	{
		PrintUserCmdText(client, L"Survey module not available");
		return;
	}

	SURVEY& sm = mapSurvey[client];

	CShip* cship = (CShip*)GetEqObjFromObjRW((IObjRW*)obj);
	if (cship->get_max_power() < sm.arch.power)
	{
		PrintUserCmdText(client, L"Insufficient power to start survey module");
		return;
	}

	// Toogle the charge state
	sm.charging_on = !sm.charging_on;
	if (sm.charging_on)
	{
		PrintUserCmdText(client, L"Survey started");
		sm.curr_charge = 0;
		return;
	}
	else
	{
		PrintUserCmdText(client, L"Survey aborted");
		sm.curr_charge = 0;
		return;
	}
}

void UserCmd_SetCoords(ClientId& client, const std::wstring& wscParam)
{
	if (!InitJumpDriveInfo(client))
	{
		PrintUserCmdText(client, L"Jump drive not available");
		return;
	}

	JUMPDRIVE& jd = mapJumpDrives[client];
	jd.iTargetSystem = 0;

	std::string sbuf = wstos(ReplaceStr(GetParam(wscParam, L' ', 0), L"-", L""));
	if (sbuf.size() != 56)
	{
		PrintUserCmdText(client, L"ERR Invalid coordinates, format error");
		return;
	}

	char ibuf[HCOORD_SIZE];
	for (uint i = 0, p = 0; i < HCOORD_SIZE && (p + 1) < sbuf.size(); i++, p += 2)
	{
		char buf[3];
		buf[0] = sbuf[p];
		buf[1] = sbuf[p + 1];
		buf[2] = 0;
		ibuf[i] = (char)strtoul(buf, 0, 16);
	}

	HYPERSPACE_COORDS coords;
	char* obuf = (char*)&coords;
	memset(&coords, 0, sizeof(coords));

	EncryptDecrypt(ibuf, obuf);

	// Calculate a simple parity check
	WORD parity = 0;
	for (int i = 2; i < HCOORD_SIZE; i++)
		parity += obuf[i];

	if (coords.parity != parity)
	{
		PrintUserCmdText(client, L"ERR Invalid coordinates, parity error");
		return;
	}

	if (coords.time < time(0))
	{
		PrintUserCmdText(client, L"Warning old coordinates detected. Jump not recommended");
		coords.accuracy *= rand() % 7;
	}

	jd.iTargetSystem = coords.system;
	jd.vTargetPosition.x = coords.x;
	jd.vTargetPosition.y = coords.y;
	jd.vTargetPosition.z = coords.z;

	const struct Universe::ISystem* sysinfo = Universe::get_system(jd.iTargetSystem);
	PrintUserCmdText(
	    client, L"OK Coordinates verified: %s %0.0f.%0.0f.%0.0f", GetWStringFromIdS(sysinfo->strid_name).c_str(),
	    *(float*)&jd.vTargetPosition.x, *(float*)&jd.vTargetPosition.y, *(float*)&jd.vTargetPosition.z);

	int wiggle_factor = (int)coords.accuracy;
	jd.vTargetPosition.x += ((rand() * 10) % wiggle_factor) - (wiggle_factor / 2);
	jd.vTargetPosition.y += ((rand() * 10) % wiggle_factor) - (wiggle_factor / 2);
	jd.vTargetPosition.z += ((rand() * 10) % wiggle_factor) - (wiggle_factor / 2);

	return;
}

void UserCmd_ChargeJumpDrive(ClientId& client, const std::wstring& wscParam)
{
	// If no ship, report a warning
	IObjInspectImpl* obj = GetInspect(client);
	if (!obj)
	{
		PrintUserCmdText(client, L"Jump drive charging failed");
		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_failed"));
		return;
	}

	if (!InitJumpDriveInfo(client))
	{
		PrintUserCmdText(client, L"Jump drive not available");
		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_failed"));
		return;
	}

	JUMPDRIVE& jd = mapJumpDrives[client];

	CShip* cship = (CShip*)GetEqObjFromObjRW((IObjRW*)obj);
	if (cship->get_max_power() < jd.arch.power)
	{
		PrintUserCmdText(client, L"Insufficient power to charge jumpdrive");
		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_failed"));
		return;
	}

	// Toogle the charge state
	jd.charging_on = !jd.charging_on;
	jd.charge_status = -1;

	// Start the jump effect
	if (jd.charging_on)
	{
		if (jd.iTargetSystem == 0)
		{
			PrintUserCmdText(client, L"WARNING NO JUMP COORDINATES");
			pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_blind_jump_warning"));
		}

		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging"));
		PrintUserCmdText(client, L"Jump drive charging");
	}
	// Cancel jump effect if it is running
	else
	{
		PrintUserCmdText(client, L"Jump drive disabled");
		StopChargeFuses(client);
		SetFuse(client, 0);
	}

	return;
}

void UserCmd_ActivateJumpDrive(ClientId& client, const std::wstring& wscParam)
{
	// If no ship, report a warning
	IObjInspectImpl* obj = GetInspect(client);
	if (!obj)
	{
		PrintUserCmdText(client, L"Jump drive charging failed");
		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_failed"));
		return;
	}

	// If no jumpdrive, report a warning.
	if (!InitJumpDriveInfo(client))
	{
		PrintUserCmdText(client, L"Jump drive not ready");
		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_charging_failed"));
		return;
	}

	JUMPDRIVE& jd = mapJumpDrives[client];

	// If insufficient charging, report a warning
	if (!jd.charging_complete)
	{
		PrintUserCmdText(client, L"Jump drive not ready");
		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_not_ready"));
		return;
	}

	if (jd.iTargetSystem == 0)
	{
		PrintUserCmdText(client, L"WARNING NO JUMP COORDINATES");
		PrintUserCmdText(client, L"BLIND JUMP ACTIVATED");

		pub::Player::SendNNMessage(client, pub::GetNicknameID("nnv_jumpdrive_blind_jump_warning"));

		std::vector<std::string> systems;
		struct Universe::ISystem* sysinfo = Universe::GetFirstSystem();
		while (sysinfo)
		{
			systems.push_back(sysinfo->nickname);
			sysinfo = Universe::GetNextSystem();
		}

		// Pick a random system and position
		jd.iTargetSystem = CreateID(systems[rand() % systems.size()].c_str());
		jd.vTargetPosition.x = ((rand() * 10) % 400000) - 200000.0f;
		jd.vTargetPosition.y = ((rand() * 10) % 400000) - 200000.0f;
		jd.vTargetPosition.z = ((rand() * 10) % 400000) - 200000.0f;
	}

	// Start the jump timer.
	jd.jump_timer = 8;

	return;
}

// Client command processing
USERCMD UserCmds[] = {
	{ L"/survey", UserCmd_Survey },
	{ L"/setcoords", UserCmd_SetCoords },
	{ L"/jump", UserCmd_ActivateJumpDrive },
	{ L"/charge", UserCmd_ChargeJumpDrive },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Admin commands
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Move to location */
void AdminCmd_JumpTest(CCmds* cmds, const std::wstring& sys)
{
	if (!(cmds->rights & RIGHT_SUPERADMIN))
	{
		cmds->Print(L"ERR No permission");
		return;
	}

	PLAYERINFO adminPlyr;
	if (GetPlayerInfo(cmds->GetAdminName(), adminPlyr, false) != E_OK || adminPlyr.ship == 0)
	{
		cmds->Print(L"ERR Not in space");
		return;
	}

	JUMPDRIVE& jd = mapJumpDrives[adminPlyr.client];

	jd.iTargetSystem = CreateID(wstos(sys).c_str());
	jd.vTargetPosition.x = ((rand() * 10) % 400000) - 200000.0f;
	jd.vTargetPosition.y = ((rand() * 10) % 400000) - 200000.0f;
	jd.vTargetPosition.z = ((rand() * 10) % 400000) - 200000.0f;
	jd.jump_timer = 8;
	return;
}

/** Start automatic zone checking */
void AdminCmd_TestBot(CCmds* cmds, const std::wstring& wscSystemNick, int iCheckZoneTime)
{
	if (!(cmds->rights & RIGHT_SUPERADMIN))
	{
		cmds->Print(L"ERR No permission");
		return;
	}

	PLAYERINFO adminPlyr;
	if (GetPlayerInfo(cmds->GetAdminName(), adminPlyr, false) != E_OK || adminPlyr.ship == 0)
	{
		cmds->Print(L"ERR Not in space");
		return;
	}

	mapTestBots.erase(adminPlyr.client);
	if (wscSystemNick == L"stop")
	{
		cmds->Print(L"OK Check stopped");
		return;
	}

	const struct Universe::ISystem* sysinfo = Universe::get_system(CreateID(wstos(wscSystemNick).c_str()));
	if (sysinfo)
	{
		if (iCheckZoneTime == 0)
			iCheckZoneTime = 60;

		TESTBOT tb;
		tb.bBaseTest = false;
		tb.iCheckZonesTimer = 0;
		tb.iCheckSystemOrBase = CreateID(wstos(wscSystemNick).c_str());
		tb.iCheckTestedZones = 0;
		tb.iCheckZoneTime = iCheckZoneTime;

		auto start = zones.lower_bound(tb.iCheckSystemOrBase);
		auto end = zones.upper_bound(tb.iCheckSystemOrBase);
		for (auto i = start; i != end; i++)
		{
			ZONE rlz;
			if (ZoneUtilities::InDeathZone(tb.iCheckSystemOrBase, i->second.pos, rlz))
				continue;
			if (!i->second.encounter)
				continue;
			tb.lstCheckZones.push_back(i->second);
		}

		cmds->Print(
		    L"Checking system %s (%x) containing %d zones", wscSystemNick.c_str(), tb.iCheckSystemOrBase,
		    tb.lstCheckZones.size());

		// If we're not in the right system then jump to it.
		if (adminPlyr.iSystem != tb.iCheckSystemOrBase)
		{
			Vector pos = { 0, 100000, 0 };
			Matrix ornt = { 0 };
			SwitchSystem(adminPlyr.client, tb.iCheckSystemOrBase, pos, ornt);
		}

		mapTestBots[adminPlyr.client] = tb;
		return;
	}

	const struct Universe::IBase* baseinfo = Universe::get_base(CreateID(wstos(wscSystemNick).c_str()));
	if (baseinfo)
	{
		if (iCheckZoneTime == 0)
			iCheckZoneTime = 15;

		TESTBOT tb;
		tb.bBaseTest = true;
		tb.iCheckZonesTimer = 0;
		tb.iCheckSystemOrBase = CreateID(wstos(wscSystemNick).c_str());
		tb.iCheckTestedZones = 0;
		tb.iCheckZoneTime = iCheckZoneTime;

		cmds->Print(L"Testing base %s (%x) containing", wscSystemNick.c_str(), tb.iCheckSystemOrBase);

		mapTestBots[adminPlyr.client] = tb;
		return;
	}

	cmds->Print(L"ERR System or base not found");
}

bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
{
	if (wscCmd == L"testbot")
	{
		returncode = ReturnCode::SkipAll;
		AdminCmd_TestBot(cmds, cmds->ArgStr(1), cmds->ArgInt(2));
		return true;
	}
	else if (wscCmd == L"jumptest")
	{
		returncode = ReturnCode::SkipAll;
		AdminCmd_JumpTest(cmds, cmds->ArgStr(1));
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
DefaultDllMain()

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("HyperJump");
	pi->shortName("hyperjump");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete, &SystemSwitchOutComplete);
	pi->emplaceHook(HookedCall::FLHook__TimerNPCAndF1Check, &Timer);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IEngine__GuidedHit, &GuidedHit);
}
