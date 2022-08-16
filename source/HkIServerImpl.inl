// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
#pragma once
#include <CInGame.h>
#include <Wildcard.hpp>

namespace HkIServerImpl {

Timer g_Timers[] = {
    { ProcessPendingCommands, 50 },
    { HkTimerCheckKick, 1000 },
    { HkTimerNPCAndF1Check, 50 },
    { HkTimerCheckResolveResults, 0 },
};

void Update__Inner() {
    static bool firstTime = true;
    if (firstTime) {
        FLHookInit();
        firstTime = false;
    }

    const auto currentTime = timeInMS();
    for (auto& timer : g_Timers) {
        // This one isn't actually in seconds, but the plugins should be
        if ((currentTime - timer.lastTime) >= timer.intervalInSeconds) {
            timer.lastTime = currentTime;
            timer.func();
        }
    }

    for (PluginData& plugin : PluginManager::ir())
    {
		for (auto& timer : plugin.timers)
		{
			if ((currentTime - timer.lastTime) >= (timer.intervalInSeconds * 1000))
			{
				timer.lastTime = currentTime;
				timer.func();
			}
		}
    }

    char *data;
    memcpy(&data, g_FLServerDataPtr + 0x40, 4);
    memcpy(&g_iServerLoad, data + 0x204, 4);
    memcpy(&g_iPlayerCount, data + 0x208, 4);
}

CInGame g_Admin;
bool g_InSubmitChat = false;
uint g_TextLength = 0;

bool SubmitChat__Inner(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID& cidTo, int) {
	TRY_HOOK
	{
		const auto* config = FLHookConfig::i();

        // Group join/leave commands are not parsed
        if (cidTo.iID == SpecialChatIDs::GROUP_EVENT)
            return true;

        // Anything outside normal bounds is aborted to prevent crashes
        if(cidTo.iID > SpecialChatIDs::GROUP_EVENT
           || cidTo.iID > SpecialChatIDs::PLAYER_MAX && cidTo.iID < SpecialChatIDs::SPECIAL_BASE)
            return false;

        // extract text from rdlReader
        BinaryRDLReader rdl;
        wchar_t wszBuf[1024] = L"";
        uint iRet1;
        rdl.extract_text_from_buffer((unsigned short *)wszBuf, sizeof(wszBuf),
                                     iRet1, (const char *)rdlReader, size);
        std::wstring buffer = wszBuf;

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
		if (config->userCommands.defaultLocalChat && cidTo.iID == SpecialChatIDs::SYSTEM)
		{
            cidTo.iID = SpecialChatIDs::LOCAL;
        }


        // fix flserver commands and change chat to id so that event logging is
        // accurate.
		bool inBuiltCommand = true;
        g_TextLength = static_cast<uint>(buffer.length());
        if (!buffer.find(L"/g ")) {
            cidTo.iID = SpecialChatIDs::GROUP;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/l ")) {
            cidTo.iID = SpecialChatIDs::LOCAL;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/s ")) {
            cidTo.iID = SpecialChatIDs::SYSTEM;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/u ")) {
            cidTo.iID = SpecialChatIDs::UNIVERSE;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/group ")) {
            cidTo.iID = SpecialChatIDs::GROUP;
            g_TextLength -= 7;
        } else if (!buffer.find(L"/local ")) {
            cidTo.iID = SpecialChatIDs::LOCAL;
            g_TextLength -= 7;
        } else if (!buffer.find(L"/system ")) {
            cidTo.iID = SpecialChatIDs::SYSTEM;
            g_TextLength -= 8;
        } else if (!buffer.find(L"/universe ")) {
            cidTo.iID = SpecialChatIDs::UNIVERSE;
            g_TextLength -= 10;
        } else
        {
			inBuiltCommand = false;
        }

        if (UserCmd_Process(cidFrom.iID, buffer))
            return false;

        else if (buffer[0] == '.') {
            CAccount *acc = Players.FindAccountFromClientID(cidFrom.iID);
            std::wstring accDirname;

            HkGetAccountDirName(acc, accDirname);
            std::string adminFile =
                scAcctPath + wstos(accDirname) + "\\flhookadmin.ini";
            WIN32_FIND_DATA fd;
            HANDLE hFind = FindFirstFile(adminFile.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) { // is admin
                FindClose(hFind);
                g_Admin.ReadRights(adminFile);
                g_Admin.iClientID = cidFrom.iID;
                g_Admin.wscAdminName = ToWChar(Players.GetActiveCharacterName(cidFrom.iID));
                g_Admin.ExecuteCommandString(buffer.data() + 1);
                return false;
            }
        }

        std::wstring eventString;
        eventString.reserve(256);
        eventString = L"chat";
        eventString += L" from=";
        if (cidFrom.iID == SpecialChatIDs::CONSOLE)
            eventString += L"console";
        else {
            const auto* fromName = ToWChar(Players.GetActiveCharacterName(cidFrom.iID));
            if (!fromName)
                eventString += L"unknown";
            else
                eventString += fromName;
        }

        eventString += L" id=";
        eventString += std::to_wstring(cidFrom.iID);

        eventString += L" type=";
        if (cidTo.iID == SpecialChatIDs::UNIVERSE)
            eventString += L"universe";
        else if (cidTo.iID == SpecialChatIDs::GROUP) {
            eventString += L"group";
            eventString += L" grpidto=";
            eventString += std::to_wstring(Players.GetGroupID(cidFrom.iID));
        } else if (cidTo.iID == SpecialChatIDs::SYSTEM)
            eventString += L"system";
        else if (cidTo.iID == SpecialChatIDs::LOCAL)
            eventString += L"local";
        else {
            eventString += L"player";
            eventString += L" to=";

            if (cidTo.iID == SpecialChatIDs::CONSOLE)
                eventString += L"console";
            else {
                const auto* toName = ToWChar(Players.GetActiveCharacterName(cidTo.iID));
                if (!toName)
                    eventString += L"unknown";
                else
                    eventString += toName;
            }

            eventString += L" idto=";
            eventString += std::to_wstring(cidTo.iID);
        }

        eventString += L" text=";
        eventString += buffer;
        ProcessEvent(L"%s", eventString.c_str());

        // check if chat should be suppressed for in-built command prefixes
		if (config->general.suppressInvalidCommands && !inBuiltCommand && (buffer.rfind(L'/', 0) == 0 || buffer.rfind(L'.', 0) == 0))
		{
			return false;
		}

        // Check if any other custom prefixes have been added
		if (!config->general.chatSuppressList.empty())
		{
            auto lcBuffer = ToLower(buffer);
			for (const auto& chat : config->general.chatSuppressList)
			{
                if (lcBuffer.rfind(chat, 0) == 0)
                    return false;
            }
		}
    }
    CATCH_HOOK({})

    return true;
}

void PlayerLaunch__Inner(uint shipID, uint clientID) {
    TRY_HOOK {
        ClientInfo[clientID].iShip = shipID;
        ClientInfo[clientID].iKillsInARow = 0;
        ClientInfo[clientID].bCruiseActivated = false;
        ClientInfo[clientID].bThrusterActivated = false;
        ClientInfo[clientID].bEngineKilled = false;
        ClientInfo[clientID].bTradelane = false;

        // adjust cash, this is necessary when cash was added while use was in
        // charmenu/had other char selected
        std::wstring charName = ToLower(ToWChar(Players.GetActiveCharacterName(clientID)));
        for (auto &i : ClientInfo[clientID].lstMoneyFix) {
            if (i.wscCharname == charName) {
                HkAddCash(charName, i.iAmount);
                ClientInfo[clientID].lstMoneyFix.remove(i);
                break;
            }
        }
    }
    CATCH_HOOK({})
}

void PlayerLaunch__InnerAfter(uint shipID, uint clientID) {
    TRY_HOOK {
        if (!ClientInfo[clientID].iLastExitedBaseID) {
            ClientInfo[clientID].iLastExitedBaseID = 1;

            // event
            ProcessEvent(L"spawn char=%s id=%d system=%s",
                         ToWChar(Players.GetActiveCharacterName(clientID)),
                         clientID, HkGetPlayerSystem(clientID).c_str());
        }
    }
    CATCH_HOOK({})
}

void SPMunitionCollision__Inner(const SSPMunitionCollisionInfo& mci, uint) {
    uint clientIDTarget;

    TRY_HOOK { clientIDTarget = HkGetClientIDByShip(mci.dwTargetShip); }
    CATCH_HOOK({})

    g_DmgTo = clientIDTarget;
}

bool SPObjUpdate__Inner(const SSPObjUpdateInfo& ui, uint clientID) {
    // NAN check
    if (!(ui.vPos.x == ui.vPos.x) || !(ui.vPos.y == ui.vPos.y) ||
        !(ui.vPos.z == ui.vPos.z) || !(ui.vDir.x == ui.vDir.x) ||
        !(ui.vDir.y == ui.vDir.y) || !(ui.vDir.z == ui.vDir.z) ||
        !(ui.vDir.w == ui.vDir.w) || !(ui.fThrottle == ui.fThrottle)) {
        AddLog(LogType::Normal, LogLevel::Info,L"ERROR: NAN found in SPObjUpdate for id=%u", clientID);
        HkKick(Players[clientID].Account);
        return false;
    }

    // Denormalized check
    float n = ui.vDir.w * ui.vDir.w + ui.vDir.x * ui.vDir.x +
              ui.vDir.y * ui.vDir.y + ui.vDir.z * ui.vDir.z;
    if (n > 1.21f || n < 0.81f) {
        AddLog(LogType::Normal, LogLevel::Info,L"ERROR: Non-normalized quaternion found in SPObjUpdate for id=%u", clientID);
        HkKick(Players[clientID].Account);
        return false;
    }

    // Far check
    if (abs(ui.vPos.x) > 1e7f || abs(ui.vPos.y) > 1e7f ||
        abs(ui.vPos.z) > 1e7f) {
        AddLog(LogType::Normal, LogLevel::Info,L"ERROR: Ship position out of bounds in SPObjUpdate for id=%u", clientID);
        HkKick(Players[clientID].Account);
        return false;
    }

    return true;
}

void LaunchComplete__Inner(uint, uint shipID) {
    TRY_HOOK {
        uint clientID = HkGetClientIDByShip(shipID);
        if (clientID) {
            ClientInfo[clientID].tmSpawnTime = timeInMS(); // save for anti-dockkill
            // is there spawnprotection?
		    if (FLHookConfig::i()->general.antiDockKill > 0)
                ClientInfo[clientID].bSpawnProtected = true;
            else
                ClientInfo[clientID].bSpawnProtected = false;
        }

        // event
        ProcessEvent(
            L"launch char=%s id=%d base=%s system=%s",
            ToWChar(Players.GetActiveCharacterName(clientID)), clientID,
            HkGetBaseNickByID(ClientInfo[clientID].iLastExitedBaseID).c_str(),
            HkGetPlayerSystem(clientID).c_str());
    }
    CATCH_HOOK({})
}

std::wstring g_CharBefore;
bool CharacterSelect__Inner(const CHARACTER_ID& cid, uint clientID) {
    try {
        const wchar_t *charName = ToWChar(Players.GetActiveCharacterName(clientID));
        g_CharBefore = charName ? ToWChar(Players.GetActiveCharacterName(clientID)) : L"";
        ClientInfo[clientID].iLastExitedBaseID = 0;
        ClientInfo[clientID].iTradePartner = 0;
    } catch (...) {
        HkAddKickLog(clientID, L"Corrupt character file?");
        HkKick(clientID);
        return false;
    }

    HkCharacterSelect(cid, clientID);
    return true;
}

void CharacterSelect__InnerAfter(const CHARACTER_ID& cId, unsigned int clientID) {
    TRY_HOOK {
        std::wstring charName = ToWChar(Players.GetActiveCharacterName(clientID));

        if (g_CharBefore.compare(charName) != 0) {
            LoadUserCharSettings(clientID);

            if (FLHookConfig::i()->userCommands.userCmdHelp)
                PrintUserCmdText(clientID,
                                 L"To get a list of available commands, type "
                                 L"\"/help\" in chat.");

            // anti-cheat check
            std::list<CARGO_INFO> lstCargo;
            int iHold;
            HkEnumCargo(clientID, lstCargo, iHold);
            for (const auto& cargo : lstCargo) {
                if (cargo.iCount < 0) {
                    HkAddCheaterLog(charName,
                                    L"Negative good-count, likely to have "
                                    L"cheated in the past");

                    wchar_t wszBuf[256];
                    swprintf_s(wszBuf, L"Possible cheating detected (%s)",
                               charName.c_str());
                    HkMsgU(wszBuf);
                    HkBan(clientID, true);
                    HkKick(clientID);
                    return;
                }
            }

            // event
            CAccount *acc = Players.FindAccountFromClientID(clientID);
            std::wstring dir;
            HkGetAccountDirName(acc, dir);
            HKPLAYERINFO pi;
            HkGetPlayerInfo(clientID, pi, false);
            ProcessEvent(L"login char=%s accountdirname=%s id=%d ip=%s",
                         charName.c_str(), dir.c_str(), clientID,
                         pi.wscIP.c_str());
        }
    }
    CATCH_HOOK({})
}

void BaseEnter__Inner(uint baseID, uint clientID) {
    TRY_HOOK {
        if (FLHookConfig::i()->general.autobuy)
            HkPlayerAutoBuy(clientID, baseID);
    }
    CATCH_HOOK({ AddLog(LogType::Normal, LogLevel::Info, L"Exception in BaseEnter on autobuy"); })
}
    
void BaseEnter__InnerAfter(uint baseID, uint clientID) {
    TRY_HOOK {
        // adjust cash, this is necessary when cash was added while use was in
        // charmenu/had other char selected
        std::wstring charName =
            ToLower(ToWChar(Players.GetActiveCharacterName(clientID)));
        for (auto &i : ClientInfo[clientID].lstMoneyFix) {
            if (i.wscCharname == charName) {
                HkAddCash(charName, i.iAmount);
                ClientInfo[clientID].lstMoneyFix.remove(i);
                break;
            }
        }

        // anti base-idle
        ClientInfo[clientID].iBaseEnterTime = static_cast<uint>(time(0));

        // event
        ProcessEvent(L"baseenter char=%s id=%d base=%s system=%s",
                     ToWChar(Players.GetActiveCharacterName(clientID)),
                     clientID, HkGetBaseNickByID(baseID).c_str(),
                     HkGetPlayerSystem(clientID).c_str());

        // print to log if the char has too much money
        float fValue;
        if (HKGetShipValue(
                (const wchar_t *)Players.GetActiveCharacterName(clientID),
                fValue) == HKE_OK) {
            if (fValue > 2100000000.0f) {
                std::wstring charname = (const wchar_t *)Players.GetActiveCharacterName(clientID);
                AddLog(LogType::Normal, LogLevel::Err, L"Possible corrupt ship charname=%s asset_value=%0.0f", charname.c_str(), fValue);
            }
        }
    }
    CATCH_HOOK({})
}

void BaseExit__Inner(uint baseID, uint clientID) {
    TRY_HOOK {
        ClientInfo[clientID].iBaseEnterTime = 0;
        ClientInfo[clientID].iLastExitedBaseID = baseID;
    }
    CATCH_HOOK({})
}

void BaseExit__InnerAfter(uint baseID, uint clientID) {
    TRY_HOOK {
        ProcessEvent(L"baseexit char=%s id=%d base=%s system=%s",
                     ToWChar(Players.GetActiveCharacterName(clientID)),
                     clientID, HkGetBaseNickByID(baseID).c_str(),
                     HkGetPlayerSystem(clientID).c_str());
    }
    CATCH_HOOK({})
}

void TerminateTrade__InnerAfter(uint clientID, int accepted) {
    TRY_HOOK {
        if (accepted) { // save both chars to prevent cheating in case of
                         // server crash
            HkSaveChar(clientID);
            if (ClientInfo[clientID].iTradePartner)
                HkSaveChar(ClientInfo[clientID].iTradePartner);
        }

        if (ClientInfo[clientID].iTradePartner)
            ClientInfo[ClientInfo[clientID].iTradePartner].iTradePartner = 0;
        ClientInfo[clientID].iTradePartner = 0;
    }
    CATCH_HOOK({})
}

void InitiateTrade__Inner(uint clientID1, uint clientID2) {
    if(clientID1 <= MaxClientId && clientID2 <= MaxClientId) {
        ClientInfo[clientID1].iTradePartner = clientID2;
        ClientInfo[clientID2].iTradePartner = clientID1;
    }
}

void ActivateEquip__Inner(uint clientID, const XActivateEquip& aq) {
    TRY_HOOK {
        std::list<CARGO_INFO> lstCargo;
        {
            int _;
            HkEnumCargo(clientID, lstCargo, _);
        }

        for (auto &cargo : lstCargo) {
            if (cargo.iID == aq.sID) {
                Archetype::Equipment *eq = Archetype::GetEquipment(cargo.iArchID);
                EQ_TYPE eqType = HkGetEqType(eq);

                if (eqType == ET_ENGINE) {
                    ClientInfo[clientID].bEngineKilled = !aq.bActivate;
                    if (!aq.bActivate)
                        ClientInfo[clientID].bCruiseActivated = false; // enginekill enabled
                }
            }
        }
    }
    CATCH_HOOK({})
}

void ActivateCruise__Inner(uint clientID, const XActivateCruise& ac) {
    TRY_HOOK {
        ClientInfo[clientID].bCruiseActivated = ac.bActivate;
    }
    CATCH_HOOK({})
}

void ActivateThrusters__Inner(uint clientID, const XActivateThrusters& at) {
    TRY_HOOK {
        ClientInfo[clientID].bThrusterActivated = at.bActivate;
    }
    CATCH_HOOK({})
}

bool GFGoodSell__Inner(const SGFGoodSellInfo& gsi, uint clientID) {
    TRY_HOOK {
        // anti-cheat check
        std::list<CARGO_INFO> lstCargo;
        {
            int _;
            HkEnumCargo(clientID, lstCargo, _);
        }
        bool legalSell = false;
        for (const auto &cargo : lstCargo) {
            if (cargo.iArchID == gsi.iArchID) {
                legalSell = true;
                if (abs(gsi.iCount) > cargo.iCount) {
                    wchar_t buf[512];
                    const auto* charName = ToWChar(Players.GetActiveCharacterName(clientID));
                    swprintf_s(buf,
                        L"Sold more good than possible item=%08x count=%u",
                        gsi.iArchID, gsi.iCount);
                    HkAddCheaterLog(charName, buf);

                    swprintf_s(buf, L"Possible cheating detected (%s)",
                               charName);
                    HkMsgU(buf);
                    HkBan(clientID, true);
                    HkKick(clientID);
                    return false;
                }
                break;
            }
        }
        if (!legalSell) {
            wchar_t buf[1000];
            const auto* charName = ToWChar(Players.GetActiveCharacterName(clientID));
            swprintf_s(buf,
                L"Sold good player does not have (buggy test), item=%08x",
                gsi.iArchID);
            HkAddCheaterLog(charName, buf);

            return false;
        }
    }
    CATCH_HOOK({
        AddLog(LogType::Normal, LogLevel::Info,L"Exception in %s (clientID=%u (%x))", stows(__FUNCTION__).c_str(), clientID, Players.GetActiveCharacterName(clientID));
    })

    return true;
}

bool CharacterInfoReq__Inner(uint clientID, bool) {
    TRY_HOOK {
        if (!ClientInfo[clientID].bCharSelected)
                ClientInfo[clientID].bCharSelected = true;
            else { // pushed f1
                uint shipID = 0;
                pub::Player::GetShip(clientID, shipID);
                if (shipID) { // in space
                    ClientInfo[clientID].tmF1Time = timeInMS() + FLHookConfig::i()->general.antiF1;
                    return false;
                }
            }
        }
    CATCH_HOOK({})

    return true;
}

bool CharacterInfoReq__Catch(uint clientID, bool) {
    HkAddKickLog(clientID, L"Corrupt charfile?");
    HkKick(clientID);
    return false;
}

bool OnConnect__Inner(uint clientID) {
    TRY_HOOK {
        // If ID is too high due to disconnect buffer time then manually drop
        // the connection.
        if (clientID > MaxClientId) {
            AddLog(LogType::Normal, LogLevel::Info,L"INFO: Blocking connect in " __FUNCTION__ " due to invalid id, id=%u", clientID);
            CDPClientProxy *cdpClient = g_cClientProxyArray[clientID - 1];
            if (!cdpClient)
                return false;
            cdpClient->Disconnect();
            return false;
        }

        // If this client is in the anti-F1 timeout then force the disconnect.
        if (ClientInfo[clientID].tmF1TimeDisconnect > timeInMS()) {
            // manual disconnect
            CDPClientProxy *cdpClient = g_cClientProxyArray[clientID - 1];
            if (!cdpClient)
                return false;
            cdpClient->Disconnect();
            return false;
        }

        ClientInfo[clientID].iConnects++;
        ClearClientInfo(clientID);
    }
    CATCH_HOOK({})

    return true;
}

void OnConnect__InnerAfter(uint clientID) {
    TRY_HOOK {
        // event
        std::wstring ip;
        HkGetPlayerIP(clientID, ip);
        ProcessEvent(L"connect id=%d ip=%s", clientID, ip.c_str());
    }
    CATCH_HOOK({})
}

void DisConnect__Inner(uint clientID, EFLConnection) {
    if (clientID <= MaxClientId && clientID > 0 && !ClientInfo[clientID].bDisconnected) {
        ClientInfo[clientID].bDisconnected = true;
        ClientInfo[clientID].lstMoneyFix.clear();
        ClientInfo[clientID].iTradePartner = 0;

        const auto* charName = ToWChar(Players.GetActiveCharacterName(clientID));
        ProcessEvent(L"disconnect char=%s id=%d", charName,
                     clientID);
    }
}

void JumpInComplete__InnerAfter(uint systemID, uint shipID) {
    TRY_HOOK {
        uint clientID = HkGetClientIDByShip(shipID);
        if (!clientID)
            return;

        // event
        ProcessEvent(L"jumpin char=%s id=%d system=%s",
                     ToWChar(Players.GetActiveCharacterName(clientID)),
                     clientID, HkGetSystemNickByID(systemID).c_str());
    }
    CATCH_HOOK({})
}

void SystemSwitchOutComplete__InnerAfter(uint, uint clientID) {
    TRY_HOOK {
        // event
        std::wstring system = HkGetPlayerSystem(clientID);
        ProcessEvent(L"switchout char=%s id=%d system=%s",
                     ToWChar(Players.GetActiveCharacterName(clientID)),
                     clientID, system.c_str());
    }
    CATCH_HOOK({})
}

bool Login__InnerBefore(const SLoginInfo &li, uint clientID) {
    // The startup cache disables reading of the banned file. Check this manually on
    // login and boot the player if they are banned.

    CAccount *acc = Players.FindAccountFromClientID(clientID);
    if (acc) {
        std::wstring wscDir;
        HkGetAccountDirName(acc, wscDir);

        char szDataPath[MAX_PATH];
        GetUserDataPath(szDataPath);

        std::string path = std::string(szDataPath) + "\\Accts\\MultiPlayer\\" +
                           wstos(wscDir) + "\\banned";

        FILE *file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);

            // Ban the player
            st6::wstring flStr((ushort *)acc->wszAccID);
            Players.BanAccount(flStr, true);

            // Kick them
            acc->ForceLogout();

            return false;
        }
    }

    return true;
}

bool Login__InnerAfter(const SLoginInfo& li, uint clientID) {
    TRY_HOOK {
        if (clientID > MaxClientId)
            return false; // DisconnectDelay bug

        if (!HkIsValidClientID(clientID))
            return false; // player was kicked

        // Kick the player if the account ID doesn't exist. This is caused
        // by a duplicate log on.
        CAccount *acc = Players.FindAccountFromClientID(clientID);
        if (acc && !acc->wszAccID) {
            acc->ForceLogout();
            return false;
        }

        bool skip = CallPluginsOther(HookedCall::IServerImpl__Login, HookStep::Mid, li, clientID);
        if(skip)
            return false;

        // check for ip ban
        std::wstring ip;
        HkGetPlayerIP(clientID, ip);

        for (const auto& ban : FLHookConfig::i()->bans.banWildcardsAndIPs) {
            if (Wildcard::Fit(wstos(ban).c_str(), wstos(ip).c_str())) 
            {
                HkAddKickLog(clientID, L"IP/Hostname ban(%s matches %s)",
                             ip.c_str(), ban.c_str());
				if (FLHookConfig::i()->bans.banAccountOnMatch)
                    HkBan(clientID, true);
                HkKick(clientID);
            }
        }

        // resolve
        RESOLVE_IP rip;
        rip.wscIP = ip;
        rip.wscHostname = L"";
        rip.iConnects =
            ClientInfo[clientID].iConnects; // security check so that wrong
                                             // person doesnt get banned
        rip.iClientID = clientID;
        EnterCriticalSection(&csIPResolve);
        g_lstResolveIPs.push_back(rip);
        LeaveCriticalSection(&csIPResolve);

        // count players
        struct PlayerData *playerData = nullptr;
        uint playerCount = 0;
        while ((playerData = Players.traverse_active(playerData)))
            playerCount++;

        if (playerCount > (Players.GetMaxPlayerCount() - FLHookConfig::i()->general.reservedSlots)) { // check if player has a reserved slot
            std::wstring dir;
            HkGetAccountDirName(acc, dir);
            std::string userFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";

            bool reserved = IniGetB(userFile, "Settings", "ReservedSlot", false);
            if (!reserved) {
                HkKick(acc);
                return false;
            }
        }

        LoadUserSettings(clientID);
        
    	HkAddConnectLog(clientID, ip);
    }
    CATCH_HOOK({
        CAccount *acc = Players.FindAccountFromClientID(clientID);
        if (acc)
            acc->ForceLogout();
        return false;
    })

    return true;
}

void GoTradelane__Inner(uint clientID, const XGoTradelane& gtl) {
    if (clientID <= MaxClientId && clientID > 0)
        ClientInfo[clientID].bTradelane = true;
}
        
bool GoTradelane__Catch(uint iClientID, const XGoTradelane& gtl) {
    uint system;
    pub::Player::GetSystem(iClientID, system);
    AddLog(LogType::Normal, LogLevel::Info,L"ERROR: Exception in HkIServerImpl::GoTradelane charname=%s "
           "sys=%08x arch=%08x arch2=%08x",
           wstos(ToWChar(Players.GetActiveCharacterName(iClientID)))
               .c_str(),
           system, gtl.iTradelaneSpaceObj1, gtl.iTradelaneSpaceObj2);
    return true;
}

void StopTradelane__Inner(uint clientID, uint, uint, uint) {
	if (clientID <= MaxClientId && clientID > 0)
        ClientInfo[clientID].bTradelane = false;
}

void Shutdown__InnerAfter() {
    FLHookShutdown();
}

// The maximum number of players we can support is MaxClientId
// Add one to the maximum number to allow renames
int g_MaxPlayers = MaxClientId + 1;

void Startup__Inner(const SStartupInfo &si) {
    FLHookInit_Pre();

    // Startup the server with this number of players.
    char *address = (reinterpret_cast<char*>(hModServer) + ADDR_SRV_PLAYERDBMAXPLAYERSPATCH);
    char nop[] = {'\x90'};
    char movECX[] = {'\xB9'};
    WriteProcMem(address, movECX, sizeof(movECX));
    WriteProcMem(address + 1, &g_MaxPlayers, sizeof(g_MaxPlayers));
    WriteProcMem(address + 5, nop, sizeof(nop));

    StartupCache::Init();
}

void Startup__InnerAfter(const SStartupInfo &si) {
    // Patch to set maximum number of players to connect. This is normally
    // less than MaxClientId
    char *address = (reinterpret_cast<char*>(hModServer) + ADDR_SRV_PLAYERDBMAXPLAYERS);
    WriteProcMem(address, reinterpret_cast<const void*>(&si.iMaxPlayers), sizeof(g_MaxPlayers));

    // read base market data from ini
    HkLoadBaseMarket();

    StartupCache::Done();

    Console::ConInfo(L"FLHook Ready");

    flhookReady = true;
}

}