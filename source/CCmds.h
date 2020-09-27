#ifndef _CCMDS_
#define _CCMDS_

#include "hook.h"

// enums
enum CCMDS_RIGHTS
{
	RIGHT_NOTHING		= 0,
	RIGHT_SUPERADMIN	= 0xFFFFFFFF,
	RIGHT_MSG			= (1 << 0),
	RIGHT_KICKBAN		= (1 << 1),
	RIGHT_EVENTMODE		= (1 << 2),
	RIGHT_CASH			= (1 << 3),
	RIGHT_BEAMKILL		= (1 << 4),
	RIGHT_REPUTATION	= (1 << 5),
	RIGHT_CARGO			= (1 << 6),
	RIGHT_CHARACTERS	= (1 << 7),
	RIGHT_SETTINGS		= (1 << 8),
	RIGHT_PLUGINS		= (1 << 9),
	RIGHT_OTHER			= (1 << 10),
	RIGHT_SPECIAL1		= (1 << 11),
	RIGHT_SPECIAL2		= (1 << 12),
	RIGHT_SPECIAL3		= (1 << 13),
};

// class
class CCmds
{
	bool bID;
	bool bShortCut;
	bool bSelf;
	bool bTarget;

public:
	DWORD rights;
	HK_ERROR hkLastErr;

	EXPORT void PrintError();

// commands
	void CmdGetCash(const std::wstring &wscCharname);
	void CmdSetCash(const std::wstring &wscCharname, int iAmount);
	void CmdSetCashSec(const std::wstring &wscCharname, int iAmountCheck, int iAmount);
	void CmdAddCash(const std::wstring &wscCharname, int iAmount);
	void CmdAddCashSec(const std::wstring &wscCharname, int iAmountCheck, int iAmount);

	void CmdKick(const std::wstring &wscCharname, const std::wstring &wscReason);
	void CmdBan(const std::wstring &wscCharname);
	void CmdUnban(const std::wstring &wscCharname);
	void CmdKickBan(const std::wstring &wscCharname, const std::wstring &wscReason);

	void CmdBeam(const std::wstring &wscCharname, const std::wstring &wscBasename);
	void CmdKill(const std::wstring &wscCharname);
	void CmdResetRep(const std::wstring &wscCharname);
	void CmdSetRep(const std::wstring &wscCharname, const std::wstring &wscRepGroup, float fValue);
	void CmdGetRep(const std::wstring &wscCharname, const std::wstring &wscRepGroup);

	void CmdMsg(const std::wstring &wscCharname, const std::wstring &wscText);
	void CmdMsgS(const std::wstring &wscSystemname, const std::wstring &wscText);
	void CmdMsgU(const std::wstring &wscText);
	void CmdFMsg(const std::wstring &wscCharname, const std::wstring &wscXML);
	void CmdFMsgS(const std::wstring &wscSystemname, const std::wstring &wscXML);
	void CmdFMsgU(const std::wstring &wscXML);

	void CmdEnumCargo(const std::wstring &wscCharname);
	void CmdRemoveCargo(const std::wstring &wscCharname, uint iID, uint iCount);
	void CmdAddCargo(const std::wstring &wscCharname, const std::wstring &wscGood, uint iCount, uint iMission);

	void CmdRename(const std::wstring &wscCharname, const std::wstring &wscNewCharname);
	void CmdDeleteChar(const std::wstring &wscCharname);

	void CmdReadCharFile(const std::wstring &wscCharname);
	void CmdWriteCharFile(const std::wstring &wscCharname, const std::wstring &wscData);

	void CmdGetBaseStatus(const std::wstring &wscBasename);
	void CmdGetClientId(const std::wstring &wscCharname);
	void PrintPlayerInfo(HKPLAYERINFO pi);
	void CmdGetPlayerInfo(const std::wstring &wscCharname);
	void CmdGetPlayers();
	void XPrintPlayerInfo(HKPLAYERINFO pi);
	void CmdXGetPlayerInfo(const std::wstring &wscCharname);
	void CmdXGetPlayers();
	void CmdGetPlayerIDs();
	void CmdHelp();
	void CmdGetAccountDirName(const std::wstring &wscCharname);
	void CmdGetCharFileName(const std::wstring &wscCharname);
	void CmdIsOnServer(const std::wstring &wscCharname);
	void CmdIsLoggedIn(const std::wstring &wscCharname);
	void CmdMoneyFixList();
	void CmdServerInfo();
	void CmdGetGroupMembers(const std::wstring &wscCharname);

	void CmdSaveChar(const std::wstring &wscCharname);

	void CmdGetReservedSlot(const std::wstring &wscCharname);
	void CmdSetReservedSlot(const std::wstring &wscCharname, int iReservedSlot);
	void CmdSetAdmin(const std::wstring &wscCharname, const std::wstring &wscRights);
	void CmdGetAdmin(const std::wstring &wscCharname);
	void CmdDelAdmin(const std::wstring &wscCharname);
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