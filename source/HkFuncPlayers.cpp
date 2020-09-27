#include <fstream>
#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkAddToGroup(uint iClientID, uint iGroupID) {
    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    uint iCurrentGroupID = Players.GetGroupID(iClientID);
    if (iCurrentGroupID == iGroupID)
        return HKE_INVALID_GROUP_ID;

    CPlayerGroup *group = CPlayerGroup::FromGroupID(iGroupID);
    if (!group)
        return HKE_INVALID_GROUP_ID;
    group->AddMember(iClientID);
    return HKE_OK;
}

HK_ERROR HkGetGroupID(uint iClientID, uint &iGroupID) {
    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    iGroupID = Players.GetGroupID(iClientID);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetCash(const std::wstring &wscCharname, int &iCash) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if ((iClientID != -1) && bIdString && HkIsInCharSelectMenu(iClientID))
        return HKE_NO_CHAR_SELECTED;
    else if ((iClientID != -1) &&
             !HkIsInCharSelectMenu(iClientID)) { // player logged in
        pub::Player::InspectCash(iClientID, iCash);
        return HKE_OK;
    } else { // player not logged in
        std::wstring wscDir;
        if (!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
            return HKE_CHAR_DOES_NOT_EXIST;
        std::wstring wscFile;
        HkGetCharFileName(wscCharname, wscFile);

        std::string scCharFile =
            scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";

        FILE *fTest;
        fopen_s(&fTest, scCharFile.c_str(), "r");
        if (!fTest)
            return HKE_CHAR_DOES_NOT_EXIST;
        else
            fclose(fTest);

        if (HkIsEncoded(scCharFile)) {
            std::string scCharFileNew = scCharFile + ".ini";
            if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
                return HKE_COULD_NOT_DECODE_CHARFILE;

            iCash = IniGetI(scCharFileNew, "Player", "money", -1);
            DeleteFile(scCharFileNew.c_str());
        } else {
            iCash = IniGetI(scCharFile, "Player", "money", -1);
        }

        return HKE_OK;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkAddCash(const std::wstring &wscCharname, int iAmount) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    uint iClientIDAcc = 0;
    if (iClientID == -1) {
        CAccount *acc = HkGetAccountByCharname(wscCharname);
        if (!acc)
            return HKE_CHAR_DOES_NOT_EXIST;
        iClientIDAcc = HkGetClientIdFromAccount(acc);
    } else
        iClientIDAcc = iClientID;

    if ((iClientID != -1) && bIdString && HkIsInCharSelectMenu(iClientID))
        return HKE_NO_CHAR_SELECTED;
    else if ((iClientID != -1) &&
             !HkIsInCharSelectMenu(iClientID)) { // player logged in
        pub::Player::AdjustCash(iClientID, iAmount);
        return HKE_OK;
    } else { // player not logged in
        std::wstring wscDir;
        if (!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
            return HKE_CHAR_DOES_NOT_EXIST;
        std::wstring wscFile;
        HkGetCharFileName(wscCharname, wscFile);

        std::string scCharFile =
            scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
        int iRet;
        if (HkIsEncoded(scCharFile)) {
            std::string scCharFileNew = scCharFile + ".ini";

            if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
                return HKE_COULD_NOT_DECODE_CHARFILE;

            iRet = IniGetI(scCharFileNew, "Player", "money", -1);
            // Add a space to the value so the ini file line looks like "<key> =
            // <value>" otherwise IFSO can't decode the file correctly
            IniWrite(scCharFileNew, "Player", "money",
                     " " + std::to_string(iRet + iAmount));

            if (!set_bDisableCharfileEncryption)
                if (!flc_encode(scCharFileNew.c_str(), scCharFile.c_str()))
                    return HKE_COULD_NOT_ENCODE_CHARFILE;

            DeleteFile(scCharFileNew.c_str());
        } else {
            iRet = IniGetI(scCharFile, "Player", "money", -1);
            // Add a space to the value so the ini file line looks like "<key> =
            // <value>" otherwise IFSO can't decode the file correctly
            IniWrite(scCharFile, "Player", "money",
                     " " + std::to_string(iRet + iAmount));
        }

        if (HkIsInCharSelectMenu(wscCharname) ||
            (iClientIDAcc !=
             -1)) { // money fix in case player logs in with this account
            bool bFound = false;
            std::wstring wscCharnameLower = ToLower(wscCharname);
            for (auto &money : ClientInfo[iClientIDAcc].lstMoneyFix) {
                if (money.wscCharname == wscCharnameLower) {
                    money.iAmount += iAmount;
                    bFound = true;
                    break;
                }
            }

            if (!bFound) {
                MONEY_FIX mf;
                mf.wscCharname = wscCharnameLower;
                mf.iAmount = iAmount;
                ClientInfo[iClientIDAcc].lstMoneyFix.push_back(mf);
            }
        }

        return HKE_OK;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkKick(CAccount *acc) {
    acc->ForceLogout();
    return HKE_OK;
}

HK_ERROR HkKick(const std::wstring &wscCharname) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    acc->ForceLogout();
    return HKE_OK;
}

HK_ERROR HkKickReason(const std::wstring &wscCharname,
                      const std::wstring &wscReason) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    if (wscReason.length())
        HkMsgAndKick(iClientID, wscReason, set_iKickMsgPeriod);
    else
        HkKick(Players.FindAccountFromClientID(iClientID));

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkBan(const std::wstring &wscCharname, bool bBan) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    CAccount *acc;
    if (iClientID != -1)
        acc = Players.FindAccountFromClientID(iClientID);
    else {
        if (!(acc = HkGetAccountByCharname(wscCharname)))
            return HKE_CHAR_DOES_NOT_EXIST;
    }

    std::wstring wscID = HkGetAccountID(acc);
    st6::wstring flStr((ushort *)wscID.c_str());
    Players.BanAccount(flStr, bBan);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkBeam(const std::wstring &wscCharname,
                const std::wstring &wscBasename) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    std::string scBasename = wstos(wscBasename);
    // check if ship in space
    uint iShip = 0;
    pub::Player::GetShip(iClientID, iShip);
    if (!iShip)
        return HKE_PLAYER_NOT_IN_SPACE;

    // get base id
    uint iBaseID;

    if (pub::GetBaseID(iBaseID, scBasename.c_str()) == -4) {
        std::string scBaseShortcut =
            IniGetS(set_scCfgFile, "names", wstos(wscBasename), "");
        if (!scBaseShortcut.length()) {
            typedef int (*_GetString)(LPVOID, uint, wchar_t *, uint);
            _GetString GetString = (_GetString)0x4176b0;
            Universe::IBase *pBase = Universe::GetFirstBase();
            while (pBase) {
                wchar_t buf[1024];
                GetString(NULL, pBase->iBaseIDS, buf, 1024);
                if (wcsstr(buf, wscBasename.c_str())) {
                    // Ignore the intro bases.
                    if (_strnicmp("intro", (char *)pBase->iDunno2, 5) != 0) {
                        iBaseID = pBase->iBaseID;
                        break;
                    }
                }
                pBase = Universe::GetNextBase();
            }
            if (iBaseID == 0)
                return HKE_INVALID_BASENAME;
        } else if (pub::GetBaseID(iBaseID, scBaseShortcut.c_str()) == -4)
            return HKE_INVALID_BASENAME;
    }

    uint iSysID;
    pub::Player::GetSystem(iClientID, iSysID);
    Universe::IBase *base = Universe::get_base(iBaseID);

    pub::Player::ForceLand(iClientID, iBaseID); // beam

    // if not in the same system, emulate F1 charload
    if (base->iSystemID != iSysID) {
        Server.BaseEnter(iBaseID, iClientID);
        Server.BaseExit(iBaseID, iClientID);
        std::wstring wscCharFileName;
        HkGetCharFileName(ARG_CLIENTID(iClientID), wscCharFileName);
        wscCharFileName += L".fl";
        CHARACTER_ID cID;
        strcpy_s(cID.szCharFilename,
                 wstos(wscCharFileName.substr(0, 14)).c_str());
        Server.CharacterSelect(cID, iClientID);
    }

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkSaveChar(const std::wstring &wscCharname) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    void *pJmp = (char *)hModServer + 0x7EFA8;
    char szNop[2] = {'\x90', '\x90'};
    char szTestAlAl[2] = {'\x74', '\x44'};
    WriteProcMem(pJmp, szNop, sizeof(szNop)); // nop the SinglePlayer() check
    pub::Save(iClientID, 1);
    WriteProcMem(pJmp, szTestAlAl, sizeof(szTestAlAl)); // restore

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EQ_ITEM {
    EQ_ITEM *next;
    uint i2;
    ushort s1;
    ushort sID;
    uint iGoodID;
    CacheString hardpoint;
    bool bMounted;
    char sz[3];
    float fStatus;
    uint iCount;
    bool bMission;
};

HK_ERROR HkEnumCargo(const std::wstring &wscCharname,
                     std::list<CARGO_INFO> &lstCargo, int &iRemainingHoldSize) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if (iClientID == -1 || HkIsInCharSelectMenu(iClientID))
        return HKE_PLAYER_NOT_LOGGED_IN;

    lstCargo.clear();

    char *szClassPtr;
    memcpy(&szClassPtr, &Players, 4);
    szClassPtr += 0x418 * (iClientID - 1);

    EQ_ITEM *eqLst;
    memcpy(&eqLst, szClassPtr + 0x27C, 4);
    EQ_ITEM *eq;
    eq = eqLst->next;
    while (eq != eqLst) {
        CARGO_INFO ci = {eq->sID,      (int)eq->iCount, eq->iGoodID,
                         eq->fStatus,  eq->bMission,    eq->bMounted,
                         eq->hardpoint};
        lstCargo.push_back(ci);

        eq = eq->next;
    }

    float fRemHold;
    pub::Player::GetRemainingHoldSize(iClientID, fRemHold);
    iRemainingHoldSize = (int)fRemHold;
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkRemoveCargo(const std::wstring &wscCharname, uint iID, int iCount) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if (iClientID == -1 || HkIsInCharSelectMenu(iClientID))
        return HKE_PLAYER_NOT_LOGGED_IN;

    std::list<CARGO_INFO> lstCargo;
    int iHold;
    HkEnumCargo(wscCharname, lstCargo, iHold);
    for (auto &cargo : lstCargo) {
        if ((cargo.iID == iID) && (cargo.iCount < iCount))
            iCount = cargo.iCount; // trying to remove more than actually there,
                                   // thus fix
    }

    pub::Player::RemoveCargo(iClientID, iID, iCount);

    // anti-cheat related
    /*	char *szClassPtr;
            memcpy(&szClassPtr, &Players, 4);
            szClassPtr += 0x418 * (iClientID - 1);
            EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;
            const EquipDesc *ed = edlList->find_equipment_item(iID);
            if(ed)
            {
                    ed->get_id();
                    edlList->remove_equipment_item(
            } */

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkAddCargo(const std::wstring &wscCharname, uint iGoodID, int iCount,
                    bool bMission) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if (iClientID == -1 || HkIsInCharSelectMenu(iClientID))
        return HKE_PLAYER_NOT_LOGGED_IN;

    /*	// anti-cheat related
            char *szClassPtr;
            memcpy(&szClassPtr, &Players, 4);
            szClassPtr += 0x418 * (iClientID - 1);
            EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;
            bool bCargoFound = true;
            if(!edlList->find_matching_cargo(iGoodID, 0, 1))
                    bCargoFound = false;*/

    // add
    const GoodInfo *gi;
    if (!(gi = GoodList::find_by_id(iGoodID)))
        return HKE_INVALID_GOOD;

    bool bMultiCount;
    memcpy(&bMultiCount, (char *)gi + 0x70, 1);

    uint iBase = 0;
    pub::Player::GetBase(iClientID, iBase);
    uint iLocation = 0;
    pub::Player::GetLocation(iClientID, iLocation);

    // trick cheat detection
    if (iBase) {
        if (iLocation)
            Server.LocationExit(iLocation, iClientID);
        Server.BaseExit(iBase, iClientID);
        if (!HkIsValidClientID(iClientID)) // got cheat kicked
            return HKE_PLAYER_NOT_LOGGED_IN;
    }

    if (bMultiCount) { // it's a good that can have multiple units(commodities,
                       // missile ammo, etc)
        int iRet;

        // we need to do this, else server or client may crash
        std::list<CARGO_INFO> lstCargo;
        HkEnumCargo(wscCharname, lstCargo, iRet);
        for (auto &cargo : lstCargo) {
            if ((cargo.iArchID == iGoodID) && (cargo.bMission != bMission)) {
                HkRemoveCargo(wscCharname, cargo.iID, cargo.iCount);
                iCount += cargo.iCount;
            }
        }

        pub::Player::AddCargo(iClientID, iGoodID, iCount, 1, bMission);
    } else {
        for (int i = 0; (i < iCount); i++)
            pub::Player::AddCargo(iClientID, iGoodID, 1, 1, bMission);
    }

    if (iBase) { // player docked on base
        ///////////////////////////////////////////////////
        // fix, else we get anti-cheat msg when undocking
        // this DOES NOT disable anti-cheat-detection, we're
        // just making some adjustments so that we dont get kicked

        Server.BaseEnter(iBase, iClientID);
        if (iLocation)
            Server.LocationEnter(iLocation, iClientID);

        /*		// fix "Ship or Equipment not sold on base" kick
                        if(!bCargoFound)
                        {
                                // get last equipid
                                char *szLastEquipID = szClassPtr + 0x3C8;
                                ushort sEquipID;
                                memcpy(&sEquipID, szLastEquipID, 2);

                                // add to check-list which is being compared to
           the users equip-list when saving char EquipDesc ed; memset(&ed, 0,
           sizeof(ed)); ed.id = sEquipID; ed.count = iCount; ed.archid =
           iGoodID; edlList->add_equipment_item(ed, true);
                        }

                        // fix "Ship Related" kick, update crc
                        ulong lCRC;
                        __asm
                        {
                                mov ecx, [szClassPtr]
                                call [CRCAntiCheat]
                                mov [lCRC], eax
                        }
                        memcpy(szClassPtr + 0x320, &lCRC, 4);*/
    }

    return HKE_OK;
}

HK_ERROR HkAddCargo(const std::wstring &wscCharname,
                    const std::wstring &wscGood, int iCount, bool bMission) {
    uint iGoodID = ToInt(wscGood.c_str());
    if (!iGoodID)
        pub::GetGoodID(iGoodID, wstos(wscGood).c_str());
    if (!iGoodID)
        return HKE_INVALID_GOOD;

    return HkAddCargo(wscCharname, iGoodID, iCount, bMission);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkRename(const std::wstring &wscCharname,
                  const std::wstring &wscNewCharname, bool bOnlyDelete) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if ((iClientID == -1) && !HkGetAccountByCharname(wscCharname))
        return HKE_CHAR_DOES_NOT_EXIST;

    if (!bOnlyDelete && HkGetAccountByCharname(wscNewCharname))
        return HKE_CHARNAME_ALREADY_EXISTS;

    if (!bOnlyDelete && (wscNewCharname.length() > 23))
        return HKE_CHARNAME_TOO_LONG;

    if (!bOnlyDelete && !wscNewCharname.length())
        return HKE_CHARNAME_TOO_SHORT;

    INI_Reader ini;
    if (!bOnlyDelete &&
        !(ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false)))
        return HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID;

    CAccount *acc;
    std::wstring wscOldCharname;
    if (iClientID != -1) {
        acc = Players.FindAccountFromClientID(iClientID);
        wscOldCharname = (wchar_t *)Players.GetActiveCharacterName(iClientID);
    } else {
        wscOldCharname = wscCharname;
        acc = HkGetAccountByCharname(wscCharname);
    }

    std::wstring wscAccountDirname;
    HkGetAccountDirName(acc, wscAccountDirname);
    std::wstring wscNewFilename;
    HkGetCharFileName(wscNewCharname, wscNewFilename);
    std::wstring wscOldFilename;
    HkGetCharFileName(wscOldCharname, wscOldFilename);

    std::string scNewCharfilePath = scAcctPath + wstos(wscAccountDirname) +
                                    "\\" + wstos(wscNewFilename) + ".fl";
    std::string scOldCharfilePath = scAcctPath + wstos(wscAccountDirname) +
                                    "\\" + wstos(wscOldFilename) + ".fl";

    if (bOnlyDelete) {
        // delete character
        st6::wstring str((ushort *)wscOldCharname.c_str());
        HkLockAccountAccess(acc, true); // also kicks player on this account
        Players.DeleteCharacterFromName(str);
        HkUnlockAccountAccess(acc);
        return HKE_OK;
    }

    HkLockAccountAccess(acc, true); // kick player if online
    HkUnlockAccountAccess(acc);

    // Copy existing char file into tmp
    std::string scTmpPath = scOldCharfilePath + ".tmp";
    DeleteFile(scTmpPath.c_str());
    CopyFile(scOldCharfilePath.c_str(), scTmpPath.c_str(), FALSE);

    // Delete existing char otherwise a rename of the char in slot 5 fails.
    st6::wstring str((ushort *)wscOldCharname.c_str());
    Players.DeleteCharacterFromName(str);

    // Emulate char create
    SLoginInfo logindata;
    wcsncpy_s(logindata.wszAccount, HkGetAccountID(acc).c_str(), 36);
    Players.login(logindata, MAX_CLIENT_ID + 1);

    SCreateCharacterInfo newcharinfo;
    wcsncpy_s(newcharinfo.wszCharname, wscNewCharname.c_str(), 23);
    newcharinfo.wszCharname[23] = 0;

    newcharinfo.iNickName = 0;
    newcharinfo.iBase = 0;
    newcharinfo.iPackage = 0;
    newcharinfo.iPilot = 0;

    while (ini.read_header()) {
        if (ini.is_header("Faction")) {
            while (ini.read_value()) {
                if (ini.is_value("nickname"))
                    newcharinfo.iNickName = CreateID(ini.get_value_string());
                else if (ini.is_value("base"))
                    newcharinfo.iBase = CreateID(ini.get_value_string());
                else if (ini.is_value("Package"))
                    newcharinfo.iPackage = CreateID(ini.get_value_string());
                else if (ini.is_value("Pilot"))
                    newcharinfo.iPilot = CreateID(ini.get_value_string());
            }
            break;
        }
    }
    ini.close();

    if (newcharinfo.iNickName == 0)
        newcharinfo.iNickName = CreateID("new_player");
    if (newcharinfo.iBase == 0)
        newcharinfo.iBase = CreateID("Li01_01_Base");
    if (newcharinfo.iPackage == 0)
        newcharinfo.iPackage = CreateID("ge_fighter");
    if (newcharinfo.iPilot == 0)
        newcharinfo.iPilot = CreateID("trent");

    // Fill struct with valid data (though it isnt used it is needed)
    newcharinfo.iDunno[4] = 65536;
    newcharinfo.iDunno[5] = 65538;
    newcharinfo.iDunno[6] = 0;
    newcharinfo.iDunno[7] = 1058642330;
    newcharinfo.iDunno[8] = 3206125978;
    newcharinfo.iDunno[9] = 65537;
    newcharinfo.iDunno[10] = 0;
    newcharinfo.iDunno[11] = 3206125978;
    newcharinfo.iDunno[12] = 65539;
    newcharinfo.iDunno[13] = 65540;
    newcharinfo.iDunno[14] = 65536;
    newcharinfo.iDunno[15] = 65538;
    Server.CreateNewCharacter(newcharinfo, MAX_CLIENT_ID + 1);
    HkSaveChar(wscNewCharname);
    Players.logout(MAX_CLIENT_ID + 1);

    // Decode the backup of the old char and overwrite the new char file
    if (!flc_decode(scTmpPath.c_str(),
                    scNewCharfilePath.c_str())) { // file wasn't encoded, thus
                                                  // simply rename it
        DeleteFile(scNewCharfilePath.c_str());    // just to get sure...
        CopyFile(scTmpPath.c_str(), scNewCharfilePath.c_str(), FALSE);
    }
    DeleteFile(scTmpPath.c_str());

    // Update the char name in the new char file.
    // Add a space to the value so the ini file line looks like "<key> =
    // <value>" otherwise Ioncross Server Operator can't decode the file
    // correctly
    std::string scValue = " ";
    for (uint i = 0; (i < wscNewCharname.length()); i++) {
        char cHiByte = wscNewCharname[i] >> 8;
        char cLoByte = wscNewCharname[i] & 0xFF;
        char szBuf[8];
        sprintf_s(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF,
                  ((uint)cLoByte) & 0xFF);
        scValue += szBuf;
    }
    IniWrite(scNewCharfilePath, "Player", "Name", scValue);

    // Re-encode the char file if needed.
    if (!set_bDisableCharfileEncryption)
        if (!flc_encode(scNewCharfilePath.c_str(), scNewCharfilePath.c_str()))
            return HKE_COULD_NOT_ENCODE_CHARFILE;

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkMsgAndKick(uint iClientID, const std::wstring &wscReason,
                      uint iIntervall) {
    if (!ClientInfo[iClientID].tmKickTime) {
        std::wstring wscMsg =
            ReplaceStr(set_wscKickMsg, L"%reason", XMLText(wscReason));
        HkFMsg(iClientID, wscMsg);
        ClientInfo[iClientID].tmKickTime = timeInMS() + iIntervall;
    }

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkKill(const std::wstring &wscCharname) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    if (!iShip)
        return HKE_PLAYER_NOT_IN_SPACE;

    pub::SpaceObj::SetRelativeHealth(iShip, 0.0f);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetReservedSlot(const std::wstring &wscCharname, bool &bResult) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    CAccount *acc;
    if (iClientID != -1)
        acc = Players.FindAccountFromClientID(iClientID);
    else
        acc = HkGetAccountByCharname(wscCharname);

    if (!acc)
        return HKE_CHAR_DOES_NOT_EXIST;

    std::wstring wscDir;
    HkGetAccountDirName(acc, wscDir);
    std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

    bResult = IniGetB(scUserFile, "Settings", "ReservedSlot", false);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkSetReservedSlot(const std::wstring &wscCharname,
                           bool bReservedSlot) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    CAccount *acc;
    if (iClientID != -1)
        acc = Players.FindAccountFromClientID(iClientID);
    else
        acc = HkGetAccountByCharname(wscCharname);

    if (!acc)
        return HKE_CHAR_DOES_NOT_EXIST;

    std::wstring wscDir;
    HkGetAccountDirName(acc, wscDir);
    std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

    if (bReservedSlot)
        IniWrite(scUserFile, "Settings", "ReservedSlot", "yes");
    else
        IniWrite(scUserFile, "Settings", "ReservedSlot", "no");
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct AUTOBUY_CARTITEM {
    uint iArchID;
    uint iCount;
    std::wstring wscDescription;
};

int HkPlayerAutoBuyGetCount(std::list<CARGO_INFO> &lstCargo, uint iItemArchID) {
    for (auto &cargo : lstCargo) {
        if (cargo.iArchID == iItemArchID)
            return cargo.iCount;
    }

    return 0;
}

#define ADD_EQUIP_TO_CART(desc)                                                \
    {                                                                          \
        aci.iArchID = ((Archetype::Launcher *)eq)->iProjectileArchID;          \
        aci.iCount =                                                           \
            MAX_PLAYER_AMMO - HkPlayerAutoBuyGetCount(lstCargo, aci.iArchID);  \
        aci.wscDescription = desc;                                             \
        lstCart.push_back(aci);                                                \
    }

void HkPlayerAutoBuy(uint iClientID, uint iBaseID) {
    // player cargo
    int iRemHoldSize;
    std::list<CARGO_INFO> lstCargo;
    HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iRemHoldSize);

    // shopping cart
    std::list<AUTOBUY_CARTITEM> lstCart;

    if (ClientInfo[iClientID].bAutoBuyReload) { // shield bats & nanobots
        Archetype::Ship *ship =
            Archetype::GetShip(Players[iClientID].iShipArchetype);

        uint iNanobotsID;
        pub::GetGoodID(iNanobotsID, "ge_s_repair_01");
        uint iRemNanobots = ship->iMaxNanobots;
        uint iShieldBatsID;
        pub::GetGoodID(iShieldBatsID, "ge_s_battery_01");
        uint iRemShieldBats = ship->iMaxShieldBats;
        bool bNanobotsFound = false;
        bool bShieldBattsFound = false;
        for (auto &cargo : lstCargo) {
            AUTOBUY_CARTITEM aci;
            if (cargo.iArchID == iNanobotsID) {
                aci.iArchID = iNanobotsID;
                aci.iCount = ship->iMaxNanobots - cargo.iCount;
                aci.wscDescription = L"Nanobots";
                lstCart.push_back(aci);
                bNanobotsFound = true;
            } else if (cargo.iArchID == iShieldBatsID) {
                aci.iArchID = iShieldBatsID;
                aci.iCount = ship->iMaxShieldBats - cargo.iCount;
                aci.wscDescription = L"Shield Batteries";
                lstCart.push_back(aci);
                bShieldBattsFound = true;
            }
        }

        if (!bNanobotsFound) { // no nanos found -> add all
            AUTOBUY_CARTITEM aci;
            aci.iArchID = iNanobotsID;
            aci.iCount = ship->iMaxNanobots;
            aci.wscDescription = L"Nanobots";
            lstCart.push_back(aci);
        }

        if (!bShieldBattsFound) { // no batts found -> add all
            AUTOBUY_CARTITEM aci;
            aci.iArchID = iShieldBatsID;
            aci.iCount = ship->iMaxShieldBats;
            aci.wscDescription = L"Shield Batteries";
            lstCart.push_back(aci);
        }
    }

    if (ClientInfo[iClientID].bAutoBuyCD || ClientInfo[iClientID].bAutoBuyCM ||
        ClientInfo[iClientID].bAutoBuyMines ||
        ClientInfo[iClientID].bAutoBuyMissiles ||
        ClientInfo[iClientID].bAutoBuyTorps) {
        // add mounted equip to a new list and eliminate double equipment(such
        // as 2x lancer etc)
        std::list<CARGO_INFO> lstMounted;
        for (auto &cargo : lstCargo) {
            if (!cargo.bMounted)
                continue;

            bool bFound = false;
            for (auto &mounted : lstMounted) {
                if (mounted.iArchID == cargo.iArchID) {
                    bFound = true;
                    break;
                }
            }

            if (!bFound)
                lstMounted.push_back(cargo);
        }

        uint iVFTableMines = (uint)hModCommon + ADDR_COMMON_VFTABLE_MINE;
        uint iVFTableCM = (uint)hModCommon + ADDR_COMMON_VFTABLE_CM;
        uint iVFTableGun = (uint)hModCommon + ADDR_COMMON_VFTABLE_GUN;

        // check mounted equip
        for (auto &mounted : lstMounted) {
            uint i = mounted.iArchID;
            AUTOBUY_CARTITEM aci;
            Archetype::Equipment *eq = Archetype::GetEquipment(mounted.iArchID);
            EQ_TYPE eq_type = HkGetEqType(eq);
            if (eq_type == ET_MINE) {
                if (ClientInfo[iClientID].bAutoBuyMines)
                    ADD_EQUIP_TO_CART(L"Mines")
            } else if (eq_type == ET_CM) {
                if (ClientInfo[iClientID].bAutoBuyCM)
                    ADD_EQUIP_TO_CART(L"Countermeasures")
            } else if (eq_type == ET_TORPEDO) {
                if (ClientInfo[iClientID].bAutoBuyTorps)
                    ADD_EQUIP_TO_CART(L"Torpedos")
            } else if (eq_type == ET_CD) {
                if (ClientInfo[iClientID].bAutoBuyCD)
                    ADD_EQUIP_TO_CART(L"Cruise Disruptors")
            } else if (eq_type == ET_MISSILE) {
                if (ClientInfo[iClientID].bAutoBuyMissiles)
                    ADD_EQUIP_TO_CART(L"Missiles")
            }
        }
    }

    // search base in base-info list
    BASE_INFO *bi = 0;
    for (auto &base : lstBases) {
        if (base.iBaseID == iBaseID) {
            bi = &base;
            break;
        }
    }

    if (!bi)
        return; // base not found

    int iCash;
    HkGetCash(ARG_CLIENTID(iClientID), iCash);

    for (auto &buy : lstCart) {
        if (!buy.iCount || !Arch2Good(buy.iArchID))
            continue;

        // check if good is available and if player has the neccessary rep
        bool bGoodAvailable = false;
        for (auto &available : bi->lstMarketMisc) {
            if (available.iArchID == buy.iArchID) {
                // get base rep
                int iSolarRep;
                pub::SpaceObj::GetSolarRep(bi->iObjectID, iSolarRep);
                uint iBaseRep;
                pub::Reputation::GetAffiliation(iSolarRep, iBaseRep);
                if (iBaseRep == -1)
                    continue; // rep can't be determined yet(space object not
                              // created yet?)

                // get player rep
                int iRepID;
                pub::Player::GetRep(iClientID, iRepID);

                // check if rep is sufficient
                float fPlayerRep;
                pub::Reputation::GetGroupFeelingsTowards(iRepID, iBaseRep,
                                                         fPlayerRep);
                if (fPlayerRep < available.fRep)
                    break; // bad rep, not allowed to buy
                bGoodAvailable = true;
                break;
            }
        }

        if (!bGoodAvailable)
            continue; // base does not sell this item or bad rep

        float fPrice;
        if (pub::Market::GetPrice(iBaseID, buy.iArchID, fPrice) == -1)
            continue; // good not available

        Archetype::Equipment *eq = Archetype::GetEquipment(buy.iArchID);
        if (iRemHoldSize < (eq->fVolume * buy.iCount)) {
            uint iNewCount = (uint)(iRemHoldSize / eq->fVolume);
            if (!iNewCount) {
                //				PrintUserCmdText(iClientID,
                // L"Auto-Buy(%s): FAILED! Insufficient cargo space",
                // (*it4).wscDescription.c_str());
                continue;
            } else
                buy.iCount = iNewCount;
        }

        int iCost = ((int)fPrice * buy.iCount);
        if (iCash < iCost)
            PrintUserCmdText(iClientID,
                             L"Auto-Buy(%s): FAILED! Insufficient Credits",
                             buy.wscDescription.c_str());
        else {
            HkAddCash(ARG_CLIENTID(iClientID), -iCost);
            iCash -= iCost;
            iRemHoldSize -= ((int)eq->fVolume * buy.iCount);
            //			HkAddCargo(ARG_CLIENTID(iClientID),
            //(*it4).iArchID, (*it4).iCount, false);

            // add the item, dont use hkaddcargo for performance/bug reasons
            // assume we only mount multicount goods (missiles, ammo, bots)
            pub::Player::AddCargo(iClientID, buy.iArchID, buy.iCount, 1, false);

            PrintUserCmdText(iClientID,
                             L"Auto-Buy(%s): Bought %u unit(s), cost: %s$",
                             buy.wscDescription.c_str(), buy.iCount,
                             ToMoneyStr(iCost).c_str());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResetRep(const std::wstring &wscCharname) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    INI_Reader ini;
    if (!ini.open("mpnewcharacter.fl", false))
        return HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID;

    ini.read_header();
    if (!ini.is_header("Player")) {
        ini.close();
        return HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID;
    }

    int iPlayerRep;
    pub::Player::GetRep(iClientID, iPlayerRep);
    while (ini.read_value()) {
        if (ini.is_value("house")) {
            float fRep = ini.get_value_float(0);
            const char *szRepGroupName = ini.get_value_string(1);

            uint iRepGroupID;
            pub::Reputation::GetReputationGroup(iRepGroupID, szRepGroupName);
            pub::Reputation::SetReputation(iPlayerRep, iRepGroupID, fRep);
        }
    }

    ini.close();
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkSetRep(const std::wstring &wscCharname,
                  const std::wstring &wscRepGroup, float fValue) {
    HK_GET_CLIENTID(iClientID, wscCharname);
    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    uint iRepGroupID;
    pub::Reputation::GetReputationGroup(iRepGroupID,
                                        wstos(wscRepGroup).c_str());
    if (iRepGroupID == -1)
        return HKE_INVALID_REP_GROUP;

    int iPlayerRep;
    pub::Player::GetRep(iClientID, iPlayerRep);
    pub::Reputation::SetReputation(iPlayerRep, iRepGroupID, fValue);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetRep(const std::wstring &wscCharname,
                  const std::wstring &wscRepGroup, float &fValue) {
    HK_GET_CLIENTID(iClientID, wscCharname);
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    uint iRepGroupID;
    pub::Reputation::GetReputationGroup(iRepGroupID,
                                        wstos(wscRepGroup).c_str());
    if (iRepGroupID == -1)
        return HKE_INVALID_REP_GROUP;

    int iPlayerRep;
    pub::Player::GetRep(iClientID, iPlayerRep);
    pub::Reputation::GetGroupFeelingsTowards(iPlayerRep, iRepGroupID, fValue);
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetGroupMembers(const std::wstring &wscCharname,
                           std::list<GROUP_MEMBER> &lstMembers) {
    lstMembers.clear();
    HK_GET_CLIENTID(iClientID, wscCharname);

    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    // hey, at least it works! beware of the VC optimiser.
    st6::vector<uint> vMembers;
    pub::Player::GetGroupMembers(iClientID, vMembers);

    for (uint i : vMembers) {
        GROUP_MEMBER gm;
        gm.iClientID = i;
        gm.wscCharname = (wchar_t *)Players.GetActiveCharacterName(i);
        lstMembers.push_back(gm);
    }

    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkReadCharFile(const std::wstring &wscCharname,
                        std::list<std::wstring> &lstOutput) {
    lstOutput.clear();
    HK_GET_CLIENTID(iClientID, wscCharname);

    std::wstring wscDir;
    CAccount *acc;
    if (iClientID != -1) {
        acc = Players.FindAccountFromClientID(iClientID);
        const wchar_t *wszCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);
        if (!wszCharname)
            return HKE_NO_CHAR_SELECTED;

        if (!HKHKSUCCESS(HkGetAccountDirName(wszCharname, wscDir)))
            return HKE_CHAR_DOES_NOT_EXIST;
    } else {
        if (!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
            return HKE_CHAR_DOES_NOT_EXIST;
    }

    std::wstring wscFile;
    HkGetCharFileName(wscCharname, wscFile);
    std::string scCharFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
    std::string scFileToRead;
    bool bDeleteAfter;
    if (HkIsEncoded(scCharFile)) {
        std::string scCharFileNew = scCharFile + ".ini";
        if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
            return HKE_COULD_NOT_DECODE_CHARFILE;
        scFileToRead = scCharFileNew;
        bDeleteAfter = true;
    } else {
        scFileToRead = scCharFile;
        bDeleteAfter = false;
    }

    std::ifstream ifs;
    ifs.open(scFileToRead.c_str(), std::ios_base::in);
    if (!ifs.is_open())
        return HKE_UNKNOWN_ERROR;

    std::string scLine;
    while (getline(ifs, scLine))
        lstOutput.push_back(stows(scLine));
    ifs.close();
    if (bDeleteAfter)
        DeleteFile(scFileToRead.c_str());
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkWriteCharFile(const std::wstring &wscCharname,
                         std::wstring wscData) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    std::wstring wscDir;
    CAccount *acc;
    if (iClientID != -1) {
        acc = Players.FindAccountFromClientID(iClientID);
        const wchar_t *wszCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);
        if (!wszCharname)
            return HKE_NO_CHAR_SELECTED;

        if (!HKHKSUCCESS(HkGetAccountDirName(wszCharname, wscDir)))
            return HKE_CHAR_DOES_NOT_EXIST;
    } else {
        if (!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
            return HKE_CHAR_DOES_NOT_EXIST;
    }

    std::wstring wscFile;
    HkGetCharFileName(wscCharname, wscFile);
    std::string scCharFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
    std::string scFileToWrite;
    bool bEncode;
    if (HkIsEncoded(scCharFile)) {
        scFileToWrite = scCharFile + ".ini";
        bEncode = true;
    } else {
        scFileToWrite = scCharFile;
        bEncode = false;
    }

    std::ofstream ofs;
    ofs.open(scFileToWrite.c_str(), std::ios_base::out);
    if (!ofs.is_open())
        return HKE_UNKNOWN_ERROR;

    size_t iPos;
    while ((iPos = wscData.find(L"\\n")) != -1) {
        std::wstring wscLine = wscData.substr(0, iPos);
        ofs << wstos(wscLine) << std::endl;
        wscData.erase(0, iPos + 2);
    }

    if (wscData.length())
        ofs << wstos(wscData);

    ofs.close();
    if (bEncode) {
        flc_encode(scFileToWrite.c_str(), scCharFile.c_str());
        DeleteFile(scFileToWrite.c_str());
    }
    return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkPlayerRecalculateCRC(uint iClientID) {
    try {
        PlayerData *pd = &Players[iClientID];
        char *ACCalcCRC = (char *)hModServer + 0x6FAF0;
        __asm {
            pushad
            mov ecx, [pd]
            call[ACCalcCRC]
            mov ecx, [pd]
            mov[ecx + 320h], eax
            popad
        }
    } catch (...) {
        return HKE_INVALID_CLIENT_ID;
    }

    return HKE_OK;
}

/** Move the client to the specified location */
void HkRelocateClient(uint iClientID, Vector vDestination,
                      Matrix mOrientation) {
    Quaternion qRotation = HkMatrixToQuaternion(mOrientation);

    FLPACKET_LAUNCH pLaunch;
    pLaunch.iShip = ClientInfo[iClientID].iShip;
    pLaunch.iBase = 0;
    pLaunch.iState = 0xFFFFFFFF;
    pLaunch.fRotate[0] = qRotation.w;
    pLaunch.fRotate[1] = qRotation.x;
    pLaunch.fRotate[2] = qRotation.y;
    pLaunch.fRotate[3] = qRotation.z;
    pLaunch.fPos[0] = vDestination.x;
    pLaunch.fPos[1] = vDestination.y;
    pLaunch.fPos[2] = vDestination.z;

    HookClient->Send_FLPACKET_SERVER_LAUNCH(iClientID, pLaunch);

    uint iSystem;
    pub::Player::GetSystem(iClientID, iSystem);
    pub::SpaceObj::Relocate(ClientInfo[iClientID].iShip, iSystem, vDestination,
                            mOrientation);
}

/** Dock the client immediately */
HK_ERROR HkInstantDock(uint iClientID, uint iDockObj) {
    // check if logged in
    if (iClientID == -1)
        return HKE_PLAYER_NOT_LOGGED_IN;

    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    if (!iShip)
        return HKE_PLAYER_NOT_IN_SPACE;

    uint iSystem, iSystem2;
    pub::SpaceObj::GetSystem(iShip, iSystem);
    pub::SpaceObj::GetSystem(iDockObj, iSystem2);
    if (iSystem != iSystem2) {
        return HKE_PLAYER_NOT_IN_SPACE;
    }

    try {
        pub::SpaceObj::InstantDock(iShip, iDockObj, 1);
    } catch (...) {
        return HKE_PLAYER_NOT_IN_SPACE;
    }

    return HKE_OK;
}

HK_ERROR HkGetRank(const std::wstring &wscCharname, int &iRank) {
    HK_ERROR err;
    std::wstring wscRet = L"";
    if ((err = HkFLIniGet(wscCharname, L"rank", wscRet)) != HKE_OK)
        return err;
    if (wscRet.length())
        iRank = ToInt(wscRet);
    else
        iRank = 0;
    return HKE_OK;
}

/// Get online time.
HK_ERROR HkGetOnlineTime(const std::wstring &wscCharname, int &iSecs) {
    std::wstring wscDir;
    if (!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
        return HKE_CHAR_DOES_NOT_EXIST;

    std::wstring wscFile;
    HkGetCharFileName(wscCharname, wscFile);

    std::string scCharFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
    if (HkIsEncoded(scCharFile)) {
        std::string scCharFileNew = scCharFile + ".ini";
        if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
            return HKE_COULD_NOT_DECODE_CHARFILE;

        iSecs =
            (int)IniGetF(scCharFileNew, "mPlayer", "total_time_played", 0.0f);
        DeleteFile(scCharFileNew.c_str());
    } else {
        iSecs = (int)IniGetF(scCharFile, "mPlayer", "total_time_played", 0.0f);
    }

    return HKE_OK;
}

/// Return true if this player is within the specified distance of any other
/// player.
bool IsInRange(uint iClientID, float fDistance) {
    std::list<GROUP_MEMBER> lstMembers;
    HkGetGroupMembers(
        (const wchar_t *)Players.GetActiveCharacterName(iClientID), lstMembers);

    uint iShip;
    pub::Player::GetShip(iClientID, iShip);

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(iShip, pos, rot);

    uint iSystem;
    pub::Player::GetSystem(iClientID, iSystem);

    // For all players in system...
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        // Get the this player's current system and location in the system.
        uint iClientID2 = HkGetClientIdFromPD(pPD);
        uint iSystem2 = 0;
        pub::Player::GetSystem(iClientID2, iSystem2);
        if (iSystem != iSystem2)
            continue;

        uint iShip2;
        pub::Player::GetShip(iClientID2, iShip2);

        Vector pos2;
        Matrix rot2;
        pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

        // Ignore players who are in your group.
        bool bGrouped = false;
        for (auto &gm : lstMembers) {
            if (gm.iClientID == iClientID2) {
                bGrouped = true;
                break;
            }
        }
        if (bGrouped)
            continue;

        // Is player within the specified range of the sending char.
        if (HkDistance3D(pos, pos2) < fDistance)
            return true;
    }
    return false;
}

/**
Delete a character.
*/
HK_ERROR HkDeleteCharacter(CAccount *acc, std::wstring &wscCharname) {
    HkLockAccountAccess(acc, true);
    st6::wstring str((ushort *)wscCharname.c_str());
    Players.DeleteCharacterFromName(str);
    HkUnlockAccountAccess(acc);
    return HKE_OK;
}

/**
Create a new character in the specified account by emulating a
create character.
*/
HK_ERROR HkNewCharacter(CAccount *acc, std::wstring &wscCharname) {
    HkLockAccountAccess(acc, true);
    HkUnlockAccountAccess(acc);

    INI_Reader ini;
    if (!ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
        return HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID;

    // Emulate char create by logging in.
    SLoginInfo logindata;
    wcsncpy_s(logindata.wszAccount, HkGetAccountID(acc).c_str(), 36);
    Players.login(logindata, Players.GetMaxPlayerCount() + 1);

    SCreateCharacterInfo newcharinfo;
    wcsncpy_s(newcharinfo.wszCharname, wscCharname.c_str(), 23);
    newcharinfo.wszCharname[23] = 0;

    newcharinfo.iNickName = 0;
    newcharinfo.iBase = 0;
    newcharinfo.iPackage = 0;
    newcharinfo.iPilot = 0;

    while (ini.read_header()) {
        if (ini.is_header("Faction")) {
            while (ini.read_value()) {
                if (ini.is_value("nickname"))
                    newcharinfo.iNickName = CreateID(ini.get_value_string());
                else if (ini.is_value("base"))
                    newcharinfo.iBase = CreateID(ini.get_value_string());
                else if (ini.is_value("package"))
                    newcharinfo.iPackage = CreateID(ini.get_value_string());
                else if (ini.is_value("pilot"))
                    newcharinfo.iPilot = CreateID(ini.get_value_string());
            }
            break;
        }
    }
    ini.close();

    if (newcharinfo.iNickName == 0)
        newcharinfo.iNickName = CreateID("new_player");
    if (newcharinfo.iBase == 0)
        newcharinfo.iBase = CreateID("Li01_01_Base");
    if (newcharinfo.iPackage == 0)
        newcharinfo.iPackage = CreateID("ge_fighter");
    if (newcharinfo.iPilot == 0)
        newcharinfo.iPilot = CreateID("trent");

    // Fill struct with valid data (though it isnt used it is needed)
    newcharinfo.iDunno[4] = 65536;
    newcharinfo.iDunno[5] = 65538;
    newcharinfo.iDunno[6] = 0;
    newcharinfo.iDunno[7] = 1058642330;
    newcharinfo.iDunno[8] = 3206125978;
    newcharinfo.iDunno[9] = 65537;
    newcharinfo.iDunno[10] = 0;
    newcharinfo.iDunno[11] = 3206125978;
    newcharinfo.iDunno[12] = 65539;
    newcharinfo.iDunno[13] = 65540;
    newcharinfo.iDunno[14] = 65536;
    newcharinfo.iDunno[15] = 65538;
    Server.CreateNewCharacter(newcharinfo, Players.GetMaxPlayerCount() + 1);
    HkSaveChar(wscCharname);
    Players.logout(Players.GetMaxPlayerCount() + 1);
    return HKE_OK;
}

typedef void(__stdcall *_FLAntiCheat)();
typedef void(__stdcall *_FLPossibleCheatingDetected)(int iReason);

/** Anti cheat checking code by mc_horst */
HK_ERROR HkAntiCheat(uint iClientID) {
#define ADDR_FL_ANTICHEAT_1 0x70120
#define ADDR_FL_ANTICHEAT_2 0x6FD20
#define ADDR_FL_ANTICHEAT_3 0x6FAF0
#define ADDR_FL_ANTICHEAT_4 0x6FAA0
#define ADDR_FL_POSSIBLE_CHEATING_DETECTED 0x6F570

    _FLAntiCheat FLAntiCheat1 =
        (_FLAntiCheat)((char *)hModServer + ADDR_FL_ANTICHEAT_1);
    _FLAntiCheat FLAntiCheat2 =
        (_FLAntiCheat)((char *)hModServer + ADDR_FL_ANTICHEAT_2);
    _FLAntiCheat FLAntiCheat3 =
        (_FLAntiCheat)((char *)hModServer + ADDR_FL_ANTICHEAT_3);
    _FLAntiCheat FLAntiCheat4 =
        (_FLAntiCheat)((char *)hModServer + ADDR_FL_ANTICHEAT_4);
    _FLPossibleCheatingDetected FLPossibleCheatingDetected =
        (_FLPossibleCheatingDetected)((char *)hModServer +
                                      ADDR_FL_POSSIBLE_CHEATING_DETECTED);

    // check if ship in space
    uint iShip = 0;
    pub::Player::GetShip(iClientID, iShip);
    if (iShip)
        return HKE_OK;

    char *szObjPtr;
    memcpy(&szObjPtr, &Players, 4);
    szObjPtr += 0x418 * (iClientID - 1);

    char cRes;

    ////////////////////////// 1
    __asm {
        mov ecx, [szObjPtr]
        call [FLAntiCheat1]
        mov [cRes], al
    }

    if (cRes != 0) { // kick
        HkKick(ARG_CLIENTID(iClientID));
        return HKE_UNKNOWN_ERROR;
    }

    ////////////////////////// 2
    __asm {
        mov ecx, [szObjPtr]
        call [FLAntiCheat2]
        mov [cRes], al
    }

    if (cRes != 0) { // kick
        HkKick(ARG_CLIENTID(iClientID));
        return HKE_UNKNOWN_ERROR;
    }

    ////////////////////////// 3
    ulong lRet;
    ulong lCompare;
    __asm {
        mov ecx, [szObjPtr]
        mov eax, [ecx+0x320]
        mov [lCompare], eax
        call [FLAntiCheat3]
        mov [lRet], eax
    }

    if (lRet > lCompare) { // kick
        HkKick(ARG_CLIENTID(iClientID));
        return HKE_UNKNOWN_ERROR;
    }

    ////////////////////////// 4
    __asm {
        mov ecx, [szObjPtr]
        call [FLAntiCheat4]
        mov [cRes], al
    }

    if (cRes != 0) { // kick
        HkKick(ARG_CLIENTID(iClientID));
        return HKE_UNKNOWN_ERROR;
    }

    return HKE_OK;
}

HK_ERROR HkAddEquip(const std::wstring &wscCharname, uint iGoodID,
                    const std::string &scHardpoint) {
    HK_GET_CLIENTID(iClientID, wscCharname);

    if ((iClientID == -1) || HkIsInCharSelectMenu(iClientID))
        return HKE_NO_CHAR_SELECTED;

    if (!Players[iClientID].iEnteredBase) {
        Players[iClientID].iEnteredBase = Players[iClientID].iBaseID;
        Server.ReqAddItem(iGoodID, scHardpoint.c_str(), 1, 1.0f, true,
                          iClientID);
        Players[iClientID].iEnteredBase = 0;
    } else {
        Server.ReqAddItem(iGoodID, scHardpoint.c_str(), 1, 1.0f, true,
                          iClientID);
    }

    // Add to check-list which is being compared to the users equip-list when
    // saving char to fix "Ship or Equipment not sold on base" kick
    EquipDesc ed;
    ed.sID = Players[iClientID].sLastEquipID;
    ed.iCount = 1;
    ed.iArchID = iGoodID;
    Players[iClientID].lShadowEquipDescList.add_equipment_item(ed, false);

    return HKE_OK;
}

HK_ERROR HkAddEquip(const std::wstring &wscCharname, uint iGoodID,
                    const std::string &scHardpoint, bool bMounted) {
    typedef bool(__stdcall * _AddCargoDocked)(
        uint iGoodID, CacheString * &hardpoint, int iNumItems, float fHealth,
        int bMounted, int bMission, uint iOne);
    static _AddCargoDocked AddCargoDocked = 0;
    if (!AddCargoDocked)
        AddCargoDocked = (_AddCargoDocked)((char *)hModServer + 0x6EFC0);

    HK_GET_CLIENTID(iClientID, wscCharname);
    if (iClientID == -1 || HkIsInCharSelectMenu(iClientID))
        return HKE_PLAYER_NOT_LOGGED_IN;

    uint iBase = 0;
    pub::Player::GetBase(iClientID, iBase);
    uint iLocation = 0;
    pub::Player::GetLocation(iClientID, iLocation);

    if (iLocation)
        Server.LocationExit(iLocation, iClientID);
    if (iBase)
        Server.BaseExit(iBase, iClientID);
    if (!HkIsValidClientID(iClientID))
        return HKE_PLAYER_NOT_LOGGED_IN;

    PlayerData *pd = &Players[iClientID];
    const char *p = scHardpoint.c_str();
    CacheString hardpoint;
    hardpoint.value = StringAlloc(p, false);

    int iOne = 1;
    int iMounted = bMounted;
    float fHealth = 1;
    CacheString *pHP = &hardpoint;
    __asm {
        push iOne
        push iMounted
        push iOne
        push fHealth
        push iOne
        push pHP
        push iGoodID
        mov ecx, pd
        call AddCargoDocked
    }

    if (iBase) Server.BaseEnter(iBase, iClientID);
    if (iLocation)
        Server.LocationEnter(iLocation, iClientID);

    return HKE_OK;
}

std::wstring GetLocation(unsigned int iClientID) {
    uint iSystemID = 0;
    uint iShip = 0;
    pub::Player::GetSystem(iClientID, iSystemID);
    pub::Player::GetShip(iClientID, iShip);
    if (!iSystemID || !iShip) {
        PrintUserCmdText(iClientID, L"ERR Not in space");
        return false;
    }

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(iShip, pos, rot);

    return VectorToSectorCoord<std::wstring>(iSystemID, pos);
}

CAccount *HkGetAccountByClientID(uint iClientID) {
    if (!HkIsValidClientID(iClientID))
        return 0;

    return Players.FindAccountFromClientID(iClientID);
}

std::wstring HkGetAccountIDByClientID(uint iClientID) {
    if (HkIsValidClientID(iClientID)) {
        CAccount *acc = HkGetAccountByClientID(iClientID);
        if (acc && acc->wszAccID) {
            return acc->wszAccID;
        }
    }
    return L"";
}

void HkDelayedKick(uint iClientID, uint secs) {
    mstime kick_time = timeInMS() + (secs * 1000);
    if (!ClientInfo[iClientID].tmKickTime ||
        ClientInfo[iClientID].tmKickTime > kick_time)
        ClientInfo[iClientID].tmKickTime = kick_time;
}

std::string HkGetPlayerSystemS(uint iClientID) {
    uint iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);
    char szSystemname[1024] = "";
    pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
    return szSystemname;
}

HK_ERROR HKGetShipValue(const std::wstring &wscCharname, float &fValue) {
    UINT iClientID = HkGetClientIdFromCharname(wscCharname);
    if (iClientID != -1 && !HkIsInCharSelectMenu(iClientID)) {
        HkSaveChar(wscCharname);
        if (!HkIsValidClientID(iClientID)) {
            return HKE_UNKNOWN_ERROR;
        }
    }

    fValue = 0.0f;

    uint iBaseID = 0;

    std::list<std::wstring> lstCharFile;
    HK_ERROR err = HkReadCharFile(wscCharname, lstCharFile);
    if (err != HKE_OK)
        return err;

    for (const auto &line : lstCharFile) {
        std::wstring wscKey = Trim(line.substr(0, line.find(L"=")));
        if (wscKey == L"base" || wscKey == L"last_base") {
            int iFindEqual = line.find(L"=");
            if (iFindEqual == -1) {
                continue;
            }

            if ((iFindEqual + 1) >= (int)line.size()) {
                continue;
            }

            iBaseID =
                CreateID(wstos(Trim(line.substr(iFindEqual + 1))).c_str());
            break;
        }
    }

    for (const auto &line : lstCharFile) {
        std::wstring wscKey = Trim(line.substr(0, line.find(L"=")));
        if (wscKey == L"cargo" || wscKey == L"equip") {
            int iFindEqual = line.find(L"=");
            if (iFindEqual == -1) {
                continue;
            }
            int iFindComma = line.find(L",", iFindEqual);
            if (iFindComma == -1) {
                continue;
            }
            uint iGoodID =
                ToUInt(Trim(line.substr(iFindEqual + 1, iFindComma)));
            uint iGoodCount = ToUInt(Trim(
                line.substr(iFindComma + 1, line.find(L",", iFindComma + 1))));

            float fItemValue;
            if (pub::Market::GetPrice(iBaseID, Arch2Good(iGoodID),
                                      fItemValue) == 0) {
                if (arch_is_combinable(iGoodID)) {
                    fValue += fItemValue * iGoodCount;
                    // ConPrint(L"market %u %0.2f = %0.2f x %u\n", iGoodID,
                    // fItemValue * iGoodCount, fItemValue, iGoodCount);
                } else {
                    float *fResaleFactor =
                        (float *)((char *)hModServer + 0x8AE7C);
                    fValue += fItemValue * (*fResaleFactor);
                    // ConPrint(L"market %u %0.2f = %0.2f x %0.2f x 1\n",
                    // iGoodID, fItemValue  * (*fResaleFactor), fItemValue,
                    // (*fResaleFactor));
                }
            }
        } else if (wscKey == L"money") {
            int iFindEqual = line.find(L"=");
            if (iFindEqual == -1) {
                continue;
            }
            uint fItemValue = ToUInt(Trim(line.substr(iFindEqual + 1)));
            fValue += fItemValue;
        } else if (wscKey == L"ship_archetype") {
            uint iShipArchID =
                ToUInt(Trim(line.substr(line.find(L"=") + 1, line.length())));
            const GoodInfo *gi = GoodList_get()->find_by_ship_arch(iShipArchID);
            if (gi) {
                gi = GoodList::find_by_id(gi->iArchID);
                if (gi) {
                    float *fResaleFactor =
                        (float *)((char *)hModServer + 0x8AE78);
                    float fItemValue = gi->fPrice * (*fResaleFactor);
                    fValue += fItemValue;
                    // ConPrint(L"ship %u %0.2f = %0.2f x %0.2f\n", iShipArchID,
                    // fItemValue, gi->fPrice, *fResaleFactor);
                }
            }
        }
    }
    return HKE_OK;
}

void HkSaveChar(uint iClientID) {
    BYTE patch[] = {0x90, 0x90};
    WriteProcMem((char *)hModServer + 0x7EFA8, patch, sizeof(patch));
    pub::Save(iClientID, 1);
}
