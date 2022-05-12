// Wardrobe
// By Raikkonen

#include "Main.h"

void UserCmd_ShowWardrobe(uint iClientID, const std::wstring &wscParam) {

    std::wstring wscType = GetParam(wscParam, ' ', 0);

    if (ToLower(wscType) == L"heads") {
        PrintUserCmdText(iClientID, L"Heads:");
        std::wstring wscHeads;
        for (auto &[name, id] : heads)
            wscHeads += (stows(name) + L" | ");
        PrintUserCmdText(iClientID, wscHeads);
    } else if (ToLower(wscType) == L"bodies") {
        PrintUserCmdText(iClientID, L"Bodies:");
        std::wstring wscBodies;
        for (auto &[name, id] : bodies)
            wscBodies += (stows(name) + L" | ");
        PrintUserCmdText(iClientID, wscBodies);
    }
}

void UserCmd_ChangeCostume(uint iClientID, const std::wstring &wscParam) {

    std::wstring wscType = GetParam(wscParam, ' ', 0);
    std::wstring wscCostume = GetParam(wscParam, ' ', 1);

    if (!wscType.length() || !wscCostume.length()) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        return;
    }

    RESTART restart;

    if (ToLower(wscType) == L"head") {
        if (heads.find(wstos(wscCostume)) == heads.end()) {
            PrintUserCmdText(
                iClientID,
                L"ERR Head not found. Use \"/show heads\" to get heads.");
            return;
        }
        restart.head = 1;
        restart.costume = heads[wstos(wscCostume)];
    } else if (ToLower(wscType) == L"body") {
        if (bodies.find(wstos(wscCostume)) == bodies.end()) {
            PrintUserCmdText(
                iClientID,
                L"ERR Body not found. Use \"/show bodies\" to get bodies.");
            return;
        }
        restart.head = 0;
        restart.costume = bodies[wstos(wscCostume)];
    } else {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        return;
    }

    // Saving the characters forces an anti-cheat checks and fixes
    // up a multitude of other problems.
    HkSaveChar(iClientID);
    if (!HkIsValidClientID(iClientID))
        return;

    // Check character is in base
    uint iBaseID;
    pub::Player::GetBase(iClientID, iBaseID);
    if (!iBaseID) {
        PrintUserCmdText(iClientID, L"ERR Not in base");
        return;
    }

    restart.wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);
    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    if (acc) {
        HkGetAccountDirName(acc, restart.wscDir);
        HkGetCharFileName(restart.wscCharname, restart.wscCharfile);
        pendingRestarts.push_back(restart);
        HkKickReason(
            restart.wscCharname,
            L"Updating character, please wait 10 seconds before reconnecting");
    }
}

void HkTimerCheckKick() {
    while (pendingRestarts.size()) {
        RESTART restart = pendingRestarts.front();
        if (HkGetClientIdFromCharname(restart.wscCharname) != -1)
            return;

        pendingRestarts.pop_front();

        try {
            // Overwrite the existing character file
            std::string scCharFile = scAcctPath + wstos(restart.wscDir) + "\\" +
                                     wstos(restart.wscCharfile) + ".fl";
            flc_decode(scCharFile.c_str(), scCharFile.c_str());
            if (restart.head) {
                IniWrite(scCharFile, "Player", "head", " " + restart.costume);
            } else
                IniWrite(scCharFile, "Player", "body", " " + restart.costume);

            if (!set_bDisableCharfileEncryption)
                flc_encode(scCharFile.c_str(), scCharFile.c_str());

            AddLog(Normal, L"NOTICE: User %s costume change to %u",
                   wstos(restart.wscCharname).c_str(), restart.costume);
        } catch (char *err) {
            AddLog(Normal, L"ERROR: User %s costume change to %u (%s)",
                   wstos(restart.wscCharname).c_str(), restart.costume, err);
        } catch (...) {
            AddLog(Normal, L"ERROR: User %s costume change to %u",
                   wstos(restart.wscCharname).c_str(), restart.costume);
        }
    }
}

void LoadSettings() {

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\wardrobe.cfg";

    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("Wardrobe")) {
                while (ini.read_value()) {
                    if (ini.is_value("head")) {
                        std::string name = ini.get_value_string(0);
                        std::string value = ini.get_value_string(1);
                        heads[name] = value;
                    } else if (ini.is_value("body")) {
                        std::string name = ini.get_value_string(0);
                        std::string value = ini.get_value_string(1);
                        bodies[name] = value;
                    }
                }
            }
        }
        ini.close();
    }
}

// Additional information related to the plugin when the /help command is used
void UserCmd_Help(uint& iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, L"/show");
    PrintUserCmdText(iClientID, L"Usage: /show <heads/bodies> - This shows the available heads and bodies for the /change command.");
    PrintUserCmdText(iClientID, L"/change");
    PrintUserCmdText(iClientID, L"Usage: /change <head/body> <name> - This changes Trent's head or body to one specified in the /show command.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMAND PROCESSING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define usable chat commands here
USERCMD UserCmds[] =
{
    {L"/show", UserCmd_ShowWardrobe },
    {L"/change", UserCmd_ChangeCostume },
};

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    // If we're being loaded from the command line while FLHook is running then
    // load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH)
        LoadSettings();

    return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Wardrobe Plugin");
    pi->shortName("wardrobe");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->versionMajor(PluginMajorVersion::VERSION_04);
    pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
}
