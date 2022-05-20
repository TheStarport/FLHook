// Misc Commands plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::MiscCommands
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/// Load the configuration
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

	/** @defgroup cmds User Commands
	 * @{
	 */

	/** Print current position */
	void UserCmdPos(uint iClientID, [[maybe_unused]] const std::wstring& wscParam)
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

	/** Move a ship a little if it is stuck in the base */
	void UserCmdStuck(uint iClientID, [[maybe_unused]] const std::wstring& wscParam)
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

	/** A command to help remove any affiliation that you might have */
	void UserCmdDropRep(uint iClientID, [[maybe_unused]] const std::wstring& wscParam)
	{
		if (global->config->repDropCost < 0)
			return;

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

	/** Throw the dice and tell all players within 6 km */
	void UserCmdDice(uint iClientID, const std::wstring& wscParam)
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

	/** Throw the dice and tell all players within 6 km */
	void UserCmdCoin(uint iClientID, const std::wstring& wscParam)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		uint number = (rand() % 2);
		std::wstring wscMsg = global->config->coinMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		wscMsg = ReplaceStr(wscMsg, L"%result", (number == 1) ? L"heads" : L"tails");
		PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
	}

	void UserCmdLights(uint iClientID, const std::wstring& wscParam)
	{
		global->mapInfo[iClientID].bLightsOn = !global->mapInfo[iClientID].bLightsOn;
		SetLights(iClientID, global->mapInfo[iClientID].bLightsOn);
	}

	void UserCmdShields(uint iClientID, const std::wstring& wscParam)
	{
		global->mapInfo[iClientID].bShieldsDown = !global->mapInfo[iClientID].bShieldsDown;
		PrintUserCmdText(iClientID, L"Shields %s", global->mapInfo[iClientID].bShieldsDown ? L"Disabled" : L"Enabled");
	}

	// Client command processing
	const std::vector<USERCMD> UserCmds = {
	    {L"/lights", UserCmdLights},
	    {L"/shields", UserCmdShields},
	    {L"/pos", UserCmdPos},
	    {L"/stuck", UserCmdStuck},
	    {L"/droprep", UserCmdDropRep},
	    {L"/dice", UserCmdDice},
	    {L"/coin", UserCmdCoin},
	};

	/** @} */ // End of user commands

	// Process user input
	bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd) { DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, global->returncode); }

	// Hook on /help
	EXPORT void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
	{
		for (auto& uc : UserCmds)
		{
			PrintUserCmdText(iClientID, uc.wszCmd);
		}
	}

	/** @defgroup admin Admin Commands
	 * @{
	 */

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

	/** @} */ // End of Admin Commands

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (IS_CMD("smiteall"))
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

// Do things when the dll is loaded
BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE hinstDll, [[maybe_unused]] DWORD fdwReason, [[maybe_unused]] LPVOID lpvReserved)
{
	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Misc Commands");
	pi->shortName("MiscCommands");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__TimerNPCAndF1Check, &Timer);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
