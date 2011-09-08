#include "global.h"
#include "CCmds.h"

#define RIGHT_CHECK(a) if(!(this->rights & a)) { Print(L"ERR No permission\n"); return; }
#define RIGHT_CHECK_SUPERADMIN() if(!(this->rights == RIGHT_SUPERADMIN)) { Print(L"ERR No permission\n"); return; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCash(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;
	if(HKSUCCESS(HkGetCash(wscCharname, iCash)))
		Print(L"cash=%d\nOK\n", iCash);
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetCash(const wstring &wscCharname, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;
	if(HKSUCCESS(HkGetCash(wscCharname, iCash))) {
		HkAddCash(wscCharname, iAmount - iCash);
		CmdGetCash(wscCharname);
	} else
		PrintError();
}

void CCmds::CmdSetCashSec(const wstring &wscCharname, int iAmountCheck, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;

	if(HKSUCCESS(HkGetCash(wscCharname, iCash))) {
		if(iCash != iAmountCheck)
			Print(L"ERR Security check failed\n");
		else 
			CmdSetCash(wscCharname, iAmount);
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCash(const wstring &wscCharname, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	if(HKSUCCESS(HkAddCash(wscCharname, iAmount)))
		CmdGetCash(wscCharname);
	else
		PrintError();
}

void CCmds::CmdAddCashSec(const wstring &wscCharname, int iAmountCheck, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	int iCash;

	if(HKSUCCESS(HkGetCash(wscCharname, iCash))) {
		if(iCash != iAmountCheck)
			Print(L"ERR Security check failed\n");
		else 
			CmdAddCash(wscCharname, iAmount);
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKick(const wstring &wscCharname, const wstring &wscReason)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkKickReason(wscCharname, wscReason)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdBan(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkBan(wscCharname, true)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnban(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(HKSUCCESS(HkBan(wscCharname, false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKickBan(const wstring &wscCharname, const wstring &wscReason)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if(!HKSUCCESS(HkBan(wscCharname, true)))
	{
		PrintError();
		return;
	}

	if(!HKSUCCESS(HkKickReason(wscCharname, wscReason)))
	{
		PrintError();
		return;
	}

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetBaseStatus(const wstring &wscBasename)
{

	RIGHT_CHECK(RIGHT_OTHER);

	float fHealth;
	float fMaxHealth;

	if(HKSUCCESS(HkGetBaseStatus(wscBasename, fHealth, fMaxHealth)))
		Print(L"hitpts=%u hitptsmax=%u\nOK\n", (long)fHealth, (long)fMaxHealth);
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetClientId(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	uint iClientID = HkGetClientIdFromCharname(wscCharname);
	if(iClientID == -1)
	{
		hkLastErr = HKE_PLAYER_NOT_LOGGED_IN;
		PrintError();
		return;
	}

	Print(L"clientid=%u\nOK\n", iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdBeam(const wstring &wscCharname, const wstring &wscBasename)
{
	RIGHT_CHECK(RIGHT_BEAMKILL);

	try {
		if(HKSUCCESS(HkBeam(wscCharname, wscBasename)))
			Print(L"OK\n");
		else
			PrintError();
	} catch(...) { // exeption, kick player
		HkKick(wscCharname);
		Print(L"ERR exception occured, player kicked\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKill(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_BEAMKILL);

	if(HKSUCCESS(HkKill(wscCharname)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdResetRep(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_REPUTATION);

	if(HKSUCCESS(HkResetRep(wscCharname)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetRep(const wstring &wscCharname, const wstring &wscRepGroup, float fValue)
{
	RIGHT_CHECK(RIGHT_REPUTATION);

	if(HKSUCCESS(HkSetRep(wscCharname, wscRepGroup, fValue)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetRep(const wstring &wscCharname, const wstring &wscRepGroup)
{
	RIGHT_CHECK(RIGHT_REPUTATION);
	
	float fValue;
	if(HKSUCCESS(HkGetRep(wscCharname, wscRepGroup, fValue))) {
		Print(L"feelings=%f\n", fValue);
		Print(L"OK\n");
	} else
		PrintError();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsg(const wstring &wscCharname, const wstring &wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkMsg(wscCharname, wscText)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgS(const wstring &wscSystemname, const wstring &wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkMsgS(wscSystemname, wscText)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgU(const wstring &wscText)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkMsgU(wscText)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsg(const wstring &wscCharname, const wstring &wscXML)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkFMsg(wscCharname, wscXML)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgS(const wstring &wscSystemname, const wstring &wscXML)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkFMsgS(wscSystemname, wscXML)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgU(const wstring &wscXML)
{
	RIGHT_CHECK(RIGHT_MSG);

	if(HKSUCCESS(HkFMsgU(wscXML)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdEnumCargo(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_CARGO);

	list<CARGO_INFO> lstCargo;
	int iRemainingHoldSize = 0;
	if(HKSUCCESS(HkEnumCargo(wscCharname, lstCargo, iRemainingHoldSize))) {
		Print(L"remainingholdsize=%d\n", iRemainingHoldSize);
		foreach(lstCargo, CARGO_INFO, it)
		{
			if(!(*it).bMounted)
				Print(L"id=%u archid=%u count=%d mission=%u\n", (*it).iID, (*it).iArchID, (*it).iCount, (*it).bMission ? 1 : 0);
		}

		Print(L"OK\n");
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRemoveCargo(const wstring &wscCharname, uint iID, uint iCount)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if(HKSUCCESS(HkRemoveCargo(wscCharname, iID, iCount)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCargo(const wstring &wscCharname, const wstring &wscGood, uint iCount, uint iMission)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if(HKSUCCESS(HkAddCargo(wscCharname, wscGood, iCount, iMission ? true : false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRename(const wstring &wscCharname, const wstring &wscNewCharname)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	if(HKSUCCESS(HkRename(wscCharname, wscNewCharname, false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDeleteChar(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	if(HKSUCCESS(HkRename(wscCharname, L"", true)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdReadCharFile(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	list<wstring> lstOut;
	if(HKSUCCESS(HkReadCharFile(wscCharname, lstOut))) {
		foreach(lstOut, wstring, it)
			Print(L"l %s\n", it->c_str());
		Print(L"OK\n");
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdWriteCharFile(const wstring &wscCharname, const wstring &wscData)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	if(HKSUCCESS(HkWriteCharFile(wscCharname, wscData)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintPlayerInfo(HKPLAYERINFO pi)
{
	RIGHT_CHECK(RIGHT_OTHER);

	Print(L"charname=%s clientid=%u ip=%s host=%s ping=%u base=%s system=%s\n", pi.wscCharname.c_str(), pi.iClientID, pi.wscIP.c_str(), pi.wscHostname.c_str(), pi.ci.dwRoundTripLatencyMS, pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdGetPlayerInfo(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	HKPLAYERINFO pi;
	if(HKSUCCESS(HkGetPlayerInfo(wscCharname, pi, false)))
		PrintPlayerInfo(pi);
	else
		PrintError();
	
	Print(L"OK\n");
}

void CCmds::CmdGetPlayers()
{
	RIGHT_CHECK(RIGHT_OTHER);

	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
		PrintPlayerInfo(*i);

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::XPrintPlayerInfo(HKPLAYERINFO pi)
{
	RIGHT_CHECK(RIGHT_OTHER);

	Print(L"Name: %s, ID: %u, IP: %s, Host: %s, Ping: %u, Base: %s, System: %s\n", pi.wscCharname.c_str(), pi.iClientID, pi.wscIP.c_str(), pi.wscHostname.c_str(), pi.ci.dwRoundTripLatencyMS, pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdXGetPlayerInfo(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	HKPLAYERINFO pi;
	if(HKSUCCESS(HkGetPlayerInfo(wscCharname, pi, false)))
		XPrintPlayerInfo(pi);
	else
		PrintError();
	
	Print(L"OK\n");
}

void CCmds::CmdXGetPlayers()
{
	RIGHT_CHECK(RIGHT_OTHER);

	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
		XPrintPlayerInfo(*i);

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetPlayerIDs()
{
	RIGHT_CHECK(RIGHT_OTHER);

	wchar_t wszLine[128] = L"";
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
	{
		wchar_t wszBuf[1024];
		swprintf(wszBuf, L"%s = %u | ", (*i).wscCharname.c_str(), (*i).iClientID);
		if((wcslen(wszBuf) + wcslen(wszLine)) >= sizeof(wszLine)/2)	{
			Print(L"%s\n", wszLine);
			wcscpy(wszLine, wszBuf);
		} else
			wcscat(wszLine, wszBuf);
	}

	if(wcslen(wszLine))
		Print(L"%s\n", wszLine);
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAccountDirName(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	wstring wscDir;
	if(HKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
		Print(L"dirname=%s\nOK\n", wscDir.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCharFileName(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	wstring wscFilename;
	if(HKSUCCESS(HkGetCharFileName(wscCharname, wscFilename)))
		Print(L"charfilename=%s\nOK\n", wscFilename.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsOnServer(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	CAccount *acc = HkGetAccountByCharname(wscCharname);
	if(!acc)
	{
		hkLastErr = HKE_CHAR_DOES_NOT_EXIST;
		PrintError();
		return;
	}

	uint iClientID = HkGetClientIdFromAccount(acc);
	if(iClientID == -1)
		Print(L"onserver=no\nOK\n");
	else
		Print(L"onserver=yes\nOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsLoggedIn(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	if(HkGetClientIdFromCharname(wscCharname) != -1) {
		if(HkIsInCharSelectMenu(wscCharname))
			Print(L"loggedin=no\nOK\n");
		else
			Print(L"loggedin=yes\nOK\n");
	} else
		Print(L"loggedin=no\nOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMoneyFixList()
{
	RIGHT_CHECK(RIGHT_OTHER);

	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);

		if(ClientInfo[iClientID].lstMoneyFix.size())
			Print(L"id=%u\n", iClientID);
	}

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdServerInfo()
{
	RIGHT_CHECK(RIGHT_OTHER);

	// calculate uptime
	FILETIME ftCreation;
	FILETIME ft;
	GetProcessTimes(GetCurrentProcess(), &ftCreation, &ft, &ft, &ft);
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ftNow;
	SystemTimeToFileTime(&st, &ftNow);
	__int64 iTimeCreation = (((__int64)ftCreation.dwHighDateTime) << 32) + ftCreation.dwLowDateTime;
	__int64 iTimeNow = (((__int64) ftNow.dwHighDateTime) << 32) + ftNow.dwLowDateTime;

	uint iUptime = (uint)((iTimeNow - iTimeCreation) / 10000000);
	uint iDays = (iUptime / (60*60*24));
	iUptime %= (60*60*24);
	uint iHours = (iUptime / (60*60));
	iUptime %= (60*60);
	uint iMinutes = (iUptime / 60);
	iUptime %= (60);
	uint iSeconds = iUptime;
	wchar_t wszUptime[16];
	swprintf(wszUptime, L"%.1u:%.2u:%.2u:%.2u", iDays, iHours, iMinutes, iSeconds);
	
	// print
	Print(L"serverload=%d npcspawn=%s uptime=%s\nOK\n", g_iServerLoad, g_bNPCDisabled ? L"disabled" : L"enabled", wszUptime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetGroupMembers(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	list<GROUP_MEMBER> lstMembers;
	if(HKSUCCESS(HkGetGroupMembers(wscCharname, lstMembers))) {
		Print(L"groupsize=%d\n", lstMembers.size());
		foreach(lstMembers, GROUP_MEMBER, it)
			Print(L"id=%d charname=%s\n", it->iClientID, it->wscCharname.c_str());
		Print(L"OK\n");
	} else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSaveChar(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_OTHER);

	if(HKSUCCESS(HkSaveChar(wscCharname)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetReservedSlot(const wstring &wscCharname)
{
	RIGHT_CHECK(RIGHT_SETTINGS);

	bool bResult;
	if(HKSUCCESS(HkGetReservedSlot(wscCharname, bResult)))
		Print(L"reservedslot=%s\nOK\n", bResult ? L"yes" : L"no");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetReservedSlot(const wstring &wscCharname, int iReservedSlot)
{
	RIGHT_CHECK(RIGHT_SETTINGS);

	if(HKSUCCESS(HkSetReservedSlot(wscCharname, iReservedSlot ? true : false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAdmin(const wstring &wscCharname, const wstring &wscRights)
{
	RIGHT_CHECK_SUPERADMIN();

	if(HKSUCCESS(HkSetAdmin(wscCharname, wscRights)))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAdmin(const wstring &wscCharname)
{
	RIGHT_CHECK_SUPERADMIN();

	wstring wscRights;
	if(HKSUCCESS(HkGetAdmin(wscCharname, wscRights)))
        Print(L"rights=%s\nOK\n", wscRights.c_str());
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDelAdmin(const wstring &wscCharname)
{
	RIGHT_CHECK_SUPERADMIN();

	if(HKSUCCESS(HkDelAdmin(wscCharname)))
        Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugins()
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	PluginManager::LoadPlugins(false, this);
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugin(const wstring &wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	PluginManager::LoadPlugin(wstos(wscPlugin), this, false);
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdListPlugins()
{

	RIGHT_CHECK(RIGHT_PLUGINS);

	foreach(lstPlugins, PLUGIN_DATA, it) 
		Print(L"%s (%s) - %s\n", stows(it->sName).c_str(), stows(it->sShortName).c_str(), (!it->bPaused ? L"running" : L"paused"));
	
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnloadPlugin(const wstring &wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	if(HKSUCCESS(PluginManager::UnloadPlugin(wstos(wscPlugin))))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdPausePlugin(const wstring &wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	if(HKSUCCESS(PluginManager::PausePlugin(wstos(wscPlugin), true)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnpausePlugin(const wstring &wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	if(HKSUCCESS(PluginManager::PausePlugin(wstos(wscPlugin), false)))
		Print(L"OK\n");
	else
		PrintError();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRehash()
{
	RIGHT_CHECK(RIGHT_SETTINGS);

	LoadSettings();
	HookRehashed();
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdHelp()
{
	wchar_t wszHelpMsg[] = 
		L"[version]\n"
		VERSION L"\n"
		L"[commands]\n"
		L"getcash <charname>\n"
		L"setcash <charname> <amount>\n"
		L"setcashsec <charname> <oldmoney> <amount>\n"
		L"addcash <charname> <amount>\n"
		L"addcashsec <charname> <oldmoney> <amount>\n"
		L"kick <charname> <reason>\n"
		L"ban <charname>\n"
		L"unban <charname>\n"
		L"kickban <charname> <reason>\n"
		L"beam <charname> <basename>\n"
		L"kill <charname>\n"
		L"resetrep <charname>\n"
		L"setrep <charname> <repgroup> <value>\n"
		L"getrep <charname> <repgroup>\n"
		L"readcharfile <charname>\n"
		L"writecharfile <charname> <data>\n"
		L"enumcargo <charname>\n"
		L"addcargo <charname> <good> <count> <mission>\n"
		L"removecargo <charname> <id> <count>\n"
		L"rename <oldcharname> <newcharname>\n"
		L"deletechar <charname>\n"
		L"msg <charname> <text>\n"
		L"msgs <systemname> <text>\n"
		L"msgu <text>\n"
		L"fmsg <charname> <xmltext>\n"
		L"fmsgs <systemname> <xmltext>\n"
		L"fmsgu <xmltext>\n"
		L"enumcargo <charname>\n"
		L"addcargo <charname> <good> <count> <mission>\n"
		L"removecargo <charname> <id> <count>\n"
		L"getgroupmembers <charname>\n"
		L"getbasestatus <basename>\n"
		L"getclientid <charname>\n"
		L"getplayerinfo <charname>\n"
		L"getplayers\n"
		L"xgetplayerinfo <charname>\n"
		L"xgetplayers\n"
		L"getplayerids\n"
		L"help\n"
		L"getaccountdirname <charname>\n"
		L"getcharfilename <charname>\n"
		L"isonserver <charname>\n"
		L"isloggedin <charname>\n"
		L"serverinfo\n"
		L"moneyfixlist\n"
		L"savechar <charname>\n"
		L"setadmin <charname> <rights>\n"
		L"getadmin <charname>\n"
		L"deladmin <charname>\n"
		L"getreservedslot <charname>\n"
		L"setreservedslot <charname> <value>\n"
		L"loadplugins\n"
		L"loadplugin <plugin filename>\n"
		L"listplugins\n"
		L"unloadplugin <plugin shortname>\n"
		L"pauseplugin <plugin shortname>\n"
		L"unpauseplugin <plugin shortname>\n"
		L"rehash\n"
		;

	Print(L"%s", wszHelpMsg);

	CALL_PLUGINS_NORET(PLUGIN_CmdHelp_Callback,,(CCmds* classptr),(this));

	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdTest(int iArg, int iArg2, int iArg3)
{
	HkTest(iArg, iArg2, iArg3);
	Print(L"OK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CCmds::ArgCharname(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	if(bID)
		return wscArg.replace((int) 0,(int) 0, L"id ");
	else if(bShortCut)
		return wscArg.replace((int) 0,(int) 0, L"sc ");
	else
		return wscArg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCmds::ArgInt(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	return ToInt(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CCmds::ArgFloat(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);
	return ToFloat(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CCmds::ArgStr(uint iArg)
{
	wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	return wscArg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CCmds::ArgStrToEnd(uint iArg)
{
	for(uint i = 0, iCurArg = 0; (i < wscCurCmdString.length()); i++)
	{
		if(wscCurCmdString[i] == ' ')
		{
			iCurArg++;

			if(iCurArg == iArg)
				return wscCurCmdString.substr(i + 1);

			while(((i + 1) < wscCurCmdString.length()) && (wscCurCmdString[i+1] == ' '))
				i++; // skip "whitechar"
		}
	}

	return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define IS_CMD(a) !wscCmd.compare(L##a)

bool ExecuteCommandString_Callback(CCmds* classptr, const wstring &wscCmdStr)
{
    CALL_PLUGINS(PLUGIN_ExecuteCommandString_Callback,bool,,(CCmds* classptr, const wstring &wscCmdStr),(classptr,wscCmdStr));
	
	return false;
}

void CCmds::ExecuteCommandString(const wstring &wscCmdStr)
{
	//check if command was sent by a socket connection
	bool bSocket = false;
	bool bLocalSocket = false;
	wstring wscAdminName = GetAdminName();

	if (wscAdminName.find(L"Socket connection") == 0)
	{
		bSocket = true;
		if(wscAdminName.find(L"127.0.0.1") != wstring::npos)
			bLocalSocket = true;
	}

	try 
	{
		if(bSocket)
		{
			if(bLocalSocket)
			{
				if(set_bLogLocalSocketCmds)
					HkAddSocketCmdLog("%s: %s", wstos(wscAdminName).c_str(), wstos(wscCmdStr).c_str());
			}
			else
			{
				if(set_bLogSocketCmds)
					HkAddSocketCmdLog("%s: %s", wstos(wscAdminName).c_str(), wstos(wscCmdStr).c_str());
			}	
		}
		else
		{
			if(set_bLogAdminCmds)
				HkAddAdminCmdLog("%s: %s", wstos(wscAdminName).c_str(), wstos(wscCmdStr).c_str());
		}

		bID = false;
		bShortCut = false;
		wscCurCmdString = wscCmdStr;

		wstring wscCmd = ToLower(GetParam(wscCmdStr, ' ', 0));
		if (wscCmd.length()==0)
		{
			Print(L"ERR unknown command\n");
			return;
		}

		if(wscCmd[wscCmd.length()-1] == '$') {
			bID = true;
			wscCmd.erase(wscCmd.length()-1, 1);
		} else if(wscCmd[wscCmd.length()-1] == '&') {
			bShortCut = true;
			wscCmd.erase(wscCmd.length()-1, 1);
		}

		if(!ExecuteCommandString_Callback(this, wscCmd))
		{

			if(IS_CMD("getcash")) {
				CmdGetCash(ArgCharname(1));
			} else if(IS_CMD("setcash")) {
				CmdSetCash(ArgCharname(1), ArgInt(2));
			} else if(IS_CMD("setcashsec")) {
				CmdSetCashSec(ArgCharname(1), ArgInt(2), ArgInt(3));
			} else if(IS_CMD("addcash")) {
				CmdAddCash(ArgCharname(1), ArgInt(2));
			} else if(IS_CMD("addcashsec")) {
				CmdAddCashSec(ArgCharname(1), ArgInt(2), ArgInt(3));
			} else if(IS_CMD("kick")) {
				CmdKick(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("ban")) {
				CmdBan(ArgCharname(1));
			} else if(IS_CMD("unban")) {
				CmdUnban(ArgCharname(1));
			} else if(IS_CMD("kickban")) {
				CmdKickBan(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("getbasestatus")) {
				CmdGetBaseStatus(ArgStr(1));
			} else if(IS_CMD("getclientid")) {
				CmdGetClientId(ArgCharname(1));
			} else if(IS_CMD("beam")) {
				CmdBeam(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("kill")) {
				CmdKill(ArgCharname(1));
			} else if(IS_CMD("resetrep")) {
				CmdResetRep(ArgCharname(1));
			} else if(IS_CMD("setrep")) {
				CmdSetRep(ArgCharname(1), ArgStr(2), ArgFloat(3));
			} else if(IS_CMD("getrep")) {
				CmdGetRep(ArgCharname(1), ArgStr(2));
			} else if(IS_CMD("msg")) {
				CmdMsg(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("msgs")) {
				CmdMsgS(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("msgu")) {
				CmdMsgU(ArgStrToEnd(1));
			} else if(IS_CMD("fmsg")) {
				CmdFMsg(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("fmsgs")) {
				CmdFMsgS(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("fmsgu")) {
				CmdFMsgU(ArgStrToEnd(1));
			} else if(IS_CMD("enumcargo")) {
				CmdEnumCargo(ArgCharname(1));
			} else if(IS_CMD("removecargo")) {
				CmdRemoveCargo(ArgCharname(1), ArgInt(2), ArgInt(3));
			} else if(IS_CMD("addcargo")) {
				CmdAddCargo(ArgCharname(1), ArgStr(2), ArgInt(3), ArgInt(4));
			} else if(IS_CMD("rename")) {
				CmdRename(ArgCharname(1), ArgStr(2));
			} else if(IS_CMD("deletechar")) {
				CmdDeleteChar(ArgCharname(1));
			} else if(IS_CMD("readcharfile")) {
				CmdReadCharFile(ArgCharname(1));
			} else if(IS_CMD("writecharfile")) {
				CmdWriteCharFile(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("getplayerinfo")) {
				CmdGetPlayerInfo(ArgCharname(1));
			} else if(IS_CMD("getplayers")) {
				CmdGetPlayers();
			} else if(IS_CMD("xgetplayerinfo")) {
				CmdXGetPlayerInfo(ArgCharname(1));
			} else if(IS_CMD("xgetplayers")) {
				CmdXGetPlayers();
			} else if(IS_CMD("getplayerids")) {
				CmdGetPlayerIDs();
			} else if(IS_CMD("getaccountdirname")) {
				CmdGetAccountDirName(ArgCharname(1));
			} else if(IS_CMD("getcharfilename")) {
				CmdGetCharFileName(ArgCharname(1));
			} else if(IS_CMD("savechar")) {
				CmdSaveChar(ArgCharname(1));
			} else if(IS_CMD("isonserver")) {
				CmdIsOnServer(ArgCharname(1));
			} else if(IS_CMD("isloggedin")) {
				CmdIsLoggedIn(ArgCharname(1));
			} else if(IS_CMD("moneyfixlist")) {
				CmdMoneyFixList();
			} else if(IS_CMD("serverinfo")) {
				CmdServerInfo();
			} else if(IS_CMD("getgroupmembers")) {
				CmdGetGroupMembers(ArgCharname(1));
			} else if(IS_CMD("getreservedslot")) {
				CmdGetReservedSlot(ArgCharname(1));
			} else if(IS_CMD("setreservedslot")) {
				CmdSetReservedSlot(ArgCharname(1), ArgInt(2));
			} else if(IS_CMD("setadmin")) {
				CmdSetAdmin(ArgCharname(1), ArgStrToEnd(2));
			} else if(IS_CMD("getadmin")) {
				CmdGetAdmin(ArgCharname(1));
			} else if(IS_CMD("deladmin")) {
				CmdDelAdmin(ArgCharname(1));
			} else if(IS_CMD("unloadplugin")) {
				CmdUnloadPlugin(ArgStrToEnd(1));
			} else if(IS_CMD("loadplugins")) {
				CmdLoadPlugins();
			} else if(IS_CMD("loadplugin")) {
				CmdLoadPlugin(ArgStrToEnd(1));
			} else if(IS_CMD("listplugins")) {
				CmdListPlugins();
			} else if(IS_CMD("pauseplugin")) {
				CmdPausePlugin(ArgStrToEnd(1));
			} else if(IS_CMD("unpauseplugin")) {
				CmdUnpausePlugin(ArgStrToEnd(1));
			} else if(IS_CMD("rehash")) {
				CmdRehash();
			} else if(IS_CMD("help")) {
				CmdHelp();
			} else if(IS_CMD("test")) {
				CmdTest(ArgInt(1), ArgInt(2), ArgInt(3));
			} else {
				Print(L"ERR unknown command\n");
			}
			
		}
		if(bSocket)
		{
			if(bLocalSocket)
			{
				if(set_bLogLocalSocketCmds)
					HkAddSocketCmdLog("finnished");
			}
			else
			{
				if(set_bLogSocketCmds)
					HkAddSocketCmdLog("finnished");
			}	
		}
		else
		{
			if(set_bLogAdminCmds)
				HkAddAdminCmdLog("finnished");
		}
	} catch(...) {
		if(bSocket)
		{
			if(bLocalSocket)
			{
				if(set_bLogLocalSocketCmds)
					HkAddSocketCmdLog("exception");
			}
			else
			{
				if(set_bLogSocketCmds)
					HkAddSocketCmdLog("exception");
			}	
		}
		else
		{
			if(set_bLogAdminCmds)
				HkAddAdminCmdLog("exception");
		}
		Print(L"ERR exception occured\n");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::SetRightsByString(const string &scRights)
{
	rights = RIGHT_NOTHING;
	string scRightStr = ToLower(scRights);
	if(scRightStr.find("superadmin") != -1)
		rights |= RIGHT_SUPERADMIN;
	if(scRightStr.find("cash") != -1)
		rights |= RIGHT_CASH;
	if(scRightStr.find("kickban") != -1)
		rights |= RIGHT_KICKBAN;
	if(scRightStr.find("beamkill") != -1)
		rights |= RIGHT_BEAMKILL;
	if(scRightStr.find("msg") != -1)
		rights |= RIGHT_MSG;
	if(scRightStr.find("other") != -1)
		rights |= RIGHT_OTHER;
	if(scRightStr.find("cargo") != -1)
		rights |= RIGHT_CARGO;
	if(scRightStr.find("characters") != -1)
		rights |= RIGHT_CHARACTERS;
	if(scRightStr.find("settings") != -1)
		rights |= RIGHT_SETTINGS;
	if(scRightStr.find("reputation") != -1)
		rights |= RIGHT_REPUTATION;
	if(scRightStr.find("plugins") != -1)
		rights |= RIGHT_PLUGINS;
	if(scRightStr.find("eventmode") != -1)
		rights |= RIGHT_EVENTMODE;
	if(scRightStr.find("special1") != -1)
		rights |= RIGHT_SPECIAL1;
	if(scRightStr.find("special2") != -1)
		rights |= RIGHT_SPECIAL2;
	if(scRightStr.find("special3") != -1)
		rights |= RIGHT_SPECIAL3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintError()
{
	Print(L"ERR %s\n", HkErrGetText(this->hkLastErr).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::Print(wstring wscText, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscText);

	_vsnwprintf(wszBuf, (sizeof(wszBuf) / 2) - 1, wscText.c_str(), marker);

	DoPrint(wszBuf);
}