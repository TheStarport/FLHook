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


IMPORT unsigned int  MakeLocationID(unsigned int,char const *);

namespace PimpShip
{

	// Intro messages when entering the room.
	static wstring set_wscIntroMsg1 = L"Pimp-my-ship facilities are available here.";
	static wstring set_wscIntroMsg2 = L"Type /pimpship on your console to see options.";

	// Cost per changed item.
	static int set_iCost = 0;

	// List of dealer rooms
	static map<uint, wstring> set_mapDealers;

	// Item of equipment for a single client.
	struct EQ_HARDPOINT
	{
		EQ_HARDPOINT() : sID(0), iArchID(0), iOrigArchID(0) {}

		uint sID;
		uint iArchID;
		uint iOrigArchID;
		wstring wscHardPoint;
	};

	// List of connected clients.
	struct INFO
	{
		INFO() : bInPimpDealer(false) {}

		// Map of hard point ID to equip.
		map<uint, EQ_HARDPOINT> mapCurrEquip;

		bool bInPimpDealer;
	};
	static map<uint, INFO> mapInfo;

	// Map of item id to ITEM INFO
	struct ITEM_INFO
	{
		ITEM_INFO() : iArchID(0) {}

		uint iArchID;
		wstring wscNickname;
		wstring wscDescription;
	};
	map<uint, ITEM_INFO> mapAvailableItems;

	bool IsItemArchIDAvailable(uint iArchID)
	{
		for (map<uint, ITEM_INFO>::iterator iter = mapAvailableItems.begin();
			iter != mapAvailableItems.end();
			iter++)
		{
			if (iter->second.iArchID == iArchID)
				return true;
		}
		return false;
	}

	wstring GetItemDescription(uint iArchID)
	{
		for (map<uint, ITEM_INFO>::iterator iter = mapAvailableItems.begin();
			iter != mapAvailableItems.end();
			iter++)
		{
			if (iter->second.iArchID == iArchID)
				return iter->second.wscDescription;
		}
		return L"";
	}

	void PimpShip::LoadSettings(const string &scPluginCfgFile)
	{
		set_iCost = 0;
		mapAvailableItems.clear();
		set_mapDealers.clear();

		// Patch BaseDataList::get_room_data to suppress annoying warnings flserver-errors.log
		unsigned char patch1[] = { 0x90, 0x90 };
		WriteProcMem((char*)0x62660F2, &patch1, 2);

		int iItemID = 1;
		INI_Reader ini;
		if (ini.open(scPluginCfgFile.c_str(), false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("ShipPimper"))
				{
					while (ini.read_value())
					{
						if (ini.is_value("cost"))
						{
							set_iCost = ini.get_value_int(0);
						}
						else if (ini.is_value("equip"))
						{
							string nickname = ini.get_value_string(0);
							string description = ini.get_value_string(1);
							uint iArchID = CreateID(nickname.c_str());
							mapAvailableItems[iItemID].iArchID = iArchID;
							mapAvailableItems[iItemID].wscNickname = stows(nickname);
							mapAvailableItems[iItemID].wscDescription = stows(description);
							if (mapAvailableItems[iItemID].wscDescription.length() == 0)
								mapAvailableItems[iItemID].wscDescription = mapAvailableItems[iItemID].wscNickname;
							iItemID++;
						}
						else if (ini.is_value("room"))
						{
							string nickname = ini.get_value_string(0);
							uint iLocationID = CreateID(nickname.c_str());
							if (!BaseDataList_get()->get_room_data(iLocationID))
							{
								if (set_iPluginDebug>0)
								{
									ConPrint(L"NOTICE: Room %s does not exist\n", stows(nickname).c_str());
								}
							}
							else
							{
								set_mapDealers[iLocationID] = stows(nickname);
							}
						}
					}
				}
			}
			ini.close();
		}

		// Unpatch BaseDataList::get_room_data to suppress annoying warnings flserver-errors.log
		unsigned char unpatch1[] = { 0xFF, 0x12 };
		WriteProcMem((char*)0x62660F2, &patch1, 2);
	}

	// On entering a room check to see if we're in a valid ship dealer room (or base if a 
	// ShipDealer is not defined). If we are then print the intro text otherwise do
	// nothing.
	void PimpShip::LocationEnter(unsigned int iLocationID, unsigned int iClientID)
	{
		if (!set_bEnablePimpShip)
			return;

		if (set_mapDealers.find(iLocationID)==set_mapDealers.end())
		{
			uint iBaseID = 0;
			pub::Player::GetBase(iClientID, iBaseID);
			if (set_mapDealers.find(iBaseID)==set_mapDealers.end())
			{
				mapInfo[iClientID].bInPimpDealer = false;
				mapInfo[iClientID].mapCurrEquip.clear();
				return;
			}
		}

		if (set_wscIntroMsg1.length() > 0)
			PrintUserCmdText(iClientID, L"%s", set_wscIntroMsg1.c_str());

		if (set_wscIntroMsg2.length() > 0)
			PrintUserCmdText(iClientID, L"%s", set_wscIntroMsg2.c_str());
	}

	bool PimpShip::UserCmd_PimpShip(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (!set_bEnablePimpShip)
			return false;

		uint iLocationID = 0;
		pub::Player::GetLocation(iClientID, iLocationID);
		if (set_mapDealers.find(iLocationID)==set_mapDealers.end())
		{
			uint iBaseID = 0;
			pub::Player::GetBase(iClientID, iBaseID);
			if (set_mapDealers.find(iBaseID)==set_mapDealers.end())
			{
				mapInfo[iClientID].bInPimpDealer = false;
				mapInfo[iClientID].mapCurrEquip.clear();
				return false;
			}
		}

		mapInfo[iClientID].mapCurrEquip.clear();
		mapInfo[iClientID].bInPimpDealer = true;

		PrintUserCmdText(iClientID, L"Available ship pimping commands:");

		PrintUserCmdText(iClientID, L"/showsetup");
		PrintUserCmdText(iClientID, L"|     Display current ship setup.");

		PrintUserCmdText(iClientID, L"/showitems");
		PrintUserCmdText(iClientID, L"|     Display items that may be added to your ship.");

		PrintUserCmdText(iClientID, L"/setitem <hardpoint id> <new item id>");
		PrintUserCmdText(iClientID, L"|     Change the item at <hp id> to <item id>.");
		PrintUserCmdText(iClientID, L"|     <hi id>s are shown by typing /show setup.");
		PrintUserCmdText(iClientID, L"|     <item id>s are shown by typing /show items.");

		PrintUserCmdText(iClientID, L"/buynow");
		PrintUserCmdText(iClientID, L"|     Confirms the changes.");
		PrintUserCmdText(iClientID, L"This facility costs " + ToMoneyStr(set_iCost) + L" credits to use.");

		wstring wscCharName = (const wchar_t*) Players.GetActiveCharacterName(iClientID);

		// Build the equipment list.
		int iSlotID = 1;

		EquipDescListItem *eqLst = Players[iClientID].equipDescList.pFirst;
		for (EquipDescListItem *eq = eqLst->next; eq != eqLst; eq = eq->next)
		{
			if (IsItemArchIDAvailable(eq->equip.iArchID))
			{
				mapInfo[iClientID].mapCurrEquip[iSlotID].sID = eq->equip.sID;
				mapInfo[iClientID].mapCurrEquip[iSlotID].iArchID = eq->equip.iArchID;
				mapInfo[iClientID].mapCurrEquip[iSlotID].iOrigArchID = eq->equip.iArchID;
				mapInfo[iClientID].mapCurrEquip[iSlotID].wscHardPoint = stows(eq->equip.szHardPoint.value);
				iSlotID++;
			}
		}
		return true;
	}

	/// Show the setup of the player's ship.
	bool PimpShip::UserCmd_ShowSetup(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (!mapInfo[iClientID].bInPimpDealer || !set_bEnablePimpShip)
			return false;

		PrintUserCmdText(iClientID, L"Current ship setup: %d", mapInfo[iClientID].mapCurrEquip.size());
		for (map<uint, EQ_HARDPOINT>::iterator iter = mapInfo[iClientID].mapCurrEquip.begin();
			iter != mapInfo[iClientID].mapCurrEquip.end();
			iter++)
		{
			PrintUserCmdText(iClientID, L"|     %.2d | %s : %s",
				iter->first, iter->second.wscHardPoint.c_str(), GetItemDescription(iter->second.iArchID).c_str());
		}
		PrintUserCmdText(iClientID, L"OK");
		return true;
	}

	/// Show the items that may be changed.
	bool PimpShip::UserCmd_ShowItems(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (!mapInfo[iClientID].bInPimpDealer || !set_bEnablePimpShip)
			return false;

		PrintUserCmdText(iClientID, L"Available items: %d", mapAvailableItems.size());
		for (map<uint, ITEM_INFO>::iterator iter = mapAvailableItems.begin(); iter != mapAvailableItems.end(); iter++)
		{
			PrintUserCmdText(iClientID, L"|     %.2d:  %s", iter->first, iter->second.wscDescription.c_str());
		}
		PrintUserCmdText(iClientID, L"OK");

		return true;
	}

	/// Change the item on the Slot ID to the specified item.
	bool PimpShip::UserCmd_ChangeItem(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (!mapInfo[iClientID].bInPimpDealer || !set_bEnablePimpShip)
			return false;

		int iHardPointID = ToInt(GetParam(wscParam, ' ', 0));
		int iSelectedItemID = ToInt(GetParam(wscParam, ' ', 1));

		if (mapInfo[iClientID].mapCurrEquip.find(iHardPointID)
			== mapInfo[iClientID].mapCurrEquip.end())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid hard point ID");
			return true;
		}

		if (mapAvailableItems.find(iSelectedItemID) == mapAvailableItems.end())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid item ID");
			return true;
		}

		mapInfo[iClientID].mapCurrEquip[iHardPointID].iArchID = mapAvailableItems[iSelectedItemID].iArchID;
		return UserCmd_ShowSetup(iClientID, wscCmd, wscParam, usage);
	}

	bool PimpShip::UserCmd_BuyNow(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		HK_ERROR err; 

		wstring wscCharName = (const wchar_t*) Players.GetActiveCharacterName(iClientID);

		// Check the that player is in a ship dealer.
		if (!mapInfo[iClientID].bInPimpDealer)
			return false;

		// Charge for the equipment pimp.
		if (set_iCost > 0)
		{
			int iCash = 0;
			if ((err = HkGetCash(wscCharName, iCash)) != HKE_OK)
			{
				PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
				return true;
			}
			if (iCash<0 && iCash<set_iCost)
			{
				PrintUserCmdText(iClientID, L"ERR Insufficient credits");
				return true;
			}
			HkAddCash(wscCharName, 0-set_iCost);
		}

		// Remove all lights.
		for (map<uint, EQ_HARDPOINT>::iterator i = mapInfo[iClientID].mapCurrEquip.begin();
			i != mapInfo[iClientID].mapCurrEquip.end(); ++i)
		{
			pub::Player::RemoveCargo(iClientID, i->second.sID, 1);
		}

		// Re-add all lights so that the order is kept the same
		for (map<uint, EQ_HARDPOINT>::iterator i = mapInfo[iClientID].mapCurrEquip.begin();
			i != mapInfo[iClientID].mapCurrEquip.end(); ++i)
		{
			HkAddEquip(wscCharName, i->second.iArchID, wstos(i->second.wscHardPoint));
		}

		PrintUserCmdText(iClientID, L"OK Ship pimp complete. Please wait 10 seconds and reconnect.");
		HkDelayedKick(iClientID, 5);
		return true;
	}
}