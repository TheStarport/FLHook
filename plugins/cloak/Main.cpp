/**
 * @date Unknown
 * @author Cannon (Ported by Raikkonen 2022)
 * @defgroup Cloak Cloak
 * @brief
 * The "Cloak" plugin allows players to cloak their ship if fitted with an appropriate cloaking device.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - cloak - Cloaks/uncloaks the ship.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "cloakingDevices": {
 *         "example": {
 *             "cooldownTime": 0,
 *             "dropShieldsOnUncloak": false,
 *             "fuelToUsage": {
 *                 "commodity_prisoners": 1
 *             },
 *             "holdSizeLimit": 0,
 *             "warmupTime": 0
 *         }
 *     },
 *     "dsAce": false
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * None
 */

// Includes
#include "Main.h"

namespace Plugins::Cloak
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		for (auto const& device : config.cloakingDevices)
		{
			CloakArch ca;
			ca.dropShieldsOnUncloak = device.second.dropShieldsOnUncloak;
			ca.cooldownTime = device.second.cooldownTime;
			ca.holdSizeLimit = device.second.holdSizeLimit;
			ca.warmupTime = device.second.warmupTime;

			for (const auto& [key, value] : device.second.fuelToUsage)
				if (key != "SomeItem")
					ca.fuelToUsage[CreateID(key.c_str())] = value;

			global->cloakingDevices[CreateID(device.first.c_str())] = ca;
		}

		if (config.dsAce)
		{
			global->cloakingText = L" Cloaking device on";
			global->uncloakingText = L" Cloaking device off";
		}

		global->config = std::make_unique<Config>(config);
	}

	/** @ingroup Cloak
	 * @brief Hook on ClearClientInfo. Remove the client from our data so we don't get confused if that client id gets recycled.
	 */
	void ClearClientInfo(uint iClientID) { global->clientCloakingInfo.erase(iClientID); }

	/** @ingroup Cloak
	 * @brief Set cloak to be on or off
	 */
	void SetCloak(uint iClientID, uint iShipID, bool bOn)
	{
		XActivateEquip ActivateEq;
		ActivateEq.bActivate = bOn;
		ActivateEq.iSpaceID = iShipID;
		ActivateEq.sID = global->clientCloakingInfo[iClientID].cloakSlot;
		Server.ActivateEquip(iClientID, ActivateEq);
	}

	/** @ingroup Cloak
	 * @brief Set the state of a cloaking device. e.g. Charging, On or Off.
	 */
	void SetState(uint iClientID, uint iShipID, int iNewState)
	{
		if (global->clientCloakingInfo[iClientID].state != iNewState)
		{
			global->clientCloakingInfo[iClientID].state = iNewState;
			global->clientCloakingInfo[iClientID].cloakTime = timeInMS();
			switch (iNewState)
			{
				case STATE_CLOAK_CHARGING: {
					PrintUserCmdText(iClientID, L"Preparing to cloak...");
					break;
				}

				case STATE_CLOAK_ON: {
					PrintUserCmdText(iClientID, global->cloakingText);
					SetCloak(iClientID, iShipID, true);
					PrintUserCmdText(iClientID, L"Cloaking device on");
					break;
				}
				case STATE_CLOAK_OFF:
				default: {
					PrintUserCmdText(iClientID, global->uncloakingText);
					SetCloak(iClientID, iShipID, false);
					PrintUserCmdText(iClientID, L"Cloaking device off");
					break;
				}
			}
		}
	}

	/** @ingroup Cloak
	 * @brief Returns false if the ship has no fuel to operate its cloaking device.
	 */
	static bool ProcessFuel(uint iClientID, CloakInfo& info)
	{
		if (info.admin || info.arch.fuelToUsage.empty())
			return true;

		for (auto item = Players[iClientID].equipDescList.equip.begin(); item != Players[iClientID].equipDescList.equip.end(); item++)
		{
			if (info.arch.fuelToUsage.find(item->iArchID) != info.arch.fuelToUsage.end())
			{
				uint fuel_usage = info.arch.fuelToUsage[item->iArchID];
				if (item->iCount >= fuel_usage)
				{
					pub::Player::RemoveCargo(iClientID, item->sID, fuel_usage);
					return true;
				}
			}
		}

		return false;
	}

	/** @ingroup Cloak
	 * @brief Hook on PlayerLaunch. Checks if they have a cloak and initialises it.
	 */
	void PlayerLaunch_AFTER(uint& iShip, uint& iClientID)
	{
		global->clientCloakingInfo[iClientID].canCloak = false;
		global->clientCloakingInfo[iClientID].admin = false;

		IObjInspectImpl* obj = GetInspect(iClientID);
		if (obj)
		{
			CShip* cship = (CShip*)GetEqObjFromObjRW((IObjRW*)obj);

			CEquipTraverser tr;

			for (CEquip* equip = GetEquipManager(cship)->Traverse(tr); equip; equip = GetEquipManager(cship)->Traverse(tr))
			{
				if (CECloakingDevice::cast(equip))
				{
					global->clientCloakingInfo[iClientID].cloakSlot = equip->GetID();

					if (global->cloakingDevices.find(equip->EquipArch()->iArchID) != global->cloakingDevices.end())
					{
						// Otherwise set the fuel usage and warm up time
						global->clientCloakingInfo[iClientID].arch = global->cloakingDevices[equip->EquipArch()->iArchID];
					}
					// If this cloaking device does not appear in the cloaking
					// device list then warming up and fuel usage is zero and it may
					// be used by any ship.
					else
					{
						global->clientCloakingInfo[iClientID].arch.dropShieldsOnUncloak = false;
						global->clientCloakingInfo[iClientID].arch.cooldownTime = 0;
						global->clientCloakingInfo[iClientID].arch.holdSizeLimit = 0;
						global->clientCloakingInfo[iClientID].arch.warmupTime = 0;
						global->clientCloakingInfo[iClientID].arch.fuelToUsage.clear();
					}

					global->clientCloakingInfo[iClientID].canCloak = true;
					global->clientCloakingInfo[iClientID].state = STATE_CLOAK_INVALID;
					SetState(iClientID, iShip, STATE_CLOAK_OFF);
					return;
				}
			}
		}
	}

	/** @ingroup Cloak
	 * @brief Hook on BaseEnter. Removes the client from our data.
	 */
	void BaseEnter(uint& iBaseID, uint& iClientID) { global->clientCloakingInfo.erase(iClientID); }

	/** @ingroup Cloak
	 * @brief A timer function. Actions the cloaking device bases on the state.
	 */
	void TimerCheckKick()
	{
		mstime now = timeInMS();

		for (std::map<uint, CloakInfo>::iterator ci = global->clientCloakingInfo.begin(); ci != global->clientCloakingInfo.end(); ++ci)
		{
			uint iClientID = ci->first;
			uint iShipID = Players[iClientID].iShipID;
			CloakInfo& info = ci->second;

			if (iShipID && info.canCloak)
			{
				switch (info.state)
				{
					case STATE_CLOAK_OFF:
						// Send cloak state for uncloaked cloak-able players (only for
						// them in space) this is the code to fix the bug where players
						// wouldnt always see uncloaked players
						XActivateEquip ActivateEq;
						ActivateEq.bActivate = false;
						ActivateEq.iSpaceID = iShipID;
						ActivateEq.sID = info.cloakSlot;
						Server.ActivateEquip(iClientID, ActivateEq);
						break;

					case STATE_CLOAK_CHARGING:
						if (!ProcessFuel(iClientID, info))
						{
							PrintUserCmdText(iClientID, L"Cloaking device shutdown, no fuel");
							SetState(iClientID, iShipID, STATE_CLOAK_OFF);
						}
						else if ((info.cloakTime + info.arch.warmupTime) < now)
						{
							SetState(iClientID, iShipID, STATE_CLOAK_ON);
						}
						else if (info.arch.dropShieldsOnUncloak && !info.admin)
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
						else if (info.arch.dropShieldsOnUncloak && !info.admin)
						{
							pub::SpaceObj::DrainShields(iShipID);
						}
						break;
				}
			}
		}
	}

	/** @ingroup Cloak
	 * @brief Is called when the player types /cloak. Sets the cloaking device state accordingly.
	 */
	void UserCmd_Cloak(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint iShip;
		pub::Player::GetShip(iClientID, iShip);
		if (!iShip)
		{
			PrintUserCmdText(iClientID, L"Not in space");
			return;
		}

		if (!global->clientCloakingInfo[iClientID].canCloak)
		{
			PrintUserCmdText(iClientID, L"Cloaking device not available");
			return;
		}

		// If this cloaking device requires more power than the ship can provide
		// no cloaking device is available.
		IObjInspectImpl* obj = GetInspect(iClientID);
		if (obj)
		{
			CShip* cship = (CShip*)GetEqObjFromObjRW((IObjRW*)obj);
			if (cship)
			{
				if (global->clientCloakingInfo[iClientID].arch.holdSizeLimit != 0 &&
				    global->clientCloakingInfo[iClientID].arch.holdSizeLimit < cship->shiparch()->fHoldSize)
				{
					PrintUserCmdText(iClientID, L"Cloaking device will not function on this ship type");
					global->clientCloakingInfo[iClientID].state = STATE_CLOAK_INVALID;
					SetState(iClientID, iShip, STATE_CLOAK_OFF);
					return;
				}

				switch (global->clientCloakingInfo[iClientID].state)
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

	const std::vector commands = {
	    CreateUserCommand(L"/cloak", L"", UserCmd_Cloak, L"This cloaks or uncloaks the player."),
	};

	/** @ingroup Cloak
	 * @brief Admin command processing.
	 */
	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"cloak")
		{
			global->returncode = ReturnCode::SkipAll;

			uint iClientID = GetClientIdFromCharname(cmds->GetAdminName());
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

			if (!global->clientCloakingInfo[iClientID].canCloak)
			{
				cmds->Print(L"ERR Cloaking device not available");
				return true;
			}

			switch (global->clientCloakingInfo[iClientID].state)
			{
				case STATE_CLOAK_OFF:
					global->clientCloakingInfo[iClientID].admin = true;
					SetState(iClientID, iShip, STATE_CLOAK_ON);
					break;
				case STATE_CLOAK_CHARGING:
				case STATE_CLOAK_ON:
					global->clientCloakingInfo[iClientID].admin = false;
					SetState(iClientID, iShip, STATE_CLOAK_OFF);
					break;
			}
			return true;
		}
		return false;
	}

	/** @ingroup Cloak
	 * @brief Hook on Cb_AddDmgEntry. Interrupts the cloak if the player is hit whilst charging.
	 */
	void __stdcall Cb_AddDmgEntry(DamageList** dmg, unsigned short p1, float damage, enum DamageEntry::SubObjFate& fate)
	{
		DamageList* dmg2 = *dmg;
		if (g_DmgToSpaceID && dmg2->get_inflictor_id())
		{
			if (dmg2->get_cause() == DamageCause::CruiseDisrupter)
			{
				float curr, max;
				pub::SpaceObj::GetHealth(g_DmgToSpaceID, curr, max);
				uint client = GetClientIDByShip(g_DmgToSpaceID);
				if (client)
				{
					if (global->clientCloakingInfo[client].canCloak && !global->clientCloakingInfo[client].admin &&
					    global->clientCloakingInfo[client].state == STATE_CLOAK_CHARGING)
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
REFL_AUTO(type(Config::CloakArch), field(warmupTime), field(cooldownTime), field(holdSizeLimit), field(dropShieldsOnUncloak),
    field(fuelToUsage));
REFL_AUTO(type(Config), field(cloakingDevices), field(dsAce))


DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cloak by Cannon");
	pi->shortName("cloak");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &TimerCheckKick);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::IEngine__AddDamageEntry, &Cb_AddDmgEntry);
}
