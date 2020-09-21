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


#include "Main.h"

namespace PurchaseRestrictions
{

	/// If true then an ID is used to check for valid IDs
	static bool set_bCheckIDRestrictions = false;

	/// If true then an ID is used to check for valid IDs
	static bool set_bEnforceIDRestrictions = false;

	/// Message if ship purchase denied
	static std::wstring set_wscShipPurchaseDenied = L"";

	/// Message if good purchase denied
	static std::wstring set_wscGoodPurchaseDenied = L"";

	/// list of items that we log transfers for
	static std::map<uint, std::string> set_mapItemsOfInterest;

	/** A map of baseid to goods that are not allowed to be bought */
	static std::multimap<uint, uint> set_mapNoBuy;
	typedef std::multimap<uint, uint, std::less<uint> >::value_type mapnobuy_map_pair_t;
	typedef std::multimap<uint, uint, std::less<uint> >::iterator mapnobuy_map_iter_t;

	/** A map of goods that are not allowed to be bought without the correct item. */
	static std::multimap<uint, uint> set_mapGoodItemRestrictions;
	typedef std::multimap<uint, uint, std::less<uint> >::value_type mapGoodItemRestrictions_map_pair_t;
	typedef std::multimap<uint, uint, std::less<uint> >::iterator mapGoodItemRestrictions_map_iter_t;

	/** A map of ship that are not allowed to be bought without the correct item. */
	static std::multimap<uint, uint> set_mapShipItemRestrictions;
	typedef std::multimap<uint, uint, std::less<uint> >::value_type mapShipItemRestrictions_map_pair_t;
	typedef std::multimap<uint, uint, std::less<uint> >::iterator mapShipItemRestrictions_map_iter_t;

	// List of connected clients.
	struct INFO
	{
		INFO() : bSuppressBuy(false) {}

		bool bSuppressBuy;
	};
	static std::map<uint, INFO> mapInfo;

	/// Log items of interest so we can see what cargo cheats people are using.
	static void LogItemsOfInterest(uint iClientID, uint iGoodID, const std::string &details)
	{
		auto iter = set_mapItemsOfInterest.find(iGoodID);
		if (iter != set_mapItemsOfInterest.end())
		{
			std::wstring wscCharName = (const wchar_t*) Players.GetActiveCharacterName(iClientID);
			AddLog("NOTICE: Item '%s' found in cargo of %s (%s) %s",
				iter->second.c_str(),
				wstos(wscCharName).c_str(), wstos(HkGetAccountID(HkGetAccountByCharname(wscCharName))).c_str(),
				details.c_str());
		}
	}


	/// Load the configuration
	void PurchaseRestrictions::LoadSettings(const std::string &scPluginCfgFile)
	{
		set_bCheckIDRestrictions = IniGetB(scPluginCfgFile, "PurchaseRestrictions", "CheckIDRestrictions", false);
		set_bEnforceIDRestrictions = IniGetB(scPluginCfgFile, "PurchaseRestrictions", "EnforceIDRestrictions", false);
		set_wscShipPurchaseDenied = stows(IniGetS(scPluginCfgFile, "PurchaseRestrictions", "ShipPurchaseDeniedMsg",  "ERR You cannot buy this ship because you do not have the correct ID."));
		set_wscGoodPurchaseDenied = stows(IniGetS(scPluginCfgFile, "PurchaseRestrictions", "GoodPurchaseDeniedMsg", "ERR You cannot buy this item because you do not have the correct ID."));

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
						set_mapNoBuy.insert(mapnobuy_map_pair_t(baseID,goodID));
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
						if (goodID!=-1 && itemID!=-1)
						{
							set_mapGoodItemRestrictions.insert(mapGoodItemRestrictions_map_pair_t(goodID,itemID));
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
						if (shipID!=-1 && itemID!=-1)
						{
							set_mapShipItemRestrictions.insert(mapGoodItemRestrictions_map_pair_t(shipID,itemID));
						}
					}
				}
			}
			ini.close();
		}
	}

	/// Check that this client is allowed to buy/mount this piece of equipment or ship
	/// Return true if the equipment is mounted to allow this good.
	bool CheckIDEquipRestrictions(uint iClientID, uint iGoodID)
	{
		std::list<CARGO_INFO> lstCargo;
		int iRemainingHoldSize;
		HkEnumCargo((const wchar_t*) Players.GetActiveCharacterName(iClientID), lstCargo, iRemainingHoldSize);
		for(auto& cargo : lstCargo)
		{
			if (cargo.bMounted)
			{
				auto start = set_mapGoodItemRestrictions.lower_bound(iGoodID);
				auto end = set_mapGoodItemRestrictions.upper_bound(iGoodID);
				for (auto i = start; i != end; ++i)
				{
					if (cargo.iArchID==i->second)
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

	void PurchaseRestrictions::ClearClientInfo(unsigned int iClientID)
	{
		mapInfo[iClientID].bSuppressBuy = false;
	}

	void PurchaseRestrictions::PlayerLaunch(unsigned int iShip, unsigned int iClientID)
	{
		mapInfo[iClientID].bSuppressBuy = false;
	}

	void PurchaseRestrictions::BaseEnter(unsigned int iBaseID, unsigned int iClientID)
	{
		mapInfo[iClientID].bSuppressBuy = false;
	}


	/// Suppress the buying of goods.
	bool PurchaseRestrictions::GFGoodBuy(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID)
	{
		mapInfo[iClientID].bSuppressBuy = false;
		LogItemsOfInterest(iClientID, gbi.iGoodID, "good-buy");

		/// Check to see if this item is on the no buy list.
		mapnobuy_map_iter_t start = set_mapNoBuy.lower_bound(gbi.iBaseID);
		mapnobuy_map_iter_t end = set_mapNoBuy.upper_bound(gbi.iBaseID);
		for (mapnobuy_map_iter_t i=start; i!=end; i++)
		{
			if (gbi.iGoodID==i->second)
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
					AddLog("INFO: %s attempting to buy %u without correct ID", wstos((const wchar_t*) Players.GetActiveCharacterName(iClientID)).c_str(), gbi.iGoodID);
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
			const GoodInfo *packageInfo = GoodList::find_by_id(gbi.iGoodID);
			if (packageInfo->iType == 3)
			{
				const GoodInfo *hullInfo = GoodList::find_by_id(packageInfo->iHullGoodID);
				if (hullInfo->iType == 2)
				{
					mapShipItemRestrictions_map_iter_t iter2 = set_mapShipItemRestrictions.find(hullInfo->iShipGoodID);
					if (iter2!=set_mapShipItemRestrictions.end())
					{
						if (!CheckIDEquipRestrictions(iClientID, hullInfo->iShipGoodID))
						{
							AddLog("INFO: %s attempting to buy %u without correct ID", wstos((const wchar_t*) Players.GetActiveCharacterName(iClientID)).c_str(), hullInfo->iShipGoodID );
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
	bool PurchaseRestrictions::ReqAddItem(unsigned int goodID, char const *hardpoint, int count, float status, bool mounted, uint iClientID)
	{
		LogItemsOfInterest(iClientID, goodID, "add-item");
		if (mapInfo[iClientID].bSuppressBuy)
		{
			return true;
		}
		return false;
	}

	/// Suppress the buying of goods.
	bool PurchaseRestrictions::ReqChangeCash(int iMoneyDiff,unsigned int iClientID)
	{
		if (mapInfo[iClientID].bSuppressBuy)
		{
			mapInfo[iClientID].bSuppressBuy = false;
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool PurchaseRestrictions::ReqSetCash(int iMoney,unsigned int iClientID)
	{
		if (mapInfo[iClientID].bSuppressBuy)
		{
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool PurchaseRestrictions::ReqEquipment(class EquipDescList const &eqDesc, unsigned int iClientID)
	{
		if (mapInfo[iClientID].bSuppressBuy)
		{
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool PurchaseRestrictions::ReqShipArch(unsigned int iArchID, unsigned int iClientID)
	{
		if (mapInfo[iClientID].bSuppressBuy)
		{
			return true;
		}
		return false;
	}



	/// Suppress ship purchases
	bool PurchaseRestrictions::ReqHullStatus(float fStatus, unsigned int iClientID)
	{
		if (mapInfo[iClientID].bSuppressBuy)
		{
			mapInfo[iClientID].bSuppressBuy = false;
			return true;
		}
		return false;
	}
}