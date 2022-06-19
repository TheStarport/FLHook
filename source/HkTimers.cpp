#include "Global.hpp"
#include <WinSock2.h>
#include <Wildcard.hpp>

#include <WS2tcpip.h>

CTimer::CTimer(std::string sFunc, uint iWarn)
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

	if (iDelta > iMax && iDelta > iWarning)
	{
		AddLog(LogType::PerfTimers, LogLevel::Info, L"Spent %d ms in %s, longest so far.", iDelta, stows(sFunction).c_str());
		iMax = iDelta;
	}
	else if (iDelta > 100)
	{
		AddLog(LogType::PerfTimers, LogLevel::Info, L"Spent %d ms in %s", iDelta, stows(sFunction).c_str());
	}

	return iDelta;
}

/**************************************************************************************************************
check if players should be kicked
**************************************************************************************************************/

void HkTimerCheckKick()
{
	CallPluginsBefore(HookedCall::FLHook__TimerCheckKick);

	TRY_HOOK
	{
		// for all players
		struct PlayerData* playerData = 0;
		while (playerData = Players.traverse_active(playerData))
		{
			uint clientID = HkGetClientIdFromPD(playerData);
			if (clientID < 1 || clientID > MaxClientId)
				continue;

			if (ClientInfo[clientID].tmKickTime)
			{
				if (timeInMS() >= ClientInfo[clientID].tmKickTime)
				{
					HkKick(clientID); // kick time expired
					ClientInfo[clientID].tmKickTime = 0;
				}
				continue; // player will be kicked anyway
			}
			const auto* config = FLHookConfig::c();
			if (config->general.antiBaseIdle)
			{ // anti base-idle check
				uint baseID;
				pub::Player::GetBase(clientID, baseID);
				if (baseID && ClientInfo[clientID].iBaseEnterTime)
				{
					if ((time(0) - ClientInfo[clientID].iBaseEnterTime) >= config->general.antiBaseIdle)
					{
						HkAddKickLog(clientID, L"Base idling");
						HkMsgAndKick(clientID, L"Base idling", 10);
						ClientInfo[clientID].iBaseEnterTime = 0;
					}
				}
			}

			if (config->general.antiCharMenuIdle)
			{ // anti charmenu-idle check
				if (HkIsInCharSelectMenu(clientID))
				{
					if (!ClientInfo[clientID].iCharMenuEnterTime)
						ClientInfo[clientID].iCharMenuEnterTime = static_cast<uint>(time(nullptr));
					else if ((time(0) - ClientInfo[clientID].iCharMenuEnterTime) >= config->general.antiCharMenuIdle)
					{
						HkAddKickLog(clientID, L"Charmenu idling");
						HkKick(clientID);
						ClientInfo[clientID].iCharMenuEnterTime = 0;
						continue;
					}
				}
				else
					ClientInfo[clientID].iCharMenuEnterTime = 0;
			}
		}
	}
	CATCH_HOOK({})
}

/**************************************************************************************************************
Check if NPC spawns should be disabled
**************************************************************************************************************/

void HkTimerNPCAndF1Check()
{
	CallPluginsBefore(HookedCall::FLHook__TimerNPCAndF1Check);

	TRY_HOOK
	{
		struct PlayerData* playerData = 0;
		while (playerData = Players.traverse_active(playerData))
		{
			uint clientID = HkGetClientIdFromPD(playerData);
			if (clientID < 1 || clientID > MaxClientId)
				continue;

			if (ClientInfo[clientID].tmF1Time && (timeInMS() >= ClientInfo[clientID].tmF1Time))
			{ // f1
				Server.CharacterInfoReq(clientID, false);
				ClientInfo[clientID].tmF1Time = 0;
			}
			else if (ClientInfo[clientID].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[clientID].tmF1TimeDisconnect))
			{
				ulong dataArray[64] = { 0 };
				dataArray[26] = clientID;

				__asm {
                    pushad
                    lea ecx, dataArray
                    mov eax, [hModRemoteClient]
                    add eax, ADDR_RC_DISCONNECT
                    call eax ; disconncet
                    popad
				}

				ClientInfo[clientID].tmF1TimeDisconnect = 0;
				continue;
			}
		}

		const auto* config = FLHookConfig::c();
		if (config->general.disableNPCSpawns && (g_iServerLoad >= config->general.disableNPCSpawns))
			HkChangeNPCSpawn(true); // serverload too high, disable npcs
		else
			HkChangeNPCSpawn(false);
	}
	CATCH_HOOK({})
}

/**************************************************************************************************************
**************************************************************************************************************/

CRITICAL_SECTION csIPResolve;
std::list<RESOLVE_IP> g_lstResolveIPs;
std::list<RESOLVE_IP> g_lstResolveIPsResult;
HANDLE hThreadResolver;

void HkThreadResolver()
{
	TRY_HOOK
	{
		while (true)
		{
			EnterCriticalSection(&csIPResolve);
			std::list<RESOLVE_IP> lstMyResolveIPs = g_lstResolveIPs;
			g_lstResolveIPs.clear();
			LeaveCriticalSection(&csIPResolve);

			for (auto& ip : lstMyResolveIPs)
			{
				SOCKADDR_IN addr { AF_INET, 2302, {}, { 0 } };
				InetPtonW(AF_INET, ip.wscIP.c_str(), &addr.sin_addr);

				wchar_t hostbuf[255];
				GetNameInfoW(
				    reinterpret_cast<const SOCKADDR*>(&addr), sizeof(addr), hostbuf, std::size(hostbuf), nullptr, 0, 0);

				ip.wscHostname = hostbuf;
			}

			EnterCriticalSection(&csIPResolve);
			for (auto& ip : lstMyResolveIPs)
			{
				if (ip.wscHostname.length())
					g_lstResolveIPsResult.push_back(ip);
			}
			LeaveCriticalSection(&csIPResolve);

			Sleep(50);
		}
	}
	CATCH_HOOK({})
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkTimerCheckResolveResults()
{
	TRY_HOOK
	{
		EnterCriticalSection(&csIPResolve);
		for (auto& ip : g_lstResolveIPsResult)
		{
			if (ip.iConnects != ClientInfo[ip.iClientID].iConnects)
				continue; // outdated

			// check if banned
			const auto* config = FLHookConfig::c();
			for (const auto& ban : config->bans.banWildcardsAndIPs)
			{
				if (Wildcard::Fit(wstos(ban).c_str(), wstos(ip.wscHostname).c_str()))
				{
					HkAddKickLog(ip.iClientID, L"IP/Hostname ban(%s matches %s)", ip.wscHostname.c_str(), ban.c_str());
					if (config->bans.banAccountOnMatch)
						HkBan(ip.iClientID, true);
					HkKick(ip.iClientID);
				}
			}
			ClientInfo[ip.iClientID].wscHostname = ip.wscHostname;
		}

		g_lstResolveIPsResult.clear();
		LeaveCriticalSection(&csIPResolve);
	}
	CATCH_HOOK({})
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
