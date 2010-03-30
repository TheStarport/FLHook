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
	void CmdGetCash(wstring wscCharname);
	void CmdSetCash(wstring wscCharname, int iAmount);
	void CmdSetCashSec(wstring wscCharname, int iAmountCheck, int iAmount);
	void CmdAddCash(wstring wscCharname, int iAmount);
	void CmdAddCashSec(wstring wscCharname, int iAmountCheck, int iAmount);

	void CmdKick(wstring wscCharname, wstring wscReason);
	void CmdBan(wstring wscCharname);
	void CmdUnban(wstring wscCharname);
	void CmdKickBan(wstring wscCharname, wstring wscReason);

	void CmdBeam(wstring wscCharname, wstring wscBasename);
	void CmdKill(wstring wscCharname);
	void CmdResetRep(wstring wscCharname);
	void CmdSetRep(wstring wscCharname, wstring wscRepGroup, float fValue);
	void CmdGetRep(wstring wscCharname, wstring wscRepGroup);

	void CmdMsg(wstring wscCharname, wstring wscText);
	void CmdMsgS(wstring wscSystemname, wstring wscText);
	void CmdMsgU(wstring wscText);
	void CmdFMsg(wstring wscCharname, wstring wscXML);
	void CmdFMsgS(wstring wscSystemname, wstring wscXML);
	void CmdFMsgU(wstring wscXML);

	void CmdEnumCargo(wstring wscCharname);
	void CmdRemoveCargo(wstring wscCharname, uint iID, uint iCount);
	void CmdAddCargo(wstring wscCharname, wstring wscGood, uint iCount, uint iMission);

	void CmdRename(wstring wscCharname, wstring wscNewCharname);
	void CmdDeleteChar(wstring wscCharname);

	void CmdReadCharFile(wstring wscCharname);
	void CmdWriteCharFile(wstring wscCharname, wstring wscData);

	void CmdGetBaseStatus(wstring wscBasename);
	void CmdGetClientId(wstring wscCharname);
	void PrintPlayerInfo(HKPLAYERINFO pi);
	void CmdGetPlayerInfo(wstring wscCharname);
	void CmdGetPlayers();
	void XPrintPlayerInfo(HKPLAYERINFO pi);
	void CmdXGetPlayerInfo(wstring wscCharname);
	void CmdXGetPlayers();
	void CmdGetPlayerIDs();
	void CmdHelp();
	void CmdGetAccountDirName(wstring wscCharname);
	void CmdGetCharFileName(wstring wscCharname);
	void CmdIsOnServer(wstring wscCharname);
	void CmdIsLoggedIn(wstring wscCharname);
	void CmdMoneyFixList();
	void CmdServerInfo();
	void CmdGetGroupMembers(wstring wscCharname);

	void CmdSaveChar(wstring wscCharname);

	void CmdGetReservedSlot(wstring wscCharname);
	void CmdSetReservedSlot(wstring wscCharname, int iReservedSlot);
	void CmdSetAdmin(wstring wscCharname, wstring wscRights);
	void CmdGetAdmin(wstring wscCharname);
	void CmdDelAdmin(wstring wscCharname);
	void CmdRehash();
	void CmdUnload(wstring wscParam);

	void CmdLoadPlugins();
	void CmdListPlugins();
	void CmdUnloadPlugin(wstring wscPlugin);
	void CmdPausePlugin(wstring wscPlugin);
	void CmdUnpausePlugin(wstring wscPlugin);

	void CmdTest(int iArg, int iArg2, int iArg3);
//
	EXPORT wstring ArgCharname(uint iArg);
	EXPORT int ArgInt(uint iArg);
	EXPORT float ArgFloat(uint iArg);
	EXPORT wstring ArgStr(uint iArg);
	EXPORT wstring ArgStrToEnd(uint iArg);
	void ExecuteCommandString(wstring wscCmd);

	void SetRightsByString(string scRightStr);
	EXPORT void Print(wstring wscText, ...);
	virtual void DoPrint(wstring wscText) {};
	EXPORT virtual wstring GetAdminName() { return L""; };

	wstring wscCurCmdString;
};

#endif