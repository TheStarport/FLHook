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

public:
	DWORD rights;
	HK_ERROR hkLastErr;

	EXPORT void PrintError();

// commands
	void CmdGetCash(const wstring &wscCharname);
	void CmdSetCash(const wstring &wscCharname, int iAmount);
	void CmdSetCashSec(const wstring &wscCharname, int iAmountCheck, int iAmount);
	void CmdAddCash(const wstring &wscCharname, int iAmount);
	void CmdAddCashSec(const wstring &wscCharname, int iAmountCheck, int iAmount);

	void CmdKick(const wstring &wscCharname, const wstring &wscReason);
	void CmdBan(const wstring &wscCharname);
	void CmdUnban(const wstring &wscCharname);
	void CmdKickBan(const wstring &wscCharname, const wstring &wscReason);

	void CmdBeam(const wstring &wscCharname, const wstring &wscBasename);
	void CmdKill(const wstring &wscCharname);
	void CmdResetRep(const wstring &wscCharname);
	void CmdSetRep(const wstring &wscCharname, const wstring &wscRepGroup, float fValue);
	void CmdGetRep(const wstring &wscCharname, const wstring &wscRepGroup);

	void CmdMsg(const wstring &wscCharname, const wstring &wscText);
	void CmdMsgS(const wstring &wscSystemname, const wstring &wscText);
	void CmdMsgU(const wstring &wscText);
	void CmdFMsg(const wstring &wscCharname, const wstring &wscXML);
	void CmdFMsgS(const wstring &wscSystemname, const wstring &wscXML);
	void CmdFMsgU(const wstring &wscXML);

	void CmdEnumCargo(const wstring &wscCharname);
	void CmdRemoveCargo(const wstring &wscCharname, uint iID, uint iCount);
	void CmdAddCargo(const wstring &wscCharname, const wstring &wscGood, uint iCount, uint iMission);

	void CmdRename(const wstring &wscCharname, const wstring &wscNewCharname);
	void CmdDeleteChar(const wstring &wscCharname);

	void CmdReadCharFile(const wstring &wscCharname);
	void CmdWriteCharFile(const wstring &wscCharname, const wstring &wscData);

	void CmdGetBaseStatus(const wstring &wscBasename);
	void CmdGetClientId(const wstring &wscCharname);
	void PrintPlayerInfo(HKPLAYERINFO pi);
	void CmdGetPlayerInfo(const wstring &wscCharname);
	void CmdGetPlayers();
	void XPrintPlayerInfo(HKPLAYERINFO pi);
	void CmdXGetPlayerInfo(const wstring &wscCharname);
	void CmdXGetPlayers();
	void CmdGetPlayerIDs();
	void CmdHelp();
	void CmdGetAccountDirName(const wstring &wscCharname);
	void CmdGetCharFileName(const wstring &wscCharname);
	void CmdIsOnServer(const wstring &wscCharname);
	void CmdIsLoggedIn(const wstring &wscCharname);
	void CmdMoneyFixList();
	void CmdServerInfo();
	void CmdGetGroupMembers(const wstring &wscCharname);

	void CmdSaveChar(const wstring &wscCharname);

	void CmdGetReservedSlot(const wstring &wscCharname);
	void CmdSetReservedSlot(const wstring &wscCharname, int iReservedSlot);
	void CmdSetAdmin(const wstring &wscCharname, const wstring &wscRights);
	void CmdGetAdmin(const wstring &wscCharname);
	void CmdDelAdmin(const wstring &wscCharname);
	void CmdRehash();
	void CmdUnload(const wstring &wscParam);

	void CmdLoadPlugins();
	void CmdListPlugins();
	void CmdUnloadPlugin(const wstring &wscPlugin);
	void CmdPausePlugin(const wstring &wscPlugin);
	void CmdUnpausePlugin(const wstring &wscPlugin);

	void CmdTest(int iArg, int iArg2, int iArg3);
//
	EXPORT wstring ArgCharname(uint iArg);
	EXPORT int ArgInt(uint iArg);
	EXPORT float ArgFloat(uint iArg);
	EXPORT wstring ArgStr(uint iArg);
	EXPORT wstring ArgStrToEnd(uint iArg);
	void ExecuteCommandString(const wstring &wscCmd);

	void SetRightsByString(const string &scRightStr);
	EXPORT void Print(wstring wscText, ...);
	virtual void DoPrint(const wstring &wscText) = 0;
	EXPORT virtual wstring GetAdminName() { return L""; };

	wstring wscCurCmdString;
};

#endif