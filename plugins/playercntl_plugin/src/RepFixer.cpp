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
#include "headers/FLHook.h"
#include "headers/plugin.h"
#include <math.h>
#include <list>
#include <set>

#include "PluginUtilities.h"
#include "Main.h"

#include "./headers/FLCoreServer.h"
#include "./headers/FLCoreCommon.h"

namespace RepFixer
{

	struct FactionRep
	{
		// The reputation group nickname to adjust.
		string scRepGroup;

		// The adjustment mode. If the player's reputation for scRepGroup
		// is greater than fRep then make the reputation equal to fRep
		static const int MODE_REP_LESSTHAN = 0;

		// The adjustment mode. If the player's reputation for scRepGroup
		// is less than fRep then make the reputation equal to fRep
		static const int MODE_REP_GREATERTHAN = 1;

		// Don't change anything/ignore this reputation group.
		static const int MODE_REP_NO_CHANGE = 2;

		// Fix the rep group to this level.
		static const int MODE_REP_STATIC = 3;

		// The adjustment mode.
		int iMode;

		// The reputation limit.
		float fRep;
	};

	/// Map of faction equipment IDs to reputations list.
	static map<unsigned int, list<FactionRep> > set_mapFactionReps;

	/// If true updates are logged to flhook.log
	static bool set_bLogUpdates = false;

	/// If true then the ID item must be mounted
	static bool set_bItemMustBeMounted = true;

	/// If true then do updates.
	static bool set_bEnableRepFixUpdates = true;

	/// Load the reputations for the specified equipment faction ID nickname.
	static void LoadFactionReps(const string &scPluginCfgFile, const string &scIDNick)
	{
		uint archID = CreateID(scIDNick.c_str());

		list<FactionRep> lstFactionReps;

		list<INISECTIONVALUE> lstValues;
		IniGetSection(scPluginCfgFile, scIDNick, lstValues);
		foreach (lstValues, INISECTIONVALUE, var)
		{
			if (var->scValue.size()>0)
			{
				FactionRep factionRep;
				factionRep.scRepGroup = var->scKey;

				factionRep.fRep = ToFloat(GetParam(stows(var->scValue), ',', 0));
				if (factionRep.fRep > 1.0f)
					factionRep.fRep = 1.0f;
				else if (factionRep.fRep < -1.0f)
					factionRep.fRep = -1.0f;

				factionRep.iMode = ToInt(GetParam(stows(var->scValue), ',', 1));
				if (factionRep.iMode == FactionRep::MODE_REP_LESSTHAN
					|| factionRep.iMode == FactionRep::MODE_REP_GREATERTHAN
					|| factionRep.iMode == FactionRep::MODE_REP_STATIC)
				{
					if (set_iPluginDebug>0)
					{
						ConPrint(L"NOTICE: Add reputation %s/%s rep=%0.2f mode=%d\n",
							stows(scIDNick).c_str(), stows(var->scKey).c_str(), factionRep.fRep, factionRep.iMode); 
					}
					lstFactionReps.push_back(factionRep);
				}
			}
		}

		set_mapFactionReps[archID] = lstFactionReps;
	}

	/// Load the plugin settings.
	void RepFixer::LoadSettings(const string &scPluginCfgFile)
	{
		set_bEnableRepFixUpdates = IniGetB(scPluginCfgFile, "RepFixer", "EnableRepFixUpdates", false);
		set_bLogUpdates = IniGetB(scPluginCfgFile, "RepFixer", "LogRepFixUpdates", false);
		set_bItemMustBeMounted = IniGetB(scPluginCfgFile, "RepFixer", "ItemMustBeMounted", true);

		// For each "ID/License" equipment item load the faction reputation list.
		set_mapFactionReps.clear();
		list<INISECTIONVALUE> lstValues;
		IniGetSection(scPluginCfgFile, "RepFixerItems", lstValues);
		foreach (lstValues, INISECTIONVALUE, var)
			LoadFactionReps(scPluginCfgFile, var->scKey);
	}

	/// For the specified client ID check and reset any factions that have reputations
	/// that are greater than the allowed value.
	static void CheckReps(unsigned int iClientID)
	{
		wstring wscCharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		list<CARGO_INFO> lstCargo;
		int remainingHoldSize = 0;
		HkEnumCargo(wscCharName, lstCargo, remainingHoldSize);

		foreach (lstCargo, CARGO_INFO, cargo)
		{
			// If the item is not mounted and we are only checking mounted items
			// then skip to the next one.
			if (!cargo->bMounted && set_bItemMustBeMounted)
				continue;

			// If the item is not an 'ID' then skip to the next one. 
			map<unsigned int, list<FactionRep> >::iterator iterIDs = set_mapFactionReps.find(cargo->iArchID);
			if (iterIDs==set_mapFactionReps.end())
				continue;

			// The item is an 'ID'; check and adjust the player reputations
			// if needed.
			for (list<FactionRep>::iterator iterReps = iterIDs->second.begin(); iterReps != iterIDs->second.end(); iterReps++)
			{
				const FactionRep &rep = *iterReps;

				float fRep = 0.0f;
				HkGetRep(wscCharName, stows(rep.scRepGroup), fRep);
				if (((fRep > rep.fRep) && (rep.iMode == FactionRep::MODE_REP_LESSTHAN))
					|| ((fRep < rep.fRep) && (rep.iMode == FactionRep::MODE_REP_GREATERTHAN)))
				{
					if (set_bLogUpdates)
					{
						AddLog("NOTICE: Updating reputation %s from %0.2f to %0.2f on %s (%s)",
							rep.scRepGroup.c_str(), fRep, rep.fRep,
							wstos(wscCharName).c_str(),
							wstos(HkGetAccountID(HkGetAccountByCharname(wscCharName))).c_str());
					}
					HkSetRep(wscCharName, stows(rep.scRepGroup), rep.fRep);
				}
				else if ((fRep != rep.fRep) && (rep.iMode == FactionRep::MODE_REP_STATIC))
				{
					if (set_bLogUpdates)
					{
						AddLog("NOTICE: Updating reputation %s from %0.2f to %0.2f on %s (%s)",
							rep.scRepGroup.c_str(), fRep, rep.fRep,
							wstos(wscCharName).c_str(),
							wstos(HkGetAccountID(HkGetAccountByCharname(wscCharName))).c_str());
					}
					HkSetRep(wscCharName, stows(rep.scRepGroup), rep.fRep);
				}
			}

			// We've adjusted the reps, stop searching the cargo list.
			return;
		}
	}

	void RepFixer::PlayerLaunch(unsigned int iShip, unsigned int iClientID)
	{
		if (set_bEnableRepFixUpdates)
			CheckReps(iClientID);
	}

	void RepFixer::BaseEnter(unsigned int iBaseID, unsigned int iClientID)
	{
		if (set_bEnableRepFixUpdates)
			CheckReps(iClientID);
	}
}