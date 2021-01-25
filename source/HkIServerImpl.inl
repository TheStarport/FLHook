// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
#pragma once
#include <CInGame.h>
#include <Hook.h>

namespace HkIServerImpl {

struct Timer {
    std::function<void()> func;
    mstime interval;
    mstime lastTime = 0;
};

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

    auto currentTime = timeInMS();
    for (auto& timer : g_Timers) {
        if ((currentTime - timer.lastTime) >= timer.interval) {
            timer.lastTime = currentTime;
            timer.func();
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
    TRY_HOOK {

        // Group join/leave commands are not parsed
        if (cidTo.iID == SpecialChatIDs::GROUP_EVENT)
            return true;

        // Anything outside normal bounds is aborted to prevent crashes
        if(cidTo.iID > SpecialChatIDs::GROUP_EVENT
           || cidTo.iID > SpecialChatIDs::PLAYER_MAX && cidTo.iID < SpecialChatIDs::SPECIAL_BASE)
            return false;

        BinaryRDLReader rdl;
        std::wstring buffer;
        buffer.resize(1024);
        {
            uint _;
            rdl.extract_text_from_buffer(ToUShort(buffer.data()), buffer.size(),
                                         _, static_cast<const char*>(rdlReader), size);
        }

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
        if (set_bDefaultLocalChat && cidTo.iID == SpecialChatIDs::SYSTEM) {
            cidTo.iID = SpecialChatIDs::LOCAL;
        }

        // fix flserver commands and change chat to id so that event logging is
        // accurate.
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
        }

        if (UserCmd_Process(cidFrom.iID, buffer))
            return false;

        if (buffer[0] == '.') {
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
                g_Admin.clientID = cidFrom.iID;
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

        // check if chat should be suppressed
        if(!set_setChatSuppress.empty()) {
            auto lcBuffer = ToLower(buffer);
            for (const auto &chat : set_setChatSuppress) {
                if (lcBuffer.find(chat) == 0)
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

    iDmgTo = clientIDTarget;
}

bool SPObjUpdate__Inner(const SSPObjUpdateInfo& ui, uint clientID) {
    // NAN check
    if (!(ui.vPos.x == ui.vPos.x) || !(ui.vPos.y == ui.vPos.y) ||
        !(ui.vPos.z == ui.vPos.z) || !(ui.vDir.x == ui.vDir.x) ||
        !(ui.vDir.y == ui.vDir.y) || !(ui.vDir.z == ui.vDir.z) ||
        !(ui.vDir.w == ui.vDir.w) || !(ui.fThrottle == ui.fThrottle)) {
        AddLog("ERROR: NAN found in SPObjUpdate for id=%u", clientID);
        HkKick(Players[clientID].Account);
        return false;
    }

    // Denormalized check
    float n = ui.vDir.w * ui.vDir.w + ui.vDir.x * ui.vDir.x +
              ui.vDir.y * ui.vDir.y + ui.vDir.z * ui.vDir.z;
    if (n > 1.21f || n < 0.81f) {
        AddLog("ERROR: Non-normalized quaternion found in SPObjUpdate for id=%u", clientID);
        HkKick(Players[clientID].Account);
        return false;
    }

    // Far check
    if (abs(ui.vPos.x) > 1e7f || abs(ui.vPos.y) > 1e7f ||
        abs(ui.vPos.z) > 1e7f) {
        AddLog("ERROR: Ship position out of bounds in SPObjUpdate for id=%u", clientID);
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
            if (set_iAntiDockKill > 0)
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
        HkKick(ARG_CLIENTID(clientID));
        return false;
    }

    return true;
}

void CharacterSelect__InnerAfter(const CHARACTER_ID& cId, unsigned int clientID) {
    TRY_HOOK {
        std::wstring charName = ToWChar(Players.GetActiveCharacterName(clientID));

        if (g_CharBefore.compare(charName) != 0) {
            LoadUserCharSettings(clientID);

            if (set_bUserCmdHelp)
                PrintUserCmdText(clientID,
                                 L"To get a list of available commands, type "
                                 L"\"/help\" in chat.");

            // anti-cheat check
            std::list<CARGO_INFO> lstCargo;
            int iHold;
            HkEnumCargo(ARG_CLIENTID(clientID), lstCargo, iHold);
            for (const auto& cargo : lstCargo) {
                if (cargo.iCount < 0) {
                    HkAddCheaterLog(charName,
                                    L"Negative good-count, likely to have "
                                    L"cheated in the past");

                    wchar_t wszBuf[256];
                    swprintf_s(wszBuf, L"Possible cheating detected (%s)",
                               charName.c_str());
                    HkMsgU(wszBuf);
                    HkBan(ARG_CLIENTID(clientID), true);
                    HkKick(ARG_CLIENTID(clientID));
                    return;
                }
            }

            // event
            CAccount *acc = Players.FindAccountFromClientID(clientID);
            std::wstring dir;
            HkGetAccountDirName(acc, dir);
            HKPLAYERINFO pi;
            HkGetPlayerInfo(ARG_CLIENTID(clientID), pi, false);
            ProcessEvent(L"login char=%s accountdirname=%s id=%d ip=%s",
                         charName.c_str(), dir.c_str(), clientID,
                         pi.wscIP.c_str());
        }
    }
    CATCH_HOOK({})
}

void BaseEnter__Inner(uint baseID, uint clientID) {
    TRY_HOOK {
        if (set_bAutoBuy)
            HkPlayerAutoBuy(clientID, baseID);
    }
    CATCH_HOOK({ AddLog("Exception in BaseEnter on autobuy"); })
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

}