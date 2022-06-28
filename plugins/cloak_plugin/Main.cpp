// Cloak Plugin for FLHook by Cannon.
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::Cloak
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		for (auto& device : config.mapCloakingDevices)
		{
			CLOAK_ARCH ca;
			ca.bDropShieldsOnUncloak = device.second.DropShieldsOnUncloak;
			ca.iCooldownTime = device.second.CooldownTime;
			ca.iHoldSizeLimit = device.second.HoldSizeLimit;
			ca.iWarmupTime = device.second.WarmupTime;

			for (auto& fuel : device.second.FuelToUsage)
				if (fuel.first != "SomeItem")
					ca.mapFuelToUsage[CreateID(fuel.first.c_str())] = fuel.second;

			global->mapCloakingDevices[CreateID(device.first.c_str())] = ca;
		}

		if (config.DsAce)
		{
			global->CloakingText = L" Cloaking device on";
			global->UncloakingText = L" Cloaking device off";
		}

		global->config = std::make_unique<Config>(config);
	}

	void ClearClientInfo(uint iClientID) { global->mapClientsCloak.erase(iClientID); }

	void SetCloak(uint iClientID, uint iShipID, bool bOn)
	{
		XActivateEquip ActivateEq;
		ActivateEq.bActivate = bOn;
		ActivateEq.iSpaceID = iShipID;
		ActivateEq.sID = global->mapClientsCloak[iClientID].iCloakSlot;
		Server.ActivateEquip(iClientID, ActivateEq);
	}

	void SetState(uint iClientID, uint iShipID, int iNewState)
	{
		if (global->mapClientsCloak[iClientID].iState != iNewState)
		{
			global->mapClientsCloak[iClientID].iState = iNewState;
			global->mapClientsCloak[iClientID].tmCloakTime = timeInMS();
			switch (iNewState)
			{
				case STATE_CLOAK_CHARGING: {
					PrintUserCmdText(iClientID, L"Preparing to cloak...");
					break;
				}

				case STATE_CLOAK_ON: {
					PrintUserCmdText(iClientID, global->CloakingText);
					SetCloak(iClientID, iShipID, true);
					PrintUserCmdText(iClientID, L"Cloaking device on");
					break;
				}
				case STATE_CLOAK_OFF:
				default: {
					PrintUserCmdText(iClientID, global->UncloakingText);
					SetCloak(iClientID, iShipID, false);
					PrintUserCmdText(iClientID, L"Cloaking device off");
					break;
				}
			}
		}
	}

	// Returns false if the ship has no fuel to operate its cloaking device.
	static bool ProcessFuel(uint iClientID, CLOAK_INFO& info)
	{
		if (info.bAdmin || info.arch.mapFuelToUsage.empty())
			return true;

		for (auto item = Players[iClientID].equipDescList.equip.begin(); item != Players[iClientID].equipDescList.equip.end(); item++)
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

	void PlayerLaunch_AFTER(uint& iShip, uint& iClientID)
	{
		global->mapClientsCloak[iClientID].bCanCloak = false;
		global->mapClientsCloak[iClientID].bAdmin = false;

		IObjInspectImpl* obj = HkGetInspect(iClientID);
		if (obj)
		{
			CShip* cship = (CShip*)HkGetEqObjFromObjRW((IObjRW*)obj);

			CEquipTraverser tr;

			for (CEquip* equip = GetEquipManager(cship)->Traverse(tr); equip; equip = GetEquipManager(cship)->Traverse(tr))
			{
				if (CECloakingDevice::cast(equip))
				{
					global->mapClientsCloak[iClientID].iCloakSlot = equip->GetID();

					if (global->mapCloakingDevices.find(equip->EquipArch()->iArchID) != global->mapCloakingDevices.end())
					{
						// Otherwise set the fuel usage and warm up time
						global->mapClientsCloak[iClientID].arch = global->mapCloakingDevices[equip->EquipArch()->iArchID];
					}
					// If this cloaking device does not appear in the cloaking
					// device list then warming up and fuel usage is zero and it may
					// be used by any ship.
					else
					{
						global->mapClientsCloak[iClientID].arch.bDropShieldsOnUncloak = false;
						global->mapClientsCloak[iClientID].arch.iCooldownTime = 0;
						global->mapClientsCloak[iClientID].arch.iHoldSizeLimit = 0;
						global->mapClientsCloak[iClientID].arch.iWarmupTime = 0;
						global->mapClientsCloak[iClientID].arch.mapFuelToUsage.clear();
					}

					global->mapClientsCloak[iClientID].bCanCloak = true;
					global->mapClientsCloak[iClientID].iState = STATE_CLOAK_INVALID;
					SetState(iClientID, iShip, STATE_CLOAK_OFF);
					return;
				}
			}
		}
	}

	void BaseEnter(uint& iBaseID, uint& iClientID) { global->mapClientsCloak.erase(iClientID); }

	void HkTimerCheckKick()
	{
		mstime now = timeInMS();

		for (std::map<uint, CLOAK_INFO>::iterator ci = global->mapClientsCloak.begin(); ci != global->mapClientsCloak.end(); ++ci)
		{
			uint iClientID = ci->first;
			uint iShipID = Players[iClientID].iShipID;
			CLOAK_INFO& info = ci->second;

			if (iShipID && info.bCanCloak)
			{
				switch (info.iState)
				{
					case STATE_CLOAK_OFF:
						// Send cloak state for uncloaked cloak-able players (only for
						// them in space) this is the code to fix the bug where players
						// wouldnt always see uncloaked players
						XActivateEquip ActivateEq;
						ActivateEq.bActivate = false;
						ActivateEq.iSpaceID = iShipID;
						ActivateEq.sID = info.iCloakSlot;
						Server.ActivateEquip(iClientID, ActivateEq);
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

	void UserCmd_Cloak(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint iShip;
		pub::Player::GetShip(iClientID, iShip);
		if (!iShip)
		{
			PrintUserCmdText(iClientID, L"Not in space");
			return;
		}

		if (!global->mapClientsCloak[iClientID].bCanCloak)
		{
			PrintUserCmdText(iClientID, L"Cloaking device not available");
			return;
		}

		// If this cloaking device requires more power than the ship can provide
		// no cloaking device is available.
		IObjInspectImpl* obj = HkGetInspect(iClientID);
		if (obj)
		{
			CShip* cship = (CShip*)HkGetEqObjFromObjRW((IObjRW*)obj);
			if (cship)
			{
				if (global->mapClientsCloak[iClientID].arch.iHoldSizeLimit != 0 &&
				    global->mapClientsCloak[iClientID].arch.iHoldSizeLimit < cship->shiparch()->fHoldSize)
				{
					PrintUserCmdText(iClientID, L"Cloaking device will not function on this ship type");
					global->mapClientsCloak[iClientID].iState = STATE_CLOAK_INVALID;
					SetState(iClientID, iShip, STATE_CLOAK_OFF);
					return;
				}

				switch (global->mapClientsCloak[iClientID].iState)
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
		return;
	}

	USERCMD UserCmds[] = {
	    CreateUserCommand(L"/cloak", L"", UserCmd_Cloak, L""),
	};

	// Process user input
	bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd) { DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, global->returncode); }

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"cloak")
		{
			global->returncode = ReturnCode::SkipAll;

			uint iClientID = HkGetClientIdFromCharname(cmds->GetAdminName());
			if (iClientID == -1)
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

			if (!global->mapClientsCloak[iClientID].bCanCloak)
			{
				cmds->Print(L"ERR Cloaking device not available");
				return true;
			}

			switch (global->mapClientsCloak[iClientID].iState)
			{
				case STATE_CLOAK_OFF:
					global->mapClientsCloak[iClientID].bAdmin = true;
					SetState(iClientID, iShip, STATE_CLOAK_ON);
					break;
				case STATE_CLOAK_CHARGING:
				case STATE_CLOAK_ON:
					global->mapClientsCloak[iClientID].bAdmin = false;
					SetState(iClientID, iShip, STATE_CLOAK_OFF);
					break;
			}
			return true;
		}
		return false;
	}

	void __stdcall HkCb_AddDmgEntry(DamageList** dmg, unsigned short p1, float damage, enum DamageEntry::SubObjFate& fate)
	{
		DamageList* dmg2 = *dmg;
		if (g_DmgToSpaceID && dmg2->get_inflictor_id())
		{
			if (dmg2->get_cause() == 0x06)
			{
				float curr, max;
				pub::SpaceObj::GetHealth(g_DmgToSpaceID, curr, max);
				uint client = HkGetClientIDByShip(g_DmgToSpaceID);
				if (client)
				{
					if (global->mapClientsCloak[client].bCanCloak && !global->mapClientsCloak[client].bAdmin &&
					    global->mapClientsCloak[client].iState == STATE_CLOAK_CHARGING)
					{
						SetState(client, g_DmgToSpaceID, STATE_CLOAK_OFF);
					}
				}
			}
		}
	}
} // namespace Plugins::Cloak

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Cloak;
// REFL_AUTO must be global namespace
REFL_AUTO(type(Config::CLOAK_ARCH_REFLECTABLE), field(WarmupTime), field(CooldownTime), field(HoldSizeLimit), field(DropShieldsOnUncloak),
    field(FuelToUsage));
REFL_AUTO(type(Config), field(mapCloakingDevices), field(DsAce))


DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cloak by Cannon");
	pi->shortName("cloak");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::IEngine__AddDamageEntry, &HkCb_AddDmgEntry);
}
