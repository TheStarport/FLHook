#include "global.h"
#include "hook.h"
#include <io.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setting variables

std::string set_scCfgFile;

// General
bool set_bLoadedSettings = false;
uint set_iAntiDockKill;
bool set_bDieMsg;
bool set_bDisableCharfileEncryption;
bool set_bChangeCruiseDisruptorBehaviour;
uint set_iAntiF1;
uint set_iDisconnectDelay;
uint set_iReservedSlots;
float set_fTorpMissileBaseDamageMultiplier;
uint set_iMaxGroupSize;
uint set_iDisableNPCSpawns;
bool set_bPersistGroup;

// log
bool set_bDebug;
uint set_iDebugMaxSize;
bool set_bLogConnects;
bool set_bLogAdminCmds;
bool set_bLogSocketCmds;
bool set_bLogLocalSocketCmds;
bool set_bLogUserCmds;
bool set_bPerfTimer;
uint set_iTimerThreshold;
uint set_iTimerDebugThreshold;

// Kick
uint set_iAntiBaseIdle;
uint set_iAntiCharMenuIdle;

// Style
std::wstring set_wscDeathMsgStyle;
std::wstring set_wscDeathMsgStyleSys;
std::wstring set_wscDeathMsgTextPlayerKill;
std::wstring set_wscDeathMsgTextSelfKill;
std::wstring set_wscDeathMsgTextNPC;
std::wstring set_wscDeathMsgTextSuicide;
std::wstring set_wscDeathMsgTextAdminKill;

uint set_iKickMsgPeriod;
std::wstring set_wscKickMsg;
std::wstring set_wscUserCmdStyle;
std::wstring set_wscAdminCmdStyle;

// Socket
bool set_bSocketActivated;
int set_iPort;
int set_iWPort;
int set_iEPort;
int set_iEWPort;
BLOWFISH_CTX *set_BF_CTX = 0;

// UserCommands
bool set_bUserCmdSetDieMsg;
bool set_bUserCmdSetDieMsgSize;
bool set_bUserCmdSetChatFont;
bool set_bUserCmdIgnore;
uint set_iUserCmdMaxIgnoreList;
bool set_bAutoBuy;
bool set_bUserCmdHelp;
bool set_bDefaultLocalChat;

// NoPVP
std::set<uint> set_setNoPVPSystems;

// Chat
std::set<std::wstring> set_setChatSuppress;

// MultiKillMessages
bool set_MKM_bActivated;
std::wstring set_MKM_wscStyle;
std::list<MULTIKILLMESSAGE> set_MKM_lstMessages;

// bans
bool set_bBanAccountOnMatch;
std::set<std::wstring> set_setBans;

// help

bool get_bUserCmdSetDieMsg(uint iClientID) { return set_bUserCmdSetDieMsg; }
bool get_bUserCmdSetDieMsgSize(uint iClientID) {
    return set_bUserCmdSetDieMsgSize;
}
bool get_bUserCmdSetChatFont(uint iClientID) { return set_bUserCmdSetChatFont; }
bool get_bUserCmdIgnore(uint iClientID) { return set_bUserCmdIgnore; }
bool get_bAutoBuy(uint iClientID) { return set_bAutoBuy; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings() {
    // init cfg filename
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    set_scCfgFile = szCurDir;

    // Use flhook.cfg if it is available. It is used in some installations (okay
    // just cannon's) to avoid FLErrorChecker whining retardly about ini entries
    // it does not understand.
    if (_access(std::string(set_scCfgFile + "\\FLHook.cfg").c_str(), 0) != -1)
        set_scCfgFile += "\\FLHook.cfg";
    else
        set_scCfgFile += "\\FLHook.ini";

    // General
    set_iAntiDockKill = IniGetI(set_scCfgFile, "General", "AntiDockKill", 0);
    set_bDieMsg = IniGetB(set_scCfgFile, "General", "EnableDieMsg", false);
    set_bDisableCharfileEncryption =
        IniGetB(set_scCfgFile, "General", "DisableCharfileEncryption", false);
    set_bChangeCruiseDisruptorBehaviour = IniGetB(
        set_scCfgFile, "General", "ChangeCruiseDisruptorBehaviour", false);
    set_iDisableNPCSpawns =
        IniGetI(set_scCfgFile, "General", "DisableNPCSpawns", 0);
    set_iAntiF1 = IniGetI(set_scCfgFile, "General", "AntiF1", 0);
    set_iDisconnectDelay =
        IniGetI(set_scCfgFile, "General", "DisconnectDelay", 0);
    set_iReservedSlots = IniGetI(set_scCfgFile, "General", "ReservedSlots", 0);
    set_fTorpMissileBaseDamageMultiplier = IniGetF(
        set_scCfgFile, "General", "TorpMissileBaseDamageMultiplier", 1.0f);
    set_iMaxGroupSize = IniGetI(set_scCfgFile, "General", "MaxGroupSize", 8);
    set_bPersistGroup = IniGetB(set_scCfgFile, "General", "PersistGroup", false);

    // Log
    set_bDebug = IniGetB(set_scCfgFile, "Log", "Debug", false);
    set_iDebugMaxSize = IniGetI(set_scCfgFile, "Log", "DebugMaxSize", 100);
    set_iDebugMaxSize *= 1000;
    set_bLogConnects = IniGetB(set_scCfgFile, "Log", "LogConnects", false);
    set_bLogAdminCmds =
        IniGetB(set_scCfgFile, "Log", "LogAdminCommands", false);
    set_bLogSocketCmds =
        IniGetB(set_scCfgFile, "Log", "LogSocketCommands", false);
    set_bLogLocalSocketCmds =
        IniGetB(set_scCfgFile, "Log", "LogLocalSocketCommands", false);
    set_bLogUserCmds = IniGetB(set_scCfgFile, "Log", "LogUserCommands", false);
    set_bPerfTimer =
        IniGetB(set_scCfgFile, "Log", "LogPerformanceTimers", false);
    set_iTimerThreshold = IniGetI(set_scCfgFile, "Log", "TimerThreshold", 100);
    set_iTimerDebugThreshold =
        IniGetI(set_scCfgFile, "Log", "TimerDebugThreshold", 0);

    // Kick
    set_iAntiBaseIdle = IniGetI(set_scCfgFile, "Kick", "AntiBaseIdle", 0);
    set_iAntiCharMenuIdle =
        IniGetI(set_scCfgFile, "Kick", "AntiCharMenuIdle", 0);

    // Style
    set_wscDeathMsgStyle =
        stows(IniGetS(set_scCfgFile, "Style", "DeathMsgStyle", "0x19198C01"));
    set_wscDeathMsgStyleSys = stows(
        IniGetS(set_scCfgFile, "Style", "DeathMsgStyleSys", "0x1919BD01"));
    set_wscDeathMsgTextPlayerKill =
        stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextPlayerKill",
                      "Death: %victim was killed by %killer (%type)"));
    set_wscDeathMsgTextSelfKill =
        stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextSelfKill",
                      "Death: %victim killed himself (%type)"));
    set_wscDeathMsgTextNPC =
        stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextNPC",
                      "Death: %victim was killed by an NPC"));
    set_wscDeathMsgTextSuicide =
        stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextSuicide",
                      "Death: %victim committed suicide"));
    set_wscDeathMsgTextAdminKill =
        stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextAdminKill",
                      "Death: %victim was killed by an admin"));

    set_wscKickMsg =
        stows(IniGetS(set_scCfgFile, "Style", "KickMsg",
                      "<TRA data=\"0x0000FF10\" mask=\"-1\"/><TEXT>You will be "
                      "kicked. Reason: %s</TEXT>"));
    set_iKickMsgPeriod = IniGetI(set_scCfgFile, "Style", "KickMsgPeriod", 5000);
    set_wscUserCmdStyle =
        stows(IniGetS(set_scCfgFile, "Style", "UserCmdStyle", "0x00FF0090"));
    set_wscAdminCmdStyle =
        stows(IniGetS(set_scCfgFile, "Style", "AdminCmdStyle", "0x00FF0090"));

    // Socket
    set_bSocketActivated = IniGetB(set_scCfgFile, "Socket", "Activated", false);
    set_iPort = IniGetI(set_scCfgFile, "Socket", "Port", 0);
    set_iWPort = IniGetI(set_scCfgFile, "Socket", "WPort", 0);
    set_iEPort = IniGetI(set_scCfgFile, "Socket", "EPort", 0);
    set_iEWPort = IniGetI(set_scCfgFile, "Socket", "EWPort", 0);
    std::string scEncryptKey = IniGetS(set_scCfgFile, "Socket", "Key", "");
    if (scEncryptKey.length()) {
        if (!set_BF_CTX)
            set_BF_CTX = (BLOWFISH_CTX *)malloc(sizeof(BLOWFISH_CTX));
        Blowfish_Init(set_BF_CTX, (unsigned char *)scEncryptKey.data(),
                      (int)scEncryptKey.length());
    }

    // UserCommands
    set_bUserCmdSetDieMsg =
        IniGetB(set_scCfgFile, "UserCommands", "SetDieMsg", false);
    set_bUserCmdSetDieMsgSize =
        IniGetB(set_scCfgFile, "UserCommands", "SetDieMsgSize", false);
    set_bUserCmdSetChatFont =
        IniGetB(set_scCfgFile, "UserCommands", "SetChatFont", false);
    set_bUserCmdIgnore =
        IniGetB(set_scCfgFile, "UserCommands", "Ignore", false);
    set_iUserCmdMaxIgnoreList =
        IniGetI(set_scCfgFile, "UserCommands", "MaxIgnoreListEntries", 30);
    set_bAutoBuy = IniGetB(set_scCfgFile, "UserCommands", "AutoBuy", false);
    set_bUserCmdHelp = IniGetB(set_scCfgFile, "UserCommands", "Help", false);
    set_bDefaultLocalChat =
        IniGetB(set_scCfgFile, "UserCommands", "DefaultLocalChat", false);

    // NoPVP
    set_setNoPVPSystems.clear();
    for (uint i = 0;; i++) {
        char szBuf[64];
        sprintf_s(szBuf, "System%u", i);
        std::string scSystem = IniGetS(set_scCfgFile, "NoPVP", szBuf, "");

        if (!scSystem.length())
            break;

        uint iSystemID;
        pub::GetSystemID(iSystemID, scSystem.c_str());
        set_setNoPVPSystems.insert(iSystemID);
    }

    // read chat suppress
    set_setChatSuppress.clear();
    for (uint i = 0;; i++) {
        char szBuf[64];
        sprintf_s(szBuf, "Suppress%u", i);
        std::string scSuppress = IniGetS(set_scCfgFile, "Chat", szBuf, "");

        if (!scSuppress.length())
            break;

        set_setChatSuppress.insert(stows(scSuppress));
    }

    // MultiKillMessages
    set_MKM_bActivated =
        IniGetB(set_scCfgFile, "MultiKillMessages", "Activated", false);
    set_MKM_wscStyle = stows(
        IniGetS(set_scCfgFile, "MultiKillMessages", "Style", "0x1919BD01"));

    set_MKM_lstMessages.clear();
    std::list<INISECTIONVALUE> lstValues;
    IniGetSection(set_scCfgFile, "MultiKillMessages", lstValues);
    for (auto &val : lstValues) {
        if (!atoi(val.scKey.c_str()))
            continue;

        MULTIKILLMESSAGE mkm;
        mkm.iKillsInARow = atoi(val.scKey.c_str());
        mkm.wscMessage = stows(val.scValue);
        set_MKM_lstMessages.push_back(mkm);
    }

    // bans
    set_bBanAccountOnMatch =
        IniGetB(set_scCfgFile, "Bans", "BanAccountOnMatch", false);
    set_setBans.clear();
    IniGetSection(set_scCfgFile, "Bans", lstValues);
    if (!lstValues.empty()) {
        lstValues.pop_front();
        for (auto &val : lstValues)
            set_setBans.insert(stows(val.scKey));
    }

    // help
    HkAddHelpEntry(
        L"/set diemsg", L"<visibility>",
        L"Sets your death message's visibility. Options: all, system, "
        L"self, none.",
        L"", get_bUserCmdSetDieMsg);
    HkAddHelpEntry(
        L"/set diemsgsize", L"<size>",
        L"Sets your death message's text size. Options: small, default.", L"",
        get_bUserCmdSetDieMsgSize);
    HkAddHelpEntry(
        L"/set chatfont", L"<size> <style>",
        L"Sets your chat messages' font. Options are small, default or big for "
        L"<size> and default, bold, italic or underline for <style>.",
        L"", get_bUserCmdSetChatFont);
    HkAddHelpEntry(
        L"/ignore", L"<charname> [<flags>]",
        L"Ignores all messages from the given character.",
        L"The possible flags are:\n p - only affect private chat\n i - "
        L"<charname> may match partially\nExamples:\n\"/ignore SomeDude\" "
        L"ignores all chatmessages from SomeDude\n\"/ignore PlayerX p\" "
        L"ignores "
        L"all private-chatmessages from PlayerX\n\"/ignore idiot i\" ignores "
        L"all "
        L"chatmessages from players whose charname contain \"idiot\" (e.g. "
        L"\"[XYZ]IDIOT\", \"MrIdiot\", etc)\n\"/ignore Fool pi\" ignores all "
        L"private-chatmessages from players whose charname contain \"fool\"",
        get_bUserCmdIgnore);
    HkAddHelpEntry(L"/ignoreid", L"<client-id> [<flags>]",
                   L"Ignores all messages from the character with the "
                   L"associated client ID "
                   L"(see /id). Use the p flag to only affect private chat.",
                   L"", get_bUserCmdIgnore);
    HkAddHelpEntry(L"/ignorelist", L"",
                   L"Displays all currently ignored characters.", L"",
                   get_bUserCmdIgnore);
    HkAddHelpEntry(L"/delignore", L"<id> [<id2> <id3> ...]",
                   L"Removes the characters with the associated ignore ID (see "
                   L"/ignorelist) from the ignore list. * deletes all.",
                   L"", get_bUserCmdIgnore);
    HkAddHelpEntry(
        L"/autobuy", L"<param> [<on/off>]",
        L"Auomatically buys the given elements upon docking. See detailed help "
        L"for more information.",
        L"<param> can take one of the following values:\tinfo - display "
        L"current "
        L"autobuy-settings\n\tmissiles - enable/disable autobuy for "
        L"missiles\n\ttorps - enable/disable autobuy for torpedos\n\tmines - "
        L"enable/disable autobuy for mines\n\tcd - enable/disable autobuy for "
        L"cruise disruptors\n\tcm - enable/disable autobuy for "
        L"countermeasures\n\treload - enable/disable autobuy for "
        L"nanobots/shield "
        L"batteries\n\tall - enable/disable autobuy for all of the "
        L"above\nExamples:\n\"/autobuy missiles on\" enable autobuy for "
        L"missiles\n\"/autobuy all off\" completely disable "
        L"autobuy\n\"/autobuy "
        L"info\" show autobuy info",
        get_bAutoBuy);
    HkAddHelpEntry(L"/ids", L"",
                   L"Lists all characters with their respective client IDs.",
                   L"", get_bTrue);
    HkAddHelpEntry(L"/id", L"", L"Gives your own client ID.", L"", get_bTrue);
    HkAddHelpEntry(L"/i$", L"<client-id>", L"Invites the given client ID.", L"",
                   get_bTrue);
    HkAddHelpEntry(L"/invite$", L"<client-id>", L"Invites the given client ID.",
                   L"", get_bTrue);
    HkAddHelpEntry(L"/credits", L"", L"Displays FLHook's credits.", L"",
                   get_bTrue);
    HkAddHelpEntry(
        L"/help", L"[<command>]",
        L"Displays the help screen. Giving a <command> gives detailed "
        L"info for that command.",
        L"", get_bTrue);

    set_bLoadedSettings = true;
}
