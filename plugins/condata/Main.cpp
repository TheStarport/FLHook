// includes 
#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include "../../sdk/headers/FLHook.h"
#include "../../sdk/headers/plugin.h"
#include "header.h"
#include <math.h>

#define PRINT_ERROR() { for(uint i = 0; (i < sizeof(wscError)/sizeof(wstring)); i++) PrintUserCmdText(iClientID, wscError[i]); return; }
#define PRINT_OK() PrintUserCmdText(iClientID, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");

CONNECTION_DATA ConData[250];
bool set_bPingCmd;


PLUGIN_RETURNCODE returncode;
list<PLUGIN_INFO> *lstPluginInfo = new list<PLUGIN_INFO>();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT list<PLUGIN_INFO>* Get_PluginInfo()
{
	lstPluginInfo->clear();
	PLUGIN_INFO pi;
	pi.sName = "Advanced Connection Data Plugin by w0dk4";
	pi.sShortName = "condata";
	pi.bMayPause = false;
	pi.bMayUnload = true;
	pi.mapHooks.insert(pair<string, int>("ClearClientInfo", 0));
	pi.mapHooks.insert(pair<string, int>("LoadSettings", 0));
	pi.mapHooks.insert(pair<string, int>("HkTimerCheckKick", 0));
	pi.mapHooks.insert(pair<string, int>("HkIServerImpl::Update", 0));
	pi.mapHooks.insert(pair<string, int>("HkIServerImpl::SPObjUpdate", 0));
	pi.mapHooks.insert(pair<string, int>("HkIServerImpl::PlayerLaunch", 0));
	pi.mapHooks.insert(pair<string, int>("UserCmd_Process", 0));
	pi.mapHooks.insert(pair<string, int>("UserCmd_Help", 0));
	pi.mapHooks.insert(pair<string, int>("Plugin_Communication_CallBack", 0));
	pi.mapHooks.insert(pair<string, int>("ExecuteCommandString_Callback", 0));
	
	lstPluginInfo->push_back(pi);

	return lstPluginInfo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;
	
	set_iPingKickFrame = IniGetI(set_scCfgFile, "Kick", "PingKickFrame", 30);
	if(!set_iPingKickFrame) 
		set_iPingKickFrame = 60;
	set_iPingKick = IniGetI(set_scCfgFile, "Kick", "PingKick", 0);
	set_iFluctKick = IniGetI(set_scCfgFile, "Kick", "FluctKick", 0);
	set_iLossKickFrame = IniGetI(set_scCfgFile, "Kick", "LossKickFrame", 30);
	if(!set_iLossKickFrame) 
		set_iLossKickFrame = 60;
	set_iLossKick = IniGetI(set_scCfgFile, "Kick", "LossKick", 0);
	set_iLagDetectionFrame = IniGetI(set_scCfgFile, "Kick", "LagDetectionFrame", 50);
	set_iLagDetectionMinimum = IniGetI(set_scCfgFile, "Kick", "LagDetectionMinimum", 200);
	set_iLagKick = IniGetI(set_scCfgFile, "Kick", "LagKick", 0);

	set_bPingCmd = IniGetB(set_scCfgFile, "UserCommands", "Ping", false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

	if(fdwReason == DLL_PROCESS_ATTACH)
		LoadSettings();

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClearConData(uint iClientID) 
{
	ConData[iClientID].iAverageLoss = 0;
	ConData[iClientID].iAveragePing = 0;
	ConData[iClientID].iLastLoss = 0;
	ConData[iClientID].iLastPacketsDropped = 0;
	ConData[iClientID].iLastPacketsReceived = 0;
	ConData[iClientID].iLastPacketsSent = 0;
	ConData[iClientID].iPingFluctuation = 0;
	ConData[iClientID].lstLoss.clear();
	ConData[iClientID].lstPing.clear();
	ConData[iClientID].lstObjUpdateIntervalls.clear();
	ConData[iClientID].iLags = 0;
	ConData[iClientID].tmLastObjUpdate = 0;
	ConData[iClientID].tmLastObjTimestamp = 0;

	ConData[iClientID].bException = false;
	ConData[iClientID].sExceptionReason = "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void ClearClientInfo(uint iClientID)
{
	returncode = DEFAULT_RETURNCODE;

	ClearConData(iClientID);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void UserCmd_Help(uint iClientID, const wstring &wscParam)
{

	if(set_bPingCmd) {
		PrintUserCmdText(iClientID, L"/ping");
		PrintUserCmdText(iClientID, L"/pingtarget");
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void HkTimerCheckKick()
{
	returncode = DEFAULT_RETURNCODE;

	// for all players
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);

		
		if(set_iLossKick)
		{ // check if loss is too high
			if(ConData[iClientID].iAverageLoss > (set_iLossKick))
			{
				HkAddKickLog(iClientID, L"High loss");
				HkMsgAndKick(iClientID, L"High loss", set_iKickMsgPeriod);
				// call tempban plugin
				TEMPBAN_BAN_STRUCT tempban;
				tempban.iClientID = iClientID;
				tempban.iDuration = 1; // 1 minute
				Plugin_Communication(TEMPBAN_BAN,&tempban);
			}
		}

		if(set_iPingKick)
		{ // check if ping is too high
			if(ConData[iClientID].iAveragePing > (set_iPingKick))
			{
				HkAddKickLog(iClientID, L"High ping");
				HkMsgAndKick(iClientID, L"High ping", set_iKickMsgPeriod);
				// call tempban plugin
				TEMPBAN_BAN_STRUCT tempban;
				tempban.iClientID = iClientID;
				tempban.iDuration = 1; // 1 minute
				Plugin_Communication(TEMPBAN_BAN,&tempban);
			}
		}

		if(set_iFluctKick)
		{ // check if ping fluct is too high
			if(ConData[iClientID].iPingFluctuation > (set_iFluctKick))
			{
				HkAddKickLog(iClientID, L"High fluct");
				HkMsgAndKick(iClientID, L"High ping fluctuation", set_iKickMsgPeriod);
				// call tempban plugin
				TEMPBAN_BAN_STRUCT tempban;
				tempban.iClientID = iClientID;
				tempban.iDuration = 1; // 1 minute
				Plugin_Communication(TEMPBAN_BAN,&tempban);
			}
		}

		if(set_iLagKick)
		{ // check if lag is too high
			if(ConData[iClientID].iLags > (set_iLagKick))
			{
				HkAddKickLog(iClientID, L"High Lag");
				HkMsgAndKick(iClientID, L"High Lag", set_iKickMsgPeriod);
				// call tempban plugin
				TEMPBAN_BAN_STRUCT tempban;
				tempban.iClientID = iClientID;
				tempban.iDuration = 1; // 1 minute
				Plugin_Communication(TEMPBAN_BAN,&tempban);
			}				
		}


	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////


/**************************************************************************************************************
Update average ping data
**************************************************************************************************************/

void TimerUpdatePingData()
{

	// for all players
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		if(ClientInfo[iClientID].tmF1TimeDisconnect)
			continue;

		DPN_CONNECTION_INFO ci;
		if(HkGetConnectionStats(iClientID, ci) != HKE_OK)
			continue;

		///////////////////////////////////////////////////////////////
		// update ping data
		if(ConData[iClientID].lstPing.size() >= set_iPingKickFrame)
		{
			// calculate average ping and ping fluctuation
			unsigned int iLastPing = 0;
			ConData[iClientID].iAveragePing = 0;
			ConData[iClientID].iPingFluctuation = 0;
			foreach(ConData[iClientID].lstPing, uint, it) {
				ConData[iClientID].iAveragePing += (*it);
				if (iLastPing != 0) {
					ConData[iClientID].iPingFluctuation +=  (uint)sqrt((double)pow(((float)(*it) - (float)iLastPing),2));
				}
				iLastPing = (*it);
			}
			
			
			ConData[iClientID].iPingFluctuation /= (uint)ConData[iClientID].lstPing.size();
			ConData[iClientID].iAveragePing /= (uint)ConData[iClientID].lstPing.size();
		}

		// remove old pingdata
		while(ConData[iClientID].lstPing.size() >= set_iPingKickFrame)
			ConData[iClientID].lstPing.pop_back();

		ConData[iClientID].lstPing.push_front(ci.dwRoundTripLatencyMS);
	}
}

/**************************************************************************************************************
Update average loss data
**************************************************************************************************************/

void TimerUpdateLossData() 
{ 

      // for all players 
      float fLossPercentage; 
      uint iNewDrops; 
      uint iNewSent; 
      struct PlayerData *pPD = 0; 
      while(pPD = Players.traverse_active(pPD)) 
      { 
         uint iClientID = HkGetClientIdFromPD(pPD); 
         if(ClientInfo[iClientID].tmF1TimeDisconnect) 
            continue; 

         DPN_CONNECTION_INFO ci; 
         if(HkGetConnectionStats(iClientID, ci) != HKE_OK) 
            continue; 

         /////////////////////////////////////////////////////////////// 
         // update loss data 
         if(ConData[iClientID].lstLoss.size() >= (set_iLossKickFrame / (LOSS_INTERVALL / 1000))) 
         { 
            // calculate average loss 
            ConData[iClientID].iAverageLoss = 0; 
            foreach(ConData[iClientID].lstLoss, uint, it) 
               ConData[iClientID].iAverageLoss += (*it); 

            ConData[iClientID].iAverageLoss /= (uint)ConData[iClientID].lstLoss.size(); 
         } 

         // remove old lossdata 
         while(ConData[iClientID].lstLoss.size() >= (set_iLossKickFrame / (LOSS_INTERVALL / 1000))) 
            ConData[iClientID].lstLoss.pop_back(); 

         //sum of Drops = Drops guaranteed + drops non-guaranteed 
         iNewDrops = (ci.dwPacketsRetried + ci.dwPacketsDropped) - ConData[iClientID].iLastPacketsDropped; 

         iNewSent = (ci.dwPacketsSentGuaranteed + ci.dwPacketsSentNonGuaranteed) - ConData[iClientID].iLastPacketsSent; 

         // % of Packets Lost = Drops / (sent+received) * 100 
		 if(iNewSent > 0) // division by zero check
			fLossPercentage = (float)((float)iNewDrops /(float)iNewSent) *100; 
		 else
			fLossPercentage = 0.0;

		 if(fLossPercentage > 100)
			 fLossPercentage = 100;
          
         //add last loss to List lstLoss and put current value into iLastLoss 
         ConData[iClientID].lstLoss.push_front(ConData[iClientID].iLastLoss); 
         ConData[iClientID].iLastLoss = (uint)fLossPercentage; 
             
         //Fill new ClientInfo-variables with current values 
         ConData[iClientID].iLastPacketsSent = ci.dwPacketsSentGuaranteed + ci.dwPacketsSentNonGuaranteed; 
         ConData[iClientID].iLastPacketsDropped = ci.dwPacketsRetried + ci.dwPacketsDropped; 
      } 
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace HkIServerImpl
{    

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		{TimerUpdatePingData,			1000,				0},
		{TimerUpdateLossData,			LOSS_INTERVALL,		0},
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EXPORT int __stdcall Update()
	{
		returncode = DEFAULT_RETURNCODE;

		static bool bFirstTime = true;
		if(bFirstTime) {
			bFirstTime = false;
			// check for logged in players and reset their connection data
			struct PlayerData *pPD = 0; 
			while(pPD = Players.traverse_active(pPD)) 
				ClearConData(HkGetClientIdFromPD(pPD));
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

		return 0; // it doesnt matter what we return here since we have set the return code to "DEFAULT_RETURNCODE", so FLHook will just ignore it
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EXPORT void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID)
	{

		ConData[iClientID].tmLastObjUpdate = 0;

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EXPORT void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID)
	{
		returncode = DEFAULT_RETURNCODE;

		// lag detection
		IObjInspectImpl *ins = HkGetInspect(iClientID);
		if(!ins)
			return; // ??? 8[

		mstime tmNow = timeInMS();
		mstime tmTimestamp = (mstime)(ui.dTimestamp*1000);

		if(set_iLagDetectionFrame && ConData[iClientID].tmLastObjUpdate && (HkGetEngineState(iClientID) != ES_TRADELANE) && (ui.cState != 7))
		{
			uint iTimeDiff = (uint)(tmNow - ConData[iClientID].tmLastObjUpdate);
			uint iTimestampDiff = (uint)(tmTimestamp - ConData[iClientID].tmLastObjTimestamp);
			int iDiff = (int)sqrt(pow((long double)((int)iTimeDiff - (int)iTimestampDiff),2));
			iDiff -= g_iServerLoad;
			if(iDiff < 0)
				iDiff = 0;

			uint iPerc;
			if(iTimestampDiff != 0)
				iPerc = (uint)((float)((float)iDiff/(float)iTimestampDiff)*100.0);
			else
				iPerc = 0;


			if(ConData[iClientID].lstObjUpdateIntervalls.size() >= set_iLagDetectionFrame)
			{
				uint iLags = 0;
				foreach(ConData[iClientID].lstObjUpdateIntervalls, uint, it)
				{
					if((*it) > set_iLagDetectionMinimum)
						iLags++;
				}

				ConData[iClientID].iLags = (iLags * 100) / set_iLagDetectionFrame;
				while(ConData[iClientID].lstObjUpdateIntervalls.size() >= set_iLagDetectionFrame)
					ConData[iClientID].lstObjUpdateIntervalls.pop_front();
			}

			ConData[iClientID].lstObjUpdateIntervalls.push_back(iPerc);
		}

		ConData[iClientID].tmLastObjUpdate = tmNow;
		ConData[iClientID].tmLastObjTimestamp = tmTimestamp;

	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ping(uint iClientID, const wstring &wscParam) 
{ 
	if(!set_bPingCmd) {
		PRINT_DISABLED();
		return;
	}

	wstring wscTargetPlayer = GetParam(wscParam, ' ', 0);

	uint iClientIDTarget;
	iClientIDTarget = iClientID;

	wstring Response; 

	Response += L"Ping: ";
	if(ConData[iClientIDTarget].lstPing.size() < set_iPingKickFrame)
		Response += L"n/a Fluct: n/a "; 
	else {
		Response += stows(itos(ConData[iClientIDTarget].iAveragePing)).c_str(); 
		Response += L"ms ";
		if(set_iPingKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iPingKick)).c_str(); 
			Response += L"ms) "; 
		}
		Response += L"Fluct: ";
		Response += stows(itos(ConData[iClientIDTarget].iPingFluctuation)).c_str(); 
		Response += L"ms ";
		if(set_iFluctKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iFluctKick)).c_str(); 
			Response += L"ms) "; 
		}
	}
	
	Response += L"Loss: ";
	if(ConData[iClientIDTarget].lstLoss.size() < (set_iLossKickFrame / (LOSS_INTERVALL / 1000))) 
		Response += L"n/a "; 
    else {
		Response += stows(itos(ConData[iClientIDTarget].iAverageLoss)).c_str(); 
		Response += L"%% "; 
		if(set_iLossKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iLossKick)).c_str(); 
			Response += L"%%) "; 
		}
	}

	Response += L"Lag: ";
	if(ConData[iClientIDTarget].lstObjUpdateIntervalls.size() < set_iLagDetectionFrame)
		Response += L"n/a";
	else {
		Response += stows(itos(ConData[iClientIDTarget].iLags)).c_str(); 
		Response += L"%% "; 
		if(set_iLagKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iLagKick)).c_str(); 
			Response += L"%%)"; 
		}
	}

	// Send the message to the user 
	PrintUserCmdText(iClientID, Response); 
    
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_PingTarget(uint iClientID, const wstring &wscParam) 
{ 
	if(!set_bPingCmd) {
		PRINT_DISABLED();
		return;
	}
	
	uint iShip = 0;
	pub::Player::GetShip(iClientID, iShip);
	if(!iShip) {
		PrintUserCmdText(iClientID, L"Error: You are docked");
		return;
	}

	uint iTarget = 0;
	pub::SpaceObj::GetTarget(iShip, iTarget);

	if(!iTarget) {
		PrintUserCmdText(iClientID, L"Error: No target");
		return;
	}	

	uint iClientIDTarget = HkGetClientIDByShip(iTarget);
	if(!HkIsValidClientID(iClientIDTarget))
	{
		PrintUserCmdText(iClientID, L"Error: Target is no player");
		return;
	}


	wstring Response; 
	
	if (iClientIDTarget != iClientID) {
		wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);
		Response += wscCharname.c_str();
		Response += L" - ";
	}

	Response += L"Ping: ";
	if(ConData[iClientIDTarget].lstPing.size() < set_iPingKickFrame)
		Response += L"n/a Fluct: n/a "; 
	else {
		Response += stows(itos(ConData[iClientIDTarget].iAveragePing)).c_str(); 
		Response += L"ms ";
		if(set_iPingKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iPingKick)).c_str(); 
			Response += L"ms) "; 
		}
		Response += L"Fluct: ";
		Response += stows(itos(ConData[iClientIDTarget].iPingFluctuation)).c_str(); 
		Response += L"ms ";
		if(set_iFluctKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iFluctKick)).c_str(); 
			Response += L"ms) "; 
		}
	}
	
	Response += L"Loss: ";
	if(ConData[iClientIDTarget].lstLoss.size() < (set_iLossKickFrame / (LOSS_INTERVALL / 1000))) 
		Response += L"n/a "; 
    else {
		Response += stows(itos(ConData[iClientIDTarget].iAverageLoss)).c_str(); 
		Response += L"%% "; 
		if(set_iLossKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iLossKick)).c_str(); 
			Response += L"%%) "; 
		}
	}

	Response += L"Lag: ";
	if(ConData[iClientIDTarget].lstObjUpdateIntervalls.size() < set_iLagDetectionFrame)
		Response += L"n/a";
	else {
		Response += stows(itos(ConData[iClientIDTarget].iLags)).c_str(); 
		Response += L"%% "; 
		if(set_iLagKick > 0) {
			Response += L"(Max: ";
			Response += stows(itos(set_iLagKick)).c_str(); 
			Response += L"%%)"; 
		}
	}

	// Send the message to the user 
	PrintUserCmdText(iClientID, Response); 
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*_UserCmdProc)(uint, const wstring &);

struct USERCMD
{
	wchar_t *wszCmd;
	_UserCmdProc proc;
};

USERCMD UserCmds[] =
{
	{ L"/ping",					UserCmd_Ping},
	{ L"/pingtarget",			UserCmd_PingTarget},
};

EXPORT bool UserCmd_Process(uint iClientID, const wstring &wscCmd)
{

	wstring wscCmdLower = ToLower(wscCmd);

	
	for(uint i = 0; (i < sizeof(UserCmds)/sizeof(USERCMD)); i++)
	{
		if(wscCmdLower.find(ToLower(UserCmds[i].wszCmd)) == 0)
		{
			wstring wscParam = L"";
			if(wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if(wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
			}
			UserCmds[i].proc(iClientID, wscParam);

			returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command, return immediatly
			return true;
		}
	}
	
	returncode = DEFAULT_RETURNCODE; // we did not handle the command, so let other plugins or FLHook kick in
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


EXPORT void Plugin_Communication_CallBack(PLUGIN_MESSAGE msg, void* data)
{
	returncode = DEFAULT_RETURNCODE;

	// this is the hooked plugin communication function
	
	// we now check, if the message is for us
	if(msg == CONDATA_EXCEPTION) {
		// the message is for us, now we know what the actual data is, so we do a reinterpret cast
		CONDATA_EXCEPTION_STRUCT* incoming_data = reinterpret_cast<CONDATA_EXCEPTION_STRUCT*>(data);
		
		ConData[incoming_data->iClientID].bException = incoming_data->bException;
		ConData[incoming_data->iClientID].sExceptionReason = incoming_data->sReason;
		if(!ConData[incoming_data->iClientID].bException)
			ClearConData(incoming_data->iClientID);
	
	} else if(msg == CONDATA_DATA){
        CONDATA_DATA_STRUCT* incoming_data = reinterpret_cast<CONDATA_DATA_STRUCT*>(data);
		incoming_data->iAverageLoss = ConData[incoming_data->iClientID].iAverageLoss;
		incoming_data->iAveragePing = ConData[incoming_data->iClientID].iAveragePing;
		incoming_data->iLags = ConData[incoming_data->iClientID].iLags;
		incoming_data->iPingFluctuation = ConData[incoming_data->iClientID].iPingFluctuation;
	}
	return; 
}


#define IS_CMD(a) !wscCmd.compare(L##a)

EXPORT bool ExecuteCommandString_Callback(CCmds* classptr, const wstring &wscCmd)
{
	returncode = DEFAULT_RETURNCODE;
	
	if (IS_CMD("getstats"))
	{
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			if (HkIsInCharSelectMenu(iClientID))
				continue;

        
		 CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
		 if (!cdpClient)
			continue;
			
		 int saturation = (int)(cdpClient->GetLinkSaturation() * 100);
		 int txqueue = cdpClient->GetSendQSize();
		 classptr->Print(L"charname=%s clientid=%u loss=%u lag=%u pingfluct=%u saturation=%u txqueue=%u\n",
				Players.GetActiveCharacterName(iClientID), iClientID,
				ConData[iClientID].iAverageLoss, ConData[iClientID].iLags, ConData[iClientID].iPingFluctuation,
				saturation, txqueue);
		}
		classptr->Print(L"OK\n");
		returncode = SKIPPLUGINS_NOFUNCTIONCALL;
		return true;
	}

	return false;
}