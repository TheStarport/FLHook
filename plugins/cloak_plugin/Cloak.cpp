// Cloak Plugin for FLHook by Cannon.
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include <FLHook.h>
#include <plugin.h>

static int set_iPluginDebug = 0;

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
ReturnCode returncode;

enum INFO_STATE {
    STATE_CLOAK_INVALID = 0,
    STATE_CLOAK_OFF = 1,
    STATE_CLOAK_CHARGING = 2,
    STATE_CLOAK_ON = 3,
};

struct CLOAK_ARCH {
    std::string scNickName;
    int iWarmupTime;
    int iCooldownTime;
    int iHoldSizeLimit;
    std::map<uint, uint> mapFuelToUsage;
    bool bDropShieldsOnUncloak;
};

struct CLOAK_INFO {
    CLOAK_INFO() {

        uint iCloakSlot = 0;
        bCanCloak = false;
        mstime tmCloakTime = 0;
        uint iState = STATE_CLOAK_INVALID;
        uint bAdmin = false;

        arch.iWarmupTime = 0;
        arch.iCooldownTime = 0;
        arch.iHoldSizeLimit = 0;
        arch.mapFuelToUsage.clear();
        arch.bDropShieldsOnUncloak = false;
    }

    uint iCloakSlot;
    bool bCanCloak;
    mstime tmCloakTime;
    uint iState;
    bool bAdmin;

    CLOAK_ARCH arch;
};

static std::map<uint, CLOAK_INFO> mapClientsCloak;
static std::map<uint, CLOAK_ARCH> mapCloakingDevices;

void LoadSettings() {
    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\cloak.cfg";

    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            CLOAK_ARCH device;
            if (ini.is_header("Cloak")) {
                while (ini.read_value()) {
                    if (ini.is_value("nickname")) {
                        device.scNickName = ini.get_value_string(0);
                    } else if (ini.is_value("warmup_time")) {
                        device.iWarmupTime = ini.get_value_int(0);
                    } else if (ini.is_value("cooldown_time")) {
                        device.iCooldownTime = ini.get_value_int(0);
                    } else if (ini.is_value("hold_size_limit")) {
                        device.iHoldSizeLimit = ini.get_value_int(0);
                    } else if (ini.is_value("fuel")) {
                        std::string scNickName = ini.get_value_string(0);
                        uint usage = ini.get_value_int(1);
                        device.mapFuelToUsage[CreateID(scNickName.c_str())] =
                            usage;
                    } else if (ini.is_value("drop_shields_on_uncloak")) {
                        device.bDropShieldsOnUncloak = ini.get_value_bool(0);
                    }
                }
                mapCloakingDevices[CreateID(device.scNickName.c_str())] =
                    device;
            }
        }
        ini.close();
    }

    struct PlayerData *pd = 0;
    while (pd = Players.traverse_active(pd)) {
    }
}

void ClearClientInfo(uint iClientID) { mapClientsCloak.erase(iClientID); }

void SetCloak(uint iClientID, uint iShipID, bool bOn) {
    XActivateEquip ActivateEq;
    ActivateEq.bActivate = bOn;
    ActivateEq.iSpaceID = iShipID;
    ActivateEq.sID = mapClientsCloak[iClientID].iCloakSlot;
    Server.ActivateEquip(iClientID, ActivateEq);
}

void SetState(uint iClientID, uint iShipID, int iNewState) {
    if (mapClientsCloak[iClientID].iState != iNewState) {
        mapClientsCloak[iClientID].iState = iNewState;
        mapClientsCloak[iClientID].tmCloakTime = timeInMS();
        switch (iNewState) {
        case STATE_CLOAK_CHARGING: {
            PrintUserCmdText(iClientID, L"Preparing to cloak...");
            break;
        }

        case STATE_CLOAK_ON: {
            PrintUserCmdText(iClientID, L" Cloaking device on");
            SetCloak(iClientID, iShipID, true);
            PrintUserCmdText(iClientID, L"Cloaking device on");
            break;
        }
        case STATE_CLOAK_OFF:
        default: {
            PrintUserCmdText(iClientID, L" Cloaking device off");
            SetCloak(iClientID, iShipID, false);
            PrintUserCmdText(iClientID, L"Cloaking device off");
            break;
        }
        }
    }
}

// Returns false if the ship has no fuel to operate its cloaking device.
static bool ProcessFuel(uint iClientID, CLOAK_INFO &info) {
    if (info.bAdmin)
        return true;

    for (auto item = Players[iClientID].equipDescList.equip.begin();
         item != Players[iClientID].equipDescList.equip.end(); item++) {
        if (info.arch.mapFuelToUsage.find(item->iArchID) !=
            info.arch.mapFuelToUsage.end()) {
            uint fuel_usage = info.arch.mapFuelToUsage[item->iArchID];
            if (item->iCount >= fuel_usage) {
                pub::Player::RemoveCargo(iClientID, item->sID, fuel_usage);
                return true;
            }
        }
    }

    return false;
}

void PlayerLaunch_AFTER(unsigned int iShip, unsigned int iClientID) {
    mapClientsCloak[iClientID].bCanCloak = false;
    mapClientsCloak[iClientID].bAdmin = false;

    IObjInspectImpl *obj = HkGetInspect(iClientID);
    if (obj) {
        CShip *cship = (CShip *)HkGetEqObjFromObjRW((IObjRW *)obj);

        CEquipTraverser tr;

        for (CEquip *equip = GetEquipManager(cship)->Traverse(tr); equip;
             equip = GetEquipManager(cship)->Traverse(tr)) {
            if (CECloakingDevice::cast(equip)) {
                mapClientsCloak[iClientID].iCloakSlot = equip->GetID();

                if (mapCloakingDevices.find(equip->EquipArch()->iArchID) !=
                    mapCloakingDevices.end()) {
                    // Otherwise set the fuel usage and warm up time
                    mapClientsCloak[iClientID].arch =
                        mapCloakingDevices[equip->EquipArch()->iArchID];
                }
                // If this cloaking device does not appear in the cloaking
                // device list then warming up and fuel usage is zero and it may
                // be used by any ship.
                else {
                    mapClientsCloak[iClientID].arch.bDropShieldsOnUncloak =
                        false;
                    mapClientsCloak[iClientID].arch.iCooldownTime = 0;
                    mapClientsCloak[iClientID].arch.iHoldSizeLimit = 0;
                    mapClientsCloak[iClientID].arch.iWarmupTime = 0;
                    mapClientsCloak[iClientID].arch.mapFuelToUsage.clear();
                }

                mapClientsCloak[iClientID].bCanCloak = true;
                mapClientsCloak[iClientID].iState = STATE_CLOAK_INVALID;
                SetState(iClientID, iShip, STATE_CLOAK_OFF);
                return;
            }
        }
    }
}

void BaseEnter(unsigned int iBaseID, unsigned int iClientID) {
    mapClientsCloak.erase(iClientID);
}

void HkTimerCheckKick() {
    mstime now = timeInMS();

    for (std::map<uint, CLOAK_INFO>::iterator ci = mapClientsCloak.begin();
         ci != mapClientsCloak.end(); ++ci) {
        uint iClientID = ci->first;
        uint iShipID = Players[iClientID].iShipID;
        CLOAK_INFO &info = ci->second;

        if (iShipID && info.bCanCloak) {
            switch (info.iState) {
            case STATE_CLOAK_OFF:
                // Send cloak state for uncloaked cloak-able players (only for
                // them in space) this is the code to fix the bug where players
                // wouldnt always see uncloaked players
                XActivateEquip ActivateEq;
                ActivateEq.bActivate = false;
                ActivateEq.iSpaceID = iShipID;
                ActivateEq.sID = info.iCloakSlot;
                Server.ActivateEquip(iClientID, ActivateEq);
                break;

            case STATE_CLOAK_CHARGING:
                if (!ProcessFuel(iClientID, info)) {
                    PrintUserCmdText(iClientID,
                                     L"Cloaking device shutdown, no fuel");
                    SetState(iClientID, iShipID, STATE_CLOAK_OFF);
                } else if ((info.tmCloakTime + info.arch.iWarmupTime) < now) {
                    SetState(iClientID, iShipID, STATE_CLOAK_ON);
                } else if (info.arch.bDropShieldsOnUncloak && !info.bAdmin) {
                    pub::SpaceObj::DrainShields(iShipID);
                }
                break;

            case STATE_CLOAK_ON:
                if (!ProcessFuel(iClientID, info)) {
                    PrintUserCmdText(iClientID,
                                     L"Cloaking device shutdown, no fuel");
                    SetState(iClientID, iShipID, STATE_CLOAK_OFF);
                } else if (info.arch.bDropShieldsOnUncloak && !info.bAdmin) {
                    pub::SpaceObj::DrainShields(iShipID);
                }
                break;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Cloak(uint iClientID, const std::wstring &wscParam) {
    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    if (!iShip) {
        PrintUserCmdText(iClientID, L"Not in space");
        return;
    }

    if (!mapClientsCloak[iClientID].bCanCloak) {
        PrintUserCmdText(iClientID, L"Cloaking device not available");
        return;
    }

    // If this cloaking device requires more power than the ship can provide
    // no cloaking device is available.
    IObjInspectImpl *obj = HkGetInspect(iClientID);
    if (obj) {
        CShip *cship = (CShip *)HkGetEqObjFromObjRW((IObjRW *)obj);
        if (cship) {
            if (mapClientsCloak[iClientID].arch.iHoldSizeLimit != 0 &&
                mapClientsCloak[iClientID].arch.iHoldSizeLimit <
                    cship->shiparch()->fHoldSize) {
                PrintUserCmdText(
                    iClientID,
                    L"Cloaking device will not function on this ship type");
                mapClientsCloak[iClientID].iState = STATE_CLOAK_INVALID;
                SetState(iClientID, iShip, STATE_CLOAK_OFF);
                return;
            }

            switch (mapClientsCloak[iClientID].iState) {
            case STATE_CLOAK_OFF:
                SetState(iClientID, iShip, STATE_CLOAK_CHARGING);
                break;
            case STATE_CLOAK_CHARGING:
            case STATE_CLOAK_ON:
                SetState(iClientID, iShip, STATE_CLOAK_OFF);
                break;
            }
        }
    }
    return;
}

USERCMD UserCmds[] = {
    {L"/cloak", UserCmd_Cloak},
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

bool ExecuteCommandString(CCmds *cmds, const std::wstring &wscCmd) {
    

    if (IS_CMD("cloak")) {
        returncode = ReturnCode::SkipAll;

        uint iClientID = HkGetClientIdFromCharname(cmds->GetAdminName());
        if (iClientID == -1) {
            cmds->Print(L"ERR On console");
            return true;
        }

        uint iShip;
        pub::Player::GetShip(iClientID, iShip);
        if (!iShip) {
            PrintUserCmdText(iClientID, L"ERR Not in space");
            return true;
        }

        if (!mapClientsCloak[iClientID].bCanCloak) {
            cmds->Print(L"ERR Cloaking device not available");
            return true;
        }

        switch (mapClientsCloak[iClientID].iState) {
        case STATE_CLOAK_OFF:
            mapClientsCloak[iClientID].bAdmin = true;
            SetState(iClientID, iShip, STATE_CLOAK_ON);
            break;
        case STATE_CLOAK_CHARGING:
        case STATE_CLOAK_ON:
            mapClientsCloak[iClientID].bAdmin = false;
            SetState(iClientID, iShip, STATE_CLOAK_OFF);
            break;
        }
        return true;
    }
    return false;
}

void __stdcall HkCb_AddDmgEntry(DamageList *dmg, unsigned short p1,
                                float damage,
                                enum DamageEntry::SubObjFate fate) {
    
    if (g_DmgToSpaceID && dmg->get_inflictor_id()) {
        if (dmg->get_cause() == 0x06) {
            float curr, max;
            pub::SpaceObj::GetHealth(g_DmgToSpaceID, curr, max);
            uint client = HkGetClientIDByShip(g_DmgToSpaceID);
            if (client) {
                if (mapClientsCloak[client].bCanCloak &&
                    !mapClientsCloak[client].bAdmin &&
                    mapClientsCloak[client].iState == STATE_CLOAK_CHARGING) {
                    SetState(client, g_DmgToSpaceID, STATE_CLOAK_OFF);
                }
            }
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));
    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (set_scCfgFile.length() > 0)
            LoadSettings();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Cloak by Cannon");
    pi->shortName("cloak");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch_AFTER, HookStep::After);
    pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
    pi->emplaceHook(HookedCall::IEngine__AddDamageEntry, &HkCb_AddDmgEntry);

}
