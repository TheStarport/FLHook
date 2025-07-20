#include "PCH.hpp"

#include "API/FLHook/PersonalityHelper.hpp"

Action<pub::AI::Personality*> PersonalityHelper::GetPersonality(const std::wstring& pilotNickname)
{
    const auto& pilot = pilots.find(pilotNickname);
    if (pilot == pilots.end())
    {
        return { cpp::fail(Error::InvalidPersonality) };
    }

    return { &pilot->second };
}

void PersonalityHelper::SetDirection(INI_Reader& ini, float (&direction)[4])
{
    if (const auto str = std::string(ini.get_value_string(0)); str == "right")
    {
        direction[0] = ini.get_value_float(1);
    }
    else if (str == "left")
    {
        direction[1] = ini.get_value_float(1);
    }
    else if (str == "up")
    {
        direction[2] = ini.get_value_float(1);
    }
    else if (str == "down")
    {
        direction[3] = ini.get_value_float(1);
    }
}

void PersonalityHelper::LoadEvadeDodge(INI_Reader& ini)
{
    pub::AI::Personality::EvadeDodgeUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("evade_dodge_style_weight"))
        {
            const auto str = std::string(ini.get_value_string(0));
            if (str == "waggle")
            {
                data.evade_dodge_style_weight[0] = ini.get_value_float(1);
            }
            else if (str == "waggle_random")
            {
                data.evade_dodge_style_weight[1] = ini.get_value_float(1);
            }
            else if (str == "slide")
            {
                data.evade_dodge_style_weight[2] = ini.get_value_float(1);
            }
            else if (str == "corkscrew")
            {
                data.evade_dodge_style_weight[3] = ini.get_value_float(1);
            }
        }
        else if (ini.is_value("evade_dodge_direction_weight"))
        {
            SetDirection(ini, data.evade_dodge_direction_weight);
        }
        else if (ini.is_value("evade_dodge_cone_angle"))
        {
            data.evade_dodge_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_interval_time"))
        {
            data.evade_dodge_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_time"))
        {
            data.evade_dodge_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_distance"))
        {
            data.evade_dodge_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_activate_range"))
        {
            data.evade_activate_range = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_roll_angle"))
        {
            data.evade_dodge_roll_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_waggle_axis_cone_angle"))
        {
            data.evade_dodge_waggle_axis_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_slide_throttle"))
        {
            data.evade_dodge_slide_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_turn_throttle"))
        {
            data.evade_dodge_turn_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_corkscrew_turn_throttle"))
        {
            data.evade_dodge_corkscrew_turn_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_corkscrew_roll_throttle"))
        {
            data.evade_dodge_corkscrew_roll_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_corkscrew_roll_flip_direction"))
        {
            data.evade_dodge_corkscrew_roll_flip_direction = ini.get_value_bool(0);
        }
        else if (ini.is_value("evade_dodge_interval_time_variance_percent"))
        {
            data.evade_dodge_interval_time_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_cone_angle_variance_percent"))
        {
            data.evade_dodge_cone_angle_variance_percent = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        evadeDodge[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadEvadeBreak(INI_Reader& ini)
{
    pub::AI::Personality::EvadeBreakUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("evade_break_roll_throttle"))
        {
            data.evade_break_roll_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_time"))
        {
            data.evade_break_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_interval_time"))
        {
            data.evade_break_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_afterburner_delay"))
        {
            data.evade_break_afterburner_delay = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_turn_throttle"))
        {
            data.evade_break_turn_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_direction_weight"))
        {
            SetDirection(ini, data.evade_break_direction_weight);
        }
        else if (ini.is_value("evade_break_roll_throttle"))
        {
            const auto str = std::string(ini.get_value_string(0));
            if (str == "sideways")
            {
                data.evade_break_style_weight[0] = ini.get_value_float(1);
            }
            else if (str == "outrun")
            {
                data.evade_break_style_weight[1] = ini.get_value_float(1);
            }
            else if (str == "reverse")
            {
                data.evade_break_style_weight[2] = ini.get_value_float(1);
            }
        }
        else if (ini.is_value("evade_break_attempt_reverse_time"))
        {
            data.evade_break_attempt_reverse_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_reverse_distance"))
        {
            data.evade_break_reverse_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_afterburner_delay_variance_percent"))
        {
            data.evade_break_afterburner_delay_variance_percent = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        evadeBreak[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadBuzzHead(INI_Reader& ini)
{
    pub::AI::Personality::BuzzHeadTowardUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("buzz_dodge_cone_angle"))
        {
            data.buzz_dodge_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_dodge_cone_angle_variance_percent"))
        {
            data.buzz_dodge_cone_angle_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_dodge_interval_time"))
        {
            data.buzz_dodge_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_dodge_interval_time_variance_percent"))
        {
            data.buzz_dodge_interval_time_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_dodge_roll_angle"))
        {
            data.buzz_dodge_roll_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_dodge_direction_weight"))
        {
            SetDirection(ini, data.buzz_dodge_direction_weight);
        }
        else if (ini.is_value("buzz_head_toward_style_weight"))
        {
            const auto str = std::string(ini.get_value_string(0));
            if (str == "straight_to")
            {
                data.buzz_head_toward_style_weight[0] = ini.get_value_float(1);
            }
            else if (str == "slide")
            {
                data.buzz_head_toward_style_weight[1] = ini.get_value_float(1);
            }
            else if (str == "waggle")
            {
                data.buzz_head_toward_style_weight[2] = ini.get_value_float(1);
            }
        }
        else if (ini.is_value("buzz_dodge_turn_throttle"))
        {
            data.buzz_dodge_turn_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_dodge_waggle_axis_cone_angle"))
        {
            data.buzz_dodge_waggle_axis_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_head_toward_engine_throttle"))
        {
            data.buzz_head_toward_engine_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_head_toward_roll_flip_direction"))
        {
            data.buzz_head_toward_roll_flip_direction = ini.get_value_bool(0);
        }
        else if (ini.is_value("buzz_head_toward_roll_throttle"))
        {
            data.buzz_head_toward_roll_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_slide_throttle"))
        {
            data.buzz_slide_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_slide_interval_time_variance_percent"))
        {
            data.buzz_slide_interval_time_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_slide_interval_time"))
        {
            data.buzz_slide_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_min_distance_to_head_toward"))
        {
            data.buzz_min_distance_to_head_toward = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_min_distance_to_head_toward_variance_percent"))
        {
            data.buzz_min_distance_to_head_toward_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_max_time_to_head_away"))
        {
            data.buzz_max_time_to_head_away = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_head_toward_turn_throttle"))
        {
            data.buzz_head_toward_turn_throttle = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        buzzHead[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadBuzzPass(INI_Reader& ini)
{
    pub::AI::Personality::BuzzPassByUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("buzz_break_direction_cone_angle"))
        {
            data.buzz_break_direction_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_break_turn_throttle"))
        {
            data.buzz_break_turn_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_distance_to_pass_by"))
        {
            data.buzz_distance_to_pass_by = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_drop_bomb_on_pass_by"))
        {
            data.buzz_drop_bomb_on_pass_by = ini.get_value_bool(0);
        }
        else if (ini.is_value("buzz_pass_by_roll_throttle"))
        {
            data.buzz_pass_by_roll_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_break_direction_weight"))
        {
            SetDirection(ini, data.buzz_break_direction_weight);
        }
        else if (ini.is_value("evade_break_roll_throttle"))
        {
            const auto str = std::string(ini.get_value_string(0));
            if (str == "straight_by")
            {
                data.buzz_pass_by_style_weight[0] = ini.get_value_float(1);
            }
            else if (str == "break_away")
            {
                data.buzz_pass_by_style_weight[1] = ini.get_value_float(1);
            }
            else if (str == "engine_kil")
            {
                data.buzz_pass_by_style_weight[2] = ini.get_value_float(1);
            }
        }
        else if (ini.is_value("buzz_pass_by_roll_throttle"))
        {
            data.buzz_pass_by_roll_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("buzz_pass_by_time"))
        {
            data.buzz_pass_by_time = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        buzzPass[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadTrail(INI_Reader& ini)
{
    pub::AI::Personality::TrailUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("trail_break_afterburner"))
        {
            data.trail_break_afterburner = ini.get_value_bool(0);
        }
        else if (ini.is_value("trail_break_roll_throttle"))
        {
            data.trail_break_roll_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("trail_break_time"))
        {
            data.trail_break_time = ini.get_value_float(0);
        }
        else if (ini.is_value("trail_distance"))
        {
            data.trail_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("trail_lock_cone_angle"))
        {
            data.trail_lock_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("trail_max_turn_throttle"))
        {
            data.trail_max_turn_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("trail_min_no_lock_time"))
        {
            data.trail_min_no_lock_time = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        trail[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadStrafe(INI_Reader& ini)
{
    pub::AI::Personality::StrafeUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("strafe_attack_throttle"))
        {
            data.strafe_attack_throttle = ini.get_value_bool(0);
        }
        else if (ini.is_value("strafe_run_away_distance"))
        {
            data.strafe_run_away_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("strafe_turn_throttle"))
        {
            data.strafe_turn_throttle = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        strafe[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadEngineKill(INI_Reader& ini)
{
    pub::AI::Personality::EngineKillUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("engine_kill_afterburner_time"))
        {
            data.engine_kill_afterburner_time = ini.get_value_float(0);
        }
        else if (ini.is_value("engine_kill_face_time"))
        {
            data.engine_kill_face_time = ini.get_value_float(0);
        }
        else if (ini.is_value("engine_kill_max_target_distance"))
        {
            data.engine_kill_max_target_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("engine_kill_search_time"))
        {
            data.engine_kill_search_time = ini.get_value_float(0);
        }
        else if (ini.is_value("engine_kill_use_afterburner"))
        {
            data.engine_kill_use_afterburner = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        engineKill[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadRepair(INI_Reader& ini)
{
    pub::AI::Personality::RepairUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("use_hull_repair_at_damage_percent"))
        {
            data.use_hull_repair_at_damage_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("use_hull_repair_post_delay"))
        {
            data.use_hull_repair_post_delay = ini.get_value_float(0);
        }
        else if (ini.is_value("use_hull_repair_pre_delay"))
        {
            data.use_hull_repair_pre_delay = ini.get_value_float(0);
        }
        else if (ini.is_value("use_shield_repair_at_damage_percent"))
        {
            data.use_shield_repair_at_damage_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("use_shield_repair_post_delay"))
        {
            data.use_shield_repair_post_delay = ini.get_value_float(0);
        }
        else if (ini.is_value("use_shield_repair_pre_delay"))
        {
            data.use_shield_repair_pre_delay = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        repair[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadGun(INI_Reader& ini)
{
    pub::AI::Personality::GunUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("gun_fire_accuracy_cone_angle"))
        {
            data.gun_fire_accuracy_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_accuracy_power"))
        {
            data.gun_fire_accuracy_power = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_accuracy_power_npc"))
        {
            data.gun_fire_accuracy_power_npc = ini.get_value_float(0);
        }
        else if (ini.is_value("use_shield_repair_at_damage_percent"))
        {
            data.gun_fire_burst_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_burst_interval_variance_percent"))
        {
            data.gun_fire_burst_interval_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_interval_time"))
        {
            data.gun_fire_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_burst_interval_variance_percent"))
        {
            data.gun_fire_burst_interval_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_no_burst_interval_time"))
        {
            data.gun_fire_no_burst_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_fire_interval_variance_percent"))
        {
            data.gun_fire_interval_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_range_threshold"))
        {
            data.gun_range_threshold = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_range_threshold_variance_percent"))
        {
            data.gun_range_threshold_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("gun_target_point_switch_time"))
        {
            data.gun_target_point_switch_time = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        gun[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadMine(INI_Reader& ini)
{
    pub::AI::Personality::MineUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("mine_launch_cone_angle"))
        {
            data.mine_launch_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("mine_launch_interva"))
        {
            data.mine_launch_interval = ini.get_value_float(0);
        }
        else if (ini.is_value("mine_launch_range"))
        {
            data.mine_launch_range = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        mine[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadMissileReaction(INI_Reader& ini)
{
    pub::AI::Personality::MissileReactionStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("evade_afterburn_missile_reaction_time"))
        {
            data.evade_afterburn_missile_reaction_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_missile_reaction_time"))
        {
            data.evade_break_missile_reaction_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_missile_distance"))
        {
            data.evade_missile_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_slide_missile_reaction_time"))
        {
            data.evade_slide_missile_reaction_time = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        missileReaction[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadDamageReaction(INI_Reader& ini)
{
    pub::AI::Personality::DamageReactionStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("afterburner_damage_trigger_percent"))
        {
            data.afterburner_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("afterburner_damage_trigger_time"))
        {
            data.afterburner_damage_trigger_time = ini.get_value_float(0);
        }
        else if (ini.is_value("brake_reverse_damage_trigger_percent"))
        {
            data.brake_reverse_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("drop_mines_damage_trigger_percent"))
        {
            data.drop_mines_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("drop_mines_damage_trigger_time"))
        {
            data.drop_mines_damage_trigger_time = ini.get_value_float(0);
        }
        else if (ini.is_value("engine_kill_face_damage_trigger_percent"))
        {
            data.engine_kill_face_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("engine_kill_face_damage_trigger_time"))
        {
            data.engine_kill_face_damage_trigger_time = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_break_damage_trigger_percent"))
        {
            data.evade_break_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("evade_dodge_more_damage_trigger_percent"))
        {
            data.evade_dodge_more_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("fire_guns_damage_trigger_percent"))
        {
            data.fire_guns_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("fire_guns_damage_trigger_time"))
        {
            data.fire_guns_damage_trigger_time = ini.get_value_float(0);
        }
        else if (ini.is_value("fire_missiles_damage_trigger_percent"))
        {
            data.fire_missiles_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("fire_missiles_damage_trigger_time"))
        {
            data.fire_missiles_damage_trigger_time = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        damageReaction[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadCM(INI_Reader& ini)
{
    pub::AI::Personality::CountermeasureUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("countermeasure_active_time"))
        {
            data.countermeasure_active_time = ini.get_value_float(0);
        }
        else if (ini.is_value("countermeasure_unactive_time"))
        {
            data.countermeasure_unactive_time = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        cm[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadFormation(INI_Reader& ini)
{
    pub::AI::Personality::FormationUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("break_apart_formation_damage_trigger_percent"))
        {
            data.break_apart_formation_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("break_apart_formation_damage_trigger_time"))
        {
            data.break_apart_formation_damage_trigger_time = ini.get_value_float(0);
        }
        else if (ini.is_value("break_apart_formation_missile_reaction_time"))
        {
            data.break_apart_formation_missile_reaction_time = ini.get_value_float(0);
        }
        else if (ini.is_value("break_apart_formation_on_buzz_head_toward"))
        {
            data.break_apart_formation_on_buzz_head_toward = ini.get_value_bool(0);
        }
        else if (ini.is_value("break_apart_formation_on_buzz_head_toward"))
        {
            data.break_apart_formation_on_buzz_head_toward = ini.get_value_bool(0);
        }
        else if (ini.is_value("break_apart_formation_on_buzz_pass_by"))
        {
            data.break_apart_formation_on_buzz_pass_by = ini.get_value_bool(0);
        }
        else if (ini.is_value("break_apart_formation_on_evade_break"))
        {
            data.break_apart_formation_on_evade_break = ini.get_value_bool(0);
        }
        else if (ini.is_value("break_apart_formation_on_evade_dodge"))
        {
            data.break_apart_formation_on_evade_dodge = ini.get_value_bool(0);
        }
        else if (ini.is_value("break_formation_damage_trigger_percent"))
        {
            data.break_formation_damage_trigger_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("break_formation_damage_trigger_time"))
        {
            data.break_formation_damage_trigger_time = ini.get_value_float(0);
        }
        else if (ini.is_value("break_formation_missile_reaction_time"))
        {
            data.break_formation_missile_reaction_time = ini.get_value_float(0);
        }
        else if (ini.is_value("break_formation_on_buzz_head_toward_time"))
        {
            data.break_formation_on_buzz_head_toward_time = ini.get_value_float(0);
        }
        else if (ini.is_value("formation_exit_top_turn_break_away_throttle"))
        {
            data.formation_exit_top_turn_break_away_throttle = ini.get_value_float(0);
        }
        else if (ini.is_value("regroup_formation_on_buzz_head_toward"))
        {
            data.regroup_formation_on_buzz_head_toward = ini.get_value_bool(0);
        }
        else if (ini.is_value("regroup_formation_on_buzz_pass_by"))
        {
            data.regroup_formation_on_buzz_pass_by = ini.get_value_bool(0);
        }
        else if (ini.is_value("regroup_formation_on_evade_break"))
        {
            data.regroup_formation_on_evade_break = ini.get_value_bool(0);
        }
        else if (ini.is_value("regroup_formation_on_evade_dodge"))
        {
            data.regroup_formation_on_evade_dodge = ini.get_value_bool(0);
        }
        else if (ini.is_value("force_attack_formation_active_time"))
        {
            data.force_attack_formation_active_time = ini.get_value_float(0);
        }
        else if (ini.is_value("force_attack_formation_unactive_time"))
        {
            data.force_attack_formation_unactive_time = ini.get_value_float(0);
        }
        else if (ini.is_value("leader_makes_me_tougher"))
        {
            data.leader_makes_me_tougher = ini.get_value_bool(0);
        }
        else if (ini.is_value("formation_exit_max_time"))
        {
            data.formation_exit_max_time = ini.get_value_float(0);
        }
        else if (ini.is_value("formation_exit_mode"))
        {
            data.formation_exit_mode = ini.get_value_int(0);
        }
        else if (ini.is_value("formation_exit_roll_outrun_throttle"))
        {
            data.formation_exit_roll_outrun_throttle = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        formation[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::GetDifficulty(INI_Reader& ini, int& difficulty)
{
    const std::string str = StringUtils::ToLower(std::string(ini.get_value_string(0)));
    if (str == "easiest")
    {
        difficulty = 0;
    }
    else if (str == "easy")
    {
        difficulty = 1;
    }
    else if (str == "equal")
    {
        difficulty = 2;
    }
    else if (str == "hard")
    {
        difficulty = 3;
    }
    else if (str == "hardest")
    {
        difficulty = 4;
    }
    else
    {
        difficulty = 2;
    }
}

void PersonalityHelper::LoadJob(INI_Reader& ini)
{
    pub::AI::Personality::JobStruct data;
    std::string nick;

    // attack_preference = anything, 5000, GUNS | GUIDED | UNGUIDED

    // Missing Attack Order / Attack Sub target - more complex
    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("allow_player_targeting"))
        {
            data.allow_player_targeting = ini.get_value_bool(0);
        }
        else if (ini.is_value("force_attack_formation"))
        {
            data.force_attack_formation = ini.get_value_bool(0);
        }
        else if (ini.is_value("force_attack_formation_used"))
        {
            data.force_attack_formation_used = ini.get_value_bool(0);
        }
        else if (ini.is_value("combat_drift_distance"))
        {
            data.combat_drift_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("flee_scene_threat_style"))
        {
            GetDifficulty(ini, data.flee_scene_threat_style);
        }
        else if (ini.is_value("field_targeting"))
        {
            const std::string str = StringUtils::ToLower(std::string(ini.get_value_string()));
            if (str == "never")
            {
                data.field_targeting = 0;
            }
            else if (str == "low_density")
            {
                data.field_targeting = 1;
            }
            else if (str == "high_density")
            {
                data.field_targeting = 2;
            }
            else if (str == "always")
            {
                data.field_targeting = 3;
            }
        }
        else if (ini.is_value("flee_no_weapons_style"))
        {
            data.flee_no_weapons_style = ini.get_value_bool(0);
        }
        else if (ini.is_value("target_toughness_preference"))
        {
            GetDifficulty(ini, data.target_toughness_preference);
        }
        else if (ini.is_value("loot_flee_threshold"))
        {
            GetDifficulty(ini, data.loot_flee_threshold);
        }
        else if (ini.is_value("wait_for_leader_target"))
        {
            data.wait_for_leader_target = ini.get_value_bool(0);
        }
        else if (ini.is_value("flee_when_leader_flees_style"))
        {
            data.flee_when_leader_flees_style = ini.get_value_bool(0);
        }
        else if (ini.is_value("maximum_leader_target_distance"))
        {
            data.maximum_leader_target_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("flee_when_hull_damaged_percent"))
        {
            data.flee_when_hull_damaged_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("loot_preference"))
        {
            const std::string str = ini.get_value_string(0);
            if (str.find("LT_COMMODITIES"))
            {
                data.loot_preference |= 1;
            }
            if (str.find("LT_EQUIPMENT"))
            {
                data.loot_preference |= 2;
            }
            if (str.find("LT_POTIONS"))
            {
                data.loot_preference |= 4;
            }
            if (str.find("LT_AL"))
            {
                data.loot_preference = 7;
            }
            if (str.find("LT_NONE"))
            {
                data.loot_preference = 0;
            }
        }
        else if (ini.is_value("scene_toughness_threshold"))
        {
            GetDifficulty(ini, data.scene_toughness_threshold);
        }
    }

    if (!nick.empty())
    {
        job[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadMissile(INI_Reader& ini)
{
    pub::AI::Personality::MissileUseStruct data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        else if (ini.is_value("anti_cruise_missile_interval_time"))
        {
            data.anti_cruise_missile_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("anti_cruise_missile_max_distance"))
        {
            data.anti_cruise_missile_max_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("anti_cruise_missile_min_distance"))
        {
            data.anti_cruise_missile_min_distance = ini.get_value_float(0);
        }
        else if (ini.is_value("anti_cruise_missile_pre_fire_delay"))
        {
            data.anti_cruise_missile_pre_fire_delay = ini.get_value_float(0);
        }
        else if (ini.is_value("missile_launch_allow_out_of_range"))
        {
            data.missile_launch_allow_out_of_range = ini.get_value_bool(0);
        }
        else if (ini.is_value("missile_launch_cone_angle"))
        {
            data.missile_launch_cone_angle = ini.get_value_float(0);
        }
        else if (ini.is_value("missile_launch_interval_time"))
        {
            data.missile_launch_interval_time = ini.get_value_float(0);
        }
        else if (ini.is_value("missile_launch_interval_variance_percent"))
        {
            data.missile_launch_interval_variance_percent = ini.get_value_float(0);
        }
        else if (ini.is_value("missile_launch_range"))
        {
            data.missile_launch_range = ini.get_value_float(0);
        }
    }

    if (!nick.empty())
    {
        missile[StringUtils::stows(nick)] = data;
    }
}

void PersonalityHelper::LoadPilot(INI_Reader& ini)
{
    pub::AI::Personality data;
    std::string nick;

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            nick = ini.get_value_string();
        }
        if (ini.is_value("gun_id"))
        {
            if (const auto entity = gun.find(StringUtils::stows(ini.get_value_string())); entity != gun.end())
            {
                data.GunUse = entity->second;
            }
        }
        else if (ini.is_value("missile_id"))
        {
            if (const auto entity = missile.find(StringUtils::stows(ini.get_value_string())); entity != missile.end())
            {
                data.MissileUse = entity->second;
            }
        }
        else if (ini.is_value("evade_dodge_id"))
        {
            if (const auto entity = evadeDodge.find(StringUtils::stows(ini.get_value_string())); entity != evadeDodge.end())
            {
                data.EvadeDodgeUse = entity->second;
            }
        }
        else if (ini.is_value("evade_break_id"))
        {
            if (const auto entity = evadeBreak.find(StringUtils::stows(ini.get_value_string())); entity != evadeBreak.end())
            {
                data.EvadeBreakUse = entity->second;
            }
        }
        else if (ini.is_value("buzz_head_toward_id"))
        {
            if (const auto entity = buzzHead.find(StringUtils::stows(ini.get_value_string())); entity != buzzHead.end())
            {
                data.BuzzHeadTowardUse = entity->second;
            }
        }
        else if (ini.is_value("buzz_pass_by_id"))
        {
            if (const auto entity = buzzPass.find(StringUtils::stows(ini.get_value_string())); entity != buzzPass.end())
            {
                data.BuzzPassByUse = entity->second;
            }
        }
        else if (ini.is_value("trail_id"))
        {
            if (const auto entity = trail.find(StringUtils::stows(ini.get_value_string())); entity != trail.end())
            {
                data.TrailUse = entity->second;
            }
        }
        else if (ini.is_value("strafe_id"))
        {
            if (const auto entity = strafe.find(StringUtils::stows(ini.get_value_string())); entity != strafe.end())
            {
                data.StrafeUse = entity->second;
            }
        }
        else if (ini.is_value("engine_kill_id"))
        {
            if (const auto entity = engineKill.find(StringUtils::stows(ini.get_value_string())); entity != engineKill.end())
            {
                data.EngineKillUse = entity->second;
            }
        }
        else if (ini.is_value("mine_id"))
        {
            if (const auto entity = mine.find(StringUtils::stows(ini.get_value_string())); entity != mine.end())
            {
                data.MineUse = entity->second;
            }
        }
        else if (ini.is_value("countermeasure_id"))
        {
            if (const auto entity = cm.find(StringUtils::stows(ini.get_value_string())); entity != cm.end())
            {
                data.CountermeasureUse = entity->second;
            }
        }
        else if (ini.is_value("damage_reaction_id"))
        {
            if (const auto entity = damageReaction.find(StringUtils::stows(ini.get_value_string())); entity != damageReaction.end())
            {
                data.DamageReaction = entity->second;
            }
        }
        else if (ini.is_value("missile_reaction_id"))
        {
            if (const auto entity = missileReaction.find(StringUtils::stows(ini.get_value_string())); entity != missileReaction.end())
            {
                data.MissileReaction = entity->second;
            }
        }
        else if (ini.is_value("formation_id"))
        {
            if (const auto entity = formation.find(StringUtils::stows(ini.get_value_string())); entity != formation.end())
            {
                data.FormationUse = entity->second;
            }
        }
        else if (ini.is_value("repair_id"))
        {
            if (const auto entity = repair.find(StringUtils::stows(ini.get_value_string())); entity != repair.end())
            {
                data.RepairUse = entity->second;
            }
        }
        else if (ini.is_value("job_id"))
        {
            if (const auto entity = job.find(StringUtils::stows(ini.get_value_string())); entity != job.end())
            {
                data.Job = entity->second;
            }
        }
    }

    pilots[StringUtils::stows(nick)] = data;
}

PersonalityHelper::PersonalityHelper()
{
    INI_Reader ini;
    if (!ini.open(R"(..\DATA\MISSIONS\pilots_population.ini)", false))
    {
        WARN("Unable to parse pilot_population");
        return;
    }

    INFO("Parsing Pilot Population");

    while (ini.read_header())
    {
        if (ini.is_header("EvadeDodgeBlock"))
        {
            LoadEvadeDodge(ini);
        }
        else if (ini.is_header("EvadeBreakBlock"))
        {
            LoadEvadeBreak(ini);
        }
        else if (ini.is_header("BuzzHeadTowardBlock"))
        {
            LoadBuzzHead(ini);
        }
        else if (ini.is_header("BuzzPassByBlock"))
        {
            LoadBuzzPass(ini);
        }
        else if (ini.is_header("TrailBlock"))
        {
            LoadTrail(ini);
        }
        else if (ini.is_header("StrafeBlock"))
        {
            LoadStrafe(ini);
        }
        else if (ini.is_header("EngineKillBlock"))
        {
            LoadEngineKill(ini);
        }
        else if (ini.is_header("RepairBlock"))
        {
            LoadRepair(ini);
        }
        else if (ini.is_header("GunBlock"))
        {
            LoadGun(ini);
        }
        else if (ini.is_header("MineBlock"))
        {
            LoadMine(ini);
        }
        else if (ini.is_header("MissileBlock"))
        {
            LoadMissile(ini);
        }
        else if (ini.is_header("DamageReactionBlock"))
        {
            LoadDamageReaction(ini);
        }
        else if (ini.is_header("MissileReactionBlock"))
        {
            LoadMissileReaction(ini);
        }
        else if (ini.is_header("CountermeasureBlock"))
        {
            LoadCM(ini);
        }
        else if (ini.is_header("FormationBlock"))
        {
            LoadFormation(ini);
        }
        else if (ini.is_header("JobBlock"))
        {
            LoadJob(ini);
        }
        else if (ini.is_header("Pilot"))
        {
            LoadPilot(ini);
        }
    }
}

pub::AI::SetPersonalityParams PersonalityHelper::MakePersonality()
{
    pub::AI::SetPersonalityParams p;
    p.stateGraph = pub::StateGraph::get_state_graph("NOTHING", pub::StateGraph::TYPE_STANDARD);
    p.stateId = true;

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

    p.personality.GunUse.gun_fire_interval_time = 0.5f;
    p.personality.GunUse.gun_fire_interval_variance_percent = 0.0f;
    p.personality.GunUse.gun_fire_burst_interval_time = 0.5f;
    p.personality.GunUse.gun_fire_burst_interval_variance_percent = 0.0f;
    p.personality.GunUse.gun_fire_no_burst_interval_time = 0.0f;
    p.personality.GunUse.gun_fire_accuracy_cone_angle = 0.00001f;
    p.personality.GunUse.gun_fire_accuracy_power = 100.0f;
    p.personality.GunUse.gun_range_threshold = 1.0f;
    p.personality.GunUse.gun_target_point_switch_time = 0.0f;
    p.personality.GunUse.fire_style = 0;
    p.personality.GunUse.auto_turret_interval_time = 0.5f;
    p.personality.GunUse.auto_turret_burst_interval_time = 0.5f;
    p.personality.GunUse.auto_turret_no_burst_interval_time = 0.0f;
    p.personality.GunUse.auto_turret_burst_interval_variance_percent = 0.0f;
    p.personality.GunUse.gun_range_threshold_variance_percent = 0.3f;
    p.personality.GunUse.gun_fire_accuracy_power_npc = 100.0f;

    p.personality.MineUse.mine_launch_interval = 8.0f;
    p.personality.MineUse.mine_launch_cone_angle = 0.7854f;
    p.personality.MineUse.mine_launch_range = 200.0f;

    p.personality.MissileUse.missile_launch_interval_time = 0.5f;
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
    p.personality.DamageReaction.drop_mines_damage_trigger_time = 1.0f;
    p.personality.DamageReaction.fire_guns_damage_trigger_percent = 0.5f;
    p.personality.DamageReaction.fire_guns_damage_trigger_time = 0.5f;
    p.personality.DamageReaction.fire_missiles_damage_trigger_percent = 0.5f;
    p.personality.DamageReaction.fire_missiles_damage_trigger_time = 0.5f;

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
    p.personality.Job.loot_flee_threshold = 1;
    p.personality.Job.attack_subtarget_order[0] = 5;
    p.personality.Job.attack_subtarget_order[1] = 6;
    p.personality.Job.attack_subtarget_order[2] = 7;
    p.personality.Job.field_targeting = 3;
    p.personality.Job.loot_preference = 1;
    p.personality.Job.combat_drift_distance = 15000;
    p.personality.Job.attack_order[0].distance = 5000;
    p.personality.Job.attack_order[0].type = 11;
    p.personality.Job.attack_order[0].flag = 15;
    p.personality.Job.attack_order[1].type = 12;

    return p;
}
