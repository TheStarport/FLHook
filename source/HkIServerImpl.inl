// ReSharper disable CppNonInlineFunctionDefinitionInHeaderFile
// ReSharper disable CppNonInlineVariableDefinitionInHeaderFile
#pragma once
#include <CInGame.h>
#include <Hook.h>

namespace HkIServerImpl {

// add timers here
typedef void (*_TimerFunc)();

struct TIMER {
    std::function<void()> func;
    mstime interval;
    mstime lastTime = 0;
};

TIMER g_Timers[] = {
    { ProcessPendingCommands, 50 },
    { HkTimerCheckKick, 1000 },
    { HkTimerNPCAndF1Check, 50 },
    { HkTimerCheckResolveResults, 0 },
};

void Update__Inner() {
    static bool bFirstTime = true;
    if (bFirstTime) {
        FLHookInit();
        bFirstTime = false;
    }

    auto currentTime = timeInMS();
    for (auto& timer : g_Timers) {
        if ((currentTime - timer.lastTime) >= timer.interval) {
            timer.lastTime = currentTime;
            timer.func();
        }
    }

    char *pData;
    memcpy(&pData, g_FLServerDataPtr + 0x40, 4);
    memcpy(&g_iServerLoad, pData + 0x204, 4);
    memcpy(&g_iPlayerCount, pData + 0x208, 4);
}

/**************************************************************************************************************
Chat-Messages are hooked here
<Parameters>
cId:       Sender's ClientID
lP1:       size of rdlReader (used when extracting text from that buffer)
rdlReader: RenderDisplayList which contains the chat-text
cIdTo:     recipient's clientid(0x10000 = universe chat else when (cIdTo &
0x10000) = true -> system chat) iP2:       ???
**************************************************************************************************************/

CInGame g_Admin;
bool g_InSubmitChat = false;
uint g_TextLength = 0;

bool SubmitChat__Inner(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID& cidTo, int) {
    TRY_HOOK {

        // Group join/leave commands
        if (cidTo.iID == 0x10004)
            return true;

        // extract text from rdlReader
        BinaryRDLReader rdl;
        std::wstring buffer;
        buffer.resize(1024);
        uint _;
        rdl.extract_text_from_buffer(reinterpret_cast<ushort*>(buffer.data()), buffer.size(),
                                     _, static_cast<const char*>(rdlReader), size);
        uint iClientID = cidFrom.iID;

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
        if (set_bDefaultLocalChat && cidTo.iID == 0x10001) {
            cidTo.iID = 0x10002;
        }

        // fix flserver commands and change chat to id so that event logging is
        // accurate.
        g_TextLength = static_cast<uint>(buffer.length());
        if (!buffer.find(L"/g ")) {
            cidTo.iID = 0x10003;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/l ")) {
            cidTo.iID = 0x10002;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/s ")) {
            cidTo.iID = 0x10001;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/u ")) {
            cidTo.iID = 0x10000;
            g_TextLength -= 3;
        } else if (!buffer.find(L"/group ")) {
            cidTo.iID = 0x10003;
            g_TextLength -= 7;
        } else if (!buffer.find(L"/local ")) {
            cidTo.iID = 0x10002;
            g_TextLength -= 7;
        } else if (!buffer.find(L"/system ")) {
            cidTo.iID = 0x10001;
            g_TextLength -= 8;
        } else if (!buffer.find(L"/universe ")) {
            cidTo.iID = 0x10000;
            g_TextLength -= 10;
        }

        // check for user cmds
        if (UserCmd_Process(iClientID, buffer))
            return false;

        if (buffer[0] == '.') { // flhook admin command
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscAccDirname;

            HkGetAccountDirName(acc, wscAccDirname);
            std::string scAdminFile =
                scAcctPath + wstos(wscAccDirname) + "\\flhookadmin.ini";
            WIN32_FIND_DATA fd;
            HANDLE hFind = FindFirstFile(scAdminFile.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) { // is admin
                FindClose(hFind);
                g_Admin.ReadRights(scAdminFile);
                g_Admin.iClientID = iClientID;
                g_Admin.wscAdminName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));
                g_Admin.ExecuteCommandString(buffer.data() + 1);
                return false;
            }
        }

        // process chat event
        std::wstring wscEvent;
        wscEvent.reserve(256);
        wscEvent = L"chat";
        wscEvent += L" from=";
        const auto *wszFrom = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(cidFrom.iID));
        if (!cidFrom.iID)
            wscEvent += L"console";
        else if (!wszFrom)
            wscEvent += L"unknown";
        else
            wscEvent += wszFrom;

        wscEvent += L" id=";
        wscEvent += std::to_wstring(cidFrom.iID);

        wscEvent += L" type=";
        if (cidTo.iID == 0x00010000)
            wscEvent += L"universe";
        else if (cidTo.iID == 0x10003) {
            wscEvent += L"group";
            wscEvent += L" grpidto=";
            wscEvent += std::to_wstring(Players.GetGroupID(cidFrom.iID));
        } else if (cidTo.iID & 0x00010000)
            wscEvent += L"system";
        else {
            wscEvent += L"player";
            wscEvent += L" to=";

            const auto *wszTo = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(cidTo.iID));
            if (!cidTo.iID)
                wscEvent += L"console";
            else if (!wszTo)
                wscEvent += L"unknown";
            else
                wscEvent += wszTo;

            wscEvent += L" idto=";
            wscEvent += std::to_wstring(cidTo.iID);
        }

        wscEvent += L" text=";
        wscEvent += buffer;
        ProcessEvent(L"%s", wscEvent.c_str());

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

void PlayerLaunch__Inner(unsigned int iShip, unsigned int iClientID) {
    TRY_HOOK {
        ClientInfo[iClientID].iShip = iShip;
        ClientInfo[iClientID].iKillsInARow = 0;
        ClientInfo[iClientID].bCruiseActivated = false;
        ClientInfo[iClientID].bThrusterActivated = false;
        ClientInfo[iClientID].bEngineKilled = false;
        ClientInfo[iClientID].bTradelane = false;

        // adjust cash, this is necessary when cash was added while use was in
        // charmenu/had other char selected
        std::wstring wscCharname =
            ToLower((wchar_t *)Players.GetActiveCharacterName(iClientID));
        for (auto &i : ClientInfo[iClientID].lstMoneyFix) {
            if (!i.wscCharname.compare(wscCharname)) {
                HkAddCash(wscCharname, i.iAmount);
                ClientInfo[iClientID].lstMoneyFix.remove(i);
                break;
            }
        }
    }
    CATCH_HOOK({})
}

void PlayerLaunch__InnerAfter(unsigned int iShip, unsigned int iClientID) {
    TRY_HOOK {
        if (!ClientInfo[iClientID].iLastExitedBaseID) {
            ClientInfo[iClientID].iLastExitedBaseID = 1;

            // event
            ProcessEvent(L"spawn char=%s id=%d system=%s",
                         (wchar_t *)Players.GetActiveCharacterName(iClientID),
                         iClientID, HkGetPlayerSystem(iClientID).c_str());
        }
    }
    CATCH_HOOK({})
}

void SPMunitionCollision__Inner(struct SSPMunitionCollisionInfo const &ci,
                                   unsigned int iClientID) {
    uint iClientIDTarget;

    TRY_HOOK { iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip); }
    CATCH_HOOK({})

    iDmgTo = iClientIDTarget;
}

bool SPObjUpdate__Inner(struct SSPObjUpdateInfo const &ui,
                           unsigned int iClientID) {
    // NAN check
    if (!(ui.vPos.x == ui.vPos.x) || !(ui.vPos.y == ui.vPos.y) ||
        !(ui.vPos.z == ui.vPos.z) || !(ui.vDir.x == ui.vDir.x) ||
        !(ui.vDir.y == ui.vDir.y) || !(ui.vDir.z == ui.vDir.z) ||
        !(ui.vDir.w == ui.vDir.w) || !(ui.fThrottle == ui.fThrottle)) {
        AddLog("ERROR: NAN found in " __FUNCTION__ " for id=%u", iClientID);
        HkKick(Players[iClientID].Account);
        return false;
    };

    float n = ui.vDir.w * ui.vDir.w + ui.vDir.x * ui.vDir.x +
              ui.vDir.y * ui.vDir.y + ui.vDir.z * ui.vDir.z;
    if (n > 1.21f || n < 0.81f) {
        AddLog(
            "ERROR: Non-normalized quaternion found in " __FUNCTION__ " for "
                                                                      "id=%u",
            iClientID);
        HkKick(Players[iClientID].Account);
        return false;
    }

    // Far check
    if (abs(ui.vPos.x) > 1e7f || abs(ui.vPos.y) > 1e7f ||
        abs(ui.vPos.z) > 1e7f) {
        AddLog(
            "ERROR: Ship position out of bounds in " __FUNCTION__ " for id=%u",
            iClientID);
        HkKick(Players[iClientID].Account);
        return false;
    }

    return true;
}

}