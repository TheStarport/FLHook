// Death Penalty Plugin
// Ported from 88Flak by Raikkonen
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

// Load configuration file
void LoadSettings() {
    returncode = DEFAULT_RETURNCODE;

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string configFile =
        std::string(szCurDir) + "\\flhook_plugins\\deathpenalty.cfg";

    INI_Reader ini;
    if (ini.open(configFile.c_str(), false)) {
        while (ini.read_header()) {
            // [General]
            if (ini.is_header("General"))
                while (ini.read_value()) {
                    if (ini.is_value("DeathPenaltyFraction"))
                        set_fDeathPenalty = ini.get_value_float(0);
                    else if (ini.is_value("DeathPenaltyKillerFraction"))
                        set_fDeathPenaltyKiller = ini.get_value_float(0);
                }

            // [ExcludedSystems]
            if (ini.is_header("ExcludedSystems"))
                while (ini.read_value()) {
                    if (ini.is_value("system"))
                        ExcludedSystems.push_back(
                            CreateID(ini.get_value_string(0)));
                }

            // [ShipOverrides]
            if (ini.is_header("ShipOverrides"))
                while (ini.read_value()) {
                    if (ini.is_value("ship"))
                        FractionOverridesbyShip[CreateID(
                            ini.get_value_string(0))] = ini.get_value_float(1);
                }
        }
        ini.close();
    }
}

void ClearClientInfo(uint iClientID) {
    returncode = DEFAULT_RETURNCODE;
    MapClients.erase(iClientID);
}

// Is the player is a system that is excluded from death penalty?
bool bExcludedSystem(uint iClientID) {
    // Get System ID
    uint iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);
    // Search list for system
    return (std::find(ExcludedSystems.begin(), ExcludedSystems.end(),
                      iSystemID) != ExcludedSystems.end());
}

// This returns the override for the specific ship as defined in the cfg file.
// If there is not override it returns the default value defined as
// "DeathPenaltyFraction" in the cfg file
float fShipFractionOverride(uint iClientID) {
    // Get ShipArchID
    uint iShipArchID;
    pub::Player::GetShipID(iClientID, iShipArchID);

    // Default return value is the default death penalty fraction
    float fOverrideValue = set_fDeathPenalty;

    // See if the ship has an override fraction
    if (FractionOverridesbyShip.find(iShipArchID) !=
        FractionOverridesbyShip.end())
        fOverrideValue = FractionOverridesbyShip[iShipArchID];

    return fOverrideValue;
}

// Hook on Player Launch. Used to work out the death penalty and display a
// message to the player warning them of such
void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;

    // No point in processing anything if there is no death penalty
    if (set_fDeathPenalty) {

        // Check to see if the player is in a system that doesn't have death
        // penalty
        if (!bExcludedSystem(iClientID)) {

            // Get the players net worth
            float fValue;
            pub::Player::GetAssetValue(iClientID, fValue);

            // Calculate what the death penalty would be upon death
            MapClients[iClientID].DeathPenaltyCredits =
                (int)(fValue * fShipFractionOverride(set_fDeathPenalty));

            // Should we print a death penalty notice?
            if (MapClients[iClientID].bDisplayDPOnLaunch)
                PrintUserCmdText(
                    iClientID,
                    L"Notice: the death penalty for your ship will be " +
                        ToMoneyStr(MapClients[iClientID].DeathPenaltyCredits) +
                        L" credits.  Type /dp for more information.");
        } else
            MapClients[iClientID].DeathPenaltyCredits = 0;
    }
}

void LoadUserCharSettings(uint iClientID) {
    returncode = DEFAULT_RETURNCODE;

    // Get Account directory then flhookuser.ini file
    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    std::wstring wscDir;
    HkGetAccountDirName(acc, wscDir);
    std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

    // Get char filename and save setting to flhookuser.ini
    std::wstring wscFilename;
    HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename);
    std::string scFilename = wstos(wscFilename);
    std::string scSection = "general_" + scFilename;

    // read death penalty settings
    CLIENT_DATA c;
    c.bDisplayDPOnLaunch = IniGetB(scUserFile, scSection, "DPnotice", true);
    MapClients[iClientID] = c;
}

// Function that will apply the death penalty on a player death
void HkPenalizeDeath(uint iClientID, uint iKillerID) {
    if (!set_fDeathPenalty)
        return;

    // Valid iClientID and the ShipArch or System isnt in the excluded list?
    if (iClientID != -1 && !bExcludedSystem(iClientID)) {

        // Get the players cash
        int iCash;
        HkGetCash(ARG_CLIENTID(iClientID), iCash);

        // Get how much the player owes
        int iOwed = MapClients[iClientID].DeathPenaltyCredits;

        // If the amount the player owes is more than they have, set the
        // amount to their total cash
        if (iOwed > iCash)
            iOwed = iCash;

        // If another player has killed the player
        if (iKillerID && set_fDeathPenaltyKiller) {
            int iGive = (int)(iOwed * set_fDeathPenaltyKiller);
            if (iGive) {
                // Reward the killer, print message to them
                HkAddCash(ARG_CLIENTID(iKillerID), iGive);
                PrintUserCmdText(iKillerID,
                                 L"Death penalty: given " + ToMoneyStr(iGive) +
                                     L" credits from %s's death penalty.",
                                 Players.GetActiveCharacterName(iClientID));
            }
        }

        if (iOwed) {
            // Print message to the player and remove cash
            PrintUserCmdText(iClientID, L"Death penalty: charged " +
                                            ToMoneyStr(iOwed) + L" credits.");
            HkAddCash(ARG_CLIENTID(iClientID), -iOwed);
        }
    }
}

// Hook on ShipDestroyed
void __stdcall ShipDestroyed(DamageList *_dmg, DWORD *ecx, uint iKill) {
    returncode = DEFAULT_RETURNCODE;

    if (iKill) {
        // Get iClientID
        CShip *cship = (CShip *)ecx[4];
        uint iClientID = cship->GetOwnerPlayer();

        // Get Killer ID if there is one
        uint iKillerID = 0;
        if (iClientID) {
            DamageList dmg;
            if (!dmg.get_cause())
                dmg = ClientInfo[iClientID].dmgLast;
            iKillerID = HkGetClientIDByShip(dmg.get_inflictor_id());
        }

        // Call function to penalize player and reward killer
        HkPenalizeDeath(iClientID, iKillerID);
    }
}

// This will save whether the player wants to receieve the /dp notice or not to the flhookuser.ini file
void SaveDPNoticeToCharFile(uint iClientID, std::string value) {
    std::wstring wscDir, wscFilename;
    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    if (HKHKSUCCESS(HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
        HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
        std::string scUserFile =
            scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
        std::string scSection = "general_" + wstos(wscFilename);
        IniWrite(scUserFile, scSection, "DPnotice", value);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// /dp command. Shows information about death penalty
bool UserCmd_DP(uint iClientID, const std::wstring &wscCmd,
                const std::wstring &wscParam, const wchar_t *usage) {

    // If there is no death penalty, no point in having death penalty commands
    if (!set_fDeathPenalty) {
        return true;
    }

    if (wscParam.length()) // Arguments passed
    {
        if (ToLower(Trim(wscParam)) == L"off") {
            MapClients[iClientID].bDisplayDPOnLaunch = false;
            SaveDPNoticeToCharFile(iClientID, "no");
            PrintUserCmdText(iClientID, L"Death penalty notices disabled.");
        } else if (ToLower(Trim(wscParam)) == L"on") {
            MapClients[iClientID].bDisplayDPOnLaunch = true;
            SaveDPNoticeToCharFile(iClientID, "yes");
            PrintUserCmdText(iClientID, L"Death penalty notices enabled.");
        } else {
            PrintUserCmdText(iClientID, L"ERR Invalid parameters");
            PrintUserCmdText(iClientID, usage);
        }
    } else {
        PrintUserCmdText(
            iClientID,
            L"The death penalty is charged immediately when you die.");
        if (!bExcludedSystem(iClientID)) {
            float fValue;
            pub::Player::GetAssetValue(iClientID, fValue);
            int iOwed =
                (int)(fValue * fShipFractionOverride(set_fDeathPenalty));
            PrintUserCmdText(iClientID,
                             L"The death penalty for your ship will be " +
                                 ToMoneyStr(iOwed) + L" credits.");
            PrintUserCmdText(
                iClientID,
                L"If you would like to turn off the death penalty notices, run "
                L"this command with the argument \"off\".");
        } else {
            PrintUserCmdText(iClientID,
                             L"You don't have to pay the death penalty "
                             L"because you are in a specific system.");
        }
    }
    return true;
}

// Additional information related to the plugin when the /help command is used
void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    returncode = DEFAULT_RETURNCODE;
    PrintUserCmdText(iClientID, L"/dp");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMAND PROCESSING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define usable chat commands here
USERCMD UserCmds[] = {
    {L"/dp", UserCmd_DP, L"Usage: /dp"},
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    returncode = DEFAULT_RETURNCODE;

    try {
        std::wstring wscCmdLineLower = ToLower(wscCmd);

        // If the chat std::string does not match the USER_CMD then we do not
        // handle the command, so let other plugins or FLHook kick in. We
        // require an exact match
        for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++) {
            if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0) {
                // Extract the parameters std::string from the chat std::string.
                // It should be immediately after the command and a space.
                std::wstring wscParam = L"";
                if (wscCmd.length() > wcslen(UserCmds[i].wszCmd)) {
                    if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
                        continue;
                    wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
                }

                // Dispatch the command to the appropriate processing function.
                if (UserCmds[i].proc(iClientID, wscCmd, wscParam,
                                     UserCmds[i].usage)) {
                    // We handled the command tell FL hook to stop processing
                    // this chat std::string.
                    returncode =
                        SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command,
                                                    // return immediatly
                    return true;
                }
            }
        }
    } catch (...) {
        AddLog("ERROR: Exception in UserCmd_Process(iClientID=%u, wscCmd=%s)",
               iClientID, wstos(wscCmd).c_str());
        LOG_EXCEPTION;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));

    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        LoadSettings();

    return true;
}

// Functions to hook
EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "Death Penalty";
    p_PI->sShortName = "deathpenalty";
    p_PI->bMayPause = true;
    p_PI->bMayUnload = true;
    p_PI->ePluginReturnCode = &returncode;
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&LoadSettings, PLUGIN_LoadSettings, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Process,
                                             PLUGIN_UserCmd_Process, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Help, PLUGIN_UserCmd_Help, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ShipDestroyed, PLUGIN_ShipDestroyed, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&PlayerLaunch, PLUGIN_HkIServerImpl_PlayerLaunch, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&LoadUserCharSettings,
                                             PLUGIN_LoadUserCharSettings, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&ClearClientInfo,
                                             PLUGIN_ClearClientInfo, 0));
    return p_PI;
}
