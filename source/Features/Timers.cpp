#include "Global.hpp"
#include <WinSock2.h>

#include <WS2tcpip.h>

CTimer::CTimer(const std::string& sFunc, uint iWarn) : sFunction(sFunc), iWarning(iWarn)
{
}

void CTimer::start()
{
	tmStart = timeInMS();
}

uint CTimer::stop()
{
	uint timeDelta = abs((int)(timeInMS() - tmStart));

	if (timeDelta > iMax && timeDelta > iWarning)
	{
		AddLog(LogType::PerfTimers, LogLevel::Info, std::format("Spent {} ms in {}, longest so far.", timeDelta, sFunction));
		iMax = timeDelta;
	}
	else if (timeDelta > 100)
	{
		AddLog(LogType::PerfTimers, LogLevel::Info, std::format("Spent {} ms in {}", timeDelta, sFunction));
	}

	return timeDelta;
}

/**************************************************************************************************************
check if players should be kicked
**************************************************************************************************************/

void TimerCheckKick()
{
	CallPluginsBefore(HookedCall::FLHook__TimerCheckKick);

	TRY_HOOK
	{
		// for all players
		struct PlayerData* playerData = nullptr;
		while (playerData = Players.traverse_active(playerData))
		{
			ClientId client = playerData->iOnlineId;
			if (client < 1 || client > MaxClientId)
				continue;

			if (ClientInfo[client].tmKickTime)
			{
				if (timeInMS() >= ClientInfo[client].tmKickTime)
				{
					Hk::Player::Kick(client); // kick time expired
					ClientInfo[client].tmKickTime = 0;
				}
				continue; // player will be kicked anyway
			}
			const auto* config = FLHookConfig::c();
			if (config->general.antiBaseIdle)
			{ // anti base-idle check
				uint baseId;
				pub::Player::GetBase(client, baseId);
				if (baseId && ClientInfo[client].iBaseEnterTime
				&&	(time(0) - ClientInfo[client].iBaseEnterTime) >= config->general.antiBaseIdle)
				{
					AddKickLog(client, "Base idling");
					Hk::Player::MsgAndKick(client, L"Base idling", 10);
					ClientInfo[client].iBaseEnterTime = 0;
				}
			}

			if (config->general.antiCharMenuIdle)
			{ // anti charmenu-idle check
				if (Hk::Client::IsInCharSelectMenu(client))
				{
					if (!ClientInfo[client].iCharMenuEnterTime)
						ClientInfo[client].iCharMenuEnterTime = static_cast<uint>(time(nullptr));
					else if ((time(0) - ClientInfo[client].iCharMenuEnterTime) >= config->general.antiCharMenuIdle)
					{
						AddKickLog(client, "Charmenu idling");
						Hk::Player::Kick(client);
						ClientInfo[client].iCharMenuEnterTime = 0;
						continue;
					}
				}
				else
					ClientInfo[client].iCharMenuEnterTime = 0;
			}
		}
	}
	CATCH_HOOK({})
}

/**************************************************************************************************************
Check if NPC spawns should be disabled
**************************************************************************************************************/

void TimerNPCAndF1Check()
{
	CallPluginsBefore(HookedCall::FLHook__TimerNPCAndF1Check);

	TRY_HOOK
	{
		struct PlayerData* playerData = nullptr;
		while (playerData = Players.traverse_active(playerData))
		{
			ClientId client = playerData->iOnlineId;
			if (client < 1 || client > MaxClientId)
				continue;

			if (ClientInfo[client].tmF1Time && (timeInMS() >= ClientInfo[client].tmF1Time))
			{ // f1
				Server.CharacterInfoReq(client, false);
				ClientInfo[client].tmF1Time = 0;
			}
			else if (ClientInfo[client].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[client].tmF1TimeDisconnect))
			{
				ulong dataArray[64] = { 0 };
				dataArray[26] = client;

				__asm {
                    pushad
                    lea ecx, dataArray
                    mov eax, [hModRemoteClient]
                    add eax, ADDR_RC_DISCONNECT
                    call eax ; disconncet
                    popad
				}

				ClientInfo[client].tmF1TimeDisconnect = 0;
				continue;
			}
		}

		const auto* config = FLHookConfig::c();
		if (config->general.disableNPCSpawns && (CoreGlobals::c()->serverLoadInMs >= config->general.disableNPCSpawns))
			Hk::Admin::ChangeNPCSpawn(true); // serverload too high, disable npcs
		else
			Hk::Admin::ChangeNPCSpawn(false);
	}
	CATCH_HOOK({})
}

/**************************************************************************************************************
**************************************************************************************************************/

CRITICAL_SECTION csIPResolve;
std::list<RESOLVE_IP> g_lstResolveIPs;
std::list<RESOLVE_IP> g_lstResolveIPsResult;
HANDLE hThreadResolver;

void ThreadResolver()
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

void TimerCheckResolveResults()
{
	TRY_HOOK
	{
		EnterCriticalSection(&csIPResolve);
		for (const auto& ip : g_lstResolveIPsResult)
		{
			if (ip.iConnects != ClientInfo[ip.client].iConnects)
				continue; // outdated

			// check if banned
			for (const auto* config = FLHookConfig::c(); 
				const auto& ban : config->bans.banWildcardsAndIPs)
			{
				if (Wildcard::Fit(wstos(ban).c_str(), wstos(ip.wscHostname).c_str()))
				{
					AddKickLog(ip.client, wstos(std::format(L"IP/Hostname ban({} matches {})", ip.wscHostname.c_str(), ban.c_str())));
					if (config->bans.banAccountOnMatch)
						Hk::Player::Ban(ip.client, true);
					Hk::Player::Kick(ip.client);
				}
			}
			ClientInfo[ip.client].wscHostname = ip.wscHostname;
		}

		g_lstResolveIPsResult.clear();
		LeaveCriticalSection(&csIPResolve);
	}
	CATCH_HOOK({})
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
