#pragma once

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

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
