#include "Main.h"

extern bool set_new_spawn;

static pub::AI::SetPersonalityParams MakePersonality()
{
	pub::AI::SetPersonalityParams p;
	p.iStateGraph = pub::StateGraph::get_state_graph("NOTHING", pub::StateGraph::TYPE_STANDARD);
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
	p.personality.BuzzHeadTowardUse.buzz_min_distance_to_head_toward_variance_percent = 0.25f;
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
	p.personality.BuzzHeadTowardUse.buzz_dodge_interval_time_variance_percent = 0.5f;
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

static uint CreateWPlatformNPC(uint iSystem, Vector position, Matrix rotation, uint solar_ids, uint type)
{
	pub::SpaceObj::ShipInfo si;
	memset(&si, 0, sizeof(si));
	si.iFlag = 4;
	si.iSystem = iSystem;
	si.vPos = position;
	si.mOrientation = rotation;

	switch (type)
	{
		case Module::TYPE_DEFENSE_3:
			si.iShipArchetype = CreateID("wplatform_pbase_01");
			si.iLoadout = CreateID("wplatform_pbase_loadout03");
			break;
		case Module::TYPE_DEFENSE_2:
			si.iShipArchetype = CreateID("wplatform_pbase_01");
			si.iLoadout = CreateID("wplatform_pbase_loadout02");
			break;
		case Module::TYPE_DEFENSE_1:
		default:
			si.iShipArchetype = CreateID("wplatform_pbase_01");
			si.iLoadout = CreateID("wplatform_pbase_loadout01");
			break;
	}

	si.iLook1 = 0;      // CreateID("li_newscaster_head_gen_hat");
	si.iLook2 = 0;      // CreateID("pl_female1_journeyman_body");
	si.iComm = 0;       // CreateID("comm_br_darcy_female");
	si.iPilotVoice = 0; // CreateID("pilot_f_leg_f01a");
	si.iHealth = -1;
	si.iLevel = 19;

	// Define the std::string used for the scanner name. Because the
	// following entry is empty, the pilot_name is used. This
	// can be overriden to display the ship type instead.
	FmtStr infoname(0, 0);
	infoname.begin_mad_lib(0);
	// infoname.append_string(solar_ids);  // ids that replaces %s0
	// infoname.append_string(261164); // ids that replaces %s1
	infoname.end_mad_lib();

	// Define the std::string used for the pilot name. The example
	// below shows the use of multiple part names.
	FmtStr infocard(0, 0);
	infocard.begin_mad_lib(16162);     //  = ids of "%s0 %s1"
	infocard.append_string(solar_ids); // ids that replaces %s0
	infocard.append_string(261164);    // ids that replaces %s1
	infocard.end_mad_lib();

	pub::Reputation::Alloc(si.iRep, infoname, infocard);

	uint obj_rep_group;
	pub::Reputation::GetReputationGroup(obj_rep_group, "fc_neutral");
	pub::Reputation::SetAffiliation(si.iRep, obj_rep_group);

	uint space_obj;
	pub::SpaceObj::Create(space_obj, si);

	pub::AI::SetPersonalityParams pers = MakePersonality();
	pub::AI::SubmitState(space_obj, &pers);

	return space_obj;
}

static void SpawnSolar(unsigned int& spaceID, pub::SpaceObj::SolarInfo const& solarInfo)
{
	// hack server.dll so it does not call create solar packet send
	char* serverHackAddress = (char*)hModServer + 0x2A62A;
	char serverHack[] = { '\xEB' };
	WriteProcMem(serverHackAddress, &serverHack, 1);

	pub::SpaceObj::CreateSolar(spaceID, solarInfo);

	uint dunno;
	IObjInspectImpl* inspect;
	if (GetShipInspect(spaceID, inspect, dunno))
	{
		CSolar* solar = (CSolar*)inspect->cobject();

		// for every player in the same system, send solar creation packet
		struct SOLAR_STRUCT
		{
			uchar dunno[0x100];
		};

		SOLAR_STRUCT packetSolar;

		char* address1 = (char*)hModServer + 0x163F0;
		char* address2 = (char*)hModServer + 0x27950;

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

		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			if (pPD->iSystemID == solarInfo.iSystemID)
				GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(
				    pPD->iOnlineID, (FLPACKET_CREATESOLAR&)packetSolar);
		}
	}

	// undo the server.dll hack
	char serverUnHack[] = { '\x74' };
	WriteProcMem(serverHackAddress, &serverUnHack, 1);
}

static uint CreateWPlatformSolar(
    PlayerBase* base, uint iSystem, Vector position, Matrix rotation, uint solar_ids, uint type)
{
	pub::SpaceObj::SolarInfo si;
	memset(&si, 0, sizeof(si));
	si.iFlag = 4;
	si.iSystemID = iSystem;
	si.vPos = position;
	si.mOrientation = rotation;

	switch (type)
	{
		case Module::TYPE_DEFENSE_3:
			si.iArchID = CreateID("wplatform_pbase_01");
			si.iLoadoutID = CreateID("wplatform_pbase_loadout03");
			break;
		case Module::TYPE_DEFENSE_2:
			si.iArchID = CreateID("wplatform_pbase_01");
			si.iLoadoutID = CreateID("wplatform_pbase_loadout02");
			break;
		case Module::TYPE_DEFENSE_1:
		default:
			si.iArchID = CreateID("wplatform_pbase_01");
			si.iLoadoutID = CreateID("wplatform_pbase_loadout01");
			break;
	}

	si.Costume.head = CreateID("pi_pirate2_head");
	si.Costume.body = CreateID("pi_pirate8_body");
	si.Costume.lefthand = 0;
	si.Costume.righthand = 0;
	si.Costume.accessories = 0;
	si.iVoiceID = CreateID("atc_leg_m01");

	std::string wplatform_nickname = base->nickname + std::to_string(rand());

	strncpy_s(si.cNickName, sizeof(si.cNickName), wplatform_nickname.c_str(), wplatform_nickname.size());

	si.iHitPointsLeft = -1;

	// Set the base name
	FmtStr infoname(solar_ids, 0);
	infoname.begin_mad_lib(solar_ids); // scanner name
	infoname.end_mad_lib();

	FmtStr infocard(solar_ids, 0);
	infocard.begin_mad_lib(solar_ids); // infocard
	infocard.end_mad_lib();
	pub::Reputation::Alloc(si.iRep, infoname, infocard);

	// infocard.begin_mad_lib(16162); //  = ids of "%s0 %s1"
	// infocard.append_string(solar_ids);  // ids that replaces %s0
	// infocard.append_string(261164); // ids that replaces %s1

	uint space_obj;
	SpawnSolar(space_obj, si);

	pub::AI::SetPersonalityParams pers = MakePersonality();
	pub::AI::SubmitState(space_obj, &pers);

	return space_obj;
}

DefenseModule::DefenseModule(PlayerBase* the_base) : Module(Module::TYPE_DEFENSE_1), base(the_base), space_obj(0)
{
	pos = base->position;
	rot = MatrixToEuler(base->rotation);
	TranslateY(pos, base->rotation, 200);
}

DefenseModule::DefenseModule(PlayerBase* the_base, uint the_type) : Module(the_type), base(the_base), space_obj(0)
{
	pos = base->position;
	rot = MatrixToEuler(base->rotation);
	TranslateY(pos, base->rotation, 200);
}

DefenseModule::~DefenseModule()
{
	if (space_obj)
	{
		pub::SpaceObj::Destroy(space_obj, DestroyType::VANISH);
		spaceobj_modules.erase(space_obj);
		space_obj = 0;
	}
}

void DefenseModule::Reset()
{
	if (space_obj)
	{
		pub::SpaceObj::Destroy(space_obj, DestroyType::VANISH);
		spaceobj_modules.erase(space_obj);
		space_obj = 0;
	}
}

std::wstring DefenseModule::GetInfo(bool xml)
{
	switch (type)
	{
		case Module::TYPE_DEFENSE_1:
			return L"Defense Platform Array Type 1";
		case Module::TYPE_DEFENSE_2:
			return L"Defense Platform Array Type 2";
		case Module::TYPE_DEFENSE_3:
			return L"Defense Platform Array Type 3";
		default:
			return L"Wibble";
	}
}

// Load module state from ini file.
void DefenseModule::LoadState(INI_Reader& ini)
{
	while (ini.read_value())
	{
		if (ini.is_value("type"))
		{
			type = ini.get_value_int(0);
		}
		else if (ini.is_value("pos"))
		{
			pos.x = ini.get_value_float(0);
			pos.y = ini.get_value_float(1);
			pos.z = ini.get_value_float(2);
		}
		else if (ini.is_value("rot"))
		{
			rot.x = ini.get_value_float(0);
			rot.y = ini.get_value_float(1);
			rot.z = ini.get_value_float(2);
		}
	}
}

// Append module state to the ini file.
void DefenseModule::SaveState(FILE* file)
{
	fprintf(file, "[DefenseModule]\n");
	fprintf(file, "type = %u\n", type);
	fprintf(file, "pos = %0.0f, %0.0f, %0.0f\n", pos.x, pos.y, pos.z);
	fprintf(file, "rot = %0.0f, %0.0f, %0.0f\n", rot.x, rot.y, rot.z);
}

bool DefenseModule::Timer(uint time)
{
	if ((time % set_tick_time) != 0)
		return false;

	if (!space_obj)
	{
		if (set_new_spawn)
			space_obj = CreateWPlatformSolar(base, base->system, pos, EulerMatrix(rot), base->solar_ids, type);
		else
			space_obj = CreateWPlatformNPC(base->system, pos, EulerMatrix(rot), base->solar_ids, type);

		spaceobj_modules[space_obj] = this;
		if (set_plugin_debug > 1)
			Console::ConInfo(L"DefenseModule::created space_obj=%u", space_obj);
		base->SyncReputationForBaseObject(space_obj);
	}

	return false;
}

float DefenseModule::SpaceObjDamaged(uint space_obj, uint attacking_space_obj, float curr_hitpoints, float damage)
{
	base->SpaceObjDamaged(space_obj, attacking_space_obj, curr_hitpoints, damage);
	return damage;
}

bool DefenseModule::SpaceObjDestroyed(uint space_obj)
{
	if (this->space_obj == space_obj)
	{
		if (set_plugin_debug > 1)
			Console::ConInfo(L"DefenseModule::destroyed space_obj=%u", space_obj);
		spaceobj_modules.erase(space_obj);
		this->space_obj = 0;
		return true;
	}
	return false;
}

void DefenseModule::SetReputation(int player_rep, float attitude)
{
	if (this->space_obj)
	{
		int obj_rep;
		pub::SpaceObj::GetRep(this->space_obj, obj_rep);
		pub::Reputation::SetAttitude(obj_rep, player_rep, attitude);
	}
}
