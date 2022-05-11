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
                static_cast<int>(fValue * fShipFractionOverride(set_fDeathPenalty));

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

// This will save whether the player wants to receieve the /dp notice or not to
// the flhookuser.ini file
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
void UserCmd_DP(uint iClientID, const std::wstring &wscParam) {

    // If there is no death penalty, no point in having death penalty commands
    if (!set_fDeathPenalty) {
        return;
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
            PrintUserCmdText(iClientID, L"/dp on | /dp off");
        }
    } else {
        PrintUserCmdText(
            iClientID,
            L"The death penalty is charged immediately when you die.");
        if (!bExcludedSystem(iClientID)) {
            float fValue;
            pub::Player::GetAssetValue(iClientID, fValue);
            int iOwed =
                static_cast<int>(fValue * fShipFractionOverride(set_fDeathPenalty));
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
}

// Additional information related to the plugin when the /help command is used
void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, L"/dp");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMAND PROCESSING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define usable chat commands here
USERCMD UserCmds[] = 
{
    {   L"/dp", UserCmd_DP },
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        LoadSettings();

    return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Death Penalty Plugin");
    pi->shortName("death_penalty");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
    pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
    pi->emplaceHook(HookedCall::FLHook__LoadCharacterSettings, &LoadUserCharSettings);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
}
