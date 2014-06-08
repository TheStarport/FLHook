// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <float.h>
#include <FLHook.h>
#include <plugin.h>
#include <math.h>
#include <list>
#include <set>

#include "PluginUtilities.h"
#include "Main.h"

#include <FLCoreServer.h>
#include <FLCoreCommon.h>

namespace MiscCmds
{
	struct INFO
	{
		INFO() : bLightsOn(false), bShieldsDown(false), bSelfDestruct(false)
		{}

		/// Lights on/off
		bool bLightsOn;

		/// Shields up/down
		bool bShieldsDown;

		/// Self destruct
		bool bSelfDestruct;
	};

	/** A list of clients that are being smited */
	map<uint, INFO> mapInfo;
	typedef map<uint, INFO, less<uint> >::value_type mapInfo_map_pair_t;
	typedef map<uint, INFO, less<uint> >::iterator mapInfo_map_iter_t;

	wstring set_wscStuckMsg = L"Attention! Stand clear. Towing %player";
	wstring set_wscDiceMsg = L"%player rolled %number";
	wstring set_wscCoinMsg = L"%player tossed %result";

	/// ID of music to play when smiting players.
	uint set_iSmiteMusicID = 0;

	/// Cost to drop reputation changes.
	int set_iRepdropCost = 0;

	/// Load the configuration
	void MiscCmds::LoadSettings(const string &scPluginCfgFile)
	{
		// Load generic settings
		set_iRepdropCost = IniGetI(scPluginCfgFile, "General", "RepDropCost", 0);

		set_wscStuckMsg = stows(IniGetS(scPluginCfgFile, "General", "StuckMsg", "Attention! Stand clear. Towing %player"));
		set_wscDiceMsg = stows(IniGetS(scPluginCfgFile, "General", "DiceMsg", "%player rolled %number of %max"));
		set_wscCoinMsg = stows(IniGetS(scPluginCfgFile, "General", "CoinMsg", "%player tossed %result"));

		set_iSmiteMusicID = CreateID(IniGetS(scPluginCfgFile, "General", "SmiteMusic", "music_danger").c_str());
	}

	/** Clean up when a client disconnects */
	void MiscCmds::ClearClientInfo(uint iClientID)
	{
		if (mapInfo[iClientID].bSelfDestruct)
		{
			mapInfo[iClientID].bSelfDestruct = false;
			uint dummy[3] = { 0 };
			pub::Player::SetShipAndLoadout(iClientID, CreateID("dsy_ge_fighter"),(const EquipDescVector&) dummy);
		}
		mapInfo.erase(iClientID);
	}

	void MiscCmds::CharacterInfoReq(unsigned int iClientID, bool p2)
	{
		if (mapInfo[iClientID].bSelfDestruct)
		{
			mapInfo[iClientID].bSelfDestruct = false;
			uint dummy[3] = { 0 };
			pub::Player::SetShipAndLoadout(iClientID, CreateID("dsy_ge_fighter"),(const EquipDescVector&) dummy);
		}
	}

	/** One second timer */
	void MiscCmds::Timer()
	{
		// Drop player sheilds and keep them down.
		for (mapInfo_map_iter_t iter = mapInfo.begin(); iter!=mapInfo.end(); iter++)
		{
			if (iter->second.bShieldsDown)
			{
				HKPLAYERINFO p;
				if (HkGetPlayerInfo((const wchar_t*) Players.GetActiveCharacterName(iter->first), p, false)==HKE_OK && p.iShip)
				{
					pub::SpaceObj::DrainShields(p.iShip);
				}
			}
		}
	}

	/** Print current position */
	bool MiscCmds::UserCmd_Pos(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		HKPLAYERINFO p;
		if (HkGetPlayerInfo((const wchar_t*) Players.GetActiveCharacterName(iClientID), p, false)!=HKE_OK || p.iShip==0)
		{
			PrintUserCmdText(iClientID, L"ERR Not in space");
			return true;
		}

		Vector pos;
		Matrix rot;
		pub::SpaceObj::GetLocation(p.iShip, pos, rot);

		Vector erot = MatrixToEuler(rot);

		wchar_t buf[100];
		_snwprintf(buf, sizeof(buf), L"Position %0.0f %0.0f %0.0f Orient %0.0f %0.0f %0.0f",
			pos.x, pos.y, pos.z, erot.x, erot.y, erot.z);
		PrintUserCmdText(iClientID, buf);
		return true;
	}

	/** Move a ship a little if it is stuck in the base */
	bool MiscCmds::UserCmd_Stuck(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		wstring wscCharname = (const wchar_t*) Players.GetActiveCharacterName(iClientID);

		HKPLAYERINFO p;
		if (HkGetPlayerInfo(wscCharname, p, false)!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR Not in space");
			return true;
		}

		Vector dir1;
		Vector dir2;
		pub::SpaceObj::GetMotion(p.iShip, dir1, dir2);
		if (dir1.x>5 || dir1.y>5 || dir1.z>5)
		{
			PrintUserCmdText(iClientID, L"ERR Ship is moving");
			return true;
		}

		Vector pos;
		Matrix rot;
		pub::SpaceObj::GetLocation(p.iShip, pos, rot);
		pos.x += 15;
		pos.y += 15;
		pos.z += 15;
		HkRelocateClient(iClientID, pos, rot);

		wstring wscMsg = set_wscStuckMsg;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);

		return true;
	}

	/** A command to help remove any affiliation that you might have */
	bool MiscCmds::UserCmd_DropRep(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		HK_ERROR err; 

		wstring wscCharname = (const wchar_t*) Players.GetActiveCharacterName(iClientID);

		wstring wscRepGroupNick;
		if (HkFLIniGet(wscCharname, L"rep_group", wscRepGroupNick)!=HKE_OK || wscRepGroupNick.length()==0)
		{
			PrintUserCmdText(iClientID, L"ERR No affiliation");
			return true;
		}

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int iCash = 0;
		if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
			return true;
		}
		if (set_iRepdropCost>0 && iCash<set_iRepdropCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return true;
		}

		float fValue = 0.0f;
		if ((err = HkGetRep(wscCharname, wscRepGroupNick, fValue)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
			return true;
		}

		HkSetRep(wscCharname, wscRepGroupNick, 0.599f);
		PrintUserCmdText(iClientID, L"OK Reputation dropped, logout for change to take effect.");

		// Remove cash if we're charging for it.
		if (set_iRepdropCost>0)
		{
			HkAddCash(wscCharname, 0-set_iRepdropCost);
		}

		return true;
	}

	/** Throw the dice and tell all players within 6 km */
	bool MiscCmds::UserCmd_Dice(uint iFromClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		wstring wscCharname = (const wchar_t*) Players.GetActiveCharacterName(iFromClientID);

		int max = ToInt(GetParam(wscParam, ' ', 0));
		if (max<=1)
			max = 6;

		uint number = (rand()%max)+1;
		wstring wscMsg = set_wscDiceMsg;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		wscMsg = ReplaceStr(wscMsg, L"%number", stows(itos(number)));
		wscMsg = ReplaceStr(wscMsg, L"%max", stows(itos(max)));
		PrintLocalUserCmdText(iFromClientID, wscMsg, 6000.0f);
		return true;
	}

	/** Throw the dice and tell all players within 6 km */
	bool MiscCmds::UserCmd_Coin(uint iFromClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		wstring wscCharname = (const wchar_t*) Players.GetActiveCharacterName(iFromClientID);

		uint number = (rand()%2);
		wstring wscMsg = set_wscCoinMsg;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		wscMsg = ReplaceStr(wscMsg, L"%result", (number==1)?L"heads":L"tails");
		PrintLocalUserCmdText(iFromClientID, wscMsg, 6000.0f);
		return true;
	}

	/** Smite all players in radar range */
	void MiscCmds::AdminCmd_SmiteAll(CCmds* cmds)
	{
		if(!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		HKPLAYERINFO adminPlyr;
		if (HkGetPlayerInfo(cmds->GetAdminName(), adminPlyr, false)!=HKE_OK || adminPlyr.iShip==0)
		{
			cmds->Print(L"ERR Not in space\n");
			return;
		}

		bool bKillAll = cmds->ArgStr(1)==L"die";

		Vector vFromShipLoc;
		Matrix mFromShipDir;
		pub::SpaceObj::GetLocation(adminPlyr.iShip, vFromShipLoc, mFromShipDir);

		pub::Audio::Tryptich music;
		music.iDunno = 0;
		music.iDunno2 = 0;
		music.iDunno3 = 0;
		music.iMusicID = set_iSmiteMusicID;
		pub::Audio::SetMusic(adminPlyr.iClientID, music);

		// For all players in system...
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
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
			float fDistance = HkDistance3D(vShipLoc, vFromShipLoc);
			if (fDistance>14999)
				continue;
		
			pub::Audio::Tryptich music;
			music.iDunno = 0;
			music.iDunno2 = 0;
			music.iDunno3 = 0;
			music.iMusicID = set_iSmiteMusicID;
			pub::Audio::SetMusic(iClientID, music);

			mapInfo[iClientID].bShieldsDown = true;

			if (bKillAll)
			{
				IObjInspectImpl *obj = HkGetInspect(iClientID);
				if (obj)
				{
					HkLightFuse((IObjRW*)obj, CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
				}
			}
		}
		
		cmds->Print(L"OK\n"); 
		return;
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
		EquipDescListItem *eqLst = Players[iClientID].equipDescList.pFirst;
		for (EquipDescListItem *eq = eqLst->next; eq != eqLst; eq = eq->next)
		{
			string hp = ToLower(eq->equip.szHardPoint.value);
			if (hp.find("dock")!=string::npos)
			{
				XActivateEquip ActivateEq;
				ActivateEq.bActivate = bOn;
				ActivateEq.iSpaceID = iShip;
				ActivateEq.sID = eq->equip.sID;
				Server.ActivateEquip(iClientID,ActivateEq);
				bLights = true;
			}
		}

		if (bLights)
			PrintUserCmdText(iClientID, L" Lights %s", bOn ? L"on" : L"off");
		else
			PrintUserCmdText(iClientID, L"Light control not available");
	}

	bool MiscCmds::UserCmd_Lights(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		mapInfo[iClientID].bLightsOn = !mapInfo[iClientID].bLightsOn;
		SetLights(iClientID, mapInfo[iClientID].bLightsOn);
		return true;
	}

	void MiscCmds::BaseEnter(unsigned int iBaseID, unsigned int iClientID)
	{
	}

	bool MiscCmds::UserCmd_SelfDestruct(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		IObjInspectImpl *obj = HkGetInspect(iClientID);
		if (!obj)
		{
			PrintUserCmdText(iClientID, L"Self destruct prohibited. Not in space.");
			return true;
		}

		if (wscParam == L"0000")
		{
			PrintUserCmdText(iClientID, L"Self destruct enabled. Standby.");
			PrintUserCmdText(iClientID, L"Ejecting pod...");
			HkLightFuse((IObjRW*)obj, CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
			mapInfo[iClientID].bSelfDestruct = true;
		}
		else
		{
			PrintUserCmdText(iClientID, L"WARNING! SELF DESTRUCT WILL COMPLETELY AND PERMANENTLY DESTROY SHIP.");
			PrintUserCmdText(iClientID, L"WARNING! WARNING! SECURITY CONFIRMATION REQUIRED. TYPE");
			PrintUserCmdText(iClientID, L"/selfdestruct 0000");
		}
		return true;
	}

	bool MiscCmds::UserCmd_Screenshot(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		Message::UserCmd_Time(iClientID, L"", L"", L"");
		PrintUserCmdText(iClientID, L" SS ");
		return true;
	}

	bool MiscCmds::UserCmd_Shields(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		mapInfo[iClientID].bShieldsDown = !mapInfo[iClientID].bShieldsDown;
		PrintUserCmdText(iClientID, L"Shields %s", mapInfo[iClientID].bShieldsDown ? L"Disabled":L"Enabled");
		return true;
	}

}