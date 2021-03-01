// NPCs for FLHookPlugin
// December 2015 by BestDiscoveryHookDevs2015
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

// Structures and Global Variables

struct NPC_ARCHTYPESSTRUCT {
    uint Shiparch;
    uint Loadout;
    uint IFF;
    uint Infocard;
    uint Infocard2;
    int Graph;
};

struct NPC_FLEETSTRUCT {
    std::wstring fleetname;
    std::map<std::wstring, int> fleetmember;
};

struct NPC {
    std::wstring name;
    Vector pos;
    uint system;
    Matrix rot;
};

static std::map<std::wstring, NPC_ARCHTYPESSTRUCT> mapNPCArchtypes;
static std::map<std::wstring, NPC_FLEETSTRUCT> mapNPCFleets;
static std::map<int, NPC> startupNPCs;
std::vector<const char *> listgraphs;
std::vector<uint> npcnames;
std::list<uint> npcs;
FILE *Logfile;

// Function to return a Personality. Possible improvements in the future to get
// this to load from a config file
pub::AI::SetPersonalityParams HkMakePersonality(int graphid) {

    pub::AI::SetPersonalityParams p;
    p.iStateGraph = pub::StateGraph::get_state_graph(
        listgraphs[graphid], pub::StateGraph::TYPE_STANDARD);
    p.bStateID = true;

    p.personality.EvadeDodgeUse.evade_dodge_style_weight[0] = 0.4f;
    p.personality.EvadeDodgeUse.evade_dodge_style_weight[1] = 0.0f;
    p.personality.EvadeDodgeUse.evade_dodge_style_weight[2] = 0.4f;
    p.personality.EvadeDodgeUse.evade_dodge_style_weight[3] = 0.2f;
    p.personality.EvadeDodgeUse.evade_dodge_cone_angle = 1.5708f;
    p.personality.EvadeDodgeUse.evade_dodge_interval_time = 10.0f;
    p.personality.EvadeDodgeUse.evade_dodge_time = 1.0f;
    p.personality.EvadeDodgeUse.evade_dodge_distance = 75.0f;
    p.personality.EvadeDodgeUse.evade_activate_range = 100.0f;
    p.personality.EvadeDodgeUse.evade_dodge_roll_angle = 1.5708f;
    p.personality.EvadeDodgeUse.evade_dodge_waggle_axis_cone_angle = 1.5708f;
    p.personality.EvadeDodgeUse.evade_dodge_slide_throttle = 1.0f;
    p.personality.EvadeDodgeUse.evade_dodge_turn_throttle = 1.0f;
    p.personality.EvadeDodgeUse.evade_dodge_corkscrew_roll_flip_direction =
        true;
    p.personality.EvadeDodgeUse.evade_dodge_interval_time_variance_percent =
        0.5f;
    p.personality.EvadeDodgeUse.evade_dodge_cone_angle_variance_percent = 0.5f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[0] = 0.25f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[1] = 0.25f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[2] = 0.25f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[3] = 0.25f;

    p.personality.EvadeBreakUse.evade_break_roll_throttle = 1.0f;
    p.personality.EvadeBreakUse.evade_break_time = 1.0f;
    p.personality.EvadeBreakUse.evade_break_interval_time = 10.0f;
    p.personality.EvadeBreakUse.evade_break_afterburner_delay = 0.0f;
    p.personality.EvadeBreakUse.evade_break_turn_throttle = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[0] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[1] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[2] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[3] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_style_weight[0] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_style_weight[1] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_style_weight[2] = 1.0f;

    p.personality.BuzzHeadTowardUse.buzz_min_distance_to_head_toward = 500.0f;
    p.personality.BuzzHeadTowardUse
        .buzz_min_distance_to_head_toward_variance_percent = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_max_time_to_head_away = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_engine_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_turn_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_roll_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_turn_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_cone_angle = 1.5708f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_cone_angle_variance_percent =
        0.5f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_waggle_axis_cone_angle = 0.3491f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_roll_angle = 1.5708f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_interval_time = 10.0f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_interval_time_variance_percent =
        0.5f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[0] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[1] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[2] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[3] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_style_weight[0] = 0.33f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_style_weight[1] = 0.33f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_style_weight[2] = 0.33f;

    p.personality.BuzzPassByUse.buzz_distance_to_pass_by = 1000.0f;
    p.personality.BuzzPassByUse.buzz_pass_by_time = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_cone_angle = 1.5708f;
    p.personality.BuzzPassByUse.buzz_break_turn_throttle = 1.0f;
    p.personality.BuzzPassByUse.buzz_drop_bomb_on_pass_by = true;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[0] = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[1] = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[2] = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[3] = 1.0f;
    p.personality.BuzzPassByUse.buzz_pass_by_style_weight[2] = 1.0f;

    p.personality.TrailUse.trail_lock_cone_angle = 0.0873f;
    p.personality.TrailUse.trail_break_time = 0.5f;
    p.personality.TrailUse.trail_min_no_lock_time = 0.1f;
    p.personality.TrailUse.trail_break_roll_throttle = 1.0f;
    p.personality.TrailUse.trail_break_afterburner = true;
    p.personality.TrailUse.trail_max_turn_throttle = 1.0f;
    p.personality.TrailUse.trail_distance = 100.0f;

    p.personality.StrafeUse.strafe_run_away_distance = 100.0f;
    p.personality.StrafeUse.strafe_attack_throttle = 1.0f;

    p.personality.EngineKillUse.engine_kill_search_time = 0.0f;
    p.personality.EngineKillUse.engine_kill_face_time = 1.0f;
    p.personality.EngineKillUse.engine_kill_use_afterburner = true;
    p.personality.EngineKillUse.engine_kill_afterburner_time = 2.0f;
    p.personality.EngineKillUse.engine_kill_max_target_distance = 100.0f;

    p.personality.RepairUse.use_shield_repair_pre_delay = 0.0f;
    p.personality.RepairUse.use_shield_repair_post_delay = 1.0f;
    p.personality.RepairUse.use_shield_repair_at_damage_percent = 0.2f;
    p.personality.RepairUse.use_hull_repair_pre_delay = 0.0f;
    p.personality.RepairUse.use_hull_repair_post_delay = 1.0f;
    p.personality.RepairUse.use_hull_repair_at_damage_percent = 0.2f;

    p.personality.GunUse.gun_fire_interval_time = 0.1f;
    p.personality.GunUse.gun_fire_interval_variance_percent = 0.05f;
    p.personality.GunUse.gun_fire_burst_interval_time = 15.0f;
    p.personality.GunUse.gun_fire_burst_interval_variance_percent = 0.05f;
    p.personality.GunUse.gun_fire_no_burst_interval_time = 1.0f;
    p.personality.GunUse.gun_fire_accuracy_cone_angle = 0.00001f;
    p.personality.GunUse.gun_fire_accuracy_power = 100.0f;
    p.personality.GunUse.gun_range_threshold = 1.0f;
    p.personality.GunUse.gun_target_point_switch_time = 0.0f;
    p.personality.GunUse.fire_style = 0;
    p.personality.GunUse.auto_turret_interval_time = 0.1f;
    p.personality.GunUse.auto_turret_burst_interval_time = 15.0f;
    p.personality.GunUse.auto_turret_no_burst_interval_time = 0.1f;
    p.personality.GunUse.auto_turret_burst_interval_variance_percent = 0.1f;
    p.personality.GunUse.gun_range_threshold_variance_percent = 1.0f;
    p.personality.GunUse.gun_fire_accuracy_power_npc = 100.0f;

    p.personality.MineUse.mine_launch_interval = 0.5f;
    p.personality.MineUse.mine_launch_cone_angle = 0.7854f;
    p.personality.MineUse.mine_launch_range = 200.0f;

    p.personality.MissileUse.missile_launch_interval_time = 0.0f;
    p.personality.MissileUse.missile_launch_interval_variance_percent = 0.5f;
    p.personality.MissileUse.missile_launch_range = 800.0f;
    p.personality.MissileUse.missile_launch_cone_angle = 0.01745f;
    p.personality.MissileUse.missile_launch_allow_out_of_range = false;

    p.personality.DamageReaction.evade_break_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.evade_dodge_more_damage_trigger_percent =
        0.25f;
    p.personality.DamageReaction.engine_kill_face_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.engine_kill_face_damage_trigger_time = 0.2f;
    p.personality.DamageReaction.roll_damage_trigger_percent = 0.4f;
    p.personality.DamageReaction.roll_damage_trigger_time = 0.2f;
    p.personality.DamageReaction.afterburner_damage_trigger_percent = 0.2f;
    p.personality.DamageReaction.afterburner_damage_trigger_time = 0.5f;
    p.personality.DamageReaction.brake_reverse_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.drop_mines_damage_trigger_percent = 0.25f;
    p.personality.DamageReaction.drop_mines_damage_trigger_time = 0.1f;
    p.personality.DamageReaction.fire_guns_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.fire_guns_damage_trigger_time = 1.0f;
    p.personality.DamageReaction.fire_missiles_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.fire_missiles_damage_trigger_time = 1.0f;

    p.personality.MissileReaction.evade_missile_distance = 800.0f;
    p.personality.MissileReaction.evade_break_missile_reaction_time = 1.0f;
    p.personality.MissileReaction.evade_slide_missile_reaction_time = 1.0f;
    p.personality.MissileReaction.evade_afterburn_missile_reaction_time = 1.0f;

    p.personality.CountermeasureUse.countermeasure_active_time = 5.0f;
    p.personality.CountermeasureUse.countermeasure_unactive_time = 0.0f;

    p.personality.FormationUse.force_attack_formation_active_time = 0.0f;
    p.personality.FormationUse.force_attack_formation_unactive_time = 0.0f;
    p.personality.FormationUse.break_formation_damage_trigger_percent = 0.01f;
    p.personality.FormationUse.break_formation_damage_trigger_time = 1.0f;
    p.personality.FormationUse.break_formation_missile_reaction_time = 1.0f;
    p.personality.FormationUse.break_apart_formation_missile_reaction_time =
        1.0f;
    p.personality.FormationUse.break_apart_formation_on_evade_break = true;
    p.personality.FormationUse.break_formation_on_evade_break_time = 1.0f;
    p.personality.FormationUse.formation_exit_top_turn_break_away_throttle =
        1.0f;
    p.personality.FormationUse.formation_exit_roll_outrun_throttle = 1.0f;
    p.personality.FormationUse.formation_exit_max_time = 5.0f;
    p.personality.FormationUse.formation_exit_mode = 1;

    p.personality.Job.wait_for_leader_target = false;
    p.personality.Job.maximum_leader_target_distance = 3000;
    p.personality.Job.flee_when_leader_flees_style = false;
    p.personality.Job.scene_toughness_threshold = 4;
    p.personality.Job.flee_scene_threat_style = 4;
    p.personality.Job.flee_when_hull_damaged_percent = 0.01f;
    p.personality.Job.flee_no_weapons_style = true;
    p.personality.Job.loot_flee_threshold = 4;
    p.personality.Job.attack_subtarget_order[0] = 5;
    p.personality.Job.attack_subtarget_order[1] = 6;
    p.personality.Job.attack_subtarget_order[2] = 7;
    p.personality.Job.field_targeting = 3;
    p.personality.Job.loot_preference = 7;
    p.personality.Job.combat_drift_distance = 25000;
    p.personality.Job.attack_order[0].distance = 5000;
    p.personality.Job.attack_order[0].type = 11;
    p.personality.Job.attack_order[0].flag = 15;
    p.personality.Job.attack_order[1].type = 12;

    return p;
}

// Returns a random float between two
float rand_FloatRange(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

// Return random infocard ID from list that was loaded in
uint rand_name() {
    int randomIndex = rand() % npcnames.size();
    return npcnames.at(randomIndex);
}

// Function to log output (usually NPCs that have been created)
void Logging(const char *szString, ...) {
    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);
    char szBuf[64];
    time_t tNow = time(0);
    tm t;
    localtime_s(&t, &tNow);
    strftime(szBuf, sizeof(szBuf), "%d/%m/%Y %H:%M:%S", &t);
    fprintf(Logfile, "%s %s\n", szBuf, szBufString);
    fflush(Logfile);
}

// Logs the NPC being created
void Log_CreateNPC(std::wstring name) {
    // internal log
    std::wstring wscMsgLog = L"created <%name>";
    wscMsgLog = ReplaceStr(wscMsgLog, L"%name", name.c_str());
    std::string scText = wstos(wscMsgLog);
    Logging("%s", scText.c_str());
}

// Function called by next function to remove spawned NPCs from our data
bool IsFLHookNPC(CShip *ship) {
    // If it's a player do nothing
    if (ship->is_player() == true) {
        return false;
    }

    // Is it a FLHook NPC?
    std::list<uint>::iterator iter = npcs.begin();
    while (iter != npcs.end()) {
        if (*iter == ship->get_id()) {
            ship->clear_equip_and_cargo();
            npcs.erase(iter);
            return true;
            break;
        }
        iter++;
    }
    return false;
}

// Hook on ship destroyed to remove from our data
void __stdcall ShipDestroyed(DamageList *_dmg, DWORD *ecx, uint iKill) {
    returncode = DEFAULT_RETURNCODE;
    if (iKill) {
        CShip *cship = (CShip *)ecx[4];
        IsFLHookNPC(cship);
    }
}

// Function that actually spawns the NPC
void CreateNPC(std::wstring name, Vector pos, Matrix rot, uint iSystem,
               bool varyPos) {
    NPC_ARCHTYPESSTRUCT arch = mapNPCArchtypes[name];

    pub::SpaceObj::ShipInfo si;
    memset(&si, 0, sizeof(si));
    si.iFlag = 1;
    si.iSystem = iSystem;
    si.iShipArchetype = arch.Shiparch;
    si.mOrientation = rot;
    si.iLoadout = arch.Loadout;
    si.iLook1 = CreateID("li_newscaster_head_gen_hat");
    si.iLook2 = CreateID("pl_female1_journeyman_body");
    si.iComm = CreateID("comm_br_darcy_female");
    si.iPilotVoice = CreateID("pilot_f_leg_f01a");
    si.iHealth = -1;
    si.iLevel = 19;

    if (varyPos) {
        si.vPos.x = pos.x + rand_FloatRange(0, 1000);
        si.vPos.y = pos.y + rand_FloatRange(0, 1000);
        si.vPos.z = pos.z + rand_FloatRange(0, 2000);
    } else {
        si.vPos.x = pos.x;
        si.vPos.y = pos.y;
        si.vPos.z = pos.z;
    }

    // Define the string used for the scanner name. Because the
    // following entry is empty, the pilot_name is used. This
    // can be overriden to display the ship type instead.
    FmtStr scanner_name(0, 0);
    scanner_name.begin_mad_lib(0);
    scanner_name.end_mad_lib();

    // Define the string used for the pilot name. The example
    // below shows the use of multiple part names.
    FmtStr pilot_name(0, 0);
    pilot_name.begin_mad_lib(16163); // ids of "%s0 %s1"
    if (arch.Infocard != 0) {
        pilot_name.append_string(arch.Infocard);
        if (arch.Infocard2 != 0) {
            pilot_name.append_string(arch.Infocard2);
        }
    } else {
        pilot_name.append_string(rand_name()); // ids that replaces %s0
        pilot_name.append_string(rand_name()); // ids that replaces %s1
    }
    pilot_name.end_mad_lib();

    pub::Reputation::Alloc(si.iRep, scanner_name, pilot_name);
    pub::Reputation::SetAffiliation(si.iRep, arch.IFF);

    uint iSpaceObj;

    pub::SpaceObj::Create(iSpaceObj, si);

    pub::AI::SetPersonalityParams pers = HkMakePersonality(arch.Graph);
    pub::AI::SubmitState(iSpaceObj, &pers);

    npcs.push_back(iSpaceObj);

    return;
}

// Load settings
void LoadNPCInfo() {
    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\npc.cfg";

    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("npcs")) {
                NPC_ARCHTYPESSTRUCT setnpcstruct;
                while (ini.read_value()) {
                    if (ini.is_value("npc")) {
                        std::string setnpcname = ini.get_value_string(0);
                        std::wstring thenpcname = stows(setnpcname);
                        setnpcstruct.Shiparch =
                            CreateID(ini.get_value_string(1));
                        setnpcstruct.Loadout =
                            CreateID(ini.get_value_string(2));

                        // IFF calc
                        pub::Reputation::GetReputationGroup(
                            setnpcstruct.IFF, ini.get_value_string(3));

                        // Selected graph
                        setnpcstruct.Graph = ini.get_value_int(4);

                        // Infocard
                        setnpcstruct.Infocard = ini.get_value_int(5);
                        setnpcstruct.Infocard2 = ini.get_value_int(6);

                        mapNPCArchtypes[thenpcname] = setnpcstruct;
                    }
                }
            } else if (ini.is_header("fleet")) {
                NPC_FLEETSTRUCT setfleet;
                std::wstring thefleetname;
                while (ini.read_value()) {
                    if (ini.is_value("fleetname")) {
                        std::string setfleetname = ini.get_value_string(0);
                        thefleetname = stows(setfleetname);
                        setfleet.fleetname = stows(setfleetname);
                    } else if (ini.is_value("fleetmember")) {
                        std::string setmembername = ini.get_value_string(0);
                        std::wstring membername = stows(setmembername);
                        int amount = ini.get_value_int(1);
                        setfleet.fleetmember[membername] = amount;
                    }
                }
                mapNPCFleets[thefleetname] = setfleet;
            } else if (ini.is_header("names")) {
                while (ini.read_value()) {
                    if (ini.is_value("name")) {
                        npcnames.push_back(ini.get_value_int(0));
                    }
                }
            } else if (ini.is_header("startupnpcs")) {
                while (ini.read_value()) {
                    if (ini.is_value("startupnpc")) {
                        NPC n;
                        n.name = stows(ini.get_value_string(0));
                        n.pos.x = ini.get_value_float(1);
                        n.pos.y = ini.get_value_float(2);
                        n.pos.z = ini.get_value_float(3);
                        n.rot.data[0][0] = ini.get_value_float(4);
                        n.rot.data[0][1] = ini.get_value_float(5);
                        n.rot.data[0][2] = ini.get_value_float(6);
                        n.rot.data[1][0] = ini.get_value_float(7);
                        n.rot.data[1][1] = ini.get_value_float(8);
                        n.rot.data[1][2] = ini.get_value_float(9);
                        n.rot.data[2][0] = ini.get_value_float(10);
                        n.rot.data[2][1] = ini.get_value_float(11);
                        n.rot.data[2][2] = ini.get_value_float(12);
                        n.system = CreateID(ini.get_value_string(13));
                        startupNPCs[startupNPCs.size()] = n;
                    }
                }
            }
        }
        ini.close();
    }
}

// Main Load Settings function, calls the one above. Had to use this hook
// instead of LoadSettings otherwise NPCs wouldnt appear on server startup
void Startup_AFTER() {
    returncode = DEFAULT_RETURNCODE;

    fopen_s(&Logfile, "./flhook_logs/npc_log.log", "at");

    LoadNPCInfo();

    listgraphs.push_back("FIGHTER");   // 0
    listgraphs.push_back("TRANSPORT"); // 1
    listgraphs.push_back("GUNBOAT");   // 2
    listgraphs.push_back("CRUISER");   // 3, doesn't seem to do anything

    for (auto &[id, npc] : startupNPCs) {
        CreateNPC(npc.name, npc.pos, npc.rot, npc.system, false);
        Log_CreateNPC(npc.name);
    }
}

// Admin command to make NPCs
void AdminCmd_AIMake(CCmds *cmds, int Amount, std::wstring NpcType) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    if (Amount == 0) {
        Amount = 1;
    }

    NPC_ARCHTYPESSTRUCT arch;

    bool wrongnpcname = 0;

    std::map<std::wstring, NPC_ARCHTYPESSTRUCT>::iterator iter =
        mapNPCArchtypes.find(NpcType);
    if (iter != mapNPCArchtypes.end()) {
        arch = iter->second;
    } else {
        cmds->Print(L"ERR Wrong NPC name\n");
        return;
    }

    uint iShip1;
    pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()),
                         iShip1);
    if (!iShip1)
        return;

    uint iSystem;
    pub::Player::GetSystem(HkGetClientIdFromCharname(cmds->GetAdminName()),
                           iSystem);

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(iShip1, pos, rot);

    // Creation counter
    for (int i = 0; i < Amount; i++) {
        CreateNPC(NpcType, pos, rot, iSystem, true);
        Log_CreateNPC(NpcType);
    }

    return;
}

// Admin command to destroy the AI
void AdminCmd_AIKill(CCmds *cmds) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    for (auto &npc : npcs)
        pub::SpaceObj::Destroy(npc, DestroyType::FUSE);

    npcs.clear();
    cmds->Print(L"OK\n");

    return;
}

// Admin command to make AI come to your position
void AdminCmd_AICome(CCmds *cmds) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    uint iShip1;
    pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()),
                         iShip1);
    if (iShip1) {
        Vector pos;
        Matrix rot;
        pub::SpaceObj::GetLocation(iShip1, pos, rot);

        for (auto &npc : npcs) {
            pub::AI::DirectiveCancelOp cancelOP;
            pub::AI::SubmitDirective(npc, &cancelOP);

            pub::AI::DirectiveGotoOp go;
            go.iGotoType = 1;
            go.vPos = pos;
            go.vPos.x = pos.x + rand_FloatRange(0, 500);
            go.vPos.y = pos.y + rand_FloatRange(0, 500);
            go.vPos.z = pos.z + rand_FloatRange(0, 500);
            go.fRange = 0;
            pub::AI::SubmitDirective(npc, &go);
        }
    }
    cmds->Print(L"OK\n");
    return;
}

// Admin command to make AI follow target (or admin) until death
void AdminCmd_AIFollow(CCmds *cmds, std::wstring &wscCharname) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    // If no player specified follow the admin
    uint iClientId;
    if (wscCharname == L"") {
        iClientId = HkGetClientIdFromCharname(cmds->GetAdminName());
        wscCharname = cmds->GetAdminName();
    }
    // Follow the player specified
    else {
        iClientId = HkGetClientIdFromCharname(wscCharname);
    }
    if (iClientId == -1) {
        cmds->Print(L"%s is not online\n", wscCharname.c_str());
    } else {
        uint iShip1;
        pub::Player::GetShip(iClientId, iShip1);
        if (iShip1) {
            for (auto &npc : npcs) {
                pub::AI::DirectiveCancelOp cancelOP;
                pub::AI::SubmitDirective(npc, &cancelOP);
                pub::AI::DirectiveFollowOp testOP;
                testOP.iFollowSpaceObj = iShip1;
                testOP.fMaxDistance = 100;
                pub::AI::SubmitDirective(npc, &testOP);
            }
            cmds->Print(L"Following %s\n", wscCharname.c_str());
        } else {
            cmds->Print(L"%s is not in space\n", wscCharname.c_str());
        }
    }
    return;
}

// Admin command to cancel the current operation
void AdminCmd_AICancel(CCmds *cmds) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    uint iShip1;
    pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()),
                         iShip1);
    if (iShip1) {
        for (auto &npc : npcs) {
            pub::AI::DirectiveCancelOp testOP;
            pub::AI::SubmitDirective(npc, &testOP);
        }
    }
    cmds->Print(L"OK\n");
    return;
}

// Admin command to list NPC fleets
void AdminCmd_ListNPCFleets(CCmds *cmds) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    cmds->Print(L"Available fleets: %d\n", mapNPCFleets.size());
    for (auto &[name, npcstruct] : mapNPCFleets)
        cmds->Print(L"|%s\n", name.c_str());

    cmds->Print(L"OK\n");

    return;
}

// Admin command to spawn a Fleet
void AdminCmd_AIFleet(CCmds *cmds, std::wstring FleetName) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    int wrongnpcname = 0;

    std::map<std::wstring, NPC_FLEETSTRUCT>::iterator iter =
        mapNPCFleets.find(FleetName);
    if (iter != mapNPCFleets.end()) {
        NPC_FLEETSTRUCT &fleetmembers = iter->second;
        for (auto &[name, amount] : fleetmembers.fleetmember)
            AdminCmd_AIMake(cmds, amount, name);
    } else {
        wrongnpcname = 1;
    }

    if (wrongnpcname == 1) {
        cmds->Print(L"ERR Wrong Fleet name\n");
        return;
    }

    cmds->Print(L"OK fleet spawned\n");
    return;
}

// Admin command processing
bool ExecuteCommandString_Callback(CCmds *cmds, const std::wstring &wscCmd) {
    returncode = DEFAULT_RETURNCODE;
    if (IS_CMD("aicreate")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_AIMake(cmds, cmds->ArgInt(1), cmds->ArgStr(2));
        return true;
    } else if (IS_CMD("aidestroy")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_AIKill(cmds);
        return true;
    } else if (IS_CMD("aicancel")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_AICancel(cmds);
        return true;
    } else if (IS_CMD("aifollow")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_AIFollow(cmds, cmds->ArgCharname(1));
        return true;
    } else if (IS_CMD("aicome")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_AICome(cmds);
        return true;
    } else if (IS_CMD("aifleet")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_AIFleet(cmds, cmds->ArgStr(1));
        return true;
    } else if (IS_CMD("fleetlist")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        AdminCmd_ListNPCFleets(cmds);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));
    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (set_scCfgFile.length() > 0)
            Startup_AFTER();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
    }
    return true;
}

// Functions to hook
EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "NPCs by Alley and Cannon";
    p_PI->sShortName = "npc";
    p_PI->bMayPause = true;
    p_PI->bMayUnload = true;
    p_PI->ePluginReturnCode = &returncode;
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&Startup_AFTER, PLUGIN_HkIServerImpl_Startup_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ExecuteCommandString_Callback,
                        PLUGIN_ExecuteCommandString_Callback, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ShipDestroyed, PLUGIN_ShipDestroyed, 0));

    return p_PI;
}
