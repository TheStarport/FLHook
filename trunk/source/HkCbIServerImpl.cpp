#include "wildcards.hh"
#include "hook.h"
#include "CInGame.h"

#define ISERVER_LOG() if(set_bDebug) AddDebugLog(__FUNCSIG__);
#define ISERVER_LOGARG_WS(a) if(set_bDebug) AddDebugLog("     " #a ": %s", wstos((const wchar_t*)a).c_str());
#define ISERVER_LOGARG_S(a) if(set_bDebug) AddDebugLog("     " #a ": %s", (const char*)a);
#define ISERVER_LOGARG_UI(a) if(set_bDebug) AddDebugLog("     " #a ": %u", (uint)a);
#define ISERVER_LOGARG_I(a) if(set_bDebug) AddDebugLog("     " #a ": %d", (int)a);
#define ISERVER_LOGARG_H(a) if(set_bDebug) AddDebugLog("     " #a ": 0x%08X", (int)a);
#define ISERVER_LOGARG_F(a) if(set_bDebug) AddDebugLog("     " #a ": %f", (float)a);
#define ISERVER_LOGARG_V(a) if(set_bDebug) AddDebugLog("     " #a ": %f %f %f", (float)a.x, (float)a.y, (float)a.z);


#define EXECUTE_SERVER_CALL(args) \
	{ \
	static CTimer timer(__FUNCTION__,set_iTimerThreshold); \
	timer.start(); \
	try { \
		args; \
	} catch(...) { AddLog("Exception in function %s", __FUNCTION__); } \
	timer.stop(); \
	}

namespace HkIServerImpl
{

/**************************************************************************************************************
this is our "main" loop
**************************************************************************************************************/

// add timers here
typedef void (*_TimerFunc)();

struct TIMER
{
	_TimerFunc	proc;
	mstime		tmIntervallMS;
	mstime		tmLastCall;
};

TIMER Timers[] = 
{
	{ProcessPendingCommands,		50,					0},
	{HkTimerCheckKick,				1000,				0},
	{HkTimerNPCAndF1Check,			50,					0},
	{HkTimerCheckResolveResults,	0,					0},
};

int __stdcall Update(void)
{

	static bool bFirstTime = true;
	if(bFirstTime)
	{
		FLHookInit();
		bFirstTime = false;
	} 

	// call timers
	for(uint i = 0; (i < sizeof(Timers)/sizeof(TIMER)); i++)
	{
		if((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS)
		{
			Timers[i].tmLastCall = timeInMS();
			Timers[i].proc();
		}
	}

	char *pData;
	memcpy(&pData, g_FLServerDataPtr + 0x40, 4);
	memcpy(&g_iServerLoad, pData + 0x204, 4);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_Update,());
	if(bPluginReturn) 
		return reinterpret_cast<int>(vPluginRet);

	int result = 0;
	EXECUTE_SERVER_CALL(result = Server.Update());
	return result;
}

/**************************************************************************************************************
Chat-Messages are hooked here
<Parameters>
cId:       Sender's ClientID
lP1:       size of rdlReader (used when extracting text from that buffer)
rdlReader: RenderDisplayList which contains the chat-text
cIdTo:     recipient's clientid(0x10000 = universe chat else when (cIdTo & 0x10000) = true -> system chat)
iP2:       ???
**************************************************************************************************************/

CInGame admin;
bool g_bInSubmitChat = false;
uint g_iTextLen = 0;

void __stdcall SubmitChat_AFTER(struct CHAT_ID cId, unsigned long lP1, void const *rdlReader, struct CHAT_ID cIdTo, int iP2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SubmitChat,(cId,lP1,rdlReader,cIdTo,iP2));
}

void __stdcall SubmitChat(struct CHAT_ID cId, unsigned long lP1, void const *rdlReader, struct CHAT_ID cIdTo, int iP2)
{
	ISERVER_LOG();

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SubmitChat,(cId,lP1,rdlReader,cIdTo,iP2));
	if(bPluginReturn)
		return;

	wchar_t wszBuf[1024] = L"";

	try {
		// Group join/leave commands
		if(cIdTo.iID == 0x10004)
		{
			g_bInSubmitChat = true;
			Server.SubmitChat(cId, lP1, rdlReader, cIdTo, iP2);
			g_bInSubmitChat = false;
			return;
		}

		// extract text from rdlReader
		BinaryRDLReader rdl;
		uint iRet1;
		rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader, lP1);
		wstring wscBuf = wszBuf;
		uint iClientID = cId.iID;

		// fix flserver commands
		g_iTextLen = (uint)wscBuf.length();
		if(!wscBuf.find(L"/u ") || !wscBuf.find(L"/s ") || !wscBuf.find(L"/g ") || !wscBuf.find(L"/l ")) {
			g_iTextLen -= 3;
		}
		if(!wscBuf.find(L"/system ")) {
			g_iTextLen -= 8;
		}
		if(!wscBuf.find(L"/group ") || !wscBuf.find(L"/local "))
			g_iTextLen -= 7;
		if(!wscBuf.find(L"/universe "))
			g_iTextLen -= 9;

		// check for user cmds
		if(UserCmd_Process(iClientID, wscBuf))
			return;

		if(wszBuf[0] == '.')
		{ // flhook admin command
			CAccount *acc = Players.FindAccountFromClientID(iClientID);
			wstring wscAccDirname;

			HkGetAccountDirName(acc, wscAccDirname);
			string scAdminFile = scAcctPath + wstos(wscAccDirname) + "\\flhookadmin.ini";
			WIN32_FIND_DATA fd;
			HANDLE hFind = FindFirstFile(scAdminFile.c_str(), &fd);
			if(hFind != INVALID_HANDLE_VALUE)
			{ // is admin
				FindClose(hFind);
				admin.ReadRights(scAdminFile);
				admin.iClientID = iClientID;
				admin.wscAdminName = (wchar_t*)Players.GetActiveCharacterName(iClientID);
				admin.ExecuteCommandString(wszBuf + 1);
				return;
			}
		}

		// process chat event
		wstring wscEvent;
		wscEvent.reserve(256);
		wscEvent = L"chat";
		wscEvent += L" from=";
		const wchar_t *wszFrom = Players.GetActiveCharacterName(cId.iID);
		if(!cId.iID)
			wscEvent += L"console";
		else if (!wszFrom)
			wscEvent += L"unknown";
		else
			wscEvent += wszFrom;

		wscEvent += L" id=";
		wscEvent += stows(itos(cId.iID));

		wscEvent += L" type=";
		if(cIdTo.iID == 0x00010000)
			wscEvent += L"universe";
		else if(cIdTo.iID & 0x00010000)
			wscEvent += L"system";
		else {
			wscEvent += L"player";
			wscEvent += L" to=";

			const wchar_t *wszTo = Players.GetActiveCharacterName(cIdTo.iID);
			if(!cIdTo.iID)
				wscEvent += L"console";
			else if (!wszTo)
				wscEvent += L"unknown";
			else
				wscEvent += wszTo;

			wscEvent += L" idto=";
			wscEvent += stows(itos(cIdTo.iID));
		}

		wscEvent += L" text=";
		wscEvent += wscBuf;
		ProcessEvent(L"%s", wscEvent.c_str());

		// check if chat should be suppressed
		foreach(set_lstChatSuppress, wstring, i)
		{
			if((ToLower(wscBuf)).find(ToLower(*i)) == 0)
				return;
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	// send
	g_bInSubmitChat = true;
	EXECUTE_SERVER_CALL(Server.SubmitChat(cId, lP1, rdlReader, cIdTo, iP2));
	g_bInSubmitChat = false;

	SubmitChat_AFTER(cId, lP1, rdlReader, cIdTo, iP2);
}

/**************************************************************************************************************
Called when player ship was created in space (after undock or login)
**************************************************************************************************************/

void __stdcall PlayerLaunch_AFTER(unsigned int iShip, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_PlayerLaunch,(iShip,iClientID));
}

void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].iShip = iShip;
		ClientInfo[iClientID].iKillsInARow = 0;
		ClientInfo[iClientID].bCruiseActivated = false;
		ClientInfo[iClientID].bThrusterActivated = false;
		ClientInfo[iClientID].bEngineKilled = false;
		ClientInfo[iClientID].bTradelane = false;

		// adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
		wstring wscCharname = ToLower((wchar_t*)Players.GetActiveCharacterName(iClientID));
		foreach(ClientInfo[iClientID].lstMoneyFix, MONEY_FIX, i)
		{
			if(!(*i).wscCharname.compare(wscCharname))
			{
				HkAddCash(wscCharname, (*i).iAmount);
				ClientInfo[iClientID].lstMoneyFix.remove(*i);
				break;
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_PlayerLaunch,(iShip,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.PlayerLaunch(iShip, iClientID));

	try {
		if(!ClientInfo[iClientID].iLastExitedBaseID)
		{
			ClientInfo[iClientID].iLastExitedBaseID = 1;

			// event
			ProcessEvent(L"spawn char=%s id=%d system=%s", 
					(wchar_t*)Players.GetActiveCharacterName(iClientID), 
					iClientID,
					HkGetPlayerSystem(iClientID).c_str());
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	PlayerLaunch_AFTER(iShip, iClientID);
}

/**************************************************************************************************************
Called when player fires a weapon
**************************************************************************************************************/

void __stdcall FireWeapon_AFTER(unsigned int iClientID, struct XFireWeaponInfo const &wpn)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_FireWeapon,(iClientID,wpn));
}

void __stdcall FireWeapon(unsigned int iClientID, struct XFireWeaponInfo const &wpn)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_FireWeapon,(iClientID,wpn));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.FireWeapon(iClientID, wpn));
	FireWeapon_AFTER(iClientID, wpn);
}

/**************************************************************************************************************
Called when one player hits a target with a gun
<Parameters>
ci:  only figured out where dwTargetShip is ...
**************************************************************************************************************/

void __stdcall SPMunitionCollision_AFTER(struct SSPMunitionCollisionInfo const & ci, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPMunitionCollision,(ci,iClientID));
}

void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const & ci, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	uint iClientIDTarget;

	try {
		iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip);
		if(iClientIDTarget && !AllowPlayerDamage(iClientID, iClientIDTarget))
			return;

	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	iDmgTo = iClientIDTarget;

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPMunitionCollision,(ci,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SPMunitionCollision(ci, iClientID));
	SPMunitionCollision_AFTER(ci, iClientID);
}

/**************************************************************************************************************
Called when player moves his ship
**************************************************************************************************************/

void __stdcall SPObjUpdate_AFTER(struct SSPObjUpdateInfo const &ui, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPObjUpdate,(ui,iClientID));
}

void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPObjUpdate,(ui,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SPObjUpdate(ui, iClientID));
	SPObjUpdate_AFTER(ui, iClientID);

}
/**************************************************************************************************************
Called when one player collides with a space object
**************************************************************************************************************/

void __stdcall SPObjCollision_AFTER(struct SSPObjCollisionInfo const &ci, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPObjCollision,(ci,iClientID));
}

void __stdcall SPObjCollision(struct SSPObjCollisionInfo const &ci, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		uint iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip);
		if(iClientIDTarget && !AllowPlayerDamage(iClientID, iClientIDTarget))
			return;

	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPObjCollision,(ci,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SPObjCollision(ci, iClientID));
	SPObjCollision_AFTER(ci, iClientID);
}

/**************************************************************************************************************
Called when player has undocked and is now ready to fly
**************************************************************************************************************/

void __stdcall LaunchComplete_AFTER(unsigned int iBaseID, unsigned int iShip)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_LaunchComplete,(iBaseID,iShip));
}

void __stdcall LaunchComplete(unsigned int iBaseID, unsigned int iShip)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iBaseID);
	ISERVER_LOGARG_UI(iShip);

	try {
		uint iClientID = HkGetClientIDByShip(iShip);
		if(iClientID) {
			ClientInfo[iClientID].tmSpawnTime = timeInMS(); // save for anti-dockkill
			// is there spawnprotection?
			if(set_iAntiDockKill > 0)
				ClientInfo[iClientID].bSpawnProtected = true; 
			else
				ClientInfo[iClientID].bSpawnProtected = false; 
		}

		// event
		ProcessEvent(L"launch char=%s id=%d base=%s system=%s", 
				(wchar_t*)Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetBaseNickByID(ClientInfo[iClientID].iLastExitedBaseID).c_str(),
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_LaunchComplete,(iBaseID,iShip));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.LaunchComplete(iBaseID, iShip));

	LaunchComplete_AFTER(iBaseID, iShip);
}

/**************************************************************************************************************
Called when player selects a character
**************************************************************************************************************/

void __stdcall CharacterSelect_AFTER(struct CHARACTER_ID const & cId, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_CharacterSelect,(cId,iClientID));
}

void __stdcall CharacterSelect(struct CHARACTER_ID const & cId, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_S(&cId);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_CharacterSelect,(cId,iClientID));
	if(bPluginReturn)
		return;

	wstring wscCharBefore;
	try {
		const wchar_t *wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
		wscCharBefore = wszCharname ? (wchar_t*)Players.GetActiveCharacterName(iClientID) : L"";
		ClientInfo[iClientID].iLastExitedBaseID = 0;
		Server.CharacterSelect(cId, iClientID);
	} catch(...) {
		HkAddKickLog(iClientID, L"Corrupt charfile?");
		HkKick(ARG_CLIENTID(iClientID));
		return;
	}

	try {
		wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);

		if(wscCharBefore.compare(wscCharname) != 0) {
			LoadUserCharSettings(iClientID);

			if(set_bUserCmdHelp)
				PrintUserCmdText(iClientID, L"To get a list of available commands, type \"/help\" in chat.");

			// anti-cheat check
			list <CARGO_INFO> lstCargo;
			int iHold;
			HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
			foreach(lstCargo, CARGO_INFO, it)
			{
				if((*it).iCount < 0)
				{
					HkAddCheaterLog(wscCharname, L"Negative good-count, likely to have cheated in the past");

					wchar_t wszBuf[256];
					swprintf(wszBuf, L"Possible cheating detected (%s)", wscCharname.c_str());
					HkMsgU(wszBuf);
					HkBan(ARG_CLIENTID(iClientID), true);
					HkKick(ARG_CLIENTID(iClientID));
					return;
				}
			}

			// event
			CAccount *acc = Players.FindAccountFromClientID(iClientID);
			wstring wscDir;
			HkGetAccountDirName(acc, wscDir);
			HKPLAYERINFO pi;
			HkGetPlayerInfo(ARG_CLIENTID(iClientID), pi, false);
			ProcessEvent(L"login char=%s accountdirname=%s id=%d ip=%s", 
					wscCharname.c_str(),
					wscDir.c_str(),
					iClientID,
					pi.wscIP.c_str());
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CharacterSelect_AFTER(cId, iClientID);
}

/**************************************************************************************************************
Called when player enters base
**************************************************************************************************************/

void __stdcall BaseEnter_AFTER(unsigned int iBaseID, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_BaseEnter,(iBaseID,iClientID));
}

void __stdcall BaseEnter(unsigned int iBaseID, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iBaseID);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_BaseEnter,(iBaseID,iClientID));
	if(bPluginReturn)
		return;

	try {
		// autobuy
		if(set_bAutoBuy)
			HkPlayerAutoBuy(iClientID, iBaseID);
	} catch(...) { AddLog("Exception in Autobuy"); }

	EXECUTE_SERVER_CALL(Server.BaseEnter(iBaseID, iClientID));

	try {
		// adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
		wstring wscCharname = ToLower((wchar_t*)Players.GetActiveCharacterName(iClientID));
		foreach(ClientInfo[iClientID].lstMoneyFix, MONEY_FIX, i)
		{
			if(!(*i).wscCharname.compare(wscCharname))
			{
				HkAddCash(wscCharname, (*i).iAmount);
				ClientInfo[iClientID].lstMoneyFix.remove(*i);
				break;
			}
		}

		// anti base-idle
		ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);

		// event
		ProcessEvent(L"baseenter char=%s id=%d base=%s system=%s", 
				(wchar_t*)Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetBaseNickByID(iBaseID).c_str(),
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	BaseEnter_AFTER(iBaseID, iClientID);
}

/**************************************************************************************************************
Called when player exits base
**************************************************************************************************************/

void __stdcall BaseExit_AFTER(unsigned int iBaseID, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_BaseExit,(iBaseID,iClientID));
}

void __stdcall BaseExit(unsigned int iBaseID, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iBaseID);
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].iBaseEnterTime = 0;
		ClientInfo[iClientID].iLastExitedBaseID = iBaseID;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_BaseExit,(iBaseID,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.BaseExit(iBaseID, iClientID));

	try {
		const wchar_t *wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);

		// event
		ProcessEvent(L"baseexit char=%s id=%d base=%s system=%s", 
				(wchar_t*)Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetBaseNickByID(iBaseID).c_str(),
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	BaseExit_AFTER(iBaseID, iClientID);
}
/**************************************************************************************************************
Called when player connects
**************************************************************************************************************/

void __stdcall OnConnect_AFTER(unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_OnConnect,(iClientID));
}

void __stdcall OnConnect(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		// also check for too high id due to disconnect buffer time
		if((ClientInfo[iClientID].tmF1TimeDisconnect > timeInMS()) || (iClientID > Players.GetMaxPlayerCount())) {
			
			// manual disconnect
			CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
			if(!cdpClient)
				return;
			cdpClient->Disconnect();
			return;
		}

		ClientInfo[iClientID].iConnects++;
		ClearClientInfo(iClientID);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }


	CALL_PLUGINS(PLUGIN_HkIServerImpl_OnConnect,(iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.OnConnect(iClientID));

	try {
		// event
		wstring wscIP;
		HkGetPlayerIP(iClientID, wscIP);
		ProcessEvent(L"connect id=%d ip=%s", 
				iClientID,
				wscIP.c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	OnConnect_AFTER(iClientID);
}

/**************************************************************************************************************
Called when player disconnects
**************************************************************************************************************/

void __stdcall DisConnect_AFTER(unsigned int iClientID, enum EFLConnection p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_DisConnect,(iClientID,p2));
}

void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	try {
		ClientInfo[iClientID].lstMoneyFix.clear();

		if(!ClientInfo[iClientID].bDisconnected)
		{
			ClientInfo[iClientID].bDisconnected = true;

			// event
			const wchar_t *wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
			ProcessEvent(L"disconnect char=%s id=%d", 
					(wszCharname ? wszCharname : L""), 
					iClientID);
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_DisConnect,(iClientID,p2));
	if(bPluginReturn)
		return;
	
	EXECUTE_SERVER_CALL(Server.DisConnect(iClientID, p2));
	DisConnect_AFTER(iClientID, p2);
}

/**************************************************************************************************************
Called when trade is being terminated
**************************************************************************************************************/

void __stdcall TerminateTrade_AFTER(unsigned int iClientID, int iAccepted)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_TerminateTrade,(iClientID,iAccepted));
}

void __stdcall TerminateTrade(unsigned int iClientID, int iAccepted)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_I(iAccepted);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_TerminateTrade,(iClientID,iAccepted));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.TerminateTrade(iClientID, iAccepted));

	try {
		if(iAccepted)
		{ // save both chars to prevent cheating in case of server crash
			HkSaveChar(ARG_CLIENTID(iClientID));
			if(ClientInfo[iClientID].iTradePartner)
				HkSaveChar(ARG_CLIENTID(ClientInfo[iClientID].iTradePartner));
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	TerminateTrade_AFTER(iClientID, iAccepted);
}

/**************************************************************************************************************
Called when new trade request
**************************************************************************************************************/

void __stdcall InitiateTrade_AFTER(unsigned int iClientID1, unsigned int iClientID2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_InitiateTrade,(iClientID1,iClientID2));
}

void __stdcall InitiateTrade(unsigned int iClientID1, unsigned int iClientID2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID1);
	ISERVER_LOGARG_UI(iClientID2);

	try {
		// save traders client-ids
		ClientInfo[iClientID1].iTradePartner = iClientID2;
		ClientInfo[iClientID2].iTradePartner = iClientID1;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_InitiateTrade,(iClientID1,iClientID2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.InitiateTrade(iClientID1, iClientID2));
	InitiateTrade_AFTER(iClientID1, iClientID2);
}

/**************************************************************************************************************
Called when equipment is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateEquip_AFTER(unsigned int iClientID, struct XActivateEquip const &aq)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ActivateEquip,(iClientID,aq));
}

void __stdcall ActivateEquip(unsigned int iClientID, struct XActivateEquip const &aq)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {

		list<CARGO_INFO> lstCargo;
		int iRem;
		HkEnumCargo(ARG_CLIENTID(iClientID),lstCargo,iRem);

		foreach(lstCargo,CARGO_INFO,it) {
			if(it->iID == aq.sID){
				Archetype::Equipment *eq = Archetype::GetEquipment(it->iArchID);
				EQ_TYPE eqType = HkGetEqType(eq);

				if(eqType == ET_ENGINE) {
					ClientInfo[iClientID].bEngineKilled = !aq.bActivate;
					if(!aq.bActivate)
						ClientInfo[iClientID].bCruiseActivated = false; // enginekill enabled
				}

			}
		}

	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ActivateEquip,(iClientID,aq));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ActivateEquip(iClientID, aq));
	ActivateEquip_AFTER(iClientID, aq);
}

/**************************************************************************************************************
Called when cruise engine is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateCruise_AFTER(unsigned int iClientID, struct XActivateCruise const &ac)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ActivateCruise,(iClientID,ac));
}

void __stdcall ActivateCruise(unsigned int iClientID, struct XActivateCruise const &ac)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].bCruiseActivated = ac.bActivate;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ActivateCruise,(iClientID,ac));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ActivateCruise(iClientID, ac));
	ActivateCruise_AFTER(iClientID, ac);
}

/**************************************************************************************************************
Called when thruster is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateThrusters_AFTER(unsigned int iClientID, struct XActivateThrusters const &at)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ActivateThrusters,(iClientID,at));
}

void __stdcall ActivateThrusters(unsigned int iClientID, struct XActivateThrusters const &at)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].bThrusterActivated = at.bActivate;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ActivateThrusters,(iClientID,at));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ActivateThrusters(iClientID, at));
	ActivateThrusters_AFTER(iClientID, at);
}

/**************************************************************************************************************
Called when player sells good on a base
**************************************************************************************************************/

void __stdcall GFGoodSell_AFTER(struct SGFGoodSellInfo const &gsi, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFGoodSell,(gsi,iClientID));
}

void __stdcall GFGoodSell(struct SGFGoodSellInfo const &gsi, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		// anti-cheat check
		list <CARGO_INFO> lstCargo;
		int iHold;
		HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
		bool bLegalSell = false;
		foreach(lstCargo, CARGO_INFO, it)
		{
			if((*it).iArchID == gsi.iArchID)
			{
				bLegalSell = true;
				if(abs(gsi.iCount) > it->iCount)
				{
					wchar_t wszBuf[1000];
					
					const wchar_t *wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
					swprintf(wszBuf, L"Sold more good than possible item=%08x count=%u", gsi.iArchID, gsi.iCount);
					HkAddCheaterLog(wszCharname, wszBuf);

					swprintf(wszBuf, L"Possible cheating detected (%s)", wszCharname);
					HkMsgU(wszBuf);
					HkBan(ARG_CLIENTID(iClientID), true);
					HkKick(ARG_CLIENTID(iClientID));
					return;
				}
				break;
			}
		}
		if(!bLegalSell)
		{
			wchar_t wszBuf[1000];
			const wchar_t *wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
			swprintf(wszBuf, L"Sold good player does not have (buggy test), item=%08x", gsi.iArchID);
			HkAddCheaterLog(wszCharname, wszBuf);

			//swprintf(wszBuf, L"Possible cheating detected (%s)", wszCharname);
			//HkMsgU(wszBuf);
			//HkBan(ARG_CLIENTID(iClientID), true);
			//HkKick(ARG_CLIENTID(iClientID));
			return;
		}
	} catch(...) { AddLog("Exception in %s (iClientID=%u (%x))", __FUNCTION__, iClientID, Players.GetActiveCharacterName(iClientID)); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFGoodSell,(gsi,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.GFGoodSell(gsi, iClientID));
	GFGoodSell_AFTER(gsi, iClientID);
}

/**************************************************************************************************************
Called when player connects or pushes f1
**************************************************************************************************************/

void __stdcall CharacterInfoReq_AFTER(unsigned int iClientID, bool p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_CharacterInfoReq,(iClientID,p2));
}

void __stdcall CharacterInfoReq(unsigned int iClientID, bool p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_CharacterInfoReq,(iClientID,p2));
	if(bPluginReturn)
		return;

	try {
		if(!ClientInfo[iClientID].bCharSelected)
			ClientInfo[iClientID].bCharSelected = true;
		else { // pushed f1
			uint iShip = 0;
			pub::Player::GetShip(iClientID, iShip);
			if(iShip)
			{ // in space
				ClientInfo[iClientID].tmF1Time = timeInMS() + set_iAntiF1;
				return;
			}
		}

		Server.CharacterInfoReq(iClientID, p2);
	} catch(...) { // something is wrong with charfile
		HkAddKickLog(iClientID, L"Corrupt charfile?");
		HkKick(ARG_CLIENTID(iClientID));
		return;
	}

	CharacterInfoReq_AFTER(iClientID, p2);
}

/**************************************************************************************************************
Called when player jumps in system
**************************************************************************************************************/

void __stdcall JumpInComplete_AFTER(unsigned int iSystemID, unsigned int iShip)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_JumpInComplete,(iSystemID,iShip));
}

void __stdcall JumpInComplete(unsigned int iSystemID, unsigned int iShip)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iSystemID);
	ISERVER_LOGARG_UI(iShip);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_JumpInComplete,(iSystemID,iShip));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.JumpInComplete(iSystemID, iShip));

	try {
		uint iClientID = HkGetClientIDByShip(iShip);
		if(!iClientID)
			return;

		// event
		ProcessEvent(L"jumpin char=%s id=%d system=%s", 
				(wchar_t*)Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetSystemNickByID(iSystemID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	JumpInComplete_AFTER(iSystemID, iShip);
}

/**************************************************************************************************************
Called when player jumps out of system
**************************************************************************************************************/

void __stdcall SystemSwitchOutComplete_AFTER(unsigned int iShip, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SystemSwitchOutComplete,(iShip,iClientID));
}

void __stdcall SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SystemSwitchOutComplete,(iShip,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SystemSwitchOutComplete(iShip, iClientID));

	try {
		// event
		ProcessEvent(L"switchout char=%s id=%d system=%s", 
				(wchar_t*)Players.GetActiveCharacterName(iClientID), 
				iClientID,
				HkGetPlayerSystem(iClientID).c_str());
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	SystemSwitchOutComplete_AFTER(iShip, iClientID);
}

/**************************************************************************************************************
Called when player logs in
**************************************************************************************************************/

void __stdcall Login_AFTER(struct SLoginInfo const &li, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_Login,(li,iClientID));
}

void __stdcall Login(struct SLoginInfo const &li, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_WS(&li);
	ISERVER_LOGARG_UI(iClientID);

	try {
		Server.Login(li, iClientID);

		if(iClientID > Players.GetMaxPlayerCount())
			return; // lalala DisconnectDelay bug

		if(!HkIsValidClientID(iClientID))
			return; // player was kicked

		// Kick the player if the account ID doesn't exist. This is caused
		// by a duplicate log on.
		CAccount *acc = Players.FindAccountFromClientID(iClientID);
		if (acc && !acc->wszAccID)
		{
			acc->ForceLogout();
			return;
		}

		CALL_PLUGINS(PLUGIN_HkIServerImpl_Login,(li,iClientID));
		if(bPluginReturn)
			return;	


		// check for ip ban
		wstring wscIP;
		HkGetPlayerIP(iClientID, wscIP);

		foreach(set_lstBans, wstring, itb)
		{
			if(Wildcard::wildcardfit(wstos(*itb).c_str(), wstos(wscIP).c_str()))
			{
				HkAddKickLog(iClientID, L"IP/Hostname ban(%s matches %s)", wscIP.c_str(), (*itb).c_str());
				if(set_bBanAccountOnMatch)
					HkBan(ARG_CLIENTID(iClientID), true);
				HkKick(ARG_CLIENTID(iClientID));
			}
		}

		// resolve
		RESOLVE_IP rip;
		rip.wscIP = wscIP;
		rip.wscHostname = L"";
		rip.iConnects = ClientInfo[iClientID].iConnects; // security check so that wrong person doesnt get banned
		rip.iClientID = iClientID;
		EnterCriticalSection(&csIPResolve);
		g_lstResolveIPs.push_back(rip);
		LeaveCriticalSection(&csIPResolve);

		// count players
		struct PlayerData *pPD = 0;
		uint iPlayers = 0;
		while(pPD = Players.traverse_active(pPD))
			iPlayers++;

		if(iPlayers > (Players.GetMaxPlayerCount() -  set_iReservedSlots))
		{ // check if player has a reserved slot
			CAccount *acc = Players.FindAccountFromClientID(iClientID);
			wstring wscDir; 
			HkGetAccountDirName(acc, wscDir); 
			string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

			bool bReserved = IniGetB(scUserFile, "Settings", "ReservedSlot", false);
			if(!bReserved)
			{
				HkKick(ARG_CLIENTID(iClientID));
				return;
			}
		}

		LoadUserSettings(iClientID);

		// log
		if(set_bLogConnects)
			HkAddConnectLog(iClientID, wscIP);

	}
	catch(...)
	{
		AddLog("Exception in %s", __FUNCTION__);
		CAccount *acc = Players.FindAccountFromClientID(iClientID);
		if (acc)
		{
			acc->ForceLogout();
		}
	}

	Login_AFTER(li, iClientID);
}

/**************************************************************************************************************
Called on item spawn
**************************************************************************************************************/

void __stdcall MineAsteroid_AFTER(unsigned int p1, class Vector const &vPos, unsigned int iLookID, unsigned int iGoodID, unsigned int iCount, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_MineAsteroid,(p1,vPos,iLookID,iGoodID,iCount,iClientID));
}

void __stdcall MineAsteroid(unsigned int p1, class Vector const &vPos, unsigned int iLookID, unsigned int iGoodID, unsigned int iCount, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_UI(vPos);
	ISERVER_LOGARG_UI(iLookID);
	ISERVER_LOGARG_UI(iGoodID);
	ISERVER_LOGARG_UI(iCount);
	ISERVER_LOGARG_UI(iClientID);


	CALL_PLUGINS(PLUGIN_HkIServerImpl_MineAsteroid,(p1,vPos,iLookID,iGoodID,iCount,iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.MineAsteroid(p1, vPos, iLookID, iGoodID, iCount, iClientID));
	MineAsteroid_AFTER(p1, vPos, iLookID, iGoodID, iCount, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GoTradelane_AFTER(unsigned int iClientID, struct XGoTradelane const &gtl)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_GoTradelane,(iClientID,gtl));
}

void __stdcall GoTradelane(unsigned int iClientID, struct XGoTradelane const &gtl)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	try {
		ClientInfo[iClientID].bTradelane = true;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_GoTradelane,(iClientID,gtl));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.GoTradelane(iClientID, gtl));
	GoTradelane_AFTER(iClientID, gtl);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall StopTradelane_AFTER(unsigned int iClientID, unsigned int p2, unsigned int p3, unsigned int p4)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_StopTradelane,(iClientID,p2,p3,p4));
}

void __stdcall StopTradelane(unsigned int iClientID, unsigned int p2, unsigned int p3, unsigned int p4)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);

	try {
		ClientInfo[iClientID].bTradelane = false;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	CALL_PLUGINS(PLUGIN_HkIServerImpl_StopTradelane,(iClientID,p2,p3,p4));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.StopTradelane(iClientID, p2, p3, p4));
	StopTradelane_AFTER(iClientID, p2, p3, p4);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall AbortMission_AFTER(unsigned int p1, unsigned int p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_AbortMission,(p1,p2));
}

void __stdcall AbortMission(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_AbortMission,(p1,p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.AbortMission(p1, p2));
	AbortMission_AFTER(p1, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall AcceptTrade_AFTER(unsigned int iClientID, bool p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_AcceptTrade,(iClientID,p2));
}

void __stdcall AcceptTrade(unsigned int iClientID, bool p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_AcceptTrade,(iClientID,p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.AcceptTrade(iClientID, p2));
	AcceptTrade_AFTER(iClientID, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall AddTradeEquip_AFTER(unsigned int iClientID, struct EquipDesc const &ed)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_AddTradeEquip,(iClientID,ed));
}

void __stdcall AddTradeEquip(unsigned int iClientID, struct EquipDesc const &ed)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_AddTradeEquip,(iClientID,ed));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.AddTradeEquip(iClientID, ed));
	AddTradeEquip_AFTER(iClientID, ed);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall BaseInfoRequest_AFTER(unsigned int p1, unsigned int p2, bool p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_BaseInfoRequest,(p1, p2, p3));
}

void __stdcall BaseInfoRequest(unsigned int p1, unsigned int p2, bool p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_BaseInfoRequest,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.BaseInfoRequest(p1, p2, p3));
	BaseInfoRequest_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall CharacterSkipAutosave(unsigned int iClientID)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	EXECUTE_SERVER_CALL(Server.CharacterSkipAutosave(iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall CommComplete(unsigned int p1, unsigned int p2, unsigned int p3,enum CommResult cr)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(cr);

	EXECUTE_SERVER_CALL(Server.CommComplete(p1, p2, p3, cr));

}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall CreateNewCharacter_AFTER(struct SCreateCharacterInfo const & scci, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_CreateNewCharacter,(scci, iClientID));
}

void __stdcall CreateNewCharacter(struct SCreateCharacterInfo const & scci, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_CreateNewCharacter,(scci, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.CreateNewCharacter(scci, iClientID));
	CreateNewCharacter_AFTER(scci, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall DelTradeEquip_AFTER(unsigned int iClientID, struct EquipDesc const &ed)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_DelTradeEquip,(iClientID, ed));
}

void __stdcall DelTradeEquip(unsigned int iClientID, struct EquipDesc const &ed)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_DelTradeEquip,(iClientID, ed));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.DelTradeEquip(iClientID, ed));
	DelTradeEquip_AFTER(iClientID, ed);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall DestroyCharacter_AFTER(struct CHARACTER_ID const &cId, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_DestroyCharacter,(cId, iClientID));
}

void __stdcall DestroyCharacter(struct CHARACTER_ID const &cId, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_S(&cId);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_DestroyCharacter,(cId, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.DestroyCharacter(cId, iClientID));
	DestroyCharacter_AFTER(cId, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall Dock(unsigned int const &p1, unsigned int const &p2)
{
	// anticheat - never let the client manually dock somewhere
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall DumpPacketStats(char const *p1)
{
	return; // not used

	ISERVER_LOG();

	EXECUTE_SERVER_CALL(Server.DumpPacketStats(p1));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ElapseTime(float p1)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_F(p1);

	EXECUTE_SERVER_CALL(Server.ElapseTime(p1));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GFGoodBuy_AFTER(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFGoodBuy,(gbi, iClientID));
}

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFGoodBuy,(gbi, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.GFGoodBuy(gbi, iClientID));
	GFGoodBuy_AFTER(gbi, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GFGoodVaporized_AFTER(struct SGFGoodVaporizedInfo const &gvi, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFGoodVaporized,(gvi, iClientID));
}

void __stdcall GFGoodVaporized(struct SGFGoodVaporizedInfo const &gvi, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFGoodVaporized,(gvi, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.GFGoodVaporized(gvi, iClientID));
	GFGoodVaporized_AFTER(gvi, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GFObjSelect_AFTER(unsigned int p1, unsigned int p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFObjSelect,(p1, p2));
}

void __stdcall GFObjSelect(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_GFObjSelect,(p1, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.GFObjSelect(p1, p2));
	GFObjSelect_AFTER(p1, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

unsigned int __stdcall GetServerID(void)
{
	ISERVER_LOG();

	return Server.GetServerID();
}

/**************************************************************************************************************
**************************************************************************************************************/

char const * __stdcall GetServerSig(void)
{
	ISERVER_LOG();

	return Server.GetServerSig();
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GetServerStats(struct ServerStats &ss)
{
	ISERVER_LOG();

	Server.GetServerStats(ss);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall Hail_AFTER(unsigned int p1, unsigned int p2, unsigned int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_Hail,(p1, p2, p3));
}

void __stdcall Hail(unsigned int p1, unsigned int p2, unsigned int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_Hail,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.Hail(p1, p2, p3));
	Hail_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall InterfaceItemUsed_AFTER(unsigned int p1, unsigned int p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_InterfaceItemUsed,(p1, p2));
}

void __stdcall InterfaceItemUsed(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_InterfaceItemUsed,(p1, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.InterfaceItemUsed(p1, p2));
	InterfaceItemUsed_AFTER(p1, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall JettisonCargo_AFTER(unsigned int iClientID, struct XJettisonCargo const &jc)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_JettisonCargo,(iClientID, jc));
}

void __stdcall JettisonCargo(unsigned int iClientID, struct XJettisonCargo const &jc)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_JettisonCargo,(iClientID, jc));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.JettisonCargo(iClientID, jc));
	JettisonCargo_AFTER(iClientID, jc);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall LocationEnter_AFTER(unsigned int p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_LocationEnter,(p1, iClientID));
}

void __stdcall LocationEnter(unsigned int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_LocationEnter,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.LocationEnter(p1, iClientID));
	LocationEnter_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall LocationExit_AFTER(unsigned int p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_LocationExit,(p1, iClientID));
}

void __stdcall LocationExit(unsigned int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_LocationExit,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.LocationExit(p1, iClientID));
	LocationExit_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall LocationInfoRequest_AFTER(unsigned int p1,unsigned int p2, bool p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_LocationInfoRequest,(p1, p2, p3));
}

void __stdcall LocationInfoRequest(unsigned int p1,unsigned int p2, bool p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_LocationInfoRequest,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.LocationInfoRequest(p1, p2, p3));
	LocationInfoRequest_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall MissionResponse_AFTER(unsigned int p1, unsigned long p2, bool p3, unsigned int p4)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_MissionResponse,(p1, p2, p3, p4));
}

void __stdcall MissionResponse(unsigned int p1, unsigned long p2, bool p3, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_MissionResponse,(p1, p2, p3, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.MissionResponse(p1, p2, p3, iClientID));
	MissionResponse_AFTER(p1, p2, p3, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/


void __stdcall MissionSaveB(unsigned int iClientID, unsigned long p2)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	EXECUTE_SERVER_CALL(Server.MissionSaveB(iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall PopUpDialog(unsigned int p1, unsigned int p2)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	EXECUTE_SERVER_CALL(Server.PopUpDialog(p1, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RTCDone(unsigned int p1, unsigned int p2)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	EXECUTE_SERVER_CALL(Server.RTCDone(p1, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqAddItem_AFTER(unsigned int p1, char const *p2, int p3, float p4, bool p5, unsigned int p6)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqAddItem,(p1, p2, p3, p4, p5, p6));
}

void __stdcall ReqAddItem(unsigned int p1, char const *p2, int p3, float p4, bool p5, unsigned int p6)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_F(p4);
	ISERVER_LOGARG_UI(p5);
	ISERVER_LOGARG_UI(p6);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqAddItem,(p1, p2, p3, p4, p5, p6));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqAddItem(p1, p2, p3, p4, p5, p6));
	ReqAddItem_AFTER(p1, p2, p3, p4, p5, p6);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqChangeCash_AFTER(int p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqChangeCash,(p1, iClientID));
}

void __stdcall ReqChangeCash(int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqChangeCash,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqChangeCash(p1, iClientID));
	ReqChangeCash_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqCollisionGroups_AFTER(class std::list<struct CollisionGroupDesc,class std::allocator<struct CollisionGroupDesc> > const &p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqCollisionGroups,(p1, iClientID));
}

void __stdcall ReqCollisionGroups(class std::list<struct CollisionGroupDesc,class std::allocator<struct CollisionGroupDesc> > const &p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqCollisionGroups,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqCollisionGroups(p1, iClientID));
	ReqCollisionGroups_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqDifficultyScale(float p1, unsigned int iClientID)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_F(p1);
	ISERVER_LOGARG_UI(iClientID);

	EXECUTE_SERVER_CALL(Server.ReqDifficultyScale(p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqEquipment_AFTER(class EquipDescList const &edl, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqEquipment,(edl, iClientID));
}

void __stdcall ReqEquipment(class EquipDescList const &edl, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqEquipment,(edl, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqEquipment(edl, iClientID));
	ReqEquipment_AFTER(edl, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqHullStatus_AFTER(float p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqHullStatus,(p1, iClientID));
}

void __stdcall ReqHullStatus(float p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_F(p1);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqHullStatus,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqHullStatus(p1, iClientID));
	ReqHullStatus_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqModifyItem_AFTER(unsigned short p1, char const *p2, int p3, float p4, bool p5, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqModifyItem,(p1, p2, p3, p4, p5, iClientID));
}

void __stdcall ReqModifyItem(unsigned short p1, char const *p2, int p3, float p4, bool p5, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);
	ISERVER_LOGARG_F(p4);
	ISERVER_LOGARG_UI(p5);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqModifyItem,(p1, p2, p3, p4, p5, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqModifyItem(p1, p2, p3, p4, p5, iClientID));
	ReqModifyItem_AFTER(p1, p2, p3, p4, p5, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqRemoveItem_AFTER(unsigned short p1, int p2, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqRemoveItem,(p1, p2, iClientID));
}

void __stdcall ReqRemoveItem(unsigned short p1, int p2, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_I(p2);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqRemoveItem,(p1, p2, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqRemoveItem(p1, p2, iClientID));
	ReqRemoveItem_AFTER(p1, p2, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqSetCash_AFTER(int p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqSetCash,(p1, iClientID));
}

void __stdcall ReqSetCash(int p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_I(p1);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqSetCash,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqSetCash(p1, iClientID));
	ReqSetCash_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqShipArch_AFTER(unsigned int p1, unsigned int p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqShipArch,(p1, p2));
}

void __stdcall ReqShipArch(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_ReqShipArch,(p1, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.ReqShipArch(p1, p2));
	ReqShipArch_AFTER(p1, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestBestPath_AFTER(unsigned int p1, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestBestPath,(p1, p2, p3));
}

void __stdcall RequestBestPath(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestBestPath,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestBestPath(p1, p2, p3));
	RequestBestPath_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestCancel_AFTER(int iType, unsigned int iShip, unsigned int p3, unsigned long p4, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestCancel,(iType, iShip, p3, p4, iClientID));
}

// Cancel a ship maneuver (goto, dock, formation).
// p1 = iType? ==0 if docking, ==1 if formation
void __stdcall RequestCancel(int iType, unsigned int iShip, unsigned int p3, unsigned long p4, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_I(iType);
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(p4);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestCancel,(iType, iShip, p3, p4, iClientID));
	if(bPluginReturn)
		return;

	try
	{
		Server.RequestCancel(iType, iShip, p3, p4, iClientID);
	}
	catch(...)
	{
		AddLog("Exception in %s(a) iType=%u iShip=%u p3=%u p4=%u iClientID=%u",
			__FUNCTION__, iType, iShip, p3, p4, iClientID);
	}

	//EXECUTE_SERVER_CALL(Server.RequestCancel(iType, iShip, p3, p4, p5iClientID);
	RequestCancel_AFTER(iType, iShip, p3, p4, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestCreateShip_AFTER(unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestCreateShip,(iClientID));
}

void __stdcall RequestCreateShip(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestCreateShip,(iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestCreateShip(iClientID));
	RequestCreateShip_AFTER(iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestEvent_AFTER(int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned long p5, unsigned int p6)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestEvent,(p1, p2, p3, p4, p5, p6));
}

/// Called upon flight maneuver (goto, dock, formation).
/// p1 = iType? ==0 if docking, ==1 if formation
/// p2 = iShip of person docking
/// p3 = iShip of dock/formation target
/// p4 seems to be 0 all the time
/// p5 seems to be 0 all the time
/// p6 = iClientID
void __stdcall RequestEvent(int iType, unsigned int iShip, unsigned int iShipTarget, unsigned int p4, unsigned long p5, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_I(iType);
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(iShipTarget);
	ISERVER_LOGARG_UI(p4);
	ISERVER_LOGARG_UI(p5);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestEvent,(iType, iShip, iShipTarget, p4, p5, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestEvent(iType, iShip, iShipTarget, p4, p5, iClientID));
	RequestEvent_AFTER(iType, iShip, iShipTarget, p4, p5, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestGroupPositions_AFTER(unsigned int p1, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestGroupPositions,(p1, p2, p3));
}

void __stdcall RequestGroupPositions(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestGroupPositions,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestGroupPositions(p1, p2, p3));
	RequestGroupPositions_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestPlayerStats_AFTER(unsigned int p1, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestPlayerStats,(p1, p2, p3));
}

void __stdcall RequestPlayerStats(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestPlayerStats,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestPlayerStats(p1, p2, p3));
	RequestPlayerStats_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestRankLevel_AFTER(unsigned int p1, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestRankLevel,(p1, p2, p3));
}

void __stdcall RequestRankLevel(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestRankLevel,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestRankLevel(p1, p2, p3));
	RequestRankLevel_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestTrade_AFTER(unsigned int p1, unsigned int p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestTrade,(p1, p2));
}

void __stdcall RequestTrade(unsigned int p1, unsigned int p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_RequestTrade,(p1, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.RequestTrade(p1, p2));
	RequestTrade_AFTER(p1, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPBadLandsObjCollision(struct SSPBadLandsObjCollisionInfo const &p1, unsigned int iClientID)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	EXECUTE_SERVER_CALL(Server.SPBadLandsObjCollision(p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPRequestInvincibility_AFTER(unsigned int iShip, bool p2, enum InvincibilityReason p3, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPRequestInvincibility,(iShip, p2, p3, iClientID));
}

/// Called when ship starts jump gate/hole acceleration but before system switch out.
void __stdcall SPRequestInvincibility(unsigned int iShip, bool p2, enum InvincibilityReason p3, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iShip);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPRequestInvincibility,(iShip, p2, p3, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SPRequestInvincibility(iShip, p2, p3, iClientID));
	SPRequestInvincibility_AFTER(iShip, p2, p3, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPRequestUseItem_AFTER(struct SSPUseItem const &p1, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPRequestUseItem,(p1, iClientID));
}

void __stdcall SPRequestUseItem(struct SSPUseItem const &p1, unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPRequestUseItem,(p1, iClientID));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SPRequestUseItem(p1, iClientID));
	SPRequestUseItem_AFTER(p1, iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPScanCargo_AFTER(unsigned int const &p1, unsigned int const &p2, unsigned int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPScanCargo,(p1, p2, p3));
}

void __stdcall SPScanCargo(unsigned int const &p1, unsigned int const &p2, unsigned int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
	ISERVER_LOGARG_UI(p2);
	ISERVER_LOGARG_UI(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SPScanCargo,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SPScanCargo(p1, p2, p3));
	SPScanCargo_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SaveGame(struct CHARACTER_ID const &cId, unsigned short const *p2, unsigned int p3)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_S(&cId);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_UI(p3);

	Server.SaveGame(cId, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetInterfaceState_AFTER(unsigned int p1, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetInterfaceState,(p1, p2, p3));
}

void __stdcall SetInterfaceState(unsigned int p1, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(p1);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetInterfaceState,(p1, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SetInterfaceState(p1, p2, p3));
	SetInterfaceState_AFTER(p1, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetManeuver_AFTER(unsigned int iClientID, struct XSetManeuver const &p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetManeuver,(iClientID, p2));
}

void __stdcall SetManeuver(unsigned int iClientID, struct XSetManeuver const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetManeuver,(iClientID, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SetManeuver(iClientID, p2));
	SetManeuver_AFTER(iClientID, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetMissionLog(unsigned int iClientID, unsigned char *p2, int p3)
{
	return; // not used

	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	Server.SetMissionLog(iClientID, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetTarget_AFTER(unsigned int iClientID, struct XSetTarget const &p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetTarget,(iClientID, p2));
}

void __stdcall SetTarget(unsigned int iClientID, struct XSetTarget const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetTarget,(iClientID, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SetTarget(iClientID, p2));
	SetTarget_AFTER(iClientID, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetTradeMoney_AFTER(unsigned int iClientID, unsigned long p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetTradeMoney,(iClientID, p2));
}

void __stdcall SetTradeMoney(unsigned int iClientID, unsigned long p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
	ISERVER_LOGARG_UI(p2);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetTradeMoney,(iClientID, p2));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SetTradeMoney(iClientID, p2));
	SetTradeMoney_AFTER(iClientID, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetVisitedState_AFTER(unsigned int iClientID, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetVisitedState,(iClientID, p2, p3));
}

void __stdcall SetVisitedState(unsigned int iClientID, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetVisitedState,(iClientID, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SetVisitedState(iClientID, p2, p3));
	SetVisitedState_AFTER(iClientID, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetWeaponGroup_AFTER(unsigned int iClientID, unsigned char *p2, int p3)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetWeaponGroup,(iClientID, p2, p3));
}

void __stdcall SetWeaponGroup(unsigned int iClientID, unsigned char *p2, int p3)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);
//	ISERVER_LOGARG_S(p2);
	ISERVER_LOGARG_I(p3);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_SetWeaponGroup,(iClientID, p2, p3));
	if(bPluginReturn)
		return;

	EXECUTE_SERVER_CALL(Server.SetWeaponGroup(iClientID, p2, p3));
	SetWeaponGroup_AFTER(iClientID, p2, p3);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall Shutdown(void)
{
	ISERVER_LOG();

	CALL_PLUGINS(PLUGIN_HkIServerImpl_Shutdown,());

	Server.Shutdown();

	FLHookShutdown();
}

/**************************************************************************************************************
**************************************************************************************************************/

bool __stdcall Startup_AFTER(struct SStartupInfo const &p1)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_Startup,(p1));
	return true;
}

bool __stdcall Startup(struct SStartupInfo const &p1)
{
	FLHookInit_Pre();

	// patch PlayerDB array (for rename feature)
	char* pAddress = ((char*)hModServer + ADDR_SRV_PLAYERDBMAXPLAYERSPATCH);
	char szNOP[] = { '\x90'};
	char szMOVECX[] = { '\xB9'};
	WriteProcMem(pAddress, szMOVECX, sizeof(szMOVECX));
	int iMaxPlayers = p1.iMaxPlayers+1; // max + 1
	WriteProcMem(pAddress+1, &iMaxPlayers, sizeof(iMaxPlayers));
	WriteProcMem(pAddress+5, szNOP, sizeof(szNOP));

	// plugins & functioncall
	bool bRet;
	CALL_PLUGINS(PLUGIN_HkIServerImpl_Startup,(p1));
	if(bPluginReturn)
		bRet = reinterpret_cast<bool>(vPluginRet);
	else
		bRet = Server.Startup(p1);

	// patch PlayerDB array (for rename feature)
	iMaxPlayers = p1.iMaxPlayers;
	pAddress = ((char*)hModServer + ADDR_SRV_PLAYERDBMAXPLAYERS);
	WriteProcMem(pAddress, &iMaxPlayers, sizeof(iMaxPlayers));

	// read base market data from ini
	HkLoadBaseMarket();

	ISERVER_LOG();

	Startup_AFTER(p1);

	return bRet;
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall StopTradeRequest_AFTER(unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_StopTradeRequest,(iClientID));
}

void __stdcall StopTradeRequest(unsigned int iClientID)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_StopTradeRequest,(iClientID));
	if(bPluginReturn)
		return;

	Server.StopTradeRequest(iClientID);
	StopTradeRequest_AFTER(iClientID);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall TractorObjects_AFTER(unsigned int iClientID, struct XTractorObjects const &p2)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_TractorObjects,(iClientID, p2));
}

void __stdcall TractorObjects(unsigned int iClientID, struct XTractorObjects const &p2)
{
	ISERVER_LOG();
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_TractorObjects,(iClientID, p2));
	if(bPluginReturn)
		return;

	Server.TractorObjects(iClientID, p2);
	TractorObjects_AFTER(iClientID, p2);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall TradeResponse_AFTER(unsigned char const *p1, int p2, unsigned int iClientID)
{
	CALL_PLUGINS(PLUGIN_HkIServerImpl_TradeResponse,(p1, p2, iClientID));
}

void __stdcall TradeResponse(unsigned char const *p1, int p2, unsigned int iClientID)
{
	ISERVER_LOG();
///	ISERVER_LOGARG_S(p1);
	ISERVER_LOGARG_I(p2);
	ISERVER_LOGARG_UI(iClientID);

	CALL_PLUGINS(PLUGIN_HkIServerImpl_TradeResponse,(p1, p2, iClientID));
	if(bPluginReturn)
		return;

	Server.TradeResponse(p1, p2, iClientID);
	TradeResponse_AFTER(p1, p2, iClientID);
}

/**************************************************************************************************************
IServImpl hook entries
**************************************************************************************************************/

	HOOKENTRY hookEntries[85] =
	{
		{(FARPROC)SubmitChat,				-0x08, 0},
		{(FARPROC)FireWeapon,				0x000, 0},
		{(FARPROC)ActivateEquip,			0x004, 0},
		{(FARPROC)ActivateCruise,			0x008, 0},
		{(FARPROC)ActivateThrusters,		0x00C, 0},
		{(FARPROC)SetTarget,				0x010, 0},
		{(FARPROC)TractorObjects,			0x014, 0},
		{(FARPROC)GoTradelane,				0x018, 0},
		{(FARPROC)StopTradelane,			0x01C, 0},
		{(FARPROC)JettisonCargo,			0x020, 0},
		{(FARPROC)ElapseTime,				0x030, 0},
		{(FARPROC)DisConnect,				0x040, 0},
		{(FARPROC)OnConnect,				0x044, 0},
		{(FARPROC)Login,					0x048, 0},
		{(FARPROC)CharacterInfoReq,			0x04C, 0},
		{(FARPROC)CharacterSelect,			0x050, 0},
		{(FARPROC)CreateNewCharacter,		0x058, 0},
		{(FARPROC)DestroyCharacter,			0x05C, 0},
		{(FARPROC)CharacterSkipAutosave,	0x060, 0},
		{(FARPROC)ReqShipArch,				0x064, 0},
		{(FARPROC)ReqHullStatus,			0x068, 0},
		{(FARPROC)ReqCollisionGroups,		0x06C, 0},
		{(FARPROC)ReqEquipment,				0x070, 0},
		{(FARPROC)ReqAddItem,				0x078, 0},
		{(FARPROC)ReqRemoveItem,			0x07C, 0},
		{(FARPROC)ReqModifyItem,			0x080, 0},
		{(FARPROC)ReqSetCash,				0x084, 0},
		{(FARPROC)ReqChangeCash,			0x088, 0},
		{(FARPROC)BaseEnter,				0x08C, 0},
		{(FARPROC)BaseExit,					0x090, 0},
		{(FARPROC)LocationEnter,			0x094, 0},
		{(FARPROC)LocationExit,				0x098, 0},
		{(FARPROC)BaseInfoRequest,			0x09C, 0},
		{(FARPROC)LocationInfoRequest,		0x0A0, 0},
		{(FARPROC)GFObjSelect,				0x0A4, 0},
		{(FARPROC)GFGoodVaporized,			0x0A8, 0},
		{(FARPROC)MissionResponse,			0x0AC, 0},
		{(FARPROC)TradeResponse,			0x0B0, 0},
		{(FARPROC)GFGoodBuy,				0x0B4, 0},
		{(FARPROC)GFGoodSell,				0x0B8, 0},
		{(FARPROC)SystemSwitchOutComplete,	0x0BC, 0},
		{(FARPROC)PlayerLaunch,				0x0C0, 0},
		{(FARPROC)LaunchComplete,			0x0C4, 0},
		{(FARPROC)JumpInComplete,			0x0C8, 0},
		{(FARPROC)Hail,						0x0CC, 0},
		{(FARPROC)SPObjUpdate,				0x0D0, 0},
		{(FARPROC)SPMunitionCollision,		0x0D4, 0},
		{(FARPROC)SPBadLandsObjCollision,	0x0D8, 0},
		{(FARPROC)SPObjCollision,			0x0DC, 0},
		{(FARPROC)SPRequestUseItem,			0x0E0, 0},
		{(FARPROC)SPRequestInvincibility,	0x0E4, 0},
		{(FARPROC)SaveGame,					0x0E8, 0},
		{(FARPROC)MissionSaveB,				0x0EC, 0},
		{(FARPROC)RequestEvent,				0x0F0, 0},
		{(FARPROC)RequestCancel,			0x0F4, 0},
		{(FARPROC)MineAsteroid,				0x0F8, 0},
		{(FARPROC)CommComplete,				0x0FC, 0},
		{(FARPROC)RequestCreateShip,		0x100, 0},
		{(FARPROC)SPScanCargo,				0x104, 0},
		{(FARPROC)SetManeuver,				0x108, 0},
		{(FARPROC)InterfaceItemUsed,		0x10C, 0},
		{(FARPROC)AbortMission,				0x110, 0},
		{(FARPROC)RTCDone,					0x114, 0},
		{(FARPROC)SetWeaponGroup,			0x118, 0},
		{(FARPROC)SetVisitedState,			0x11C, 0},
		{(FARPROC)RequestBestPath,			0x120, 0},
		{(FARPROC)RequestPlayerStats,		0x124, 0},
		{(FARPROC)PopUpDialog,				0x128, 0},
		{(FARPROC)RequestGroupPositions,	0x12C, 0},
		{(FARPROC)SetMissionLog,			0x130, 0},
		{(FARPROC)SetInterfaceState,		0x134, 0},
		{(FARPROC)RequestRankLevel,			0x138, 0},
		{(FARPROC)InitiateTrade,			0x13C, 0},
		{(FARPROC)TerminateTrade,			0x140, 0},
		{(FARPROC)AcceptTrade,				0x144, 0},
		{(FARPROC)SetTradeMoney,			0x148, 0},
		{(FARPROC)AddTradeEquip,			0x14C, 0},
		{(FARPROC)DelTradeEquip,			0x150, 0},
		{(FARPROC)RequestTrade,				0x154, 0},
		{(FARPROC)StopTradeRequest,			0x158, 0},
		{(FARPROC)ReqDifficultyScale,		0x15C, 0},
		{(FARPROC)GetServerID,				0x160, 0},
		{(FARPROC)GetServerSig,				0x164, 0},
		{(FARPROC)DumpPacketStats,			0x168, 0},
		{(FARPROC)Dock,						0x16C, 0},
	};

}
