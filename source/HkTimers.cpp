#include <winsock2.h>
#include "wildcards.hh"
#include "hook.h"
#include <math.h>

CTimer::CTimer(string sFunc, uint iWarn)
{
	iMax = 0;
	iWarning = iWarn;
	sFunction = sFunc;
}

void CTimer::start()
{
	tmStart = timeInMS();
}

uint CTimer::stop()
{
	uint iDelta = abs((int)(timeInMS() - tmStart));

	if(iDelta > iMax && iDelta > iWarning) {

		// log
		if(set_bPerfTimer)
			HkAddPerfTimerLog("Spent %d ms in %s, longest so far.", iDelta, sFunction.c_str());
		iMax = iDelta;
	}

	return iDelta;

}

/**************************************************************************************************************
check if players should be kicked
**************************************************************************************************************/

void HkTimerCheckKick()
{
	
	CALL_PLUGINS(PLUGIN_HkTimerCheckKick,());
		if(bPluginReturn)
			return;

	try {
		// for all players
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			if(ClientInfo[iClientID].tmKickTime)
			{
				if(timeInMS() >= ClientInfo[iClientID].tmKickTime)
				{
					HkKick(ARG_CLIENTID(iClientID)); // kick time expired
					ClientInfo[iClientID].tmKickTime = 0;
				}
				continue; // player will be kicked anyway
			}

			if(set_iAntiBaseIdle)
			{ // anti base-idle check
				uint iBaseID;
				pub::Player::GetBase(iClientID, iBaseID);
				if(iBaseID && ClientInfo[iClientID].iBaseEnterTime)
				{
					if((time(0) - ClientInfo[iClientID].iBaseEnterTime) >= set_iAntiBaseIdle)
					{
						HkAddKickLog(iClientID, L"Base idling");
						HkMsgAndKick(iClientID, L"Base idling", set_iKickMsgPeriod);
						ClientInfo[iClientID].iBaseEnterTime = 0;
					}
				}
			}

			if(set_iAntiCharMenuIdle)
			{ // anti charmenu-idle check
				if(HkIsInCharSelectMenu(iClientID))	{
					if(!ClientInfo[iClientID].iCharMenuEnterTime)
						ClientInfo[iClientID].iCharMenuEnterTime = (uint)time(0);
					else if((time(0) - ClientInfo[iClientID].iCharMenuEnterTime) >= set_iAntiCharMenuIdle) {
						HkAddKickLog(iClientID, L"Charmenu idling");
						HkKick(ARG_CLIENTID(iClientID));
						ClientInfo[iClientID].iCharMenuEnterTime = 0;
						continue;
					}
				} else
					ClientInfo[iClientID].iCharMenuEnterTime = 0;
			}

		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
Check if NPC spawns should be disabled
**************************************************************************************************************/

void HkTimerNPCAndF1Check()
{
	try {
		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			if(ClientInfo[iClientID].tmF1Time && (timeInMS() >= ClientInfo[iClientID].tmF1Time)) { // f1
				Server.CharacterInfoReq(iClientID, false);
				ClientInfo[iClientID].tmF1Time = 0;
			} else if(ClientInfo[iClientID].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[iClientID].tmF1TimeDisconnect)) {
				ulong lArray[64] = {0};
				lArray[26] = iClientID;
				__asm
				{
					pushad
					lea ecx, lArray
					mov eax, [hModRemoteClient]
					add eax, ADDR_RC_DISCONNECT
					call eax ; disconncet
					popad
				}

				ClientInfo[iClientID].tmF1TimeDisconnect = 0;
				continue;
			}
		}

		// npc
		if(set_iDisableNPCSpawns && (g_iServerLoad >= set_iDisableNPCSpawns))
			HkChangeNPCSpawn(true); // serverload too high, disable npcs
		else
			HkChangeNPCSpawn(false);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
**************************************************************************************************************/

CRITICAL_SECTION csIPResolve;
list<RESOLVE_IP> g_lstResolveIPs;
list<RESOLVE_IP> g_lstResolveIPsResult;
HANDLE hThreadResolver;

void HkThreadResolver()
{
	try {
		while(1)
		{
			EnterCriticalSection(&csIPResolve);
			list<RESOLVE_IP> lstMyResolveIPs = g_lstResolveIPs;
			g_lstResolveIPs.clear();
			LeaveCriticalSection(&csIPResolve);

			foreach(lstMyResolveIPs, RESOLVE_IP, it)
			{
				ulong addr = inet_addr(wstos(it->wscIP).c_str());
				hostent *host = gethostbyaddr((const char*)&addr, sizeof(addr), AF_INET);
				if(host)
					it->wscHostname = stows(host->h_name);
			}

			EnterCriticalSection(&csIPResolve);
			foreach(lstMyResolveIPs, RESOLVE_IP, it2)
			{
				if(it2->wscHostname.length())
					g_lstResolveIPsResult.push_back(*it2);
			}
			LeaveCriticalSection(&csIPResolve);

			Sleep(50);
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkTimerCheckResolveResults()
{
	try {
		EnterCriticalSection(&csIPResolve);
		foreach(g_lstResolveIPsResult, RESOLVE_IP, it)
		{
			if(it->iConnects != ClientInfo[it->iClientID].iConnects)
				continue; // outdated

			// check if banned
			foreach(set_lstBans, wstring, itb)
			{
				if(Wildcard::wildcardfit(wstos(*itb).c_str(), wstos(it->wscHostname).c_str()))
				{
					HkAddKickLog(it->iClientID, L"IP/Hostname ban(%s matches %s)", it->wscHostname.c_str(), (*itb).c_str());
					if(set_bBanAccountOnMatch)
						HkBan(ARG_CLIENTID(it->iClientID), true);
					HkKick(ARG_CLIENTID(it->iClientID));
				}
			}
			ClientInfo[it->iClientID].wscHostname = it->wscHostname;
		}

		g_lstResolveIPsResult.clear();
		LeaveCriticalSection(&csIPResolve);
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

