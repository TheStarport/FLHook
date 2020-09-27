#include <windows.h>
#include <FLHook.h>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <plugin_comms.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "Main.h"

CoreModule::CoreModule(PlayerBase *the_base)
    : Module(TYPE_CORE), base(the_base), space_obj(0), dont_eat(false),
      dont_rust(false) {}

CoreModule::~CoreModule() {
  if (space_obj) {
    pub::SpaceObj::Destroy(space_obj, DestroyType::VANISH);
    spaceobj_modules.erase(space_obj);
    space_obj = 0;
  }
}

static pub::AI::SetPersonalityParams MakePersonality() {
  pub::AI::SetPersonalityParams p;
  p.iStateGraph = pub::StateGraph::get_state_graph(
      "NOTHING", pub::StateGraph::TYPE_STANDARD);
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
  p.personality.EvadeDodgeUse.evade_dodge_corkscrew_roll_flip_direction = true;
  p.personality.EvadeDodgeUse.evade_dodge_interval_time_variance_percent = 0.5f;
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
  p.personality.BuzzHeadTowardUse.buzz_dodge_cone_angle_variance_percent = 0.5f;
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

  p.personality.MineUse.mine_launch_interval = 8.0f;
  p.personality.MineUse.mine_launch_cone_angle = 0.7854f;
  p.personality.MineUse.mine_launch_range = 200.0f;

  p.personality.MissileUse.missile_launch_interval_time = 0.0f;
  p.personality.MissileUse.missile_launch_interval_variance_percent = 0.5f;
  p.personality.MissileUse.missile_launch_range = 800.0f;
  p.personality.MissileUse.missile_launch_cone_angle = 0.01745f;
  p.personality.MissileUse.missile_launch_allow_out_of_range = false;

  p.personality.DamageReaction.evade_break_damage_trigger_percent = 1.0f;
  p.personality.DamageReaction.evade_dodge_more_damage_trigger_percent = 0.25f;
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
  p.personality.FormationUse.break_apart_formation_missile_reaction_time = 1.0f;
  p.personality.FormationUse.break_apart_formation_on_evade_break = true;
  p.personality.FormationUse.break_formation_on_evade_break_time = 1.0f;
  p.personality.FormationUse.formation_exit_top_turn_break_away_throttle = 1.0f;
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

static void SpawnSolar(unsigned int &spaceID,
                       pub::SpaceObj::SolarInfo const &solarInfo) {
  // hack server.dll so it does not call create solar packet send
  char *serverHackAddress = (char *)hModServer + 0x2A62A;
  char serverHack[] = {'\xEB'};
  WriteProcMem(serverHackAddress, &serverHack, 1);

  pub::SpaceObj::CreateSolar(spaceID, solarInfo);

  uint dunno;
  IObjInspectImpl *inspect;
  if (GetShipInspect(spaceID, inspect, dunno)) {
    CSolar *solar = (CSolar *)inspect->cobject();

    // for every player in the same system, send solar creation packet
    struct SOLAR_STRUCT {
      uchar dunno[0x100];
    };

    SOLAR_STRUCT packetSolar;

    char *address1 = (char *)hModServer + 0x163F0;
    char *address2 = (char *)hModServer + 0x27950;

    // fill struct
    __asm {
        pushad
        lea ecx, packetSolar
        mov eax, address1
        call eax
        push solar
        lea ecx, packetSolar
        push ecx
        mov eax, address2
        call eax
        add esp, 8
        popad
    }

    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
      if (pPD->iSystemID == solarInfo.iSystemID)
        GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(
            pPD->iOnlineID, (FLPACKET_CREATESOLAR &)packetSolar);
    }
  }

  // undo the server.dll hack
  char serverUnHack[] = {'\x74'};
  WriteProcMem(serverHackAddress, &serverUnHack, 1);
}

void CoreModule::Spawn() {
  if (!space_obj) {
    pub::SpaceObj::SolarInfo si;
    memset(&si, 0, sizeof(si));
    si.iFlag = 4;

    char archname[100];
    _snprintf_s(archname, sizeof(archname), "dsy_playerbase_%02u",
                base->base_level);
    si.iArchID = CreateID(archname);
    si.iLoadoutID = CreateID(archname);

    si.iHitPointsLeft = 1;
    si.iSystemID = base->system;
    si.mOrientation = base->rotation;
    si.vPos = base->position;
    si.Costume.head = CreateID("pi_pirate2_head");
    si.Costume.body = CreateID("pi_pirate8_body");
    si.Costume.lefthand = 0;
    si.Costume.righthand = 0;
    si.Costume.accessories = 0;
    si.iVoiceID = CreateID("atc_leg_m01");
    strncpy_s(si.cNickName, sizeof(si.cNickName), base->nickname.c_str(),
              base->nickname.size());

    // Check to see if the hook IDS limit has been reached
    static uint solar_ids = 526000;
    if (++solar_ids > 526999) {
      solar_ids = 0;
      return;
    }

    // Send the base name to all players that are online
    base->solar_ids = solar_ids;

    std::wstring basename = base->basename;
    // if (base->affiliation)
    //{
    //	basename = HkGetWStringFromIDS(Reputation::get_name(base->affiliation))
    //+ L" - " + base->basename;
    //}

    struct PlayerData *pd = 0;
    while (pd = Players.traverse_active(pd)) {
      HkChangeIDSString(pd->iOnlineID, base->solar_ids, basename);
    }

    // Set the base name
    FmtStr infoname(solar_ids, 0);
    infoname.begin_mad_lib(solar_ids); // scanner name
    infoname.end_mad_lib();

    FmtStr infocard(solar_ids, 0);
    infocard.begin_mad_lib(solar_ids); // infocard
    infocard.end_mad_lib();
    pub::Reputation::Alloc(si.iRep, infoname, infocard);

    SpawnSolar(space_obj, si);
    spaceobj_modules[space_obj] = this;

    // Set base health to reflect saved value unless this is a new base with
    // a health of zero in which case we set it to 5% of the maximum and let
    // players repair it.
    float current;
    pub::SpaceObj::GetHealth(space_obj, current, base->max_base_health);
    if (base->base_health <= 0)
      base->base_health = base->max_base_health * 0.05f;
    else if (base->base_health > base->max_base_health)
      base->base_health = base->max_base_health;
    pub::SpaceObj::SetRelativeHealth(space_obj,
                                     base->base_health / base->max_base_health);

    base->SyncReputationForBaseObject(space_obj);
    if (set_plugin_debug > 1)
      ConPrint(L"CoreModule::created space_obj=%u health=%f\n", space_obj,
               base->base_health);

    pub::AI::SetPersonalityParams pers = MakePersonality();
    pub::AI::SubmitState(space_obj, &pers);
  }
}

std::wstring CoreModule::GetInfo(bool xml) { return L"Core"; }

void CoreModule::LoadState(INI_Reader &ini) {
  while (ini.read_value()) {
    if (ini.is_value("dont_eat")) {
      dont_eat = (ini.get_value_int(0) == 1);
    } else if (ini.is_value("dont_rust")) {
      dont_rust = (ini.get_value_int(0) == 1);
    }
  }
}

void CoreModule::SaveState(FILE *file) {
  fprintf(file, "[CoreModule]\n");
  fprintf(file, "dont_eat = %d\n", dont_eat);
  fprintf(file, "dont_rust = %d\n", dont_rust);
}

void CoreModule::RepairDamage(float max_base_health) {
  if (base->base_health >= max_base_health)
    return;

  // The bigger the base the more damage can be repaired.
  for (uint repair_cycles = 0; repair_cycles < base->base_level;
       ++repair_cycles) {
    for (auto &item : set_base_repair_items) {
      if (base->HasMarketItem(item.good) >= item.quantity) {
        base->RemoveMarketGood(item.good, item.quantity);
        base->base_health += 60000;
        base->repairing = true;
      }
    }
  }
}

bool CoreModule::Timer(uint time) {
  if ((time % set_tick_time) != 0)
    return false;

  if (space_obj) {
    pub::SpaceObj::GetHealth(space_obj, base->base_health,
                             base->max_base_health);

    if (!dont_rust) {
      // Reduce hitpoints to reflect wear and tear. This will eventually
      // destroy the base unless it is able to repair itself.
      base->base_health -=
          set_damage_per_tick + (set_damage_per_tick * base->base_level);
    }

    // Repair damage if we have sufficient crew on the base.
    base->repairing = false;
    uint number_of_crew = base->HasMarketItem(set_base_crew_type);
    if (number_of_crew >= (base->base_level * 200))
      RepairDamage(base->max_base_health);

    if (base->base_health > base->max_base_health)
      base->base_health = base->max_base_health;
    else if (base->base_health <= 0)
      base->base_health = 0;

    if (!dont_eat) {
      // Humans use commodity_oxygen, commodity_water. Consume these for
      // the crew or kill 10 crew off and repeat this every 8 hours.
      if (time % 28800 == 0) {
        for (auto i = set_base_crew_consumption_items.begin();
             i != set_base_crew_consumption_items.end(); ++i) {
          // Use water and oxygen.
          if (base->HasMarketItem(i->first) >= number_of_crew) {
            base->RemoveMarketGood(i->first, number_of_crew);
          }
          // Insufficient water and oxygen, kill crew.
          else {
            base->RemoveMarketGood(set_base_crew_type, (number_of_crew >= 10)
                                                           ? 10
                                                           : number_of_crew);
          }
        }

        // Humans use food but may eat one of a number of types.
        uint crew_to_feed = number_of_crew;
        for (auto i = set_base_crew_food_items.begin();
             i != set_base_crew_food_items.end(); ++i) {
          if (!crew_to_feed)
            break;

          uint food_available = base->HasMarketItem(i->first);
          if (food_available) {
            uint food_to_use = (food_available >= crew_to_feed)
                                   ? crew_to_feed
                                   : food_available;
            base->RemoveMarketGood(i->first, food_to_use);
            crew_to_feed -= food_to_use;
          }
        }

        // Insufficent food so kill crew.
        if (crew_to_feed) {
          base->RemoveMarketGood(set_base_crew_type,
                                 (crew_to_feed >= 10) ? 10 : crew_to_feed);
        }
      }
    }

    // Save the new base health
    float rhealth = base->base_health / base->max_base_health;
    pub::SpaceObj::SetRelativeHealth(space_obj, rhealth);
    if (set_plugin_debug > 1)
      ConPrint(L"CoreModule::timer space_obj=%u health=%f\n", space_obj,
               base->base_health);

    // if health is 0 then the object will be destroyed but we won't
    // receive a notification of this so emulate it.
    if (base->base_health < 1)
      return SpaceObjDestroyed(space_obj);
  }

  return false;
}

float CoreModule::SpaceObjDamaged(uint space_obj, uint attacking_space_obj,
                                  float curr_hitpoints, float damage) {
  base->SpaceObjDamaged(space_obj, attacking_space_obj, curr_hitpoints, damage);

  // Reduce the damage to 10% if the shield is or will be online.
  if (base->shield_state != PlayerBase::SHIELD_STATE_OFFLINE) {
    return curr_hitpoints -
           ((curr_hitpoints - damage) * set_shield_damage_multiplier);
  }

  return 0.0f;
}

bool CoreModule::SpaceObjDestroyed(uint space_obj) {
  if (this->space_obj == space_obj) {
    if (set_plugin_debug > 1)
      ConPrint(L"CoreModule::destroyed space_obj=%u\n", space_obj);
    pub::SpaceObj::LightFuse(space_obj, "player_base_explode_fuse", 0);
    spaceobj_modules.erase(space_obj);
    this->space_obj = 0;

    struct PlayerData *pd = 0;
    while (pd = Players.traverse_active(pd)) {
      PrintUserCmdText(pd->iOnlineID, L"Base %s destroyed",
                       base->basename.c_str());
    }

    // Unspawn, delete base and save file.
    DeleteBase(base);

    // Careful not to access this as this object will have been deleted by now.
    return true;
  }
  return false;
}

void CoreModule::SetReputation(int player_rep, float attitude) {
  if (space_obj) {
    int obj_rep;
    pub::SpaceObj::GetRep(this->space_obj, obj_rep);
    if (set_plugin_debug > 1)
      ConPrint(L"CoreModule::SetReputation player_rep=%u obj_rep=%u "
               L"attitude=%f base=%08x\n",
               player_rep, obj_rep, attitude, base->base);
    pub::Reputation::SetAttitude(obj_rep, player_rep, attitude);
  }
}
