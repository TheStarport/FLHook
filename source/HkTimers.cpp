#include <winsock2.h>
#include "hook.h"
#include "wildcards.hh"
#include <WS2tcpip.h>
#include <math.h>

CTimer::CTimer(std::string sFunc, uint iWarn) {
    iMax = 0;
    iWarning = iWarn;
    sFunction = sFunc;
}

void CTimer::start() { tmStart = timeInMS(); }

uint CTimer::stop() {
    uint iDelta = abs((int)(timeInMS() - tmStart));

    if (iDelta > iMax && iDelta > iWarning) {

        // log
        if (set_bPerfTimer)
            HkAddPerfTimerLog("Spent %d ms in %s, longest so far.", iDelta,
                              sFunction.c_str());
        iMax = iDelta;
    } else if (iDelta > set_iTimerDebugThreshold &&
               set_iTimerDebugThreshold > 0) {
        if (set_bPerfTimer)
            HkAddPerfTimerLog("Spent %d ms in %s", iDelta, sFunction.c_str());
    }

    return iDelta;
}

/**************************************************************************************************************
check if players should be kicked
**************************************************************************************************************/

void HkTimerCheckKick() {
    CallPluginsBefore(HookedCall::FLHook__TimerCheckKick);

    TRY_HOOK {
        // for all players
        struct PlayerData *playerData = 0;
        while (playerData = Players.traverse_active(playerData)) {
            uint clientID = HkGetClientIdFromPD(playerData);
            if (clientID < 1 || clientID > MAX_CLIENT_ID)
                continue;

            if (ClientInfo[clientID].tmKickTime) {
                if (timeInMS() >= ClientInfo[clientID].tmKickTime) {
                    HkKick(ARG_CLIENTID(clientID)); // kick time expired
                    ClientInfo[clientID].tmKickTime = 0;
                }
                continue; // player will be kicked anyway
            }

            if (set_iAntiBaseIdle) { // anti base-idle check
                uint baseID;
                pub::Player::GetBase(clientID, baseID);
                if (baseID && ClientInfo[clientID].iBaseEnterTime) {
                    if ((time(0) - ClientInfo[clientID].iBaseEnterTime) >=
                        set_iAntiBaseIdle) {
                        HkAddKickLog(clientID, L"Base idling");
                        HkMsgAndKick(clientID, L"Base idling",
                                     set_iKickMsgPeriod);
                        ClientInfo[clientID].iBaseEnterTime = 0;
                    }
                }
            }

            if (set_iAntiCharMenuIdle) { // anti charmenu-idle check
                if (HkIsInCharSelectMenu(clientID)) {
                    if (!ClientInfo[clientID].iCharMenuEnterTime)
                        ClientInfo[clientID].iCharMenuEnterTime = static_cast<uint>(time(0));
                    else if ((time(0) - ClientInfo[clientID].iCharMenuEnterTime) >= set_iAntiCharMenuIdle) {
                        HkAddKickLog(clientID, L"Charmenu idling");
                        HkKick(ARG_CLIENTID(clientID));
                        ClientInfo[clientID].iCharMenuEnterTime = 0;
                        continue;
                    }
                } else
                    ClientInfo[clientID].iCharMenuEnterTime = 0;
            }
        }
    }
    CATCH_HOOK({})
}

/**************************************************************************************************************
Check if NPC spawns should be disabled
**************************************************************************************************************/

void HkTimerNPCAndF1Check() {
    CallPluginsBefore(HookedCall::FLHook__TimerNPCAndF1Check);

    TRY_HOOK {
        struct PlayerData *playerData = 0;
        while (playerData = Players.traverse_active(playerData)) {
            uint clientID = HkGetClientIdFromPD(playerData);
            if (clientID < 1 || clientID > MAX_CLIENT_ID)
                continue;

            if (ClientInfo[clientID].tmF1Time && (timeInMS() >= ClientInfo[clientID].tmF1Time)) { // f1
                Server.CharacterInfoReq(clientID, false);
                ClientInfo[clientID].tmF1Time = 0;
            } else if (ClientInfo[clientID].tmF1TimeDisconnect &&
                       (timeInMS() >=
                        ClientInfo[clientID].tmF1TimeDisconnect)) {
                ulong dataArray[64] = {0};
                dataArray[26] = clientID;

                __asm {
                    pushad
                    lea ecx, lArray
                    mov eax, [hModRemoteClient]
                    add eax, ADDR_RC_DISCONNECT
                    call eax ; disconncet
                    popad
                }

                ClientInfo[clientID].tmF1TimeDisconnect = 0;
                continue;
            }
        }

        // npc
        if (set_iDisableNPCSpawns && (g_iServerLoad >= set_iDisableNPCSpawns))
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

void HkThreadResolver() {
    TRY_HOOK {
        while (true) {
            EnterCriticalSection(&csIPResolve);
            std::list<RESOLVE_IP> lstMyResolveIPs = g_lstResolveIPs;
            g_lstResolveIPs.clear();
            LeaveCriticalSection(&csIPResolve);

            for (auto &ip : lstMyResolveIPs) {
                SOCKADDR_IN addr{AF_INET, 2302, {}, {0}};
                InetPtonW(AF_INET, ip.wscIP.c_str(), &addr.sin_addr);

                wchar_t hostbuf[255];
                GetNameInfoW(reinterpret_cast<const SOCKADDR *>(&addr),
                             sizeof(addr), hostbuf, std::size(hostbuf), nullptr,
                             0, 0);

                ip.wscHostname = hostbuf;
            }

            EnterCriticalSection(&csIPResolve);
            for (auto &ip : lstMyResolveIPs) {
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

void HkTimerCheckResolveResults() {
    TRY_HOOK {
        EnterCriticalSection(&csIPResolve);
        for (auto &ip : g_lstResolveIPsResult) {
            if (ip.iConnects != ClientInfo[ip.iClientID].iConnects)
                continue; // outdated

            // check if banned
            for (const auto& ban : set_setBans) {
                if (Wildcard::wildcardfit(wstos(ban).c_str(),
                                          wstos(ip.wscHostname).c_str())) {
                    HkAddKickLog(ip.iClientID,
                                 L"IP/Hostname ban(%s matches %s)",
                                 ip.wscHostname.c_str(), ban.c_str());
                    if (set_bBanAccountOnMatch)
                        HkBan(ARG_CLIENTID(ip.iClientID), true);
                    HkKick(ARG_CLIENTID(ip.iClientID));
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
