// Mobile Docking Plugin by Cannon
// 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes

#include "Main.h"

#include <FLHook.hpp>
#include <plugin.h>

void LoadDockInfo(ClientId client);
void SaveDockInfo(ClientId client);
void UpdateDockInfo(ClientId client, uint iSystem, Vector pos, Matrix rot);

void SendSetBaseInfoText2(UINT client, const std::wstring& message);
void SendResetMarketOverride(UINT client);

std::map<uint, CLIENT_DATA> clients;

struct DEFERREDJUMPS
{
	UINT system;
	Vector pos;
	Matrix ornt;
};
static std::map<UINT, DEFERREDJUMPS> mapDeferredJumps;

struct DOCKING_REQUEST
{
	uint iTargetClientId;
};
std::map<uint, DOCKING_REQUEST> mapPendingDockingRequests;

/// The debug mode
static int set_iPluginDebug = 0;

/// The distance to undock from carrier
static int set_iMobileDockOffset = 100;

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
ReturnCode returncode = ReturnCode::Default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void JumpToLocation(ClientId client, uint system, Vector pos, Matrix ornt)
{
	mapDeferredJumps[client].system = system;
	mapDeferredJumps[client].pos = pos;
	mapDeferredJumps[client].ornt = ornt;

	// Send the jump command to the client. The client will send a system switch
	// out complete event which we intercept to set the new starting positions.
	PrintUserCmdText(client, L" ChangeSys %u", system);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UpdateDockedShips(ClientId client)
{
	uint tgt;
	uint system;
	Vector pos, vec, bvec;
	Matrix rot;

	pub::Player::GetShip(client, tgt);
	if (!tgt)
	{
		auto base = Hk::Player::GetCurrentBase(client);
		if (base.hasError())
			return;

		Universe::IBase* base = Universe::get_base(base.value());
		tgt = base->lSpaceObjId;

		SystemId system = Hk::Solar::GetSystemBySpaceId(tgt).value();
		pub::SpaceObj::GetLocation(tgt, pos, rot);

		float rad, brad;
		pub::SpaceObj::GetRadius(tgt, rad, vec);
		pub::SpaceObj::GetBurnRadius(tgt, brad, bvec);
		if (rad < brad)
			rad = brad;

		pos.x += rot.data[0][1] * (rad + set_iMobileDockOffset);
		pos.y += rot.data[1][1] * (rad + set_iMobileDockOffset);
		pos.z += rot.data[2][1] * (rad + set_iMobileDockOffset);
	}
	else
	{
		SystemId system = Hk::Solar::GetSystemBySpaceId(tgt).value();
		pub::SpaceObj::GetLocation(tgt, pos, rot);

		pos.x += rot.data[0][1] * set_iMobileDockOffset;
		pos.y += rot.data[1][1] * set_iMobileDockOffset;
		pos.z += rot.data[2][1] * set_iMobileDockOffset;
	}

	// For each docked ship, update it's last location to reflect that of the
	// carrier
	if (clients[client].mapDockedShips.size())
	{
		for (std::map<std::wstring, std::wstring>::iterator i = clients[client].mapDockedShips.begin();
		     i != clients[client].mapDockedShips.end(); ++i)
		{
			uint iDockedClientId = Hk::Client::GetClientIdFromCharName(i->first);
			if (iDockedClientId)
			{
				clients[iDockedClientId].iCarrierSystem = system;
				clients[iDockedClientId].vCarrierLocation = pos;
				clients[iDockedClientId].mCarrierLocation = rot;
				SaveDockInfo(iDockedClientId);
			}
			else
			{
				UpdateDockInfo(iDockedClientId, system, pos, rot);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Clear client info when a client connects.
void ClearClientInfo(ClientId& client)
{
	clients.erase(client);
	mapDeferredJumps.erase(client);
	mapPendingDockingRequests.erase(client);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Load the configuration
void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\mobiledocking.cfg";

	// Load generic settings
	set_iPluginDebug = IniGetI(scPluginCfgFile, "General", "Debug", 0);

	struct PlayerData* pd = 0;

	while (pd = Players.traverse_active(pd))
	{
		if (!IsInCharSelectMenu(pd->iOnlineId))
			LoadDockInfo(pd->iOnlineId);
	}
}

DefaultDllMainSettings(LoadSettings)

bool UserCmd_Process(ClientId& client, const std::wstring& wscCmd)
{
	if (wscCmd.find(L"/listdocked") == 0)
	{
		if (clients[client].mapDockedShips.size() == 0)
		{
			PrintUserCmdText(client, L"No ships docked");
		}
		else
		{
			PrintUserCmdText(client, L"Docked ships:");
			for (std::map<std::wstring, std::wstring>::iterator i = clients[client].mapDockedShips.begin();
			     i != clients[client].mapDockedShips.end(); ++i)
			{
				PrintUserCmdText(client, i->first);
			}
		}
	}
	else if (wscCmd.find(L"/jettisonship") == 0)
	{
		std::wstring charname = Trim(GetParam(wscCmd, ' ', 1));
		if (!charname.size())
		{
			PrintUserCmdText(client, L"Usage: /jettisonship <charname>");
			return true;
		}

		if (clients[client].mapDockedShips.find(charname) == clients[client].mapDockedShips.end())
		{
			PrintUserCmdText(client, L"Ship not docked");
			return true;
		}

		UpdateDockedShips(client);

		// Send a system switch to force the ship to launch. Do nothing
		// if the ship is in space for some reason.
		uint iDockedClientId = Hk::Client::GetClientIdFromCharName(charname);
		if (iDockedClientId)
		{
			uint ship;
			pub::Player::GetShip(iDockedClientId, ship);
			if (!ship)
			{
				JumpToLocation(
				    iDockedClientId, clients[iDockedClientId].iCarrierSystem, clients[iDockedClientId].vCarrierLocation,
				    clients[iDockedClientId].mCarrierLocation);
			}
		}

		// Remove the ship from the list
		clients[client].mapDockedShips.erase(charname);
		SaveDockInfo(client);
		PrintUserCmdText(client, L"Ship jettisoned");
		return true;
	}
	// The allow dock command accepts a docking request.
	else if (wscCmd.find(L"/allowdock") == 0)
	{
		// If not in space then ignore the request.
		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
			return true;

		// If no target then ignore the request.
		uint iTargetShip = Hk::Player::GetTarget(ship).value();
		if (!iTargetShip)
			return true;

		// If target is not player ship or ship is too far away then ignore the
		// request.
		uint iTargetClientId = GetClientIdByShip(iTargetShip);
		if (!iTargetClientId || Distance3DByShip(ship, iTargetShip) > 1000.0f)
		{
			PrintUserCmdText(client, L"Ship is out of range");
			return true;
		}

		// Find the docking request. If there is no docking request then ignore
		// this command
		if (mapPendingDockingRequests.find(iTargetClientId) == mapPendingDockingRequests.end() ||
		    mapPendingDockingRequests[iTargetClientId].iTargetClientId != client)
		{
			PrintUserCmdText(client, L"No pending docking requests for this ship");
			return true;
		}

		// Check that the target ship has an empty docking module. Report the
		// error
		if (clients[client].mapDockedShips.size() >= clients[client].iDockingModules)
		{
			mapPendingDockingRequests.erase(iTargetClientId);
			PrintUserCmdText(client, L"No free docking capacity");
			return true;
		}

		// Delete the docking request and dock the player.
		mapPendingDockingRequests.erase(iTargetClientId);

		std::string scProxyBase = GetPlayerSystemS(client) + "_proxy_base";
		uint iBaseId;
		if (pub::GetBaseID(iBaseId, scProxyBase.c_str()) == -4)
		{
			PrintUserCmdText(client, L"No proxy base, contact administrator");
			return true;
		}

		// Save the carrier info
		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iTargetClientId);
		clients[client].mapDockedShips[charname] = charname;
		SaveDockInfo(client);

		// Save the docking ship info
		clients[iTargetClientId].mobile_docked = true;
		clients[iTargetClientId].wscDockedWithCharname = (const wchar_t*)Players.GetActiveCharacterName(client);
		if (clients[iTargetClientId].iLastBaseId != 0)
			clients[iTargetClientId].iLastBaseId = Players[iTargetClientId].iLastBaseId;
		clients[iTargetClientId].iCarrierSystem = Hk::Solar::GetSystemBySpaceId(ship).value();
		pub::SpaceObj::GetLocation(
		    ship, clients[iTargetClientId].vCarrierLocation, clients[iTargetClientId].mCarrierLocation);
		SaveDockInfo(iTargetClientId);

		// Land the ship on the proxy base.
		Hk::Player::Beam(iTargetClientId, iBaseId);
		PrintUserCmdText(client, L"Ship docked");
		return true;
	}
	return false;
}

// If this is a docking request at a player ship then process it.
int __cdecl Dock_Call(
    unsigned int const& ship, unsigned int const& iBaseId, int& iCancel, enum DOCK_HOST_RESPONSE& response)
{
	UINT client = GetClientIdByShip(ship);
	if (client)
	{
		// If no target then ignore the request.
		uint iTargetShip = Hk::Player::GetTarget(ship).value();
		if (!iTargetShip)
			return 0;

		uint iType;
		pub::SpaceObj::GetType(iTargetShip, iType);
		if (iType != OBJ_FREIGHTER)
			return 0;

		// If target is not player ship or ship is too far away then ignore the
		// request.
		uint iTargetClientId = GetClientIdByShip(iTargetShip);
		if (!iTargetClientId || Distance3DByShip(ship, iTargetShip) > 1000.0f)
		{
			PrintUserCmdText(client, L"Ship is out of range");
			return 0;
		}

		// Check that the target ship has an empty docking module. Report the
		// error
		if (clients[iTargetClientId].mapDockedShips.size() >= clients[iTargetClientId].iDockingModules)
		{
			PrintUserCmdText(client, L"Target ship has no free docking capacity");
			return 0;
		}

		// Check that the requesting ship is of the appropriate size to dock.
		CShip* cship = (CShip*)GetEqObjFromObjRW((IObjRW*)GetInspect(client));
		if (cship->shiparch()->fHoldSize > 275)
		{
			PrintUserCmdText(client, L"Target ship is too small");
			return 0;
		}

		returncode = ReturnCode::SkipAll;

		// Create a docking request and send a notification to the target ship.
		mapPendingDockingRequests[client].iTargetClientId = iTargetClientId;
		PrintUserCmdText(
		    iTargetClientId, L"%s is requesting to dock, authorise with /allowdock",
		    Players.GetActiveCharacterName(client));
		PrintUserCmdText(client, L"Docking request sent to %s", Players.GetActiveCharacterName(iTargetClientId));
		return -1;
	}
	return 0;
}

void __stdcall CharacterSelect_AFTER(std::string& szCharFilename, ClientId& client)
{
	mapPendingDockingRequests.erase(client);
	LoadDockInfo(client);
}

bool IsShipDockedOnCarrier(std::wstring& carrier_charname, std::wstring& docked_charname)
{
	ClientId client = Hk::Client::GetClientIdFromCharName(carrier_charname);
	if (client != -1)
	{
		return clients[client].mapDockedShips.find(docked_charname) != clients[client].mapDockedShips.end();
	}
	else
	{
		return false;
	}
}

void __stdcall BaseEnter(uint& iBaseId, ClientId& client)
{
	// Update the location of any docked ships.
	if (clients[client].mapDockedShips.size())
	{
		UpdateDockedShips(client);
	}

	if (clients[client].mobile_docked)
	{
		// Clear the market. We don't support transfers in this version.
		SendResetMarketOverride(client);

		// Set the base name
		std::wstring status = L"<RDL><PUSH/>";
		status += L"<TEXT>" + XMLText(clients[client].wscDockedWithCharname) + L"</TEXT><PARA/><PARA/>";
		status += L"<POP/></RDL>";
		SendSetBaseInfoText2(client, status);

		// Check to see that the carrier thinks this ship is docked to it.
		// If it isn't then eject the ship to space.
		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
		if (!IsShipDockedOnCarrier(clients[client].wscDockedWithCharname, charname))
		{
			JumpToLocation(
			    client, clients[client].iCarrierSystem, clients[client].vCarrierLocation,
			    clients[client].mCarrierLocation);
			return;
		}
	}
}

void __stdcall BaseExit(uint& iBaseId, ClientId& client)
{
	LoadDockInfo(client);

	if (clients[client].mobile_docked)
	{
		// Clear the market. We don't support transfers in this version.
		SendResetMarketOverride(client);
	}
}

void __stdcall PlayerLaunch(uint& ship, ClientId& client)
{
	if (clients[client].mobile_docked)
	{
		// Update the location of the carrier and remove the docked ship from
		// the carrier
		uint carrier_client = Hk::Client::GetClientIdFromCharName(clients[client].wscDockedWithCharname);
		if (carrier_client != -1)
		{
			UpdateDockedShips(carrier_client);
			clients[carrier_client].mapDockedShips.erase((const wchar_t*)Players.GetActiveCharacterName(client));
		}

		// Jump the ship to the last location of the carrier and allow
		// the undock to proceed.
		JumpToLocation(
		    client, clients[client].iCarrierSystem, clients[client].vCarrierLocation, clients[client].mCarrierLocation);

		clients[client].mobile_docked = false;
		clients[client].wscDockedWithCharname = L"";
		SaveDockInfo(client);
	}
}

void SystemSwitchOutComplete(uint& ship, ClientId& client)
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
		returncode = ReturnCode::SkipAll;

		SwitchOut[0x0d7] = 0xeb; // ignore exit object
		SwitchOut[0x0d8] = 0x40;
		SwitchOut[0x119] = 0xbb; // set the destination system
		*(PDWORD)(SwitchOut + 0x11a) = mapDeferredJumps[client].system;
		SwitchOut[0x266] = 0x45;                                       // don't generate warning
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

		// Unpatch the code.
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
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall DisConnect(ClientId& client, enum EFLConnection& p2)
{
	UpdateDockedShips(client);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall CharacterInfoReq(ClientId& client, bool& p2)
{
	UpdateDockedShips(client);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall GFGoodSell(struct SGFGoodSellInfo const& gsi, ClientId& client)
{
	if (clients[client].mobile_docked)
	{
		returncode = ReturnCode::SkipPlugins;

		PrintUserCmdText(client, L"ERR: Ship will not accept goods");
		clients[client].reverse_sell = true;
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall ReqRemoveItem(unsigned short& slot, int& count, ClientId& client)
{
	if (clients[client].mobile_docked)
	{
		returncode = ReturnCode::SkipPlugins;

		if (clients[client].reverse_sell)
		{
			int hold_size;
			EnumCargo((const wchar_t*)Players.GetActiveCharacterName(client), clients[client].cargo, hold_size);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall ReqRemoveItem_AFTER(unsigned short& iId, int& count, ClientId& client)
{
	if (clients[client].mobile_docked)
	{
		returncode = ReturnCode::SkipPlugins;

		if (clients[client].reverse_sell)
		{
			clients[client].reverse_sell = false;

			for (auto& ci : clients[client].cargo)
			{
				if (ci.iId == iId)
				{
					Server.ReqAddItem(ci.iArchId, ci.hardpoint.value, count, ci.fStatus, ci.bMounted, client);
					return;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const& gbi, ClientId& client)
{
	// If the client is in a player controlled base
	if (clients[client].mobile_docked)
	{
		PrintUserCmdText(client, L"ERR Base will not sell goods");
		returncode = ReturnCode::SkipAll;
		clients[client].stop_buy = true;
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall ReqAddItem(uint& good, char const** hardpoint, int& count, float& fStatus, bool& bMounted, ClientId& client)
{
	if (clients[client].mobile_docked)
	{
		returncode = ReturnCode::SkipPlugins;

		if (clients[client].stop_buy)
		{
			clients[client].stop_buy = false;
			returncode = ReturnCode::SkipAll;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Ignore cash commands from the client when we're in a player base.
void __stdcall ReqChangeCash(uint& cash, ClientId& client)
{
	if (clients[client].mobile_docked)
		returncode = ReturnCode::SkipAll;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Ignore cash commands from the client when we're in a player base.
void __stdcall ReqSetCash(uint& cash, ClientId& client)
{
	if (clients[client].mobile_docked)
		returncode = ReturnCode::SkipAll;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall ReqEquipment(class EquipDescList const& edl, ClientId& client)
{
	if (clients[client].mobile_docked)
		returncode = ReturnCode::SkipPlugins;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void __stdcall ShipDestroyed(DamageList** _dmg, DWORD** ecx, uint& kill)
{
	CShip* cship = (CShip*)(*ecx)[4];
	ClientId client = cship->GetOwnerPlayer();
	if (kill)
	{
		if (client)
		{
			// If this is a carrier then drop all docked ships into space.
			if (clients[client].mapDockedShips.size())
			{
				// Update the coordinates
				UpdateDockedShips(client);

				// Send a system switch to force the ship to launch
				for (std::map<std::wstring, std::wstring>::iterator i = clients[client].mapDockedShips.begin();
				     i != clients[client].mapDockedShips.end(); ++i)
				{
					uint iDockedClientId = Hk::Client::GetClientIdFromCharName(i->first);
					if (iDockedClientId)
					{
						JumpToLocation(
						    iDockedClientId, clients[iDockedClientId].iCarrierSystem,
						    clients[iDockedClientId].vCarrierLocation, clients[iDockedClientId].mCarrierLocation);
					}
				}

				// Add clear the list.
				clients[client].mapDockedShips.clear();
				SaveDockInfo(client);
			}
			// If this was last docked at a carrier then set the last base to
			// the to last real base the ship docked at.
			else if (clients[client].iLastBaseId)
			{
				Players[client].iLastBaseId = clients[client].iLastBaseId;
				clients[client].iLastBaseId = 0;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Functions to hook */
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Mobile Docking by Cannon");
	pi->shortName("MobileDocking");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete, &SystemSwitchOutComplete);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::IServerImpl__BaseExit, &BaseExit);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::IEngine__DockCall, &Dock_Call);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &GFGoodSell);
	pi->emplaceHook(HookedCall::IServerImpl__ReqRemoveItem, &ReqRemoveItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqRemoveItem, &ReqRemoveItem_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqAddItem, &ReqAddItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqChangeCash, &ReqChangeCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqSetCash, &ReqSetCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqEquipment, &ReqEquipment);
}