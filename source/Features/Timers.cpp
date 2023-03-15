#include "PCH.hpp"
#include "Global.hpp"

#include <WS2tcpip.h>
#include "Features/TempBan.hpp"

CTimer::CTimer(const std::string& func, uint warn)
	: function(func), warning(warn)
{
}

void CTimer::start()
{
	tmStart = Hk::Time::GetUnixMiliseconds();
}

uint CTimer::stop()
{
	auto timeDelta = static_cast<uint>(Hk::Time::GetUnixMiliseconds() - tmStart);

	if (FLHookConfig::i()->general.logPerformanceTimers)
	{
		if (timeDelta > max && timeDelta > warning)
		{
			Logger::i()->Log(LogLevel::Info, std::format("Spent {} ms in {}, longest so far.", timeDelta, function));
			max = timeDelta;
		}
		else if (timeDelta > 100)
		{
			Logger::i()->Log(LogLevel::Info, std::format("Spent {} ms in {}", timeDelta, function));
		}
	}
	return timeDelta;
}

/*************************************************************************************************************
check if temporary ban has expired
**************************************************************************************************************/

void TimerTempBanCheck()
{
	const auto* config = FLHookConfig::c();
	if (config->general.tempBansEnabled)
	{
		TempBanManager::i()->ClearFinishedTempBans();
	}
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
			PlayerData* playerData = nullptr;
			while ((playerData = Players.traverse_active(playerData)))
			{
				ClientId client = playerData->onlineId;
				if (client < 1 || client > MaxClientId)
					continue;

				if (ClientInfo[client].tmKickTime)
				{
					if (Hk::Time::GetUnixMiliseconds() >= ClientInfo[client].tmKickTime)
					{
						Hk::Player::Kick(client); // kick time expired
						ClientInfo[client].tmKickTime = 0;
					}
					continue; // player will be kicked anyway
				}
				const auto* config = FLHookConfig::c();
				if (config->general.antiBaseIdle)
				{
					// anti base-idle check
					uint baseId;
					pub::Player::GetBase(client, baseId);
					if (baseId && ClientInfo[client].baseEnterTime
						&& (time(nullptr) - ClientInfo[client].baseEnterTime) >= config->general.antiBaseIdle)
					{
						//AddKickLog(client, "base idling");
						Hk::Player::MsgAndKick(client, L"Base idling", 10);
						ClientInfo[client].baseEnterTime = 0;
					}
				}

				if (config->general.antiCharMenuIdle)
				{
					// anti charmenu-idle check
					if (Hk::Client::IsInCharSelectMenu(client))
					{
						if (!ClientInfo[client].charMenuEnterTime)
							ClientInfo[client].charMenuEnterTime = static_cast<uint>(time(nullptr));
						else if ((time(nullptr) - ClientInfo[client].charMenuEnterTime) >= config->general.antiCharMenuIdle)
						{
							//AddKickLog(client, "Charmenu idling");
							Hk::Player::Kick(client);
							ClientInfo[client].charMenuEnterTime = 0;
						}
					}
					else
						ClientInfo[client].charMenuEnterTime = 0;
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
			PlayerData* playerData = nullptr;
			while ((playerData = Players.traverse_active(playerData)))
			{
				ClientId client = playerData->onlineId;
				if (client < 1 || client > MaxClientId)
					continue;

				if (ClientInfo[client].tmF1Time && (Hk::Time::GetUnixMiliseconds() >= ClientInfo[client].tmF1Time))
				{
					// f1
					Server.CharacterInfoReq(client, false);
					ClientInfo[client].tmF1Time = 0;
				}
				else if (ClientInfo[client].tmF1TimeDisconnect && (Hk::Time::GetUnixMiliseconds() >= ClientInfo[client].tmF1TimeDisconnect))
				{
					ulong dataArray[64] = {0};
					dataArray[26] = client;

					__asm {
						pushad
						lea ecx, dataArray
						mov eax, [remoteClient]
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
std::list<RESOLVE_IP> g_ResolveIPs;
std::list<RESOLVE_IP> g_ResolveIPsResult;
HANDLE hThreadResolver;

void ThreadResolver()
{
	TRY_HOOK
		{
			while (true)
			{
				EnterCriticalSection(&csIPResolve);
				std::list<RESOLVE_IP> MyResolveIPs = g_ResolveIPs;
				g_ResolveIPs.clear();
				LeaveCriticalSection(&csIPResolve);

				for (auto& ip : MyResolveIPs)
				{
					SOCKADDR_IN addr{AF_INET, 2302, {}, {0}};
					InetPtonW(AF_INET, ip.IP.c_str(), &addr.sin_addr);

					wchar_t hostbuf[255];
					GetNameInfoW(
						reinterpret_cast<const SOCKADDR*>(&addr),
						sizeof(addr),
						hostbuf,
						std::size(hostbuf),
						nullptr,
						0,
						0);

					ip.hostname = hostbuf;
				}

				EnterCriticalSection(&csIPResolve);
				for (auto& ip : MyResolveIPs)
				{
					if (ip.hostname.length())
						g_ResolveIPsResult.push_back(ip);
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
			for (const auto& ip : g_ResolveIPsResult)
			{
				if (ip.connects != ClientInfo[ip.client].connects)
					continue; // outdated

				// check if banned
				for (const auto* config = FLHookConfig::c();
				     const auto& ban : config->bans.banWildcardsAndIPs)
				{
					if (Wildcard::Fit(wstos(ban).c_str(), wstos(ip.hostname).c_str()))
					{
						//AddKickLog(ip.client, wstos(std::format(L"IP/hostname ban({} matches {})", ip.hostname.c_str(), ban.c_str())));
						if (config->bans.banAccountOnMatch)
							Hk::Player::Ban(ip.client, true);
						Hk::Player::Kick(ip.client);
					}
				}
				ClientInfo[ip.client].hostname = ip.hostname;
			}

			g_ResolveIPsResult.clear();
			LeaveCriticalSection(&csIPResolve);
		}
	CATCH_HOOK({})
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

