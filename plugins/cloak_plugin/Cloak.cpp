/**
 Cloak (Yet another) Docking Plugin for FLHook-Plugin
 by Cannon.

0.1:
 Initial release
*/

// includes 

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <list>
#include <map>
#include <algorithm>
#include <FLHook.h>
#include <plugin.h>
#include "PluginUtilities.h"
#include <math.h>
#include "Cloak.h"

static int set_iPluginDebug = 0;

/// A return code to indicate to FLHook if we want the hook processing to continue.
PLUGIN_RETURNCODE returncode;

enum INFO_STATE
{
	STATE_CLOAK_INVALID = 0,
	STATE_CLOAK_OFF = 1,
	STATE_CLOAK_CHARGING = 2,
	STATE_CLOAK_ON = 3,
};

struct CLOAK_ARCH
{
	string scNickName;
	int iWarmupTime;
	int iCooldownTime;
	int iHoldSizeLimit;
	map<uint, uint> mapFuelToUsage;
	bool bDropShieldsOnUncloak;
};

struct CLOAK_INFO
{
	CLOAK_INFO()
	{

		uint iCloakSlot = 0;
		bCanCloak = false;
		mstime tmCloakTime = 0;
		uint iState = STATE_CLOAK_INVALID;
		uint bAdmin = false;

		arch.iWarmupTime = 0;
		arch.iCooldownTime = 0;
		arch.iHoldSizeLimit = 0;
		arch.mapFuelToUsage.clear();
		arch.bDropShieldsOnUncloak = false;
	}

	uint iCloakSlot;
	bool bCanCloak;
	mstime tmCloakTime;
	uint iState;
	bool bAdmin;

	CLOAK_ARCH arch;
};

static map<uint, CLOAK_INFO> mapClientsCloak;

static map<uint, CLOAK_ARCH> mapCloakingDevices;

void LoadSettings();

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));
	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
	if(fdwReason == DLL_PROCESS_ATTACH)
	{
		if (set_scCfgFile.length()>0)
			LoadSettings();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
	}
	return true;
}


void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	string scPluginCfgFile = string(szCurDir) + "\\flhook_plugins\\cloak.cfg";

	INI_Reader ini;
	if (ini.open(scPluginCfgFile.c_str(), false))
	{
		while (ini.read_header())
		{
			CLOAK_ARCH device;
			if (ini.is_header("Cloak"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
					{
						device.scNickName = ini.get_value_string(0);
					}
					else if (ini.is_value("warmup_time"))
					{
						device.iWarmupTime = ini.get_value_int(0);
					}
					else if (ini.is_value("cooldown_time"))
					{
						device.iCooldownTime = ini.get_value_int(0);
					}
					else if (ini.is_value("hold_size_limit"))
					{
						device.iHoldSizeLimit = ini.get_value_int(0);
					}
					else if (ini.is_value("fuel"))
					{
						string scNickName = ini.get_value_string(0);
						uint usage = ini.get_value_int(1);
						device.mapFuelToUsage[CreateID(scNickName.c_str())] = usage;
					}
					else if (ini.is_value("drop_shields_on_uncloak"))
					{
						device.bDropShieldsOnUncloak = ini.get_value_bool(0);
					}
				}
				mapCloakingDevices[CreateID(device.scNickName.c_str())] = device;
			}
		}
		ini.close();
	}

	struct PlayerData *pd = 0;
	while (pd = Players.traverse_active(pd))
	{
	}
}

void ClearClientInfo(uint iClientID)
{
	mapClientsCloak.erase(iClientID);
}

void SetCloak(uint iClientID, uint iShipID, bool bOn)
{
	XActivateEquip ActivateEq;
	ActivateEq.bActivate = bOn;
	ActivateEq.iSpaceID = iShipID;
	ActivateEq.sID = mapClientsCloak[iClientID].iCloakSlot;
	Server.ActivateEquip(iClientID,ActivateEq);
}

void SetState(uint iClientID, uint iShipID, int iNewState)
{	
	if (mapClientsCloak[iClientID].iState != iNewState)
	{
		mapClientsCloak[iClientID].iState = iNewState;
		mapClientsCloak[iClientID].tmCloakTime = timeInMS();
		switch (iNewState)
		{
		case STATE_CLOAK_CHARGING:
		{
			PrintUserCmdText(iClientID, L"Preparing to cloak...");
			break;
		}

		case STATE_CLOAK_ON:
		{
			PrintUserCmdText(iClientID, L" Cloaking device on");
			SetCloak(iClientID, iShipID, true);
			PrintUserCmdText(iClientID, L"Cloaking device on");
			break;
		}
		case STATE_CLOAK_OFF:
		default:
		{
			PrintUserCmdText(iClientID, L" Cloaking device off");
			SetCloak(iClientID, iShipID, false);
			PrintUserCmdText(iClientID, L"Cloaking device off");
			break;
		}
		}
	}
}



// Returns false if the ship has no fuel to operate its cloaking device.
static bool ProcessFuel(uint iClientID, CLOAK_INFO &info)
{
	if (info.bAdmin)
		return true;

	for (list<EquipDesc>::iterator item = Players[iClientID].equipDescList.equip.begin(); item != Players[iClientID].equipDescList.equip.end(); item++)
	{
		if (info.arch.mapFuelToUsage.find(item->iArchID) != info.arch.mapFuelToUsage.end())
		{
			uint fuel_usage = info.arch.mapFuelToUsage[item->iArchID];
			if (item->iCount >= fuel_usage)
			{
				pub::Player::RemoveCargo(iClientID, item->sID, fuel_usage);
				return true;
			}
		}
	}

	return false;
}

void PlayerLaunch_AFTER(unsigned int iShip, unsigned int iClientID)
{
	mapClientsCloak[iClientID].bCanCloak = false;
	mapClientsCloak[iClientID].bAdmin = false;

	IObjInspectImpl *obj = HkGetInspect(iClientID);
	if (obj)
	{
		CShip* cship = (CShip*)HkGetEqObjFromObjRW((IObjRW*)obj);

		CEquipTraverser tr;

		for (CEquip *equip = GetEquipManager(cship)->Traverse(tr);
			equip; equip = GetEquipManager(cship)->Traverse(tr))
		{
			if (CECloakingDevice::cast(equip))
			{		
				mapClientsCloak[iClientID].iCloakSlot = equip->GetID();			
				
				if (mapCloakingDevices.find(equip->EquipArch()->iArchID)!=mapCloakingDevices.end())
				{
					// Otherwise set the fuel usage and warm up time
					mapClientsCloak[iClientID].arch = mapCloakingDevices[equip->EquipArch()->iArchID];
				}
				// If this cloaking device does not appear in the cloaking device list
				// then warming up and fuel usage is zero and it may be used by any
				// ship.
				else
				{
					mapClientsCloak[iClientID].arch.bDropShieldsOnUncloak = false;
					mapClientsCloak[iClientID].arch.iCooldownTime = 0;
					mapClientsCloak[iClientID].arch.iHoldSizeLimit = 0;
					mapClientsCloak[iClientID].arch.iWarmupTime = 0;
					mapClientsCloak[iClientID].arch.mapFuelToUsage.clear();
				}

				mapClientsCloak[iClientID].bCanCloak = true;
				mapClientsCloak[iClientID].iState = STATE_CLOAK_INVALID;
				SetState(iClientID, iShip, STATE_CLOAK_OFF);
				return;
			}
		}
	}
}

void BaseEnter(unsigned int iBaseID, unsigned int iClientID)
{
	mapClientsCloak.erase(iClientID);
}

void HkTimerCheckKick()
{
	mstime now = timeInMS();

	

	for (map<uint, CLOAK_INFO>::iterator ci = mapClientsCloak.begin(); ci != mapClientsCloak.end(); ++ci)
	{
		uint iClientID = ci->first;
		uint iShipID = Players[iClientID].iShipID;
		CLOAK_INFO &info = ci->second;
	
		if (iShipID && info.bCanCloak)
		{
			switch (info.iState)
			{
			case STATE_CLOAK_OFF:
				// Send cloak state for uncloaked cloak-able players (only for them in space)
				// this is the code to fix the bug where players wouldnt always see uncloaked players
				XActivateEquip ActivateEq;
				ActivateEq.bActivate = false;
				ActivateEq.iSpaceID = iShipID;
				ActivateEq.sID = info.iCloakSlot;
				Server.ActivateEquip(iClientID,ActivateEq);
				break;

			case STATE_CLOAK_CHARGING:
				if (!ProcessFuel(iClientID, info))
				{
					PrintUserCmdText(iClientID, L"Cloaking device shutdown, no fuel");	
					SetState(iClientID, iShipID, STATE_CLOAK_OFF);	
				}
				else if ((info.tmCloakTime + info.arch.iWarmupTime) < now)
				{
					SetState(iClientID, iShipID, STATE_CLOAK_ON);
				}
				else if (info.arch.bDropShieldsOnUncloak && !info.bAdmin)
				{
					pub::SpaceObj::DrainShields(iShipID);
				}
				break;

			case STATE_CLOAK_ON:
				if (!ProcessFuel(iClientID, info))
				{
					PrintUserCmdText(iClientID, L"Cloaking device shutdown, no fuel");	
					SetState(iClientID, iShipID, STATE_CLOAK_OFF);	
				}
				else if (info.arch.bDropShieldsOnUncloak && !info.bAdmin)
				{
					pub::SpaceObj::DrainShields(iShipID);
				}
				break;
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool UserCmd_Cloak(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
{		
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	if (!iShip)
	{
		PrintUserCmdText(iClientID, L"Not in space");
		return true;
	}

	if (!mapClientsCloak[iClientID].bCanCloak)
	{
		PrintUserCmdText(iClientID, L"Cloaking device not available");
		return true;
	}

	
	// If this cloaking device requires more power than the ship can provide
	// no cloaking device is available.
	IObjInspectImpl *obj = HkGetInspect(iClientID);
	if (obj)
	{
		CShip* cship = (CShip*)HkGetEqObjFromObjRW((IObjRW*)obj);
		if (cship)
		{
			if (mapClientsCloak[iClientID].arch.iHoldSizeLimit != 0
				&& mapClientsCloak[iClientID].arch.iHoldSizeLimit < cship->shiparch()->fHoldSize)
			{
				PrintUserCmdText(iClientID, L"Cloaking device will not function on this ship type");
				mapClientsCloak[iClientID].iState = STATE_CLOAK_INVALID;
				SetState(iClientID, iShip, STATE_CLOAK_OFF);
				return true;
			}

			switch (mapClientsCloak[iClientID].iState)
			{
			case STATE_CLOAK_OFF:
				SetState(iClientID, iShip, STATE_CLOAK_CHARGING);
				break;
			case STATE_CLOAK_CHARGING:
			case STATE_CLOAK_ON:
				SetState(iClientID, iShip, STATE_CLOAK_OFF);
				break;
			}
		}
	}
	return true;
}


typedef bool (*_UserCmdProc)(uint, const wstring &, const wstring &, const wchar_t*);

struct USERCMD
{
	wchar_t *wszCmd;
	_UserCmdProc proc;
	wchar_t *usage;
};

USERCMD UserCmds[] =
{
	{ L"/cloak", UserCmd_Cloak, L"Usage: /cloak"},
	{ L"/cloak*", UserCmd_Cloak, L"Usage: /cloak"},
	
};

/**
This function is called by FLHook when a user types a chat string. We look at the
string they've typed and see if it starts with one of the above commands. If it 
does we try to process it.
*/
bool UserCmd_Process(uint iClientID, const wstring &wscCmd)
{
	returncode = DEFAULT_RETURNCODE;

	wstring wscCmdLineLower = ToLower(wscCmd);

	// If the chat string does not match the USER_CMD then we do not handle the
	// command, so let other plugins or FLHook kick in. We require an exact match
	for(uint i = 0; (i < sizeof(UserCmds)/sizeof(USERCMD)); i++)
	{
		if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0)
        {
			// Extract the parameters string from the chat string. It should
            // be immediately after the command and a space.
            wstring wscParam = L"";
            if (wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd)+1);
			}

			// Dispatch the command to the appropriate processing function.
			if (UserCmds[i].proc(iClientID, wscCmd, wscParam, UserCmds[i].usage))
			{
				// We handled the command tell FL hook to stop processing this
				// chat string.
				returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command, return immediatly
				return true;
			}
		}
	}
	return false;
}

#define IS_CMD(a) !wscCmd.compare(L##a)

bool ExecuteCommandString_Callback(CCmds* cmds, const wstring &wscCmd)
{
	returncode = DEFAULT_RETURNCODE;

	if (IS_CMD("cloak"))
	{
		returncode = SKIPPLUGINS_NOFUNCTIONCALL;

		uint iClientID = HkGetClientIdFromCharname(cmds->GetAdminName());
		if (iClientID==-1)
		{
			cmds->Print(L"ERR On console");
			return true;
		}	
		
		uint iShip;
		pub::Player::GetShip(iClientID, iShip);
		if (!iShip)
		{
			PrintUserCmdText(iClientID, L"ERR Not in space");
			return true;
		}

		if (!mapClientsCloak[iClientID].bCanCloak)
		{
			cmds->Print(L"ERR Cloaking device not available");
			return true;
		}

		switch (mapClientsCloak[iClientID].iState)
		{
		case STATE_CLOAK_OFF:
			mapClientsCloak[iClientID].bAdmin = true;
			SetState(iClientID, iShip, STATE_CLOAK_ON);
			break;
		case STATE_CLOAK_CHARGING:
		case STATE_CLOAK_ON:
			mapClientsCloak[iClientID].bAdmin = false;
			SetState(iClientID, iShip, STATE_CLOAK_OFF);
			break;
		}
		return true;
	}
	return false;
}

void __stdcall HkCb_AddDmgEntry(DamageList *dmg, unsigned short p1, float damage, enum DamageEntry::SubObjFate fate)
{
	returncode = DEFAULT_RETURNCODE;
	if (iDmgToSpaceID && dmg->get_inflictor_id())
	{
		if (dmg->get_cause() == 0x06)
		{
			float curr, max;
			pub::SpaceObj::GetHealth(iDmgToSpaceID, curr, max);
			uint client = HkGetClientIDByShip(iDmgToSpaceID);
			if (client)
			{
				if (mapClientsCloak[client].bCanCloak
					&& !mapClientsCloak[client].bAdmin
					&& mapClientsCloak[client].iState == STATE_CLOAK_CHARGING)
				{
					SetState(client, iDmgToSpaceID, STATE_CLOAK_OFF);
				}
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Functions to hook */
EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "Cloak Plugin by cannon";
	p_PI->sShortName = "cloak";
	p_PI->bMayPause = true;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;
	
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&ClearClientInfo, PLUGIN_ClearClientInfo, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&PlayerLaunch_AFTER, PLUGIN_HkIServerImpl_PlayerLaunch_AFTER, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&BaseEnter, PLUGIN_HkIServerImpl_BaseEnter, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkTimerCheckKick, PLUGIN_HkTimerCheckKick, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Process, PLUGIN_UserCmd_Process, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&ExecuteCommandString_Callback, PLUGIN_ExecuteCommandString_Callback, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkCb_AddDmgEntry, PLUGIN_HkCb_AddDmgEntry, 0));
	
	
	return p_PI;
}