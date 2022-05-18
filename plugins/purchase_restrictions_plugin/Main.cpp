// Purchase Restrictions plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

/// Log items of interest so we can see what cargo cheats people are using.
static void LogItemsOfInterest(uint iClientID, uint iGoodID, const std::string& details)
{
	auto iter = set_mapItemsOfInterest.find(iGoodID);
	if (iter != set_mapItemsOfInterest.end())
	{
		std::wstring wscCharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
		AddLog(
		    Normal, L"Item '%s' found in cargo of %s (%s) %s", iter->second.c_str(), wscCharName.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscCharName)).c_str(), details.c_str());
	}
}

/// Load the configuration
void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\purchase_restrictions.cfg";

	set_bCheckIDRestrictions = IniGetB(scPluginCfgFile, "PurchaseRestrictions", "CheckIDRestrictions", false);
	set_bEnforceIDRestrictions = IniGetB(scPluginCfgFile, "PurchaseRestrictions", "EnforceIDRestrictions", false);
	set_wscShipPurchaseDenied = stows(IniGetS(
	    scPluginCfgFile, "PurchaseRestrictions", "ShipPurchaseDeniedMsg",
	    "ERR You cannot buy this ship because you do not have the correct "
	    "ID."));
	set_wscGoodPurchaseDenied = stows(IniGetS(
	    scPluginCfgFile, "PurchaseRestrictions", "GoodPurchaseDeniedMsg",
	    "ERR You cannot buy this item because you do not have the correct "
	    "ID."));

	INI_Reader ini;
	if (ini.open(scPluginCfgFile.c_str(), false))
	{
		while (ini.read_header())
		{
			// Read the no buy items list (base-nick = good-nick)
			if (ini.is_header("NoBuy"))
			{
				while (ini.read_value())
				{
					uint baseID = CreateID(ini.get_name_ptr());
					uint goodID = CreateID(ini.get_value_string());
					set_mapNoBuy.insert(mapnobuy_map_pair_t(baseID, goodID));
				}
			}
			// Read the log items list (item-nick)
			else if (ini.is_header("LogItems"))
			{
				while (ini.read_value())
				{
					uint itemID = CreateID(ini.get_name_ptr());
					std::string itemNick = ini.get_value_string();
					set_mapItemsOfInterest[itemID] = itemNick;
				}
			}
			// Read the ID mounting/buying restrictions list.
			// ID, good
			else if (ini.is_header("GoodItemRestrictions"))
			{
				while (ini.read_value())
				{
					uint goodID = CreateID(ini.get_name_ptr());
					uint itemID = CreateID(ini.get_value_string());
					if (goodID != -1 && itemID != -1)
					{
						set_mapGoodItemRestrictions.insert(mapGoodItemRestrictions_map_pair_t(goodID, itemID));
					}
				}
			}
			// Read the ID mounting/buying restrictions list.
			// ship, ID
			else if (ini.is_header("ShipItemRestrictions"))
			{
				while (ini.read_value())
				{
					uint shipID = CreateID(ini.get_name_ptr());
					uint itemID = CreateID(ini.get_value_string());
					if (shipID != -1 && itemID != -1)
					{
						set_mapShipItemRestrictions.insert(mapGoodItemRestrictions_map_pair_t(shipID, itemID));
					}
				}
			}
		}
		ini.close();
	}
}

/// Check that this client is allowed to buy/mount this piece of equipment or
/// ship Return true if the equipment is mounted to allow this good.
bool CheckIDEquipRestrictions(uint iClientID, uint iGoodID)
{
	std::list<CARGO_INFO> lstCargo;
	int iRemainingHoldSize;
	HkEnumCargo((const wchar_t*)Players.GetActiveCharacterName(iClientID), lstCargo, iRemainingHoldSize);
	for (auto& cargo : lstCargo)
	{
		if (cargo.bMounted)
		{
			auto start = set_mapGoodItemRestrictions.lower_bound(iGoodID);
			auto end = set_mapGoodItemRestrictions.upper_bound(iGoodID);
			for (auto i = start; i != end; ++i)
			{
				if (cargo.iArchID == i->second)
				{
					return true;
				}
			}

			auto start2 = set_mapShipItemRestrictions.lower_bound(iGoodID);
			auto end2 = set_mapShipItemRestrictions.upper_bound(iGoodID);
			for (auto i = start2; i != end2; ++i)
			{
				if (cargo.iArchID == i->second)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void ClearClientInfo(uint& iClientID)
{
	mapInfo[iClientID].bSuppressBuy = false;
}

void PlayerLaunch(uint& iShip, uint& iClientID)
{
	mapInfo[iClientID].bSuppressBuy = false;
}

void BaseEnter(uint& iBaseID, uint& iClientID)
{
	mapInfo[iClientID].bSuppressBuy = false;
}

/// Suppress the buying of goods.
bool GFGoodBuy(struct SGFGoodBuyInfo const& gbi, uint& iClientID)
{
	mapInfo[iClientID].bSuppressBuy = false;
	LogItemsOfInterest(iClientID, gbi.iGoodID, "good-buy");

	/// Check to see if this item is on the no buy list.
	mapnobuy_map_iter_t start = set_mapNoBuy.lower_bound(gbi.iBaseID);
	mapnobuy_map_iter_t end = set_mapNoBuy.upper_bound(gbi.iBaseID);
	for (mapnobuy_map_iter_t i = start; i != end; i++)
	{
		if (gbi.iGoodID == i->second)
		{
			mapInfo[iClientID].bSuppressBuy = true;
			pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
			PrintUserCmdText(iClientID, L"ERR Temporarily out of stock");
			return true;
		}
	}

	/// Check restrictions for the ID that a player has.
	if (set_bCheckIDRestrictions)
	{
		mapGoodItemRestrictions_map_iter_t iter = set_mapGoodItemRestrictions.find(gbi.iGoodID);
		if (iter != set_mapGoodItemRestrictions.end())
		{
			if (!CheckIDEquipRestrictions(iClientID, gbi.iGoodID))
			{
				std::wstring CharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
				AddLog(Normal, L"%s attempting to buy %u without correct ID", CharName.c_str(), gbi.iGoodID);
				if (set_bEnforceIDRestrictions)
				{
					PrintUserCmdText(iClientID, set_wscGoodPurchaseDenied);
					pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
					mapInfo[iClientID].bSuppressBuy = true;
					return true;
				}
			}
		}
	}

	// Check for ship purchase restrictions.
	if (set_bCheckIDRestrictions)
	{
		const GoodInfo* packageInfo = GoodList::find_by_id(gbi.iGoodID);
		if (packageInfo->iType == 3)
		{
			const GoodInfo* hullInfo = GoodList::find_by_id(packageInfo->iHullGoodID);
			if (hullInfo->iType == 2)
			{
				mapShipItemRestrictions_map_iter_t iter2 = set_mapShipItemRestrictions.find(hullInfo->iShipGoodID);
				if (iter2 != set_mapShipItemRestrictions.end())
				{
					if (!CheckIDEquipRestrictions(iClientID, hullInfo->iShipGoodID))
					{
						std::wstring CharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
						AddLog(
						    Normal, L"%s attempting to buy %u without correct ID", CharName.c_str(),
						    hullInfo->iShipGoodID);
						if (set_bEnforceIDRestrictions)
						{
							PrintUserCmdText(iClientID, set_wscShipPurchaseDenied);
							pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
							mapInfo[iClientID].bSuppressBuy = true;
							return true;
						}
					}
				}
			}
		}
	}
	return false;
}

/// Suppress the buying of goods.
bool ReqAddItem(uint& goodID, char const* hardpoint, int& count, float& status, bool& mounted, uint& iClientID)
{
	LogItemsOfInterest(iClientID, goodID, "add-item");
	if (mapInfo[iClientID].bSuppressBuy)
	{
		return true;
	}
	return false;
}

/// Suppress the buying of goods.
bool ReqChangeCash(int& iMoneyDiff, uint& iClientID)
{
	if (mapInfo[iClientID].bSuppressBuy)
	{
		mapInfo[iClientID].bSuppressBuy = false;
		return true;
	}
	return false;
}

/// Suppress ship purchases
bool ReqSetCash(int& iMoney, uint& iClientID)
{
	if (mapInfo[iClientID].bSuppressBuy)
	{
		return true;
	}
	return false;
}

/// Suppress ship purchases
bool ReqEquipment(class EquipDescList const& eqDesc, uint& iClientID)
{
	if (mapInfo[iClientID].bSuppressBuy)
	{
		return true;
	}
	return false;
}

/// Suppress ship purchases
bool ReqShipArch(uint& iArchID, uint& iClientID)
{
	if (mapInfo[iClientID].bSuppressBuy)
	{
		return true;
	}
	return false;
}

/// Suppress ship purchases
bool ReqHullStatus(float& fStatus, uint& iClientID)
{
	if (mapInfo[iClientID].bSuppressBuy)
	{
		mapInfo[iClientID].bSuppressBuy = false;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		LoadSettings();
	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Purchase Restrictions");
	pi->shortName("PurchaseRestrictions");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__ReqAddItem, &ReqAddItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqChangeCash, &ReqChangeCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqEquipment, &ReqEquipment);
	pi->emplaceHook(HookedCall::IServerImpl__ReqHullStatus, &ReqHullStatus);
	pi->emplaceHook(HookedCall::IServerImpl__ReqSetCash, &ReqSetCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqShipArch, &ReqShipArch);
}
