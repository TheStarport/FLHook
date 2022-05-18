#ifndef _CCMDS_
#define _CCMDS_

#include "hook.h"

// enums
enum CCMDS_RIGHTS {
    RIGHT_NOTHING = 0,
    RIGHT_SUPERADMIN = 0xFFFFFFFF,
    RIGHT_MSG = (1 << 0),
    RIGHT_KICKBAN = (1 << 1),
    RIGHT_EVENTMODE = (1 << 2),
    RIGHT_CASH = (1 << 3),
    RIGHT_BEAMKILL = (1 << 4),
    RIGHT_REPUTATION = (1 << 5),
    RIGHT_CARGO = (1 << 6),
    RIGHT_CHARACTERS = (1 << 7),
    RIGHT_SETTINGS = (1 << 8),
    RIGHT_PLUGINS = (1 << 9),
    RIGHT_OTHER = (1 << 10),
    RIGHT_SPECIAL1 = (1 << 11),
    RIGHT_SPECIAL2 = (1 << 12),
    RIGHT_SPECIAL3 = (1 << 13),
};

// class
class CCmds {
    bool bID;
    bool bShortCut;
    bool bSelf;
    bool bTarget;

  public:
    DWORD rights;
    HK_ERROR hkLastErr;

    EXPORT void PrintError();

    // commands
    void CmdGetCash(std::variant<uint, std::wstring> player);
    void CmdSetCash(std::variant<uint, std::wstring> player, int iAmount);
    void CmdSetCashSec(std::variant<uint, std::wstring> player, int iAmountCheck,
                       int iAmount);
    void CmdAddCash(std::variant<uint, std::wstring> player, int iAmount);
    void CmdAddCashSec(std::variant<uint, std::wstring> player, int iAmountCheck,
                       int iAmount);

    void CmdKick(std::variant<uint, std::wstring> player,
                 const std::wstring &wscReason);
    void CmdBan(std::variant<uint, std::wstring> player);
    void CmdUnban(std::variant<uint, std::wstring> player);
    void CmdKickBan(std::variant<uint, std::wstring> player,
                    const std::wstring &wscReason);

    void CmdBeam(std::variant<uint, std::wstring> player,
                 const std::wstring &wscBasename);
    void CmdChase(std::wstring wscAdminName, std::variant<uint, std::wstring> player);
    void CmdPull(std::wstring wscAdminName, std::variant<uint, std::wstring> player);
    void CmdMove(std::wstring wscAdminName, float x, float y, float z);
    void CmdKill(std::variant<uint, std::wstring> player);
    void CmdResetRep(std::variant<uint, std::wstring> player);
    void CmdSetRep(std::variant<uint, std::wstring> player,
                   const std::wstring &wscRepGroup, float fValue);
    void CmdGetRep(std::variant<uint, std::wstring> player,
                   const std::wstring &wscRepGroup);

    void CmdMsg(std::variant<uint, std::wstring> player, const std::wstring &wscText);
    void CmdMsgS(const std::wstring &wscSystemname,
                 const std::wstring &wscText);
    void CmdMsgU(const std::wstring &wscText);
    void CmdFMsg(std::variant<uint, std::wstring> player, const std::wstring &wscXML);
    void CmdFMsgS(const std::wstring &wscSystemname,
                  const std::wstring &wscXML);
    void CmdFMsgU(const std::wstring &wscXML);

    void CmdEnumCargo(std::variant<uint, std::wstring> player);
    void CmdRemoveCargo(std::variant<uint, std::wstring> player, uint iID, uint iCount);
    void CmdAddCargo(std::variant<uint, std::wstring> player,
                     const std::wstring &wscGood, uint iCount, uint iMission);

    void CmdRename(std::variant<uint, std::wstring> player, const std::wstring &wscNewCharname);
    void CmdDeleteChar(std::variant<uint, std::wstring> player);

    void CmdReadCharFile(std::variant<uint, std::wstring> player);
    void CmdWriteCharFile(std::variant<uint, std::wstring> player,
                          const std::wstring &wscData);

    void CmdGetBaseStatus(const std::wstring &wscBasename);
    void CmdGetClientId(std::wstring player);
    void PrintPlayerInfo(HKPLAYERINFO pi);
    void CmdGetPlayerInfo(std::variant<uint, std::wstring> player);
    void CmdGetPlayers();
    void XPrintPlayerInfo(HKPLAYERINFO pi);
    void CmdXGetPlayerInfo(std::variant<uint, std::wstring> player);
    void CmdXGetPlayers();
    void CmdGetPlayerIDs();
    void CmdHelp();
    void CmdGetAccountDirName(std::variant<uint, std::wstring> player);
    void CmdGetCharFileName(std::variant<uint, std::wstring> player);
    void CmdIsOnServer(std::wstring player);
    void CmdIsLoggedIn(std::wstring player);
    void CmdMoneyFixList();
    void CmdServerInfo();
    void CmdGetGroupMembers(std::variant<uint, std::wstring> player);

    void CmdSaveChar(std::variant<uint, std::wstring> player);

    void CmdGetReservedSlot(std::variant<uint, std::wstring> player);
    void CmdSetReservedSlot(std::variant<uint, std::wstring> player, int iReservedSlot);
    void CmdSetAdmin(std::variant<uint, std::wstring> player,
                     const std::wstring &wscRights);
    void CmdGetAdmin(std::variant<uint, std::wstring> player);
    void CmdDelAdmin(std::variant<uint, std::wstring> player);
    void CmdRehash();
    void CmdUnload(const std::wstring &wscParam);

    void CmdLoadPlugins();
    void CmdLoadPlugin(const std::wstring &wscPlugin);
    void CmdListPlugins();
    void CmdUnloadPlugin(const std::wstring &wscPlugin);
    void CmdPausePlugin(const std::wstring &wscPlugin);
    void CmdUnpausePlugin(const std::wstring &wscPlugin);

    EXPORT std::wstring ArgCharname(uint iArg);
    EXPORT int ArgInt(uint iArg);
    EXPORT uint ArgUInt(uint iArg);
    EXPORT float ArgFloat(uint iArg);
    EXPORT std::wstring ArgStr(uint iArg);
    EXPORT std::wstring ArgStrToEnd(uint iArg);
    void ExecuteCommandString(const std::wstring &wscCmd);

    void SetRightsByString(const std::string &scRightStr);
    EXPORT void Print(std::wstring wscText, ...);
    virtual void DoPrint(const std::wstring &wscText) = 0;
    EXPORT virtual std::wstring GetAdminName() { return L""; };

    std::wstring wscCurCmdString;
};

#endif
