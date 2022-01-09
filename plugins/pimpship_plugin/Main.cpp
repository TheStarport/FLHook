// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

bool IsItemArchIDAvailable(uint iArchID) {
    for (std::map<uint, ITEM_INFO>::iterator iter = mapAvailableItems.begin();
         iter != mapAvailableItems.end(); iter++) {
        if (iter->second.iArchID == iArchID)
            return true;
    }
    return false;
}

std::wstring GetItemDescription(uint iArchID) {
    for (std::map<uint, ITEM_INFO>::iterator iter = mapAvailableItems.begin();
         iter != mapAvailableItems.end(); iter++) {
        if (iter->second.iArchID == iArchID)
            return iter->second.wscDescription;
    }
    return L"";
}

void LoadSettings() {
    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\pimpship.cfg";
    set_iCost = 0;
    mapAvailableItems.clear();
    set_mapDealers.clear();

    // Patch BaseDataList::get_room_data to suppress annoying warnings
    // flserver-errors.log
    unsigned char patch1[] = {0x90, 0x90};
    WriteProcMem((char *)0x62660F2, &patch1, 2);

    int iItemID = 1;
    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("ShipPimper")) {
                while (ini.read_value()) {
                    if (ini.is_value("cost")) {
                        set_iCost = ini.get_value_int(0);
                    } else if (ini.is_value("equip")) {
                        std::string nickname = ini.get_value_string(0);
                        std::string description = ini.get_value_string(1);
                        uint iArchID = CreateID(nickname.c_str());
                        mapAvailableItems[iItemID].iArchID = iArchID;
                        mapAvailableItems[iItemID].wscNickname =
                            stows(nickname);
                        mapAvailableItems[iItemID].wscDescription =
                            stows(description);
                        if (mapAvailableItems[iItemID]
                                .wscDescription.length() == 0)
                            mapAvailableItems[iItemID].wscDescription =
                                mapAvailableItems[iItemID].wscNickname;
                        iItemID++;
                    } else if (ini.is_value("room")) {
                        std::string nickname = ini.get_value_string(0);
                        uint iLocationID = CreateID(nickname.c_str());
                        if (!BaseDataList_get()->get_room_data(iLocationID)) {
                            if (set_bDebug) {
                                ConPrint(L"NOTICE: Room %s does not exist\n",
                                         stows(nickname).c_str());
                            }
                        } else {
                            set_mapDealers[iLocationID] = stows(nickname);
                        }
                    }
                }
            }
        }
        ini.close();
    }

    // Unpatch BaseDataList::get_room_data to suppress annoying warnings
    // flserver-errors.log
    unsigned char unpatch1[] = {0xFF, 0x12};
    WriteProcMem((char *)0x62660F2, &patch1, 2);
}

// On entering a room check to see if we're in a valid ship dealer room (or base
// if a ShipDealer is not defined). If we are then print the intro text
// otherwise do nothing.
void LocationEnter(unsigned int iLocationID, unsigned int iClientID) {
    if (set_mapDealers.find(iLocationID) == set_mapDealers.end()) {
        uint iBaseID = 0;
        pub::Player::GetBase(iClientID, iBaseID);
        if (set_mapDealers.find(iBaseID) == set_mapDealers.end()) {
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_PimpShip(uint iClientID, const std::wstring &wscParam) {
    uint iLocationID = 0;
    pub::Player::GetLocation(iClientID, iLocationID);
    if (set_mapDealers.find(iLocationID) == set_mapDealers.end()) {
        uint iBaseID = 0;
        pub::Player::GetBase(iClientID, iBaseID);
        if (set_mapDealers.find(iBaseID) == set_mapDealers.end()) {
            mapInfo[iClientID].bInPimpDealer = false;
            mapInfo[iClientID].mapCurrEquip.clear();
            return;
        }
    }

    mapInfo[iClientID].mapCurrEquip.clear();
    mapInfo[iClientID].bInPimpDealer = true;

    PrintUserCmdText(iClientID, L"Available ship pimping commands:");

    PrintUserCmdText(iClientID, L"/showsetup");
    PrintUserCmdText(iClientID, L"|     Display current ship setup.");

    PrintUserCmdText(iClientID, L"/showitems");
    PrintUserCmdText(iClientID,
                     L"|     Display items that may be added to your ship.");

    PrintUserCmdText(iClientID, L"/setitem <hardpoint id> <new item id>");
    PrintUserCmdText(iClientID,
                     L"|     Change the item at <hp id> to <item id>.");
    PrintUserCmdText(iClientID,
                     L"|     <hi id>s are shown by typing /show setup.");
    PrintUserCmdText(iClientID,
                     L"|     <item id>s are shown by typing /show items.");

    PrintUserCmdText(iClientID, L"/buynow");
    PrintUserCmdText(iClientID, L"|     Confirms the changes.");
    PrintUserCmdText(iClientID, L"This facility costs " +
                                    ToMoneyStr(set_iCost) +
                                    L" credits to use.");

    std::wstring wscCharName =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    // Build the equipment list.
    int iSlotID = 1;

    st6::list<EquipDesc> &eqLst = Players[iClientID].equipDescList.equip;
    for (auto eq = eqLst.begin(); eq != eqLst.end(); eq++) {
        if (IsItemArchIDAvailable(eq->iArchID)) {
            mapInfo[iClientID].mapCurrEquip[iSlotID].sID = eq->sID;
            mapInfo[iClientID].mapCurrEquip[iSlotID].iArchID = eq->iArchID;
            mapInfo[iClientID].mapCurrEquip[iSlotID].iOrigArchID = eq->iArchID;
            mapInfo[iClientID].mapCurrEquip[iSlotID].wscHardPoint =
                stows(eq->szHardPoint.value);
            iSlotID++;
        }
    }
}

/// Show the setup of the player's ship.
void UserCmd_ShowSetup(uint iClientID, const std::wstring &wscParam) {
    if (!mapInfo[iClientID].bInPimpDealer)
        return;

    PrintUserCmdText(iClientID, L"Current ship setup: %d",
                     mapInfo[iClientID].mapCurrEquip.size());
    for (auto iter = mapInfo[iClientID].mapCurrEquip.begin();
         iter != mapInfo[iClientID].mapCurrEquip.end(); iter++) {
        PrintUserCmdText(iClientID, L"|     %.2d | %s : %s", iter->first,
                         iter->second.wscHardPoint.c_str(),
                         GetItemDescription(iter->second.iArchID).c_str());
    }
    PrintUserCmdText(iClientID, L"OK");
}

/// Show the items that may be changed.
void UserCmd_ShowItems(uint iClientID, const std::wstring &wscParam) {
    if (!mapInfo[iClientID].bInPimpDealer)
        return;

    PrintUserCmdText(iClientID, L"Available items: %d",
                     mapAvailableItems.size());
    for (auto iter = mapAvailableItems.begin(); iter != mapAvailableItems.end();
         iter++) {
        PrintUserCmdText(iClientID, L"|     %.2d:  %s", iter->first,
                         iter->second.wscDescription.c_str());
    }
    PrintUserCmdText(iClientID, L"OK");
}

/// Change the item on the Slot ID to the specified item.
void UserCmd_ChangeItem(uint iClientID, const std::wstring &wscParam) {
    if (!mapInfo[iClientID].bInPimpDealer)
        return;

    int iHardPointID = ToInt(GetParam(wscParam, ' ', 0));
    int iSelectedItemID = ToInt(GetParam(wscParam, ' ', 1));

    if (mapInfo[iClientID].mapCurrEquip.find(iHardPointID) ==
        mapInfo[iClientID].mapCurrEquip.end()) {
        PrintUserCmdText(iClientID, L"ERR Invalid hard point ID");
        return;
    }

    if (mapAvailableItems.find(iSelectedItemID) == mapAvailableItems.end()) {
        PrintUserCmdText(iClientID, L"ERR Invalid item ID");
        return;
    }

    mapInfo[iClientID].mapCurrEquip[iHardPointID].iArchID =
        mapAvailableItems[iSelectedItemID].iArchID;
    return UserCmd_ShowSetup(iClientID, wscParam);
}

void UserCmd_BuyNow(uint iClientID, const std::wstring &wscParam) {
    HK_ERROR err;

    std::wstring wscCharName =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    // Check the that player is in a ship dealer.
    if (!mapInfo[iClientID].bInPimpDealer)
        return;

    // Charge for the equipment pimp.
    if (set_iCost > 0) {
        int iCash = 0;
        if ((err = HkGetCash(wscCharName, iCash)) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
            return;
        }
        if (iCash < 0 && iCash < set_iCost) {
            PrintUserCmdText(iClientID, L"ERR Insufficient credits");
            return;
        }
        HkAddCash(wscCharName, 0 - set_iCost);
    }

    // Remove all lights.
    for (auto i = mapInfo[iClientID].mapCurrEquip.begin();
         i != mapInfo[iClientID].mapCurrEquip.end(); ++i) {
        pub::Player::RemoveCargo(iClientID, i->second.sID, 1);
    }

    // Re-add all lights so that the order is kept the same
    for (auto i = mapInfo[iClientID].mapCurrEquip.begin();
         i != mapInfo[iClientID].mapCurrEquip.end(); ++i) {
        HkAddEquip(wscCharName, i->second.iArchID,
                   wstos(i->second.wscHardPoint));
    }

    PrintUserCmdText(
        iClientID,
        L"OK Ship pimp complete. Please wait 10 seconds and reconnect.");
    HkDelayedKick(iClientID, 5);
}

// Client command processing
USERCMD UserCmds[] = {
    {L"/pimpship", UserCmd_PimpShip},
    {L"/showsetup", UserCmd_ShowSetup},
    {L"/showitems", UserCmd_ShowItems},
    {L"/setitem", UserCmd_ChangeItem},
    {L"/buynow", UserCmd_BuyNow},
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, L"/afk ");
    PrintUserCmdText(iClientID,
                     L"Sets the player to AFK. If any other player messages "
                     L"directly, they will be told you are afk.");
    PrintUserCmdText(iClientID, L"/back ");
    PrintUserCmdText(iClientID, L"Turns off AFK for a the player.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return true;
}

// Functions to hook
EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Pimpship");
    pi->shortName("pimpship");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::IServerImpl__LocationEnter, &LocationEnter);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
}
