// Wardrobe
// By Raikkonen

#include <FLHook.h>
#include "Main.h"

namespace Wardrobe {
std::map<std::string, std::string> heads;
std::map<std::string, std::string> bodies;

struct RESTART {
    std::wstring wscCharname;
    std::wstring wscDir;
    std::wstring wscCharfile;
    std::string costume;
    bool head; // 1 Head, 0 Body
};

std::list<RESTART> pendingRestarts;

bool Wardrobe::UserCmd_ShowWardrobe(uint iClientID, const std::wstring &wscCmd,
                                    const std::wstring &wscParam,
                                    const wchar_t *usage) {
    if (!set_bEnableWardrobe)
        return false;

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
    } else {
        PrintUserCmdText(iClientID, usage);
    }
    return true;
}

bool Wardrobe::UserCmd_ChangeCostume(uint iClientID, const std::wstring &wscCmd,
                                     const std::wstring &wscParam,
                                     const wchar_t *usage) {
    if (!set_bEnableWardrobe)
        return false;

    std::wstring wscType = GetParam(wscParam, ' ', 0);
    std::wstring wscCostume = GetParam(wscParam, ' ', 1);

    if (!wscType.length() || !wscCostume.length()) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, usage);
        return true;
    }

    RESTART restart;

    if (ToLower(wscType) == L"head") {
        if (heads.find(wstos(wscCostume)) == heads.end()) {
            PrintUserCmdText(
                iClientID,
                L"ERR Head not found. Use \"/show heads\" to get heads.");
            return true;
        }
        restart.head = 1;
        restart.costume = heads[wstos(wscCostume)];
    } else if (ToLower(wscType) == L"body") {
        if (bodies.find(wstos(wscCostume)) == bodies.end()) {
            PrintUserCmdText(
                iClientID,
                L"ERR Body not found. Use \"/show bodies\" to get bodies.");
            return true;
        }
        restart.head = 0;
        restart.costume = bodies[wstos(wscCostume)];
    } else {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, usage);
        return true;
    }

    // Saving the characters forces an anti-cheat checks and fixes
    // up a multitude of other problems.
    HkSaveChar(iClientID);
    if (!HkIsValidClientID(iClientID))
        return true;

    // Check character is in base
    uint iBaseID;
    pub::Player::GetBase(iClientID, iBaseID);
    if (!iBaseID) {
        PrintUserCmdText(iClientID, L"ERR Not in base");
        return true;
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
            flc_decode(scCharFile.c_str(), scCharFile.c_str());
            if (restart.head) {
                IniWrite(scCharFile, "Player", "head", " " + restart.costume);
            } else
                IniWrite(scCharFile, "Player", "body", " " + restart.costume);

            if (!set_bDisableCharfileEncryption)
                flc_encode(scCharFile.c_str(), scCharFile.c_str());

            AddLog("NOTICE: User %s costume change to %u",
                   wstos(restart.wscCharname).c_str(), restart.costume);
        } catch (char *err) {
            AddLog("ERROR: User %s costume change to %u (%s)",
                   wstos(restart.wscCharname).c_str(), restart.costume, err);
        } catch (...) {
            AddLog("ERROR: User %s costume change to %u",
                   wstos(restart.wscCharname).c_str(), restart.costume);
        }
    }
}

void LoadSettings(const std::string &scPluginCfgFile) {
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
} // namespace Wardrobe