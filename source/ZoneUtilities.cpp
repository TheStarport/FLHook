// Jan 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Hook.h"
#include "global.h"
#include <algorithm>
#include <cmath>
#include <float.h>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

/** A map of system id to system info */
std::map<uint, SYSTEMINFO> mapSystems;

/** A map of system id to zones */
std::multimap<uint, ZONE> zones;

/** A map of system id to jumppoint info */
std::multimap<uint, JUMPPOINT> jumpPoints;

/** Multiply mat1 by mat2 and return the result */
static TransformMatrix MultiplyMatrix(TransformMatrix &mat1,
                                      TransformMatrix &mat2) {
    TransformMatrix result = {0};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            for (int k = 0; k < 4; k++)
                result.d[i][j] += mat1.d[i][k] * mat2.d[k][j];
    return result;
}

/** Setup of transformation matrix using the vector p and the rotation r */
static TransformMatrix SetupTransform(Vector &p, Vector &r) {
// Convert degrees into radians
#define PI (3.14159265358f)
    float ax = r.x * (PI / 180);
    float ay = r.y * (PI / 180);
    float az = r.z * (PI / 180);

    // Initial matrix
    TransformMatrix smat = {0};
    smat.d[0][0] = smat.d[1][1] = smat.d[2][2] = smat.d[3][3] = 1;

    // Translation matrix
    TransformMatrix tmat;
    tmat.d[0][0] = 1;
    tmat.d[0][1] = 0;
    tmat.d[0][2] = 0;
    tmat.d[0][3] = 0;
    tmat.d[1][0] = 0;
    tmat.d[1][1] = 1;
    tmat.d[1][2] = 0;
    tmat.d[1][3] = 0;
    tmat.d[2][0] = 0;
    tmat.d[2][1] = 0;
    tmat.d[2][2] = 1;
    tmat.d[2][3] = 0;
    tmat.d[3][0] = -p.x;
    tmat.d[3][1] = -p.y;
    tmat.d[3][2] = -p.z;
    tmat.d[3][3] = 1;

    // X-axis rotation matrix
    TransformMatrix xmat;
    xmat.d[0][0] = 1;
    xmat.d[0][1] = 0;
    xmat.d[0][2] = 0;
    xmat.d[0][3] = 0;
    xmat.d[1][0] = 0;
    xmat.d[1][1] = std::cos(ax);
    xmat.d[1][2] = std::sin(ax);
    xmat.d[1][3] = 0;
    xmat.d[2][0] = 0;
    xmat.d[2][1] = -std::sin(ax);
    xmat.d[2][2] = std::cos(ax);
    xmat.d[2][3] = 0;
    xmat.d[3][0] = 0;
    xmat.d[3][1] = 0;
    xmat.d[3][2] = 0;
    xmat.d[3][3] = 1;

    // Y-axis rotation matrix
    TransformMatrix ymat;
    ymat.d[0][0] = std::cos(ay);
    ymat.d[0][1] = 0;
    ymat.d[0][2] = -std::sin(ay);
    ymat.d[0][3] = 0;
    ymat.d[1][0] = 0;
    ymat.d[1][1] = 1;
    ymat.d[1][2] = 0;
    ymat.d[1][3] = 0;
    ymat.d[2][0] = std::sin(ay);
    ymat.d[2][1] = 0;
    ymat.d[2][2] = std::cos(ay);
    ymat.d[2][3] = 0;
    ymat.d[3][0] = 0;
    ymat.d[3][1] = 0;
    ymat.d[3][2] = 0;
    ymat.d[3][3] = 1;

    // Z-axis rotation matrix
    TransformMatrix zmat;
    zmat.d[0][0] = std::cos(az);
    zmat.d[0][1] = std::sin(az);
    zmat.d[0][2] = 0;
    zmat.d[0][3] = 0;
    zmat.d[1][0] = -std::sin(az);
    zmat.d[1][1] = std::cos(az);
    zmat.d[1][2] = 0;
    zmat.d[1][3] = 0;
    zmat.d[2][0] = 0;
    zmat.d[2][1] = 0;
    zmat.d[2][2] = 1;
    zmat.d[2][3] = 0;
    zmat.d[3][0] = 0;
    zmat.d[3][1] = 0;
    zmat.d[3][2] = 0;
    zmat.d[3][3] = 1;

    TransformMatrix tm;
    tm = MultiplyMatrix(smat, tmat);
    tm = MultiplyMatrix(tm, xmat);
    tm = MultiplyMatrix(tm, ymat);
    tm = MultiplyMatrix(tm, zmat);

    return tm;
}

/**
Parse the specified ini file (usually in the data/solar/asteriods) and retrieve
the lootable zone details.
*/
void ZoneUtilities::ReadLootableZone(zone_map_t &set_mmapZones,
                                     const std::string &systemNick,
                                     const std::string &defaultZoneNick,
                                     const std::string &file) {
    std::string path = "..\\data\\";
    path += file;

    INI_Reader ini;
    if (ini.open(path.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("LootableZone")) {
                std::string zoneNick = defaultZoneNick;
                std::string crateNick = "";
                std::string lootNick = "";
                int iMinLoot = 0;
                int iMaxLoot = 0;
                uint iLootDifficulty = 0;
                while (ini.read_value()) {
                    if (ini.is_value("zone")) {
                        zoneNick = ToLower(ini.get_value_string());
                    } else if (ini.is_value("dynamic_loot_container")) {
                        crateNick = ToLower(ini.get_value_string());
                    } else if (ini.is_value("dynamic_loot_commodity")) {
                        lootNick = ToLower(ini.get_value_string());
                    } else if (ini.is_value("dynamic_loot_count")) {
                        iMinLoot = ini.get_value_int(0);
                        iMaxLoot = ini.get_value_int(1);
                    } else if (ini.is_value("dynamic_loot_difficulty")) {
                        iLootDifficulty = ini.get_value_int(0);
                    }
                }

                LOOTABLE_ZONE lz;
                lz.systemID = CreateID(systemNick.c_str());
                lz.zoneNick = zoneNick;
                lz.lootNick = lootNick;
                lz.iLootID = CreateID(lootNick.c_str());
                lz.iCrateID = CreateID(crateNick.c_str());
                lz.iMinLoot = iMinLoot;
                lz.iMaxLoot = iMaxLoot;
                lz.iLootDifficulty = iLootDifficulty;
                lz.pos.x = lz.pos.y = lz.pos.z = 0;
                lz.size.x = lz.size.y = lz.size.z = 0;

                bool exists = false;
                for (const auto &z : set_mmapZones) {
                    if (z.second.zoneNick == zoneNick) {
                        exists = true;
                        break;
                    }
                }
                if (!exists)
                    set_mmapZones.insert({lz.systemID, lz});
            }
        }
        ini.close();
    }
}

/** Read the asteroid sections out of the system ini */
void ZoneUtilities::ReadSystemLootableZones(zone_map_t &set_mmapZones,
                                            const std::string &systemNick,
                                            const std::string &file) {
    std::string path = "..\\data\\universe\\";
    path += file;

    INI_Reader ini;
    if (ini.open(path.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("Asteroids")) {
                std::string file = "";
                std::string zoneNick = "";
                while (ini.read_value()) {
                    if (ini.is_value("zone"))
                        zoneNick = ToLower(ini.get_value_string());
                    if (ini.is_value("file"))
                        file = ini.get_value_string();
                }
                ReadLootableZone(set_mmapZones, systemNick, zoneNick, file);
            }
        }
        ini.close();
    }
}

/** Read the zone size/rotation and position information out of the
 specified file and calcuate the lootable zone transformation matrix */
void ZoneUtilities::ReadSystemZones(zone_map_t &set_mmapZones,
                                    const std::string &systemNick,
                                    const std::string &file) {
    std::string path = "..\\data\\universe\\";
    path += file;

    INI_Reader ini;
    if (ini.open(path.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("zone")) {
                std::string zoneNick = "";
                Vector size = {0, 0, 0};
                Vector pos = {0, 0, 0};
                Vector rotation = {0, 0, 0};
                int damage = 0;
                bool encounter = false;
                int idsName = 0;
                std::string idsNameTxt = "";
                int idsInfo = 0;
                std::string idsInfoTxt = "";

                while (ini.read_value()) {
                    if (ini.is_value("nickname")) {
                        zoneNick = ToLower(ini.get_value_string());
                    } else if (ini.is_value("pos")) {
                        pos.x = ini.get_value_float(0);
                        pos.y = ini.get_value_float(1);
                        pos.z = ini.get_value_float(2);
                    } else if (ini.is_value("rotate")) {
                        rotation.x = 180 + ini.get_value_float(0);
                        rotation.y = 180 + ini.get_value_float(1);
                        rotation.z = 180 + ini.get_value_float(2);
                    } else if (ini.is_value("size")) {
                        size.x = ini.get_value_float(0);
                        size.y = ini.get_value_float(1);
                        size.z = ini.get_value_float(2);
                        if (size.y == 0 || size.z == 0) {
                            size.y = size.x;
                            size.z = size.x;
                        }
                    } else if (ini.is_value("damage")) {
                        damage = ini.get_value_int(0);
                    } else if (ini.is_value("encounter")) {
                        encounter = true;
                    } else if (ini.is_value("ids_name")) {
                        idsName = ini.get_value_int(0);
                    } else if (ini.is_value("ids_info")) {
                        idsInfo = ini.get_value_int(0);
                    }
                }

                for (auto &z : set_mmapZones) {
                    if (z.second.zoneNick == zoneNick) {
                        z.second.pos = pos;
                        z.second.size = size;
                        break;
                    }
                }

                ZONE lz;
                lz.sysNick = systemNick;
                lz.zoneNick = zoneNick;
                lz.systemId = CreateID(systemNick.c_str());
                lz.size = size;
                lz.pos = pos;
                lz.damage = damage;
                lz.encounter = encounter;
                lz.transform = SetupTransform(pos, rotation);
                zones.insert({lz.systemId, lz});
            } else if (ini.is_header("Object")) {
                std::string nickname;
                std::string jumpDestSysNick;
                bool bIsJump = false;
                bool bMissionObject = false;
                bool bDestructible = false;

                while (ini.read_value()) {
                    if (ini.is_value("nickname")) {
                        nickname = ToLower(ini.get_value_string());
                    } else if (ini.is_value("goto")) {
                        bIsJump = true;
                        jumpDestSysNick = ini.get_value_string(0);
                    }
                }

                if (bIsJump) {
                    JUMPPOINT jp;
                    jp.sysNick = systemNick;
                    jp.jumpNick = nickname;
                    jp.jumpDestSysNick = jumpDestSysNick;
                    jp.System = CreateID(systemNick.c_str());
                    jp.jumpID = CreateID(nickname.c_str());
                    jp.jumpDestSysID = CreateID(jumpDestSysNick.c_str());
                    jumpPoints.insert({jp.System, jp});
                }
            }
        }
        ini.close();
    }
}

/** Read all systems in the universe ini */
void ZoneUtilities::ReadUniverse(zone_map_t *set_mmapZones) {
    zones.clear();

    // Read all system ini files again this time extracting zone size/postion
    // information for the zone list.
    INI_Reader ini;
    if (ini.open("..\\data\\universe\\universe.ini", false)) {
        while (ini.read_header()) {
            if (ini.is_header("System")) {
                std::string systemNick = "";
                std::string file = "";
                float scale = 1.0f;
                while (ini.read_value()) {
                    if (ini.is_value("nickname"))
                        systemNick = ToLower(ini.get_value_string());
                    if (ini.is_value("file"))
                        file = ini.get_value_string();
                    if (ini.is_value("NavMapScale"))
                        scale = ini.get_value_float(0);
                }
                if (set_mmapZones)
                    ReadSystemLootableZones(*set_mmapZones, systemNick, file);

                SYSTEMINFO sysInfo;
                sysInfo.sysNick = systemNick;
                sysInfo.systemId = CreateID(systemNick.c_str());
                sysInfo.scale = scale;
                mapSystems[sysInfo.systemId] = sysInfo;
            }
        }
        ini.close();
    }

    // Read all system ini files again this time extracting zone size/postion
    // information for the lootable zone list.
    if (ini.open("..\\data\\universe\\universe.ini", false)) {
        while (ini.read_header()) {
            if (ini.is_header("System")) {
                std::string systemNick = "";
                std::string file = "";
                while (ini.read_value()) {
                    if (ini.is_value("nickname"))
                        systemNick = ini.get_value_string();
                    if (ini.is_value("file"))
                        file = ini.get_value_string();
                }
                if (set_mmapZones)
                    ReadSystemZones(*set_mmapZones, systemNick, file);
            }
        }
        ini.close();
    }
}

/**
 Return true if the ship location as specified by the position parameter is in a
 lootable zone.
*/
bool ZoneUtilities::InZone(uint system, const Vector &pos, ZONE &rlz) {
    // For each zone in the system test that pos is inside the
    // zone.
    auto start = zones.lower_bound(system);
    auto end = zones.upper_bound(system);
    for (auto i = start; i != end; ++i) {
        const ZONE &lz = i->second;
        /** Transform the point pos onto coordinate system defined by matrix m
         */
        float x = pos.x * lz.transform.d[0][0] + pos.y * lz.transform.d[1][0] +
                  pos.z * lz.transform.d[2][0] + lz.transform.d[3][0];
        float y = pos.x * lz.transform.d[0][1] + pos.y * lz.transform.d[1][1] +
                  pos.z * lz.transform.d[2][1] + lz.transform.d[3][1];
        float z = pos.x * lz.transform.d[0][2] + pos.y * lz.transform.d[1][2] +
                  pos.z * lz.transform.d[2][2] + lz.transform.d[3][2];

        // If r is less than/equal to 1 then the point is inside the ellipsoid.
        float result = sqrt(powf(x / lz.size.x, 2) + powf(y / lz.size.y, 2) +
                            powf(z / lz.size.z, 2));
        if (result <= 1) {
            rlz = lz;
            return true;
        }
    }

    // Return the iterator. If the iter is zones.end() then the point
    // is not in a lootable zone.
    return false;
}

/**
 Return true if the ship location as specified by the position parameter is in a
 death zone
*/
bool ZoneUtilities::InDeathZone(uint system, const Vector &pos, ZONE &rlz) {
    // For each zone in the system test that pos is inside the
    // zone.
    auto start = zones.lower_bound(system);
    auto end = zones.upper_bound(system);
    for (auto i = start; i != end; ++i) {
        const ZONE &lz = i->second;

        /** Transform the point pos onto coordinate system defined by matrix m
         */
        float x = pos.x * lz.transform.d[0][0] + pos.y * lz.transform.d[1][0] +
                  pos.z * lz.transform.d[2][0] + lz.transform.d[3][0];
        float y = pos.x * lz.transform.d[0][1] + pos.y * lz.transform.d[1][1] +
                  pos.z * lz.transform.d[2][1] + lz.transform.d[3][1];
        float z = pos.x * lz.transform.d[0][2] + pos.y * lz.transform.d[1][2] +
                  pos.z * lz.transform.d[2][2] + lz.transform.d[3][2];

        // If r is less than/equal to 1 then the point is inside the ellipsoid.
        float result = sqrt(powf(x / lz.size.x, 2) + powf(y / lz.size.y, 2) +
                            powf(z / lz.size.z, 2));
        if (result <= 1 && lz.damage > 250) {
            rlz = lz;
            return true;
        }
    }

    // Return the iterator. If the iter is zones.end() then the point
    // is not in a death zone.
    return false;
}

/**
        Return a pointer to the system info object for the specified system ID.
        Return 0 if the system ID does not exist.
*/
SYSTEMINFO *ZoneUtilities::GetSystemInfo(uint systemID) {
    if (mapSystems.find(systemID) != mapSystems.end())
        return &mapSystems[systemID];
    return 0;
}

void ZoneUtilities::PrintZones() {
    zone_map_t set_mmapZones;
    ReadUniverse(&set_mmapZones);

    ConPrint(
        L"Zone, Commodity, MinLoot, MaxLoot, Difficultly, PosX, PosY, PosZ, "
        L"SizeX, SizeY, SizeZ, IdsName, IdsInfo, Bonus\n");
    for (const auto &z : set_mmapZones) {
        ConPrint(
            L"%s, %s, %d, %d, %d, %0.0f, %0.0f, %0.0f, %0.0f, %0.0f, %0.0f, "
            L"%d, %d, %2.2f\n",
            stows(z.second.zoneNick).c_str(), stows(z.second.lootNick).c_str(),
            z.second.iMinLoot, z.second.iMaxLoot, z.second.iLootDifficulty,
            z.second.pos.x, z.second.pos.y, z.second.pos.z, z.second.size.x,
            z.second.size.y, z.second.size.z);
    }
    ConPrint(L"Zones=%d\n", set_mmapZones.size());
}
