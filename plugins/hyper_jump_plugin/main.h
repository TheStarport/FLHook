#pragma once

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

bool InitJumpDriveInfo(uint iClientID);

/// Zone testing state and lists.
struct TESTBOT {
    bool bBaseTest;
    int iCheckZoneTime;
    int iCheckSystemOrBase;
    int iCheckZonesTimer;
    int iCheckTestedZones;
    std::list<ZONE> lstCheckZones;
};

static std::map<uint, TESTBOT> mapTestBots;

struct DEFERREDJUMPS {
    uint system;
    Vector pos;
    Matrix ornt;
};

static std::map<uint, DEFERREDJUMPS> mapDeferredJumps;

struct JUMPDRIVE_ARCH {
    uint nickname;
    float can_jump_charge;
    float charge_rate;
    float discharge_rate;
    std::vector<uint> charge_fuse;
    uint jump_fuse;
    std::map<uint, uint> mapFuelToUsage;
    float power;
    float field_range;
};

static std::map<uint, JUMPDRIVE_ARCH> mapJumpDriveArch;

struct SURVEY_ARCH {
    uint nickname;
    float survey_complete_charge;
    float charge_rate;
    std::map<uint, uint> mapFuelToUsage;
    float power;
    float coord_accuracy;
};

static std::map<uint, SURVEY_ARCH> mapSurveyArch;

struct SURVEY {
    SURVEY_ARCH arch;
    float curr_charge;
    bool charging_on;
};

static std::map<uint, SURVEY> mapSurvey;

struct JUMPDRIVE {
    JUMPDRIVE_ARCH arch;

    bool charging_on;
    float curr_charge;
    uint active_fuse;
    std::list<uint> active_charge_fuse;
    bool charging_complete;
    uint charge_status;

    int jump_timer;
    uint iTargetSystem;
    Vector vTargetPosition;
};

static std::map<uint, JUMPDRIVE> mapJumpDrives;

#define HCOORD_SIZE 28

struct HYPERSPACE_COORDS {
    WORD parity;
    WORD seed;
    DWORD system;
    float x;
    float y;
    float z;
    DWORD time;
    float accuracy;
};

static std::string set_scEncryptKey = "secretcode";

static std::map<uint, bool> set_death_systems;
