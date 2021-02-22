/**
 Mining Plugin for FLHook-Plugin
 by Cannon.

0.1:
 Initial release
0.2:
 Use own asteriod field calculations.
0.3:
 On loot-cheat make player's ship explode and log to flhook cheaters.log
0.4:
 Fixed zone calculation problems.
 Added field by field bonus
0.5:
 Fixed the fix for zone calculation problems
 Added commodity modification for fields
1.0:
 Gave up on my own zone calculations and went back to using the FL ones.
 Changed the bonuses to only work if all equipment items are present.
 Changed the configuration file format to make setup a little quicker.
1.1:
 Fixed player bonus initialisation problem.
 Added playerbonus section error messages and fixed annoying warning
 in flserver-errors.log
1.2:
 Changed mined loot to go directly into cargo hold. Also mining only works
 if the floating loot is hit with a mining gun. Regular guns don't work.
 The system now maintains a historical record of mined ore from fields. Fields
 recharge over time and are depleted as they're mined.
*/

// Includes
#include <FLHook.h>
#include <plugin.h>

static float set_fGenericFactor = 1.0f;
static int set_iPluginDebug = 0;
static std::string set_scStatsPath;

struct PLAYER_BONUS {
    PLAYER_BONUS() : iLootID(0), fBonus(0.0f), iRep(-1) {}

    // The loot commodity id this configuration applies to.
    uint iLootID;

    // The loot bonus multiplier.
    float fBonus;

    // The affiliation/reputation of the player
    uint iRep;

    // The list of ships that this bonus applies to
    std::list<uint> lstShips;

    // The list of equipment items that the ship must carry
    std::list<uint> lstItems;

    // The list of ammo arch ids for mining guns
    std::list<uint> lstAmmo;
};
std::multimap<uint, PLAYER_BONUS> set_mmapPlayerBonus;

struct ZONE_BONUS {
    ZONE_BONUS()
        : fBonus(0.0f), iReplacementLootID(0), fRechargeRate(0),
          fCurrReserve(100000), fMaxReserve(50000), fMined(0) {}

    std::string scZone;

    // The loot bonus multiplier.
    float fBonus;

    // The hash of the item to replace the dropped
    uint iReplacementLootID;

    // The recharge rate of the zone. This is the number of units of ore added
    // to the reserve per minute.
    float fRechargeRate;

    // The current amount of ore in the zone. When this gets low, ore gets
    // harder to mine. When it gets to 0, ore is impossible to mine.
    float fCurrReserve;

    // The maximum limit for the amount of ore in the field
    float fMaxReserve;

    // The amount of ore that has been mined.
    float fMined;
};
std::map<uint, ZONE_BONUS> set_mapZoneBonus;

struct CLIENT_DATA {
    CLIENT_DATA()
        : bSetup(false), iDebug(0), iPendingMineAsteroidEvents(0),
          iMineAsteroidEvents(0) {}

    bool bSetup;
    std::map<uint, float> mapLootBonus;
    std::map<uint, std::list<uint>> mapLootAmmoLst;
    std::map<uint, std::list<uint>> mapLootShipLst;
    int iDebug;

    int iPendingMineAsteroidEvents;
    int iMineAsteroidEvents;
    time_t tmMineAsteroidSampleStart;
};
std::map<uint, CLIENT_DATA> mapClients;

/** A return code to indicate to FLHook if we want the hook processing to
 * continue. */
ReturnCode returncode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Return the std::string parameter at position iPos from the ini line. The
/// delimiter is a ',' character. A chunk of code is from Motah's Flak.
static std::string GetTrimParam(const std::string &scLine, uint iPos) {
    std::string scOut = "";
    for (uint i = 0, j = 0; (i <= iPos) && (j < scLine.length()); j++) {
        if (scLine[j] == ',')
            i++;
        else if (i == iPos)
            scOut += scLine[j];
        else if (i > iPos)
            break;
    }

    while (scOut.size() && (scOut[0] == L' ' || scOut[0] == L'\t' ||
                            scOut[0] == L'\n' || scOut[0] == L'\r')) {
        scOut = scOut.substr(1);
    }
    while (scOut.size() && (scOut[scOut.size() - 1] == L' ' ||
                            scOut[scOut.size() - 1] == L'\t' ||
                            scOut[scOut.size() - 1] == L'\n' ||
                            scOut[scOut.size() - 1] == L'\r')) {
        scOut = scOut.substr(0, scOut.length() - 1);
    }
    return scOut;
}

/// Return true if the cargo list contains the specified good.
static bool ContainsEquipment(std::list<CARGO_INFO> &lstCargo, uint iArchID) {
    for (auto &c : lstCargo)
        if (c.bMounted && c.iArchID == iArchID)
            return true;
    return false;
}

/// Return the factor to modify a mining loot drop by.
static float GetBonus(uint iRep, uint iShipID, std::list<CARGO_INFO> lstCargo,
                      uint iLootID) {
    if (!set_mmapPlayerBonus.size())
        return 0.0f;

    // Get all player bonuses for this commodity.
    auto start = set_mmapPlayerBonus.lower_bound(iLootID);
    auto end = set_mmapPlayerBonus.upper_bound(iLootID);
    for (; start != end; start++) {
        // Check for matching reputation if reputation is required.
        if (start->second.iRep != -1 && iRep != start->second.iRep)
            continue;

        // Check for matching ship.
        if (find(start->second.lstShips.begin(), start->second.lstShips.end(),
                 iShipID) == start->second.lstShips.end())
            continue;

        // Check that every simple item in the equipment list is present and
        // mounted.
        bool bEquipMatch = true;
        for (auto item : start->second.lstItems) {
            if (!ContainsEquipment(lstCargo, item)) {
                bEquipMatch = false;
                break;
            }
        }

        // This is a match.
        if (bEquipMatch)
            return start->second.fBonus;
    }

    return 0.0f;
}

void CheckClientSetup(uint iClientID) {
    if (!mapClients[iClientID].bSetup) {
        if (set_iPluginDebug > 1)
            ConPrint(L"NOTICE: iClientID=%d setup bonuses\n", iClientID);
        mapClients[iClientID].bSetup = true;

        // Get the player affiliation
        uint iRepGroupID = -1;
        IObjInspectImpl *oship = HkGetInspect(iClientID);
        if (oship)
            oship->get_affiliation(iRepGroupID);

        // Get the ship type
        uint iShipID;
        pub::Player::GetShipID(iClientID, iShipID);

        // Get the ship cargo so that we can check ids, guns, etc.
        std::list<CARGO_INFO> lstCargo;
        int remainingHoldSize = 0;
        HkEnumCargo((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                    lstCargo, remainingHoldSize);
        if (set_iPluginDebug > 1) {
            ConPrint(
                L"NOTICE: iClientID=%d iRepGroupID=%u iShipID=%u lstCargo=",
                iClientID, iRepGroupID, iShipID);
            for (auto &ci : lstCargo) {
                ConPrint(L"%u ", ci.iArchID);
            }
            ConPrint(L"\n");
        }

        // Check the player bonus list and if this player has the right ship and
        // equipment then record the bonus and the weapon types that can be used
        // to gather the ore.
        mapClients[iClientID].mapLootBonus.clear();
        mapClients[iClientID].mapLootAmmoLst.clear();
        mapClients[iClientID].mapLootShipLst.clear();
        for (auto &i : set_mmapPlayerBonus) {
            uint iLootID = i.first;
            float fBonus = GetBonus(iRepGroupID, iShipID, lstCargo, iLootID);
            if (fBonus > 0.0f) {
                mapClients[iClientID].mapLootBonus[iLootID] = fBonus;
                mapClients[iClientID].mapLootAmmoLst[iLootID] =
                    i.second.lstAmmo;
                mapClients[iClientID].mapLootShipLst[iLootID] =
                    i.second.lstShips;
                if (set_iPluginDebug > 1) {
                    ConPrint(
                        L"NOTICE: iClientID=%d iLootID=%08x fBonus=%2.2f\n",
                        iClientID, iLootID, fBonus);
                }
            }
        }

        std::wstring wscRights;
        HkGetAdmin((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                   wscRights);
        if (wscRights.size())
            mapClients[iClientID].iDebug = set_iPluginDebug;
    }
}

EXPORT void HkTimerCheckKick() {
    // Perform 60 second tasks.
    if ((time(0) % 60) == 0) {
        // Recharge the fields
        for (auto &i : set_mapZoneBonus) {
            i.second.fCurrReserve += i.second.fRechargeRate;
            if (i.second.fCurrReserve > i.second.fMaxReserve)
                i.second.fCurrReserve = i.second.fMaxReserve;
        }

        // Save the zone status to disk
        char szDataPath[MAX_PATH];
        GetUserDataPath(szDataPath);
        std::string scStatsPath =
            std::string(szDataPath) + "\\Accts\\MultiPlayer\\mining_stats.txt";
        FILE *file;
        fopen_s(&file, scStatsPath.c_str(), "w");
        if (file) {
            fprintf(file, "[Zones]\n");
            for (auto &i : set_mapZoneBonus) {
                if (i.second.scZone.size()) {
                    fprintf(file, "%s, %0.0f, %0.0f\n", i.second.scZone.c_str(),
                            i.second.fCurrReserve, i.second.fMined);
                }
            }
            fclose(file);
        }
    }
}

/// Clear client info when a client connects.
EXPORT void ClearClientInfo(uint iClientID) {
    mapClients[iClientID].bSetup = false;
    mapClients[iClientID].mapLootBonus.clear();
    mapClients[iClientID].mapLootAmmoLst.clear();
    mapClients[iClientID].iDebug = 0;
    mapClients[iClientID].iPendingMineAsteroidEvents = 0;
    mapClients[iClientID].iMineAsteroidEvents = 0;
    mapClients[iClientID].tmMineAsteroidSampleStart = 0;
}

/// Load the configuration
EXPORT void LoadSettings() {
    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\minecontrol.cfg";

    // Load generic settings
    set_fGenericFactor =
        IniGetF(scPluginCfgFile, "MiningGeneral", "GenericFactor", 1.0);
    set_iPluginDebug = IniGetI(scPluginCfgFile, "MiningGeneral", "Debug", 0);
    set_scStatsPath =
        IniGetS(scPluginCfgFile, "MiningGeneral", "StatsPath", "");

    if (set_iPluginDebug)
        ConPrint(L"NOTICE: generic_factor=%0.0f debug=%d\n", set_fGenericFactor,
                 set_iPluginDebug);

    // Patch Archetype::GetEquipment & Archetype::GetShip to suppress annoying
    // warnings flserver-errors.log
    unsigned char patch1[] = {0x90, 0x90};
    WriteProcMem((char *)0x62F327E, &patch1, 2);
    WriteProcMem((char *)0x62F944E, &patch1, 2);
    WriteProcMem((char *)0x62F123E, &patch1, 2);

    // Load the player bonus list and the field bonus list.
    // To receive the bonus for the particular commodity the player has to have
    // the affiliation (unless this field is empty), one of the ships and
    // all of the equipment items.
    // The [PlayerBonus] section has the following format:
    // Commodity, Bonus, Affiliation, List of ships, equipment or cargo
    // separated by commas. The [FieldBonus] section has the following format:
    // Field, Bonus, Replacement Commodity
    set_mapZoneBonus.clear();
    set_mmapPlayerBonus.clear();
    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("PlayerBonus")) {
                while (ini.read_value()) {
                    std::string scLine = ini.get_name_ptr();

                    PLAYER_BONUS pb;
                    pb.iLootID = CreateID(GetTrimParam(scLine, 0).c_str());
                    if (!Archetype::GetEquipment(pb.iLootID) &&
                        !Archetype::GetSimple(pb.iLootID)) {
                        ConPrint(L"ERROR: %s:%d: item '%s' not valid\n",
                                 stows(ini.get_file_name()).c_str(),
                                 ini.get_line_num(),
                                 stows(GetTrimParam(scLine, 0)).c_str());
                        continue;
                    }

                    pb.fBonus = (float)atof(GetTrimParam(scLine, 1).c_str());
                    if (pb.fBonus <= 0.0f) {
                        ConPrint(L"ERROR: %s:%d: bonus not valid\n",
                                 stows(ini.get_file_name()).c_str(),
                                 ini.get_line_num());
                        continue;
                    }

                    pb.iRep = -1;
                    if (GetTrimParam(scLine, 2) != "*") {
                        pub::Reputation::GetReputationGroup(
                            pb.iRep, GetTrimParam(scLine, 2).c_str());
                        if (pb.iRep == -1) {
                            ConPrint(L"ERROR: %s:%d: reputation not valid\n",
                                     stows(ini.get_file_name()).c_str(),
                                     ini.get_line_num());
                            continue;
                        }
                    }

                    int i = 3;
                    std::string scShipOrEquip = GetTrimParam(scLine, i++);
                    while (scShipOrEquip.size()) {
                        uint iItemID = CreateID(scShipOrEquip.c_str());
                        if (Archetype::GetShip(iItemID)) {
                            pb.lstShips.push_back(iItemID);
                        } else if (Archetype::GetEquipment(iItemID)) {
                            Archetype::Equipment *eq =
                                Archetype::GetEquipment(iItemID);
                            if (eq->get_class_type() == Archetype::GUN) {
                                Archetype::Gun *gun = (Archetype::Gun *)eq;
                                if (gun->iProjectileArchID &&
                                    gun->iProjectileArchID != 0xBAADF00D &&
                                    gun->iProjectileArchID != 0x3E07E70) {
                                    pb.lstAmmo.push_back(
                                        gun->iProjectileArchID);
                                }
                            } else {
                                pb.lstItems.push_back(iItemID);
                            }
                        } else if (Archetype::GetSimple(iItemID)) {
                            pb.lstItems.push_back(iItemID);
                        } else {
                            ConPrint(L"ERROR: %s:%d: item '%s' not valid\n",
                                     stows(ini.get_file_name()).c_str(),
                                     ini.get_line_num(),
                                     stows(scShipOrEquip).c_str());
                        }

                        scShipOrEquip = GetTrimParam(scLine, i++);
                    }

                    set_mmapPlayerBonus.insert(
                        std::multimap<uint, PLAYER_BONUS>::value_type(
                            pb.iLootID, pb));
                    if (set_iPluginDebug) {
                        ConPrint(L"NOTICE: mining player bonus %s(%u) %2.2f "
                                 L"%s(%u)\n",
                                 stows(GetTrimParam(scLine, 0)).c_str(),
                                 pb.iLootID, pb.fBonus,
                                 stows(GetTrimParam(scLine, 2)).c_str(),
                                 pb.iRep);
                    }
                }
            } else if (ini.is_header("ZoneBonus")) {
                while (ini.read_value()) {
                    std::string scLine = ini.get_name_ptr();

                    std::string scZone = GetTrimParam(scLine, 0);
                    if (!scZone.size()) {
                        ConPrint(L"ERROR: %s:%d: zone not valid\n",
                                 stows(ini.get_file_name()).c_str(),
                                 ini.get_line_num());
                        continue;
                    }

                    float fBonus = (float)atof(GetTrimParam(scLine, 1).c_str());
                    if (fBonus <= 0.0f) {
                        ConPrint(L"ERROR: %s:%d: bonus not valid\n",
                                 stows(ini.get_file_name()).c_str(),
                                 ini.get_line_num());
                        continue;
                    }

                    uint iReplacementLootID = 0;
                    std::string scReplacementLoot = GetTrimParam(scLine, 2);
                    if (scReplacementLoot.size()) {
                        iReplacementLootID =
                            CreateID(scReplacementLoot.c_str());
                    }

                    float fRechargeRate =
                        (float)atof(GetTrimParam(scLine, 3).c_str());
                    if (fRechargeRate <= 0.0f) {
                        fRechargeRate = 50;
                    }

                    float fMaxReserve =
                        (float)atof(GetTrimParam(scLine, 4).c_str());
                    if (fMaxReserve <= 0.0f) {
                        fMaxReserve = 100000;
                    }

                    uint iZoneID = CreateID(scZone.c_str());
                    set_mapZoneBonus[iZoneID].scZone = scZone;
                    set_mapZoneBonus[iZoneID].fBonus = fBonus;
                    set_mapZoneBonus[iZoneID].iReplacementLootID =
                        iReplacementLootID;
                    set_mapZoneBonus[iZoneID].fRechargeRate = fRechargeRate;
                    set_mapZoneBonus[iZoneID].fMaxReserve = fMaxReserve;
                    if (set_iPluginDebug) {
                        ConPrint(L"NOTICE: zone bonus %s fBonus=%2.2f "
                                 L"iReplacementLootID=%s(%u) "
                                 L"fRechargeRate=%0.0f fMaxReserve=%0.0f\n",
                                 stows(scZone).c_str(), fBonus,
                                 stows(scReplacementLoot).c_str(),
                                 iReplacementLootID, fRechargeRate,
                                 fMaxReserve);
                    }
                }
            }
        }
        ini.close();
    }

    // Read the last saved zone reserve.
    char szDataPath[MAX_PATH];
    GetUserDataPath(szDataPath);
    std::string scStatsPath =
        std::string(szDataPath) + "\\Accts\\MultiPlayer\\mining_stats.txt";
    if (ini.open(scStatsPath.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("Zones")) {
                while (ini.read_value()) {
                    std::string scLine = ini.get_name_ptr();
                    std::string scZone = GetTrimParam(scLine, 0);
                    float fCurrReserve =
                        (float)atof(GetTrimParam(scLine, 1).c_str());
                    float fMined = (float)atof(GetTrimParam(scLine, 2).c_str());
                    uint iZoneID = CreateID(scZone.c_str());
                    if (set_mapZoneBonus.find(iZoneID) !=
                        set_mapZoneBonus.end()) {
                        set_mapZoneBonus[iZoneID].fCurrReserve = fCurrReserve;
                        set_mapZoneBonus[iZoneID].fMined = fMined;
                    }
                }
            }
        }
        ini.close();
    }

    // Remove patch now that we've finished loading.
    unsigned char patch2[] = {0xFF, 0x12};
    WriteProcMem((char *)0x62F327E, &patch2, 2);
    WriteProcMem((char *)0x62F944E, &patch2, 2);
    WriteProcMem((char *)0x62F123E, &patch2, 2);

    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        uint iClientID = HkGetClientIdFromPD(pPD);
        ClearClientInfo(iClientID);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));
    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        LoadSettings();
    return true;
}

void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID) {
    ClearClientInfo(iClientID);
}

/// Called when a gun hits something
void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const &ci,
                                   unsigned int iClientID) {
    // If this is not a lootable rock, do no other processing.
    if (ci.dwTargetShip != 0)
        return;

    returncode = ReturnCode::SkipAll;

    // Initialise the mining setup for this client if it hasn't been done
    // already.
    CheckClientSetup(iClientID);

    uint iShip;
    pub::Player::GetShip(iClientID, iShip);

    Vector vPos;
    Matrix mRot;
    pub::SpaceObj::GetLocation(iShip, vPos, mRot);

    uint iClientSystemID;
    pub::Player::GetSystem(iClientID, iClientSystemID);
    CmnAsteroid::CAsteroidSystem *csys = CmnAsteroid::Find(iClientSystemID);
    if (csys) {
        // Find asteroid field that matches the best.
        for (CmnAsteroid::CAsteroidField *cfield = csys->FindFirst(); cfield;
             cfield = csys->FindNext()) {
            try {
                const Universe::IZone *zone = cfield->get_lootable_zone(vPos);
                if (cfield->near_field(vPos) && zone && zone->lootableZone) {
                    CLIENT_DATA &cd = mapClients[iClientID];

                    // If a non-rock is being shot we won't have an associated
                    // mining event so ignore this.
                    cd.iPendingMineAsteroidEvents--;
                    if (cd.iPendingMineAsteroidEvents < 0) {
                        cd.iPendingMineAsteroidEvents = 0;
                        return;
                    }

                    // Adjust the bonus based on the zone.
                    float fZoneBonus = 0.25f;
                    if (set_mapZoneBonus[zone->iZoneID].fBonus)
                        fZoneBonus = set_mapZoneBonus[zone->iZoneID].fBonus;

                    // If the field is getting mined out, reduce the bonus
                    fZoneBonus *= set_mapZoneBonus[zone->iZoneID].fCurrReserve /
                                  set_mapZoneBonus[zone->iZoneID].fMaxReserve;

                    uint iLootID = zone->lootableZone->dynamic_loot_commodity;
                    uint iCrateID = zone->lootableZone->dynamic_loot_container;

                    // Change the commodity if appropriate.
                    if (set_mapZoneBonus[zone->iZoneID].iReplacementLootID)
                        iLootID =
                            set_mapZoneBonus[zone->iZoneID].iReplacementLootID;

                    // If no mining bonus entry for this commodity is found,
                    // flag as no bonus
                    auto ammolst = cd.mapLootAmmoLst.find(iLootID);
                    bool bNoMiningCombo = false;
                    if (ammolst == cd.mapLootAmmoLst.end()) {
                        bNoMiningCombo = true;
                        if (cd.iDebug)
                            PrintUserCmdText(iClientID,
                                             L"* Wrong ship/equip/rep");
                    }
                    // If this minable commodity was not hit by the right type
                    // of gun, flag as no bonus
                    else if (find(ammolst->second.begin(),
                                  ammolst->second.end(),
                                  ci.iProjectileArchID) ==
                             ammolst->second.end()) {
                        bNoMiningCombo = true;
                        if (cd.iDebug)
                            PrintUserCmdText(iClientID, L"* Wrong gun");
                    }

                    // If either no mining gun was used in the shot, or the
                    // character isn't using a valid mining combo for this
                    // commodity, set bonus to *0.5
                    float fPlayerBonus = 0.5f;
                    if (bNoMiningCombo)
                        fPlayerBonus = 0.5f;
                    else
                        fPlayerBonus = cd.mapLootBonus[iLootID];

                    // If this ship is has another ship targetted then send the
                    // ore into the cargo hold of the other ship.
                    uint iSendToClientID = iClientID;
                    if (!bNoMiningCombo) {
                        uint iTargetShip;
                        pub::SpaceObj::GetTarget(iShip, iTargetShip);
                        if (iTargetShip) {
                            uint iTargetClientID =
                                HkGetClientIDByShip(iTargetShip);
                            if (iTargetClientID) {
                                if (HkDistance3DByShip(iShip, iTargetShip) <
                                    1000.0f) {
                                    iSendToClientID = iTargetClientID;
                                }
                            }
                        }
                    }

                    // Calculate the loot drop count
                    float fRand = (float)rand() / (float)RAND_MAX;

                    // Calculate the loot drop and drop it.
                    int iLootCount =
                        (int)(fRand * set_fGenericFactor * fZoneBonus *
                              fPlayerBonus *
                              zone->lootableZone->dynamic_loot_count2);

                    // Remove this lootCount from the field
                    set_mapZoneBonus[zone->iZoneID].fCurrReserve -= iLootCount;
                    set_mapZoneBonus[zone->iZoneID].fMined += iLootCount;
                    if (set_mapZoneBonus[zone->iZoneID].fCurrReserve <= 0) {
                        set_mapZoneBonus[zone->iZoneID].fCurrReserve = 0;
                        iLootCount = 0;
                    }

                    if (mapClients[iClientID].iDebug) {
                        PrintUserCmdText(
                            iClientID,
                            L"* fRand=%2.2f fGenericBonus=%2.2f "
                            L"fPlayerBonus=%2.2f fZoneBonus=%2.2f "
                            L"iLootCount=%d iLootID=%u/%u fCurrReserve=%0.0f",
                            fRand, set_fGenericFactor, fPlayerBonus, fZoneBonus,
                            iLootCount, iLootID, iCrateID,
                            set_mapZoneBonus[zone->iZoneID].fCurrReserve);
                    }

                    mapClients[iClientID].iMineAsteroidEvents++;
                    if (mapClients[iClientID].tmMineAsteroidSampleStart <
                        time(0)) {
                        float average =
                            mapClients[iClientID].iMineAsteroidEvents / 30.0f;
                        if (average > 2.0f) {
                            AddLog("NOTICE: high mining rate charname=%s "
                                   "rate=%0.1f/sec "
                                   "location=%0.0f,%0.0f,%0.0f system=%08x "
                                   "zone=%08x",
                                   wstos((const wchar_t *)Players
                                             .GetActiveCharacterName(iClientID))
                                       .c_str(),
                                   average, vPos.x, vPos.y, vPos.z,
                                   zone->iSystemID, zone->iZoneID);
                        }

                        mapClients[iClientID].tmMineAsteroidSampleStart =
                            time(0) + 30;
                        mapClients[iClientID].iMineAsteroidEvents = 0;
                    }

                    if (iLootCount) {
                        float fHoldRemaining;
                        pub::Player::GetRemainingHoldSize(iSendToClientID,
                                                          fHoldRemaining);
                        if (fHoldRemaining < iLootCount) {
                            iLootCount = (int)fHoldRemaining;
                        }
                        if (iLootCount == 0) {
                            pub::Player::SendNNMessage(
                                iClientID,
                                CreateID("insufficient_cargo_space"));
                            return;
                        }
                        pub::Player::AddCargo(iSendToClientID, iLootID,
                                              iLootCount, 1.0, false);
                    }
                    return;
                }
            } catch (...) {
            }
        }
    }
}

/// Called when an asteriod is mined. We ignore all of the parameters from the
/// client.
void __stdcall MineAsteroid(uint iClientSystemID, class Vector const &vPos,
                            uint iCrateID, uint iLootID, uint iCount,
                            uint iClientID) {
    mapClients[iClientID].iPendingMineAsteroidEvents += 4;
    //	ConPrint(L"mine_asteroid %d %d %d\n", iCrateID, iLootID, iCount);
    return;
}

#define IS_CMD(a) !wscCmd.compare(L##a)

bool ExecuteCommandString(CCmds *cmd, const std::wstring &wscCmd) {
    if (IS_CMD("printminezones")) {
        returncode = ReturnCode::SkipAll;
        ZoneUtilities::PrintZones();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Functions to hook */
EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Mine Control Plugin by cannon");
    pi->shortName("minecontrol");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
    pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
    pi->emplaceHook(HookedCall::IServerImpl__MineAsteroid, &MineAsteroid);
    pi->emplaceHook(HookedCall::IServerImpl__SPMunitionCollision,
                    &SPMunitionCollision);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
}