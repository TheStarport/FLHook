// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.
//
// This file includes code that was not written by me but I can't find
// the original author (I know they posted on the-starport.net about it).

#include <FLHook.h>
#include <algorithm>
#include <float.h>
#include <list>
#include <math.h>
#include <plugin.h>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#include "Main.h"

#include "Shlwapi.h"

#include <FLCoreCommon.h>
#include <FLCoreServer.h>

namespace SystemSensor {
enum MODE {
    MODE_OFF = 0x00,
    MODE_JUMPGATE = 0x01,
    MODE_TRADELANE = 0x02,
};

struct SENSOR {
    uint iSystemID;
    uint iEquipID;
    uint iNetworkID;
};

/// Map of equipment and systems that have sensor networks
static std::multimap<unsigned int, SENSOR> set_mmapSensorEquip;
static std::multimap<unsigned int, SENSOR> set_mmapSensorSystem;

struct INFO {
    INFO()
        : iAvailableNetworkID(0), iLastScanNetworkID(0), bInJumpGate(false),
          iMode(MODE_OFF) {}
    uint iAvailableNetworkID;

    std::list<CARGO_INFO> lstLastScan;
    uint iLastScanNetworkID;

    bool bInJumpGate;

    uint iMode;
};

static std::map<UINT, INFO> mapInfo;

void LoadSettings(const std::string &scPluginCfgFile) {
    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("SystemSensor")) {
                while (ini.read_value()) {
                    SENSOR sensor;
                    sensor.iSystemID = CreateID(ini.get_name_ptr());
                    sensor.iEquipID = CreateID(ini.get_value_string(0));
                    sensor.iNetworkID = ini.get_value_int(1);
                    set_mmapSensorEquip.insert(
                        std::multimap<uint, SENSOR>::value_type(sensor.iEquipID,
                                                                sensor));
                    set_mmapSensorSystem.insert(
                        std::multimap<uint, SENSOR>::value_type(
                            sensor.iSystemID, sensor));
                }
            }
        }
        ini.close();
    }
}

bool UserCmd_Net(uint iClientID, const std::wstring &wscCmd,
                 const std::wstring &wscParam, const wchar_t *usage) {
    std::wstring wscMode = ToLower(GetParam(wscParam, ' ', 0));
    if (wscMode.size() == 0) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, usage);
        return true;
    }

    if (!mapInfo[iClientID].iAvailableNetworkID) {
        PrintUserCmdText(iClientID,
                         L"ERR Sensor network monitoring is not available");
        mapInfo[iClientID].iMode = MODE_OFF;
        return true;
    }

    if (wscMode == L"all") {
        PrintUserCmdText(iClientID,
                         L"OK Sensor network monitoring all traffic");
        mapInfo[iClientID].iMode = MODE_JUMPGATE | MODE_TRADELANE;
    } else if (wscMode == L"jumponly") {
        PrintUserCmdText(iClientID,
                         L"OK Sensor network monitoring jumpgate traffic only");
        mapInfo[iClientID].iMode = MODE_JUMPGATE;
    } else {
        PrintUserCmdText(iClientID, L"OK Sensor network monitoring disabled");
        mapInfo[iClientID].iMode = MODE_OFF;
    }
    return true;
}

bool UserCmd_ShowScan(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage) {
    std::wstring wscTargetCharname = GetParam(wscParam, ' ', 0);
    if (wscCmd.find(L"/showscan$ ") == 0) {
        wchar_t *wszTargetCharname =
            (wchar_t *)Players.GetActiveCharacterName(ToInt(wscTargetCharname));
        if (wszTargetCharname)
            wscTargetCharname = wszTargetCharname;
    }

    if (wscTargetCharname.size() == 0) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, usage);
        return true;
    }

    uint iTargetClientID = HkGetClientIDFromArg(wscTargetCharname);
    if (iTargetClientID == -1) {
        PrintUserCmdText(iClientID, L"ERR Target not found");
        return true;
    }

    auto iterTargetClientID = mapInfo.find(iTargetClientID);
    if (iterTargetClientID == mapInfo.end() ||
        !mapInfo[iClientID].iAvailableNetworkID ||
        !iterTargetClientID->second.iLastScanNetworkID ||
        mapInfo[iClientID].iAvailableNetworkID !=
            iterTargetClientID->second.iLastScanNetworkID) {
        PrintUserCmdText(iClientID, L"ERR Scan data not available");
        return true;
    }

    std::wstring wscEqList;
    for (auto &ci : iterTargetClientID->second.lstLastScan) {
        std::string scHardpoint = ci.hardpoint.value;
        if (scHardpoint.length()) {
            Archetype::Equipment *eq = Archetype::GetEquipment(ci.iArchID);
            if (eq && eq->iIdsName) {
                std::wstring wscResult;
                switch (HkGetEqType(eq)) {
                case ET_GUN:
                case ET_MISSILE:
                case ET_CD:
                case ET_CM:
                case ET_TORPEDO:
                case ET_OTHER:
                    if (wscEqList.length())
                        wscEqList += L",";
                    wscResult = HkGetWStringFromIDS(eq->iIdsName);
                    wscEqList += wscResult;
                    break;
                default:
                    break;
                }
            }
        }
    }
    PrintUserCmdText(iClientID, L"%s", wscEqList.c_str());
    PrintUserCmdText(iClientID, L"OK");
    return true;
}

void ClearClientInfo(uint iClientID) { mapInfo.erase(iClientID); }

static void EnableSensorAccess(uint iClientID) {
    // Retrieve the location and cargo list.
    int iHoldSize;
    std::list<CARGO_INFO> lstCargo;
    HkEnumCargo((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                lstCargo, iHoldSize);

    unsigned int iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);

    // If this is ship has the right equipment and is in the right system then
    // enable access.
    uint iAvailableNetworkID = 0;
    for (auto &ci : lstCargo) {
        if (ci.bMounted) {
            auto start = set_mmapSensorEquip.lower_bound(ci.iArchID);
            auto end = set_mmapSensorEquip.upper_bound(ci.iArchID);
            while (start != end) {
                if (start->second.iSystemID == iSystemID) {
                    iAvailableNetworkID = start->second.iNetworkID;
                    break;
                }
                ++start;
            }
        }
    }

    if (iAvailableNetworkID != mapInfo[iClientID].iAvailableNetworkID) {
        mapInfo[iClientID].iAvailableNetworkID = iAvailableNetworkID;
        if (iAvailableNetworkID)
            PrintUserCmdText(iClientID,
                             L"Connection to tradelane sensor network "
                             L"established. Type /net access network.");
        else
            PrintUserCmdText(iClientID,
                             L"Connection to tradelane sensor network lost.");
    }
}

void PlayerLaunch(unsigned int iShip, unsigned int iClientID) {
    EnableSensorAccess(iClientID);
}

static void DumpSensorAccess(uint iClientID, const std::wstring &wscType,
                             uint iType) {
    unsigned int iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);

    // Find the sensor network for this system.
    auto siter = set_mmapSensorSystem.lower_bound(iSystemID);
    auto send = set_mmapSensorSystem.upper_bound(iSystemID);
    if (siter == send)
        return;

    if (mapInfo.find(iClientID) == mapInfo.end()) {
        SystemSensor::ClearClientInfo(iClientID);
    }

    // Record the ship's cargo.
    int iHoldSize;
    HkEnumCargo((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                mapInfo[iClientID].lstLastScan, iHoldSize);
    mapInfo[iClientID].iLastScanNetworkID = siter->second.iNetworkID;

    // Notify any players connected to the the sensor network that this ship is
    // in
    auto piter = mapInfo.begin();
    auto pend = mapInfo.end();
    while (piter != pend) {
        if (piter->second.iAvailableNetworkID == siter->second.iNetworkID) {
            const Universe::ISystem *iSys = Universe::get_system(iSystemID);
            if (iSys) {
                if (piter->second.iMode & iType) {
                    std::wstring wscSysName =
                        HkGetWStringFromIDS(iSys->strid_name);
                    PrintUserCmdText(piter->first, L"%s[$%u] %s at %s %s",
                                     Players.GetActiveCharacterName(iClientID),
                                     iClientID, wscType.c_str(),
                                     wscSysName.c_str(),
                                     GetLocation(iClientID).c_str());
                }
            }
        }
        piter++;
    }
}

// Record jump type.
void Dock_Call(unsigned int const &iShip, unsigned int const &iDockTarget,
               int iCancel, enum DOCK_HOST_RESPONSE response) {
    uint iClientID = HkGetClientIDByShip(iShip);
    if (iClientID && (response == PROCEED_DOCK || response == DOCK) &&
        !iCancel) {
        uint iTypeID;
        pub::SpaceObj::GetType(iDockTarget, iTypeID);
        if (iTypeID == OBJ_JUMP_GATE) {
            mapInfo[iClientID].bInJumpGate = true;
        } else {
            mapInfo[iClientID].bInJumpGate = false;
        }
    }
}

void JumpInComplete(unsigned int iSystem, unsigned int iShip,
                    unsigned int iClientID) {
    EnableSensorAccess(iClientID);
    if (mapInfo[iClientID].bInJumpGate) {
        mapInfo[iClientID].bInJumpGate = false;
        DumpSensorAccess(iClientID, L"exited jumpgate", MODE_JUMPGATE);
    }
}

void GoTradelane(unsigned int iClientID, struct XGoTradelane const &xgt) {
    DumpSensorAccess(iClientID, L"entered tradelane", MODE_TRADELANE);
}

void StopTradelane(unsigned int iClientID, unsigned int p1, unsigned int p2,
                   unsigned int p3) {
    DumpSensorAccess(iClientID, L"exited tradelane", MODE_TRADELANE);
}
} // namespace SystemSensor
