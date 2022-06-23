// Misc Commands plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

// Setup Doxygen Group

/** @defgroup MiscCommands Misc Commands (plugin) */

namespace Plugins::MiscCommands
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// Load the configuration
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->smiteMusicHash = CreateID(config.smiteMusicId.c_str());
		global->config = std::make_unique<Config>(config);
	}

	/** Clean up when a client disconnects */
	void ClearClientInfo(const uint& iClientID) { global->mapInfo.erase(iClientID); }

	/** One second timer */
	void Timer()
	{
		// Drop player shields and keep them down.
		for (const auto& [id, info] : global->mapInfo)
		{
			if (info.bShieldsDown)
			{
				HKPLAYERINFO p;
				if (HkGetPlayerInfo(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(id)), p, false) == HKE_OK && p.iShip)
				{
					pub::SpaceObj::DrainShields(p.iShip);
				}
			}
		}
	}

	static void SetLights(uint iClientID, bool bOn)
	{
		uint iShip;
		pub::Player::GetShip(iClientID, iShip);
		if (!iShip)
		{
			PrintUserCmdText(iClientID, L"ERR Not in space");
			return;
		}

		bool bLights = false;
		st6::list<EquipDesc> const& eqLst = Players[iClientID].equipDescList.equip;
		for (const auto& eq : eqLst)
		{
			std::string hp = ToLower(eq.szHardPoint.value);
			if (hp.find("dock") != std::string::npos)
			{
				XActivateEquip ActivateEq;
				ActivateEq.bActivate = bOn;
				ActivateEq.iSpaceID = iShip;
				ActivateEq.sID = eq.sID;
				Server.ActivateEquip(iClientID, ActivateEq);
				bLights = true;
			}
		}

		if (bLights)
			PrintUserCmdText(iClientID, L" Lights %s", bOn ? L"on" : L"off");
		else
			PrintUserCmdText(iClientID, L"Light control not available");
	}

	/** @ingroup MiscCommands
	 * @brief Print the current location of your ship
	 */
	void UserCmdPos(const uint& iClientID, const std::wstring_view& wscParam)
	{
		HKPLAYERINFO p;
		if (HkGetPlayerInfo(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID)), p, false) != HKE_OK || p.iShip == 0)
		{
			PrintUserCmdText(iClientID, L"ERR Not in space");
			return;
		}

		Vector pos;
		Matrix rot;
		pub::SpaceObj::GetLocation(p.iShip, pos, rot);
		
		Vector erot = MatrixToEuler(rot);

		wchar_t buf[100];
		_snwprintf_s(buf, sizeof(buf), L"Position %0.0f %0.0f %0.0f Orient %0.0f %0.0f %0.0f", pos.x, pos.y, pos.z, erot.x, erot.y, erot.z);
		PrintUserCmdText(iClientID, buf);
	}

	/** @ingroup MiscCommands
	 * @brief Nudge your ship 15 meters on all axis to try and dislodge a stuck ship.
	 */
	void UserCmdStuck(const uint& iClientID, const std::wstring_view& wscParam)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		HKPLAYERINFO p;
		if (HkGetPlayerInfo(wscCharname, p, false) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR Not in space");
			return;
		}

		Vector dir1;
		Vector dir2;
		pub::SpaceObj::GetMotion(p.iShip, dir1, dir2);
		if (dir1.x > 5 || dir1.y > 5 || dir1.z > 5)
		{
			PrintUserCmdText(iClientID, L"ERR Ship is moving");
			return;
		}

		Vector pos;
		Matrix rot;
		pub::SpaceObj::GetLocation(p.iShip, pos, rot);
		pos.x += 15;
		pos.y += 15;
		pos.z += 15;
		HkRelocateClient(iClientID, pos, rot);

		std::wstring wscMsg = global->config->stuckMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
	}

	/** @ingroup MiscCommands
	 * @brief Command to remove your current affiliation if applicable.
	 */
	void UserCmdDropRep(const uint& iClientID, const std::wstring_view& wscParam)
	{
		if (global->config->repDropCost < 0)
		{
			PrintUserCmdText(iClientID, L"Command Disabled");
			return;
		}

		std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));

		std::wstring wscRepGroupNick;
		if (HkFLIniGet(wscCharname, L"rep_group", wscRepGroupNick) != HKE_OK || wscRepGroupNick.length() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR No affiliation");
			return;
		}

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int iCash = 0;
		if (HK_ERROR err; (err = HkGetCash(wscCharname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
			return;
		}
		if (global->config->repDropCost > 0 && iCash < global->config->repDropCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return;
		}

		float fValue = 0.0f;
		if (HK_ERROR err; (err = HkGetRep(wscCharname, wscRepGroupNick, fValue)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
			return;
		}

		HkSetRep(wscCharname, wscRepGroupNick, 0.599f);
		PrintUserCmdText(iClientID, L"OK Reputation dropped, logout for change to take effect.");

		// Remove cash if we're charging for it.
		if (global->config->repDropCost > 0)
		{
			HkAddCash(wscCharname, 0 - global->config->repDropCost);
		}
	}

	/** @ingroup MiscCommands
	 * @brief Roll a dice with the specified number of sides, or 6 is not specified.
	 */
	void UserCmdDice(const uint& iClientID, const std::wstring_view& wscParam)
	{
		std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));

		int max = ToInt(GetParam(wscParam, ' ', 0));
		if (max <= 1)
			max = 6;

		const uint number = rand() % max + 1;
		std::wstring wscMsg = global->config->diceMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		wscMsg = ReplaceStr(wscMsg, L"%number", std::to_wstring(number));
		wscMsg = ReplaceStr(wscMsg, L"%max", std::to_wstring(max));
		PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
	}

	/** @ingroup MiscCommands
	 * @brief Throw the dice and tell all players within 6 km
	 */
	void UserCmdCoin(const uint& iClientID, const std::wstring_view& wscParam)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		uint number = (rand() % 2);
		std::wstring wscMsg = global->config->coinMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		wscMsg = ReplaceStr(wscMsg, L"%result", (number == 1) ? L"heads" : L"tails");
		PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
	}

	/** @ingroup MiscCommands
	 * @brief Activate or deactivate docking lights on your ship.
	 */
	void UserCmdLights(const uint& iClientID, const std::wstring_view& wscParam)
	{
		global->mapInfo[iClientID].bLightsOn = !global->mapInfo[iClientID].bLightsOn;
		SetLights(iClientID, global->mapInfo[iClientID].bLightsOn);
	}

	/** @ingroup MiscCommands
	 * @brief Disable/Enable your shields at will.
	 */
	void UserCmdShields(const uint& iClientID, const std::wstring_view& wscParam)
	{
		global->mapInfo[iClientID].bShieldsDown = !global->mapInfo[iClientID].bShieldsDown;
		PrintUserCmdText(iClientID, L"Shields %s", global->mapInfo[iClientID].bShieldsDown ? L"Disabled" : L"Enabled");
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/lights", L"", UserCmdLights, L""),
	    CreateUserCommand(L"/shields", L"", UserCmdShields, L""),
	    CreateUserCommand(L"/pos", L"", UserCmdPos, L""),
	    CreateUserCommand(L"/stuck", L"", UserCmdStuck, L""),
	    CreateUserCommand(L"/droprep", L"", UserCmdDropRep, L""),
	    CreateUserCommand(L"/dice", L"", UserCmdDice, L""),
	    CreateUserCommand(L"/coin", L"", UserCmdCoin, L""),
	}};

	/** @} */ // End of user commands

	//! Smite all players in radar range
	void AdminCmdSmiteAll(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		HKPLAYERINFO adminPlyr;
		if (HkGetPlayerInfo(cmds->GetAdminName(), adminPlyr, false) != HKE_OK || adminPlyr.iShip == 0)
		{
			cmds->Print(L"ERR Not in space");
			return;
		}

		const bool bKillAll = cmds->ArgStr(1) == L"die";

		Vector vFromShipLoc;
		Matrix mFromShipDir;
		pub::SpaceObj::GetLocation(adminPlyr.iShip, vFromShipLoc, mFromShipDir);

		pub::Audio::Tryptich music;
		music.iDunno = 0;
		music.iDunno2 = 0;
		music.iDunno3 = 0;
		music.iMusicID = global->smiteMusicHash;
		pub::Audio::SetMusic(adminPlyr.iClientID, music);

		// For all players in system...
		struct PlayerData* pPD = nullptr;
		while ((pPD = Players.traverse_active(pPD)))
		{
			// Get the this player's current system and location in the system.
			uint iClientID = HkGetClientIdFromPD(pPD);
			if (iClientID == adminPlyr.iClientID)
				continue;

			uint iClientSystem = 0;
			pub::Player::GetSystem(iClientID, iClientSystem);
			if (adminPlyr.iSystem != iClientSystem)
				continue;

			uint iShip;
			pub::Player::GetShip(iClientID, iShip);

			Vector vShipLoc;
			Matrix mShipDir;
			pub::SpaceObj::GetLocation(iShip, vShipLoc, mShipDir);

			// Is player within scanner range (15K) of the sending char.
			if (HkDistance3D(vShipLoc, vFromShipLoc) > 14999)
				continue;

			pub::Audio::SetMusic(iClientID, music);

			global->mapInfo[iClientID].bShieldsDown = true;

			if (bKillAll)
			{
				if (IObjInspectImpl* obj = HkGetInspect(iClientID))
				{
					HkLightFuse(reinterpret_cast<IObjRW*>(obj), CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
				}
			}
		}

		cmds->Print(L"OK");
		return;
	}

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"smiteall")
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmdSmiteAll(cmds);
			return true;
		}
		return false;
	}
} // namespace Plugins::MiscCommands

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::MiscCommands;

// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(repDropCost), field(stuckMessage), field(diceMessage), field(coinMessage), field(smiteMusicId))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Misc Commands");
	pi->shortName("MiscCommands");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__TimerNPCAndF1Check, &Timer);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
