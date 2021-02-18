// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.
//
// This file includes code that was not written by me but I can't find
// the original author (I know they posted on the-starport.net about it).

#include <FLHook.h>
#include <float.h>
#include <list>
#include <math.h>
#include <plugin.h>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#include "Main.h"

#include "Shlwapi.h"

#include <FLCoreCommon.h>
#include <FLCoreServer.h>

namespace Restart {

// Players with a rank above this value cannot use the restart command.
static int set_iMaxRank;

// Players with a cash above this value cannot use the restart command.
static int set_iMaxCash;

// Name of each restart and how much cash they cost
static std::map<std::wstring, int> shipPrices;

void Restart::LoadSettings(const std::string &scPluginCfgFile) {
    set_iMaxRank = IniGetI(scPluginCfgFile, "Restart", "MaxRank", 5);
    set_iMaxCash = IniGetI(scPluginCfgFile, "Restart", "MaxCash", 1000000);

    if (set_bEnableRestartCost) {
        INI_Reader ini;
        if (ini.open(scPluginCfgFile.c_str(), false)) {
            while (ini.read_header()) {
                if (ini.is_header("Restart")) {
                    while (ini.read_value()) {
                        if (ini.is_value("restart")) {
                            std::wstring name = stows(ini.get_value_string(0));
                            int iCash = ini.get_value_int(1);
                            shipPrices[name] = iCash;
                        }
                    }
                }
            }
        }
    }
}

bool Restart::UserCmd_ShowRestarts(uint iClientID, const std::wstring &wscCmd,
                                   const std::wstring &wscParam,
                                   const wchar_t *usage) {
    WIN32_FIND_DATA FileData;
    HANDLE hSearch;

    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scRestartFiles =
        std::string(szCurDir) + "\\flhook_plugins\\restart\\*.fl";

    // Start searching for .fl files in the current directory.
    hSearch = FindFirstFile(scRestartFiles.c_str(), &FileData);
    if (hSearch == INVALID_HANDLE_VALUE) {
        PrintUserCmdText(iClientID, L"Restart files not found");
        return true;
    }

    PrintUserCmdText(iClientID, L"You can use these restarts:");
    do {
        // add filename
        std::string scFileName = FileData.cFileName;
        size_t len = scFileName.length();
        scFileName.erase(len - 3, len);
        if (scFileName[0] != '_') {
            if (set_bEnableRestartCost)
                PrintUserCmdText(
                    iClientID,
                    stows(scFileName) + L" - $" +
                        std::to_wstring(shipPrices[stows(scFileName)]));
            else
                PrintUserCmdText(iClientID, stows(scFileName));
        }
    } while (FindNextFile(hSearch, &FileData));

    FindClose(hSearch);
    return true;
}

struct RESTART {
    std::wstring wscCharname;
    std::string scRestartFile;
    std::wstring wscDir;
    std::wstring wscCharfile;
    int iCash;
};
std::list<RESTART> pendingRestarts;

bool Restart::UserCmd_Restart(uint iClientID, const std::wstring &wscCmd,
                              const std::wstring &wscParam,
                              const wchar_t *usage) {
    std::wstring wscFaction = GetParam(wscParam, ' ', 0);
    if (!wscFaction.length()) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, usage);
        return true;
    }

    // Get the character name for this connection.
    RESTART restart;
    restart.wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    // Searching restart
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    restart.scRestartFile = std::string(szCurDir) +
                            "\\flhook_plugins\\restart\\" + wstos(wscFaction) +
                            ".fl";
    if (!::PathFileExistsA(restart.scRestartFile.c_str())) {
        restart.scRestartFile = std::string(szCurDir) +
                                "\\flhook_plugins\\restart\\_" +
                                wstos(wscFaction) + ".fl";
        if (!PathFileExistsA(restart.scRestartFile.c_str())) {
            PrintUserCmdText(iClientID, L"ERR Template does not exist");
            return true;
        }
    }

    // Saving the characters forces an anti-cheat checks and fixes
    // up a multitude of other problems.
    HkSaveChar(iClientID);
    if (!HkIsValidClientID(iClientID))
        return true;

    uint iBaseID;
    pub::Player::GetBase(iClientID, iBaseID);
    if (!iBaseID) {
        PrintUserCmdText(iClientID, L"ERR Not in base");
        return true;
    }

    if (set_iMaxRank != 0) {
        int iRank = 0;
        HkGetRank(restart.wscCharname, iRank);
        if (iRank == 0 || iRank > set_iMaxRank) {
            PrintUserCmdText(iClientID, L"ERR You must create a new char to "
                                        L"restart. Your rank is too high");
            return true;
        }
    }

    HK_ERROR err;
    int iCash = 0;
    if ((err = HkGetCash(restart.wscCharname, iCash)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return true;
    }

    if (set_iMaxCash != 0 && iCash > set_iMaxCash) {
        PrintUserCmdText(iClientID, L"ERR You must create a new char to "
                                    L"restart. Your cash is too high");
        return true;
    }

    if (set_bEnableRestartCost) {
        if (iCash < shipPrices[wscFaction]) {
            PrintUserCmdText(
                iClientID, L"You need $" +
                               std::to_wstring(shipPrices[wscFaction] - iCash) +
                               L" more credits to use this template");
            return true;
        }
        restart.iCash = iCash - shipPrices[wscFaction];
    } else
        restart.iCash = iCash;

    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    if (acc) {
        HkGetAccountDirName(acc, restart.wscDir);
        HkGetCharFileName(restart.wscCharname, restart.wscCharfile);
        pendingRestarts.push_back(restart);
        HkKickReason(
            restart.wscCharname,
            L"Updating character, please wait 10 seconds before reconnecting");
    }
    return true;
}

void Timer() {
    while (pendingRestarts.size()) {
        RESTART restart = pendingRestarts.front();
        if (HkGetClientIdFromCharname(restart.wscCharname) != -1)
            return;

        pendingRestarts.pop_front();

        try {
            // Overwrite the existing character file
            std::string scCharFile = scAcctPath + wstos(restart.wscDir) + "\\" +
                                     wstos(restart.wscCharfile) + ".fl";
            std::string scTimeStampDesc =
                IniGetS(scCharFile, "Player", "description", "");
            std::string scTimeStamp =
                IniGetS(scCharFile, "Player", "tstamp", "0");
            if (!::CopyFileA(restart.scRestartFile.c_str(), scCharFile.c_str(),
                             FALSE))
                throw("copy template");

            flc_decode(scCharFile.c_str(), scCharFile.c_str());
            IniWriteW(scCharFile, "Player", "name", restart.wscCharname);
            IniWrite(scCharFile, "Player", "description", scTimeStampDesc);
            IniWrite(scCharFile, "Player", "tstamp", scTimeStamp);
            IniWrite(scCharFile, "Player", "money",
                     std::to_string(restart.iCash));

            if (!set_bDisableCharfileEncryption)
                flc_encode(scCharFile.c_str(), scCharFile.c_str());

            AddLog("NOTICE: User restart %s for %s",
                   restart.scRestartFile.c_str(),
                   wstos(restart.wscCharname).c_str());
        } catch (char *err) {
            AddLog("ERROR: User restart failed (%s) for %s", err,
                   wstos(restart.wscCharname).c_str());
        } catch (...) {
            AddLog("ERROR: User restart failed for %s",
                   wstos(restart.wscCharname).c_str());
        }
    }
}
} // namespace Restart
