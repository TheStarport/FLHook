#include "global.h"
#include "CCmds.h"

#define RIGHT_CHECK(a)                                                         \
    if (!(this->rights & a)) {                                                 \
        Print(L"ERR No permission");                                         \
        return;                                                                \
    }
#define RIGHT_CHECK_SUPERADMIN()                                               \
    if (!(this->rights == RIGHT_SUPERADMIN)) {                                 \
        Print(L"ERR No permission");                                         \
        return;                                                                \
    }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCash(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_CASH);

    int iCash;
    if (HKSUCCESS(HkGetCash(player, iCash)))
        Print(L"cash=%dOK\n", iCash);
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetCash(std::variant<uint, std::wstring> player, int iAmount) {
    RIGHT_CHECK(RIGHT_CASH);

    int iCash;
    if (HKSUCCESS(HkGetCash(player, iCash))) {
        HkAddCash(player, iAmount - iCash);
        CmdGetCash(player);
    } else
        PrintError();
}

void CCmds::CmdSetCashSec(std::variant<uint, std::wstring> player, int iAmountCheck,
                          int iAmount) {
    RIGHT_CHECK(RIGHT_CASH);

    int iCash;

    if (HKSUCCESS(HkGetCash(player, iCash))) {
        if (iCash != iAmountCheck)
            Print(L"ERR Security check failed");
        else
            CmdSetCash(player, iAmount);
    } else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCash(std::variant<uint, std::wstring> player, int iAmount) {
    RIGHT_CHECK(RIGHT_CASH);

    if (HKSUCCESS(HkAddCash(player, iAmount)))
        CmdGetCash(player);
    else
        PrintError();
}

void CCmds::CmdAddCashSec(std::variant<uint, std::wstring> player, int iAmountCheck,
                          int iAmount) {
    RIGHT_CHECK(RIGHT_CASH);

    int iCash;

    if (HKSUCCESS(HkGetCash(player, iCash))) {
        if (iCash != iAmountCheck)
            Print(L"ERR Security check failed");
        else
            CmdAddCash(player, iAmount);
    } else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKick(std::variant<uint, std::wstring> player,
                    const std::wstring &wscReason) {
    RIGHT_CHECK(RIGHT_KICKBAN);

    if (HKSUCCESS(HkKickReason(player, wscReason)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdBan(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_KICKBAN);

    if (HKSUCCESS(HkBan(player, true)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnban(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_KICKBAN);

    if (HKSUCCESS(HkBan(player, false)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKickBan(std::variant<uint, std::wstring> player,
                       const std::wstring &wscReason) {
    RIGHT_CHECK(RIGHT_KICKBAN);

    if (!HKSUCCESS(HkBan(player, true))) {
        PrintError();
        return;
    }

    if (!HKSUCCESS(HkKickReason(player, wscReason))) {
        PrintError();
        return;
    }

    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetBaseStatus(const std::wstring &wscBasename) {

    RIGHT_CHECK(RIGHT_OTHER);

    float fHealth;
    float fMaxHealth;

    if (HKSUCCESS(HkGetBaseStatus(wscBasename, fHealth, fMaxHealth)))
        Print(L"hitpts=%u hitptsmax=%uOK\n", (long)fHealth, (long)fMaxHealth);
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetClientId(std::wstring player) {
    RIGHT_CHECK(RIGHT_OTHER);

    uint iClientID = HkGetClientIdFromCharname(player);
    if (iClientID == -1) {
        hkLastErr = HKE_PLAYER_NOT_LOGGED_IN;
        PrintError();
        return;
    }

    Print(L"clientid=%uOK\n", iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKill(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_BEAMKILL);

    if (HKSUCCESS(HkKill(player)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdResetRep(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_REPUTATION);

    if (HKSUCCESS(HkResetRep(player)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetRep(std::variant<uint, std::wstring> player,
                      const std::wstring &wscRepGroup, float fValue) {
    RIGHT_CHECK(RIGHT_REPUTATION);

    if (HKSUCCESS(HkSetRep(player, wscRepGroup, fValue)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetRep(std::variant<uint, std::wstring> player,
                      const std::wstring &wscRepGroup) {
    RIGHT_CHECK(RIGHT_REPUTATION);

    float fValue;
    if (HKSUCCESS(HkGetRep(player, wscRepGroup, fValue))) {
        Print(L"feelings=%f", fValue);
        Print(L"OK");
    } else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsg(std::variant<uint, std::wstring> player,
                   const std::wstring &wscText) {
    RIGHT_CHECK(RIGHT_MSG);

    if (HKSUCCESS(HkMsg(player, wscText)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgS(const std::wstring &wscSystemname,
                    const std::wstring &wscText) {
    RIGHT_CHECK(RIGHT_MSG);

    if (HKSUCCESS(HkMsgS(wscSystemname, wscText)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgU(const std::wstring &wscText) {
    RIGHT_CHECK(RIGHT_MSG);

    if (HKSUCCESS(HkMsgU(wscText)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsg(std::variant<uint, std::wstring> player,
                    const std::wstring &wscXML) {
    RIGHT_CHECK(RIGHT_MSG);

    if (HKSUCCESS(HkFMsg(player, wscXML)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgS(const std::wstring &wscSystemname,
                     const std::wstring &wscXML) {
    RIGHT_CHECK(RIGHT_MSG);

    if (HKSUCCESS(HkFMsgS(wscSystemname, wscXML)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgU(const std::wstring &wscXML) {
    RIGHT_CHECK(RIGHT_MSG);

    if (HKSUCCESS(HkFMsgU(wscXML)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdEnumCargo(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_CARGO);

    std::list<CARGO_INFO> lstCargo;
    int iRemainingHoldSize = 0;
    if (HKSUCCESS(HkEnumCargo(player, lstCargo, iRemainingHoldSize))) {
        Print(L"remainingholdsize=%d", iRemainingHoldSize);
        for (auto &cargo : lstCargo) {
            if (!cargo.bMounted)
                Print(L"id=%u archid=%u count=%d mission=%u", cargo.iID,
                      cargo.iArchID, cargo.iCount, cargo.bMission ? 1 : 0);
        }

        Print(L"OK");
    } else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRemoveCargo(std::variant<uint, std::wstring> player, uint iID,
                           uint iCount) {
    RIGHT_CHECK(RIGHT_CARGO);

    if (HKSUCCESS(HkRemoveCargo(player, iID, iCount)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCargo(std::variant<uint, std::wstring> player,
                        const std::wstring &wscGood, uint iCount,
                        uint iMission) {
    RIGHT_CHECK(RIGHT_CARGO);

    if (HKSUCCESS(
            HkAddCargo(player, wscGood, iCount, iMission ? true : false)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRename(std::variant<uint, std::wstring> player,
                      const std::wstring &wscNewCharname) {
    RIGHT_CHECK(RIGHT_CHARACTERS);

    if (HKSUCCESS(HkRename(player, wscNewCharname, false)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDeleteChar(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_CHARACTERS);

    if (HKSUCCESS(HkRename(player, L"", true)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdReadCharFile(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_CHARACTERS);

    std::list<std::wstring> lstOut;
    if (HKSUCCESS(HkReadCharFile(player, lstOut))) {
        for (auto &l : lstOut)
            Print(L"l %s", l.c_str());
        Print(L"OK");
    } else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdWriteCharFile(std::variant<uint, std::wstring> player,
                             const std::wstring &wscData) {
    RIGHT_CHECK(RIGHT_CHARACTERS);

    if (HKSUCCESS(HkWriteCharFile(player, wscData)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintPlayerInfo(HKPLAYERINFO pi) {
    RIGHT_CHECK(RIGHT_OTHER);

    Print(L"charname=%s clientid=%u ip=%s host=%s ping=%u base=%s system=%s",
          pi.wscCharname.c_str(), pi.iClientID, pi.wscIP.c_str(),
          pi.wscHostname.c_str(), pi.ci.dwRoundTripLatencyMS,
          pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdGetPlayerInfo(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_OTHER);

    HKPLAYERINFO pi;
    if (HKSUCCESS(HkGetPlayerInfo(player, pi, false)))
        PrintPlayerInfo(pi);
    else
        PrintError();

    Print(L"OK");
}

void CCmds::CmdGetPlayers() {
    RIGHT_CHECK(RIGHT_OTHER);

    for (auto &p : HkGetPlayers())
        PrintPlayerInfo(p);

    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::XPrintPlayerInfo(HKPLAYERINFO pi) {
    RIGHT_CHECK(RIGHT_OTHER);

    Print(
        L"Name: %s, ID: %u, IP: %s, Host: %s, Ping: %u, Base: %s, System: %s\n",
        pi.wscCharname.c_str(), pi.iClientID, pi.wscIP.c_str(),
        pi.wscHostname.c_str(), pi.ci.dwRoundTripLatencyMS, pi.wscBase.c_str(),
        pi.wscSystem.c_str());
}

void CCmds::CmdXGetPlayerInfo(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_OTHER);

    HKPLAYERINFO pi;
    if (HKSUCCESS(HkGetPlayerInfo(player, pi, false)))
        XPrintPlayerInfo(pi);
    else
        PrintError();

    Print(L"OK");
}

void CCmds::CmdXGetPlayers() {
    RIGHT_CHECK(RIGHT_OTHER);

    for (auto &p : HkGetPlayers())
        XPrintPlayerInfo(p);

    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetPlayerIDs() {
    RIGHT_CHECK(RIGHT_OTHER);

    wchar_t wszLine[128] = L"";
    for (auto &p : HkGetPlayers()) {
        wchar_t wszBuf[1024];
        swprintf_s(wszBuf, L"%s = %u | ", p.wscCharname.c_str(), p.iClientID);
        if ((wcslen(wszBuf) + wcslen(wszLine)) >= sizeof(wszLine) / 2) {
            Print(L"%s", wszLine);
            wcscpy_s(wszLine, wszBuf);
        } else
            wcscat_s(wszLine, wszBuf);
    }

    if (wcslen(wszLine))
        Print(L"%s", wszLine);
    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAccountDirName(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_OTHER);

    std::wstring wscDir;
    if (HKSUCCESS(HkGetAccountDirName(player, wscDir)))
        Print(L"dirname=%sOK\n", wscDir.c_str());
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCharFileName(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_OTHER);

    std::wstring wscFilename;
    if (HKSUCCESS(HkGetCharFileName(player, wscFilename)))
        Print(L"charfilename=%sOK\n", wscFilename.c_str());
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsOnServer(std::wstring player) {
    RIGHT_CHECK(RIGHT_OTHER);

    CAccount *acc = HkGetAccountByCharname(player);
    if (!acc) {
        hkLastErr = HKE_CHAR_DOES_NOT_EXIST;
        PrintError();
        return;
    }

    uint iClientID = HkGetClientIdFromAccount(acc);
    if (iClientID == -1)
        Print(L"onserver=noOK\n");
    else
        Print(L"onserver=yesOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsLoggedIn(std::wstring player) {
    RIGHT_CHECK(RIGHT_OTHER);

    if (HkGetClientIdFromCharname(player) != -1) {
        if (HkIsInCharSelectMenu(player))
            Print(L"loggedin=noOK\n");
        else
            Print(L"loggedin=yesOK\n");
    } else
        Print(L"loggedin=noOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMoneyFixList() {
    RIGHT_CHECK(RIGHT_OTHER);

    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        uint iClientID = HkGetClientIdFromPD(pPD);

        if (ClientInfo[iClientID].lstMoneyFix.size())
            Print(L"id=%u", iClientID);
    }

    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdServerInfo() {
    RIGHT_CHECK(RIGHT_OTHER);

    // calculate uptime
    FILETIME ftCreation;
    FILETIME ft;
    GetProcessTimes(GetCurrentProcess(), &ftCreation, &ft, &ft, &ft);
    SYSTEMTIME st;
    GetSystemTime(&st);
    FILETIME ftNow;
    SystemTimeToFileTime(&st, &ftNow);
    __int64 iTimeCreation =
        (((__int64)ftCreation.dwHighDateTime) << 32) + ftCreation.dwLowDateTime;
    __int64 iTimeNow =
        (((__int64)ftNow.dwHighDateTime) << 32) + ftNow.dwLowDateTime;

    uint iUptime = (uint)((iTimeNow - iTimeCreation) / 10000000);
    uint iDays = (iUptime / (60 * 60 * 24));
    iUptime %= (60 * 60 * 24);
    uint iHours = (iUptime / (60 * 60));
    iUptime %= (60 * 60);
    uint iMinutes = (iUptime / 60);
    iUptime %= (60);
    uint iSeconds = iUptime;
    wchar_t wszUptime[16];
    swprintf_s(wszUptime, L"%.1u:%.2u:%.2u:%.2u", iDays, iHours, iMinutes,
               iSeconds);

    // print
    Print(L"serverload=%d npcspawn=%s uptime=%sOK\n", g_iServerLoad,
          g_bNPCDisabled ? L"disabled" : L"enabled", wszUptime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetGroupMembers(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_OTHER);

    std::list<GROUP_MEMBER> lstMembers;
    if (HKSUCCESS(HkGetGroupMembers(player, lstMembers))) {
        Print(L"groupsize=%d", lstMembers.size());
        for (auto &m : lstMembers)
            Print(L"id=%d charname=%s", m.iClientID, m.wscCharname.c_str());
        Print(L"OK");
    } else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSaveChar(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_OTHER);

    if (HKSUCCESS(HkSaveChar(player)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetReservedSlot(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK(RIGHT_SETTINGS);

    bool bResult;
    if (HKSUCCESS(HkGetReservedSlot(player, bResult)))
        Print(L"reservedslot=%sOK\n", bResult ? L"yes" : L"no");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetReservedSlot(std::variant<uint, std::wstring> player,
                               int iReservedSlot) {
    RIGHT_CHECK(RIGHT_SETTINGS);

    if (HKSUCCESS(HkSetReservedSlot(player, iReservedSlot ? true : false)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAdmin(std::variant<uint, std::wstring> player,
                        const std::wstring &wscRights) {
    RIGHT_CHECK_SUPERADMIN();

    if (HKSUCCESS(HkSetAdmin(player, wscRights)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAdmin(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK_SUPERADMIN();

    std::wstring wscRights;
    if (HKSUCCESS(HkGetAdmin(player, wscRights)))
        Print(L"rights=%sOK\n", wscRights.c_str());
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDelAdmin(std::variant<uint, std::wstring> player) {
    RIGHT_CHECK_SUPERADMIN();

    if (HKSUCCESS(HkDelAdmin(player)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugins() {
    RIGHT_CHECK(RIGHT_PLUGINS);

    PluginManager::i()->loadAll(false, this);
    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugin(const std::wstring &wscPlugin) {
    RIGHT_CHECK(RIGHT_PLUGINS);

    PluginManager::i()->load(wscPlugin, this, false);
    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdListPlugins() {

    RIGHT_CHECK(RIGHT_PLUGINS);

    for (const auto &data : PluginManager::ir())
        Print(L"%s (%s) - %s", stows(data.name).c_str(),
              stows(data.shortName).c_str(),
              (!data.paused ? L"running" : L"paused"));

    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnloadPlugin(const std::wstring &wscPlugin) {
    RIGHT_CHECK(RIGHT_PLUGINS);

    if (HKSUCCESS(PluginManager::i()->unload(wstos(wscPlugin))))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdPausePlugin(const std::wstring &wscPlugin) {
    RIGHT_CHECK(RIGHT_PLUGINS);

    if (HKSUCCESS(PluginManager::i()->pause(wstos(wscPlugin), true)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnpausePlugin(const std::wstring &wscPlugin) {
    RIGHT_CHECK(RIGHT_PLUGINS);

    if (HKSUCCESS(PluginManager::i()->pause(wstos(wscPlugin), false)))
        Print(L"OK");
    else
        PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRehash() {
    RIGHT_CHECK(RIGHT_SETTINGS);
    
    LoadSettings();
    CallPluginsAfter(HookedCall::FLHook__LoadSettings);

    HookRehashed();
    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Chase a player. Only works in system as you'd need a client hook to do across system */
void CCmds::CmdChase(std::wstring wscAdminName, std::variant<uint, std::wstring> player) {

    RIGHT_CHECK_SUPERADMIN();

    HKPLAYERINFO adminPlyr;
    if (HkGetPlayerInfo(wscAdminName, adminPlyr, false) != HKE_OK) {
        Print(L"ERR Not in space");
        return;
    }

    HKPLAYERINFO targetPlyr;
    if (HkGetPlayerInfo(player, targetPlyr, false) != HKE_OK ||
        targetPlyr.iShip == 0) {
        Print(L"ERR Player not found or not in space");
        return;
    }

    Vector pos;
    Matrix ornt;
    pub::SpaceObj::GetLocation(targetPlyr.iShip, pos, ornt);
    pos.y += 100;

    Print(L"Jump to system=%s x=%0.0f y=%0.0f z=%0.0f",
                targetPlyr.wscSystem.c_str(), pos.x, pos.y, pos.z);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Beam admin to a base. Works across systems but needs improvement of the path
 * selection algorithm */
void CCmds::CmdBeam(std::variant<uint, std::wstring> player,
                   const std::wstring &wscTargetBaseName) {
    RIGHT_CHECK(RIGHT_BEAMKILL);

    HKPLAYERINFO info;
    if (HkGetPlayerInfo(player, info, false) != HKE_OK) {
        Print(L"ERR Player not found");
        return;
    }

    if (info.iShip == 0) {
        Print(L"ERR Player not in space");
        return;
    }

    // Search for an exact match at the start of the name
    struct Universe::IBase *baseinfo = Universe::GetFirstBase();
    while (baseinfo) {
        std::wstring basename = HkGetWStringFromIDS(baseinfo->iBaseIDS);
        if (ToLower(basename).find(ToLower(wscTargetBaseName)) == 0) {
            pub::Player::ForceLand(info.iClientID, baseinfo->iBaseID);
            if (info.iSystem != baseinfo->iSystemID) {
                Server.BaseEnter(baseinfo->iBaseID, info.iClientID);
                Server.BaseExit(baseinfo->iBaseID, info.iClientID);
                std::wstring wscCharFileName;
                HkGetCharFileName(info.wscCharname, wscCharFileName);
                wscCharFileName += L".fl";
                CHARACTER_ID cID;
                strcpy(cID.szCharFilename,
                       wstos(wscCharFileName.substr(0, 14)).c_str());
                Server.CharacterSelect(cID, info.iClientID);
            }
            return;
        }
        baseinfo = Universe::GetNextBase();
    }

    // Exact match failed, try a for an partial match
    baseinfo = Universe::GetFirstBase();
    while (baseinfo) {
        std::wstring basename = HkGetWStringFromIDS(baseinfo->iBaseIDS);
        if (ToLower(basename).find(ToLower(wscTargetBaseName)) != -1) {
            pub::Player::ForceLand(info.iClientID, baseinfo->iBaseID);
            if (info.iSystem != baseinfo->iSystemID) {
                Server.BaseEnter(baseinfo->iBaseID, info.iClientID);
                Server.BaseExit(baseinfo->iBaseID, info.iClientID);
                std::wstring wscCharFileName;
                HkGetCharFileName(info.wscCharname, wscCharFileName);
                wscCharFileName += L".fl";
                CHARACTER_ID cID;
                strcpy(cID.szCharFilename,
                       wstos(wscCharFileName.substr(0, 14)).c_str());
                Server.CharacterSelect(cID, info.iClientID);
            }
            return;
        }
        baseinfo = Universe::GetNextBase();
    }

    // Fall back to default flhook .beam command
    try {
        if (HKSUCCESS(HkBeam(player, wscTargetBaseName)))
            Print(L"OK");
        else
            PrintError();
    } catch (...) { // exeption, kick player
        HkKick(player);
        Print(L"ERR exception occured, player kicked");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Pull a player to you. Only works in system as you'd need a client hook to move their system **/
void CCmds::CmdPull(std::wstring wscAdminName, std::variant<uint, std::wstring> player) {

    RIGHT_CHECK_SUPERADMIN();

    HKPLAYERINFO adminPlyr;
    if (HkGetPlayerInfo(wscAdminName, adminPlyr, false) != HKE_OK ||
        adminPlyr.iShip == 0) {
        Print(L"ERR Not in space");
        return;
    }

    HKPLAYERINFO targetPlyr;
    if (HkGetPlayerInfo(player, targetPlyr, false) != HKE_OK) {
        Print(L"ERR Player not found");
        return;
    }

    Vector pos;
    Matrix ornt;
    pub::SpaceObj::GetLocation(adminPlyr.iShip, pos, ornt);
    pos.y += 400;

    Print(L"Jump to system=%s x=%0.0f y=%0.0f z=%0.0f",
                adminPlyr.wscSystem.c_str(), pos.x, pos.y, pos.z);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Move to location */
void CCmds::CmdMove(std::wstring wscAdminName, float x, float y, float z) {

    RIGHT_CHECK_SUPERADMIN();

    HKPLAYERINFO adminPlyr;
    if (HkGetPlayerInfo(wscAdminName, adminPlyr, false) != HKE_OK ||
        adminPlyr.iShip == 0) {
        Print(L"ERR Not in space");
        return;
    }

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(adminPlyr.iShip, pos, rot);
    pos.x = x;
    pos.y = y;
    pos.z = z;
    Print(L"Moving to %0.0f %0.0f %0.0f", pos.x, pos.y, pos.z);
    HkRelocateClient(adminPlyr.iClientID, pos, rot);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdHelp() {
    std::wstring wszHelpMsg = std::wstring(L"[version]\n") + VersionInformation + L"\n"
                           L"[commands]\n"
                           L"getcash <charname>\n"
                           L"setcash <charname> <amount>\n"
                           L"setcashsec <charname> <oldmoney> <amount>\n"
                           L"addcash <charname> <amount>\n"
                           L"addcashsec <charname> <oldmoney> <amount>\n"
                           L"kick <charname> <reason>\n"
                           L"ban <charname>\n"
                           L"unban <charname>\n"
                           L"kickban <charname> <reason>\n"
                           L"beam <charname> <basename>\n"
                           L"pull <charname>\n"
                           L"chase <charname>\n"
                           L"move <x> <y> <z>\n"
                           L"kill <charname>\n"
                           L"resetrep <charname>\n"
                           L"setrep <charname> <repgroup> <value>\n"
                           L"getrep <charname> <repgroup>\n"
                           L"readcharfile <charname>\n"
                           L"writecharfile <charname> <data>\n"
                           L"enumcargo <charname>\n"
                           L"addcargo <charname> <good> <count> <mission>\n"
                           L"removecargo <charname> <id> <count>\n"
                           L"rename <oldcharname> <newcharname>\n"
                           L"deletechar <charname>\n"
                           L"msg <charname> <text>\n"
                           L"msgs <systemname> <text>\n"
                           L"msgu <text>\n"
                           L"fmsg <charname> <xmltext>\n"
                           L"fmsgs <systemname> <xmltext>\n"
                           L"fmsgu <xmltext>\n"
                           L"enumcargo <charname>\n"
                           L"addcargo <charname> <good> <count> <mission>\n"
                           L"removecargo <charname> <id> <count>\n"
                           L"getgroupmembers <charname>\n"
                           L"getbasestatus <basename>\n"
                           L"getclientid <charname>\n"
                           L"getplayerinfo <charname>\n"
                           L"getplayers\n"
                           L"xgetplayerinfo <charname>\n"
                           L"xgetplayers\n"
                           L"getplayerids\n"
                           L"help\n"
                           L"getaccountdirname <charname>\n"
                           L"getcharfilename <charname>\n"
                           L"isonserver <charname>\n"
                           L"isloggedin <charname>\n"
                           L"serverinfo\n"
                           L"moneyfixlist\n"
                           L"savechar <charname>\n"
                           L"setadmin <charname> <rights>\n"
                           L"getadmin <charname>\n"
                           L"deladmin <charname>\n"
                           L"getreservedslot <charname>\n"
                           L"setreservedslot <charname> <value>\n"
                           L"loadplugins\n"
                           L"loadplugin <plugin filename>\n"
                           L"listplugins\n"
                           L"unloadplugin <plugin shortname>\n"
                           L"pauseplugin <plugin shortname>\n"
                           L"unpauseplugin <plugin shortname>\n"
                           L"rehash\n";
    

    CallPluginsBefore(HookedCall::FLHook__AdminCommand__Help, this);

    Print(L"%s", wszHelpMsg.c_str());

    CallPluginsAfter(HookedCall::FLHook__AdminCommand__Help, this);

    Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgCharname(uint iArg) {
    std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

    if (iArg == 1) {
        if (bID)
            return wscArg.replace((int)0, (int)0, L"id ");
        else if (bShortCut)
            return wscArg.replace((int)0, (int)0, L"sc ");
        else if (bSelf)
            return this->GetAdminName();
        else if (bTarget) {
            uint iClientID = HkGetClientIdFromCharname(this->GetAdminName());
            if (!iClientID)
                return L"";
            uint iShip;
            pub::Player::GetShip(iClientID, iShip);
            if (!iShip)
                return L"";
            uint iTarget;
            pub::SpaceObj::GetTarget(iShip, iTarget);
            if (!iTarget)
                return L"";
            iClientID = HkGetClientIDByShip(iTarget);
            if (!iClientID)
                return L"";
            return L"id " + std::to_wstring(iClientID);
        }
    }

    {
        if (wscArg == L">s")
            return this->GetAdminName();
        else if (wscArg.find(L">i") == 0)
            return L"id " + wscArg.substr(2);
        else if (wscArg == L">t") {
            uint iClientID = HkGetClientIdFromCharname(this->GetAdminName());
            if (!iClientID)
                return L"";
            uint iShip;
            pub::Player::GetShip(iClientID, iShip);
            if (!iShip)
                return L"";
            uint iTarget;
            pub::SpaceObj::GetTarget(iShip, iTarget);
            if (!iTarget)
                return L"";
            iClientID = HkGetClientIDByShip(iTarget);
            if (!iClientID)
                return L"";
            return L"id " + std::to_wstring(iClientID);
        } else
            return wscArg;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCmds::ArgInt(uint iArg) {
    std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

    return ToInt(wscArg);
}

uint CCmds::ArgUInt(uint iArg) {
    std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

    return ToUInt(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CCmds::ArgFloat(uint iArg) {
    std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);
    return ToFloat(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgStr(uint iArg) {
    std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

    return wscArg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgStrToEnd(uint iArg) {
    for (uint i = 0, iCurArg = 0; (i < wscCurCmdString.length()); i++) {
        if (wscCurCmdString[i] == ' ') {
            iCurArg++;

            if (iCurArg == iArg)
                return wscCurCmdString.substr(i + 1);

            while (((i + 1) < wscCurCmdString.length()) &&
                   (wscCurCmdString[i + 1] == ' '))
                i++; // skip "whitechar"
        }
    }

    return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_CMD(a) !wscCmd.compare(L##a)

void CCmds::ExecuteCommandString(const std::wstring &wscCmdStr) {
    // check if command was sent by a socket connection
    bool bSocket = false;
    bool bLocalSocket = false;
    std::wstring wscAdminName = GetAdminName();

    if (wscAdminName.find(L"Socket connection") == 0) {
        bSocket = true;
        if (wscAdminName.find(L"127.0.0.1") != std::wstring::npos)
            bLocalSocket = true;
    }

    try {
        if (bSocket && ((bLocalSocket && set_bLogLocalSocketCmds) || set_bLogSocketCmds))
            AddLog(SocketCmds,L"%s: %s", wscAdminName.c_str(), wscCmdStr.c_str());

        if (set_bLogAdminCmds)
            AddLog(AdminCmds,L"%s: %s", wscAdminName.c_str(), wscCmdStr.c_str());

        bID = false;
        bShortCut = false;
        bSelf = false;
        bTarget = false;
        wscCurCmdString = wscCmdStr;

        std::wstring wscCmd = ToLower(GetParam(wscCmdStr, ' ', 0));
        if (wscCmd.length() == 0) {
            Print(L"ERR unknown command");
            return;
        }

        size_t wscCmd_pos = wscCmdStr.find(wscCmd);

        if (wscCmd[wscCmd.length() - 1] == '$') {
            bID = true;
            wscCmd.erase(wscCmd.length() - 1, 1);
        } else if (wscCmd[wscCmd.length() - 1] == '&') {
            bShortCut = true;
            wscCmd.erase(wscCmd.length() - 1, 1);
        } else if (wscCmd[wscCmd.length() - 1] == '!') {
            bSelf = true;
            wscCurCmdString.insert(wscCmd_pos + wscCmd.length() - 1, L" ");
            wscCmd.erase(wscCmd.length() - 1, 1);
        } else if (wscCmd[wscCmd.length() - 1] == '?') {
            bTarget = true;
            wscCurCmdString.insert(wscCmd_pos + wscCmd.length() - 1, L" ");
            wscCmd.erase(wscCmd.length() - 1, 1);
        }

        bool plugins = CallPluginsBefore(HookedCall::FLHook__AdminCommand__Process, this, wscCmd);

        if (!plugins) {

            if (IS_CMD("getcash")) {
                CmdGetCash(ArgCharname(1));
            } else if (IS_CMD("setcash")) {
                CmdSetCash(ArgCharname(1), ArgInt(2));
            } else if (IS_CMD("setcashsec")) {
                CmdSetCashSec(ArgCharname(1), ArgInt(2), ArgInt(3));
            } else if (IS_CMD("addcash")) {
                CmdAddCash(ArgCharname(1), ArgInt(2));
            } else if (IS_CMD("addcashsec")) {
                CmdAddCashSec(ArgCharname(1), ArgInt(2), ArgInt(3));
            } else if (IS_CMD("kick")) {
                CmdKick(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("ban")) {
                CmdBan(ArgCharname(1));
            } else if (IS_CMD("unban")) {
                CmdUnban(ArgCharname(1));
            } else if (IS_CMD("kickban")) {
                CmdKickBan(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("getbasestatus")) {
                CmdGetBaseStatus(ArgStr(1));
            } else if (IS_CMD("getclientid")) {
                CmdGetClientId(ArgCharname(1));
            } else if (IS_CMD("beam")) {
                CmdBeam(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("kill")) {
                CmdKill(ArgCharname(1));
            } else if (IS_CMD("resetrep")) {
                CmdResetRep(ArgCharname(1));
            } else if (IS_CMD("setrep")) {
                CmdSetRep(ArgCharname(1), ArgStr(2), ArgFloat(3));
            } else if (IS_CMD("getrep")) {
                CmdGetRep(ArgCharname(1), ArgStr(2));
            } else if (IS_CMD("msg")) {
                CmdMsg(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("msgs")) {
                CmdMsgS(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("msgu")) {
                CmdMsgU(ArgStrToEnd(1));
            } else if (IS_CMD("fmsg")) {
                CmdFMsg(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("fmsgs")) {
                CmdFMsgS(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("fmsgu")) {
                CmdFMsgU(ArgStrToEnd(1));
            } else if (IS_CMD("enumcargo")) {
                CmdEnumCargo(ArgCharname(1));
            } else if (IS_CMD("removecargo")) {
                CmdRemoveCargo(ArgCharname(1), ArgInt(2), ArgInt(3));
            } else if (IS_CMD("addcargo")) {
                CmdAddCargo(ArgCharname(1), ArgStr(2), ArgInt(3), ArgInt(4));
            } else if (IS_CMD("rename")) {
                CmdRename(ArgCharname(1), ArgStr(2));
            } else if (IS_CMD("deletechar")) {
                CmdDeleteChar(ArgCharname(1));
            } else if (IS_CMD("readcharfile")) {
                CmdReadCharFile(ArgCharname(1));
            } else if (IS_CMD("writecharfile")) {
                CmdWriteCharFile(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("getplayerinfo")) {
                CmdGetPlayerInfo(ArgCharname(1));
            } else if (IS_CMD("getplayers")) {
                CmdGetPlayers();
            } else if (IS_CMD("xgetplayerinfo")) {
                CmdXGetPlayerInfo(ArgCharname(1));
            } else if (IS_CMD("xgetplayers")) {
                CmdXGetPlayers();
            } else if (IS_CMD("getplayerids")) {
                CmdGetPlayerIDs();
            } else if (IS_CMD("getaccountdirname")) {
                CmdGetAccountDirName(ArgCharname(1));
            } else if (IS_CMD("getcharfilename")) {
                CmdGetCharFileName(ArgCharname(1));
            } else if (IS_CMD("savechar")) {
                CmdSaveChar(ArgCharname(1));
            } else if (IS_CMD("isonserver")) {
                CmdIsOnServer(ArgCharname(1));
            } else if (IS_CMD("isloggedin")) {
                CmdIsLoggedIn(ArgCharname(1));
            } else if (IS_CMD("moneyfixlist")) {
                CmdMoneyFixList();
            } else if (IS_CMD("serverinfo")) {
                CmdServerInfo();
            } else if (IS_CMD("getgroupmembers")) {
                CmdGetGroupMembers(ArgCharname(1));
            } else if (IS_CMD("getreservedslot")) {
                CmdGetReservedSlot(ArgCharname(1));
            } else if (IS_CMD("setreservedslot")) {
                CmdSetReservedSlot(ArgCharname(1), ArgInt(2));
            } else if (IS_CMD("setadmin")) {
                CmdSetAdmin(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("getadmin")) {
                CmdGetAdmin(ArgCharname(1));
            } else if (IS_CMD("deladmin")) {
                CmdDelAdmin(ArgCharname(1));
            } else if (IS_CMD("unloadplugin")) {
                CmdUnloadPlugin(ArgStrToEnd(1));
            } else if (IS_CMD("loadplugins")) {
                CmdLoadPlugins();
            } else if (IS_CMD("loadplugin")) {
                CmdLoadPlugin(ArgStrToEnd(1));
            } else if (IS_CMD("listplugins")) {
                CmdListPlugins();
            } else if (IS_CMD("pauseplugin")) {
                CmdPausePlugin(ArgStrToEnd(1));
            } else if (IS_CMD("unpauseplugin")) {
                CmdUnpausePlugin(ArgStrToEnd(1));
            } else if (IS_CMD("rehash")) {
                CmdRehash();
            } else if (IS_CMD("help")) {
                CmdHelp();
            } else if (IS_CMD("move")) {
                CmdMove(wscAdminName, ArgFloat(1), ArgFloat(2), ArgFloat(3));
            } else if (IS_CMD("chase")) {
                CmdChase(wscAdminName, ArgCharname(1));
            } else if (IS_CMD("beam")) {
                CmdBeam(ArgCharname(1), ArgStrToEnd(2));
            } else if (IS_CMD("pull")) {
                CmdPull(wscAdminName, ArgCharname(1));
            } else {
                Print(L"ERR unknown command");
            }
        }
        if (bSocket) {
            if (bLocalSocket) {
                if (set_bLogLocalSocketCmds)
                    AddLog(SocketCmds,L"finnished");
            } else {
                if (set_bLogSocketCmds)
                    AddLog(SocketCmds,L"finnished");
            }
        } else {
            if (set_bLogAdminCmds)
                AddLog(AdminCmds,L"finnished");
        }
    } catch (...) {
        if (bSocket) {
            if (bLocalSocket) {
                if (set_bLogLocalSocketCmds)
                    AddLog(SocketCmds,L"exception");
            } else {
                if (set_bLogSocketCmds)
                    AddLog(SocketCmds,L"exception");
            }
        } else {
            if (set_bLogAdminCmds)
                AddLog(AdminCmds,L"exception");
        }
        Print(L"ERR exception occured");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::SetRightsByString(const std::string &scRights) {
    rights = RIGHT_NOTHING;
    std::string scRightStr = ToLower(scRights);
    if (scRightStr.find("superadmin") != -1)
        rights |= RIGHT_SUPERADMIN;
    if (scRightStr.find("cash") != -1)
        rights |= RIGHT_CASH;
    if (scRightStr.find("kickban") != -1)
        rights |= RIGHT_KICKBAN;
    if (scRightStr.find("beamkill") != -1)
        rights |= RIGHT_BEAMKILL;
    if (scRightStr.find("msg") != -1)
        rights |= RIGHT_MSG;
    if (scRightStr.find("other") != -1)
        rights |= RIGHT_OTHER;
    if (scRightStr.find("cargo") != -1)
        rights |= RIGHT_CARGO;
    if (scRightStr.find("characters") != -1)
        rights |= RIGHT_CHARACTERS;
    if (scRightStr.find("settings") != -1)
        rights |= RIGHT_SETTINGS;
    if (scRightStr.find("reputation") != -1)
        rights |= RIGHT_REPUTATION;
    if (scRightStr.find("plugins") != -1)
        rights |= RIGHT_PLUGINS;
    if (scRightStr.find("eventmode") != -1)
        rights |= RIGHT_EVENTMODE;
    if (scRightStr.find("special1") != -1)
        rights |= RIGHT_SPECIAL1;
    if (scRightStr.find("special2") != -1)
        rights |= RIGHT_SPECIAL2;
    if (scRightStr.find("special3") != -1)
        rights |= RIGHT_SPECIAL3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintError() {
    Print(L"ERR %s", HkErrGetText(this->hkLastErr).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::Print(std::wstring wscText, ...) {
    wchar_t wszBuf[1024 * 8] = L"";
    va_list marker;
    va_start(marker, wscText);

    _vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, wscText.c_str(), marker);

    DoPrint(wszBuf);
}
