#include "global.h"
#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setting variables

string			set_scCfgFile;

// General
uint			set_iAntiDockKill;
bool			set_bDieMsg;
bool			set_bDisableCharfileEncryption;
bool			set_bChangeCruiseDisruptorBehaviour;
uint			set_iAntiF1;
uint			set_iDisconnectDelay;
uint			set_iReservedSlots;
float			set_fTorpMissileBaseDamageMultiplier;
uint			set_iMaxGroupSize;
uint			set_iDisableNPCSpawns;

// log
bool			set_bDebug;
uint			set_iDebugMaxSize;
bool			set_bLogConnects;
bool			set_bLogAdminCmds;
bool			set_bLogUserCmds;
bool			set_bPerfTimer;
uint			set_iTimerThreshold;
uint			set_iTimerDebugThreshold;

// Kick
uint			set_iAntiBaseIdle;
uint			set_iAntiCharMenuIdle;



// Style
wstring			set_wscDeathMsgStyle;
wstring			set_wscDeathMsgStyleSys;
wstring			set_wscDeathMsgTextPlayerKill;
wstring			set_wscDeathMsgTextSelfKill;
wstring			set_wscDeathMsgTextNPC;
wstring			set_wscDeathMsgTextSuicide;
wstring			set_wscDeathMsgTextAdminKill;

uint			set_iKickMsgPeriod;
wstring			set_wscKickMsg;
wstring			set_wscUserCmdStyle;
wstring			set_wscAdminCmdStyle;

// Socket
bool			set_bSocketActivated;
int				set_iPort;
int				set_iWPort;
int				set_iEPort;
int				set_iEWPort;
BLOWFISH_CTX	*set_BF_CTX = 0;

// UserCommands
bool			set_bUserCmdSetDieMsg;
bool			set_bUserCmdSetDieMsgSize;
bool			set_bUserCmdSetChatFont;
bool			set_bUserCmdIgnore;
uint			set_iUserCmdMaxIgnoreList;
bool			set_bAutoBuy;
bool			set_bUserCmdHelp;

// NoPVP
list<uint>		set_lstNoPVPSystems;

// Chat
list<wstring>	set_lstChatSuppress;

// MultiKillMessages
bool			set_MKM_bActivated;
wstring			set_MKM_wscStyle;
list<MULTIKILLMESSAGE> set_MKM_lstMessages;

// bans
bool			set_bBanAccountOnMatch;
list<wstring>	set_lstBans;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings()
{
// init cfg filename
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	set_scCfgFile = string(szCurDir) + "\\FLHook.ini";

// General
	set_iAntiDockKill = IniGetI(set_scCfgFile, "General", "AntiDockKill", 0);
	set_bDieMsg = IniGetB(set_scCfgFile, "General", "EnableDieMsg", false);
	set_bDisableCharfileEncryption = IniGetB(set_scCfgFile, "General", "DisableCharfileEncryption", false);
	set_bChangeCruiseDisruptorBehaviour = IniGetB(set_scCfgFile, "General", "ChangeCruiseDisruptorBehaviour", false);
	set_iDisableNPCSpawns = IniGetI(set_scCfgFile, "General", "DisableNPCSpawns", 0);
	set_iAntiF1 = IniGetI(set_scCfgFile, "General", "AntiF1", 0);
	set_iDisconnectDelay = IniGetI(set_scCfgFile, "General", "DisconnectDelay", 0);
	set_iReservedSlots = IniGetI(set_scCfgFile, "General", "ReservedSlots", 0);
	set_fTorpMissileBaseDamageMultiplier = IniGetF(set_scCfgFile, "General", "TorpMissileBaseDamageMultiplier", 1.0f);
	set_iMaxGroupSize = IniGetI(set_scCfgFile, "General", "MaxGroupSize", 8);

// Log
	set_bDebug = IniGetB(set_scCfgFile, "Log", "Debug", false);
	set_iDebugMaxSize = IniGetI(set_scCfgFile, "Log", "DebugMaxSize", 100);
	set_iDebugMaxSize *= 1000;
	set_bLogConnects = IniGetB(set_scCfgFile, "Log", "LogConnects", false);
	set_bLogAdminCmds = IniGetB(set_scCfgFile, "Log", "LogAdminCommands", false);
	set_bLogUserCmds = IniGetB(set_scCfgFile, "Log", "LogUserCommands", false);
	set_bPerfTimer = IniGetB(set_scCfgFile, "Log", "LogPerformanceTimers", false);
	set_iTimerThreshold = IniGetI(set_scCfgFile, "Log", "TimerThreshold", 100);
	set_iTimerDebugThreshold = IniGetI(set_scCfgFile, "Log", "TimerDebugThreshold", 0);

// Kick
	set_iAntiBaseIdle = IniGetI(set_scCfgFile, "Kick", "AntiBaseIdle", 0);
	set_iAntiCharMenuIdle = IniGetI(set_scCfgFile, "Kick", "AntiCharMenuIdle", 0);


// Style
	set_wscDeathMsgStyle = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgStyle", "0x19198C01"));
	set_wscDeathMsgStyleSys = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgStyleSys", "0x1919BD01"));
	set_wscDeathMsgTextPlayerKill = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextPlayerKill", "Death: %victim was killed by %killer (%type)"));
	set_wscDeathMsgTextSelfKill = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextSelfKill", "Death: %victim killed himself (%type)"));
	set_wscDeathMsgTextNPC = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextNPC", "Death: %victim was killed by an NPC"));
	set_wscDeathMsgTextSuicide = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextSuicide", "Death: %victim committed suicide"));
	set_wscDeathMsgTextAdminKill = stows(IniGetS(set_scCfgFile, "Style", "DeathMsgTextAdminKill", "Death: %victim was killed by an admin"));

	set_wscKickMsg = stows(IniGetS(set_scCfgFile, "Style", "KickMsg", "<TRA data=\"0x0000FF10\" mask=\"-1\"/><TEXT>You will be kicked. Reason: %s</TEXT>"));
	set_iKickMsgPeriod = IniGetI(set_scCfgFile, "Style", "KickMsgPeriod", 5000);
	set_wscUserCmdStyle = stows(IniGetS(set_scCfgFile, "Style", "UserCmdStyle", "0x00FF0090"));
	set_wscAdminCmdStyle = stows(IniGetS(set_scCfgFile, "Style", "AdminCmdStyle", "0x00FF0090"));

// Socket
	set_bSocketActivated = IniGetB(set_scCfgFile, "Socket", "Activated", false);
	set_iPort = IniGetI(set_scCfgFile, "Socket", "Port", 0);
	set_iWPort = IniGetI(set_scCfgFile, "Socket", "WPort", 0);
	set_iEPort = IniGetI(set_scCfgFile, "Socket", "EPort", 0);
	set_iEWPort = IniGetI(set_scCfgFile, "Socket", "EWPort", 0);
	string scEncryptKey = IniGetS(set_scCfgFile, "Socket", "Key", "");
	if(scEncryptKey.length())
	{
		if(!set_BF_CTX)
			set_BF_CTX = (BLOWFISH_CTX*)malloc(sizeof(BLOWFISH_CTX));
		Blowfish_Init(set_BF_CTX, (unsigned char *)scEncryptKey.data(), (int)scEncryptKey.length());
	}

// UserCommands
	set_bUserCmdSetDieMsg = IniGetB(set_scCfgFile, "UserCommands", "SetDieMsg", false);
	set_bUserCmdSetDieMsgSize = IniGetB(set_scCfgFile, "UserCommands", "SetDieMsgSize", false);
	set_bUserCmdSetChatFont = IniGetB(set_scCfgFile, "UserCommands", "SetChatFont", false);
	set_bUserCmdIgnore = IniGetB(set_scCfgFile, "UserCommands", "Ignore", false);
	set_iUserCmdMaxIgnoreList = IniGetI(set_scCfgFile, "UserCommands", "MaxIgnoreListEntries", 30);
	set_bAutoBuy = IniGetB(set_scCfgFile, "UserCommands", "AutoBuy", false);
	set_bUserCmdHelp = IniGetB(set_scCfgFile, "UserCommands", "Help", false);

// NoPVP
	set_lstNoPVPSystems.clear();
	for(uint i = 0;; i++)
	{
		char szBuf[64];
		sprintf(szBuf, "System%u", i);
		string scSystem = IniGetS(set_scCfgFile, "NoPVP", szBuf, "");

		if(!scSystem.length())
			break;

		uint iSystemID;
		pub::GetSystemID(iSystemID, scSystem.c_str());
		set_lstNoPVPSystems.push_back(iSystemID);
	}

// read chat suppress
	set_lstChatSuppress.clear();
	for(uint i = 0;; i++)
	{
		char szBuf[64];
		sprintf(szBuf, "Suppress%u", i);
		string scSuppress = IniGetS(set_scCfgFile, "Chat", szBuf, "");

		if(!scSuppress.length())
			break;

		set_lstChatSuppress.push_back(stows(scSuppress));
	}

// MultiKillMessages
	set_MKM_bActivated = IniGetB(set_scCfgFile, "MultiKillMessages", "Activated", false);
	set_MKM_wscStyle = stows(IniGetS(set_scCfgFile, "MultiKillMessages", "Style", "0x1919BD01"));

	set_MKM_lstMessages.clear();
	list<INISECTIONVALUE> lstValues;
	IniGetSection(set_scCfgFile, "MultiKillMessages", lstValues);
	foreach(lstValues, INISECTIONVALUE, it)
	{
		if(!atoi(it->scKey.c_str()))
			continue;

		MULTIKILLMESSAGE mkm;
		mkm.iKillsInARow = atoi(it->scKey.c_str());
		mkm.wscMessage = stows(it->scValue);
		set_MKM_lstMessages.push_back(mkm);
	}

// bans
	set_bBanAccountOnMatch = IniGetB(set_scCfgFile, "Bans", "BanAccountOnMatch", false);
	set_lstBans.clear();
	IniGetSection(set_scCfgFile, "Bans", lstValues);
	if(!lstValues.empty())
	{
		lstValues.pop_front();
		foreach(lstValues, INISECTIONVALUE, itisv)
			set_lstBans.push_back(stows(itisv->scKey));
	}

	CALL_PLUGINS(PLUGIN_LoadSettings,());
}
