#pragma once

// Struct Definitions for objects defined in shiparch.ini
namespace ShipArch
{
	struct Ship
	{
		int ids_name;
		int ids_info1;
		int ids_info2;
		int ids_info3;

		int ship_class;
		std::wstring nick_name;

		std::vector<int> LODranges;

		std::wstring msg_id_prefix;
		std::wstring mission_property;
		std::wstring TYPE;

		float mass;
		int hold_size;
		float linear_drag;

		// TODO: Decide on if we want fuses or not;

		float max_bank_angle;
		float camera_offset[2];
		float camera_horizontal_turng_angle;
		float camera_verticle_turn_down_angle;
		float camera_look_ahead_slerp_amount;

		int nanobot_limit;
		int shield_battery_limit;
		float hit_points;

		std::wstring DA_archetype;

		// TODO: Ditto for material library;

		std::wstring envmap_material;
		std::wstring cockpit;
		std::wstring pilot_mesh;
		std::wstring explosion_arch;

		// Surface hit effects as well..

		float steering_torque[3];
		float angular_drag[3];
		float rotation_inertia[3];

		float nudge_force;
		float strafe_force;
		float strafe_power_usage;

		std::wstring HpTractor_Source;
		std::wstring shield_link;

		// TODO: hp_type struct.
		std::vector<int> hp_type;
	};

	struct CollisionGroup
	{
		std::wstring obj;

		float parent_impulse;
		float child_impulse;

		std::wstring debris_type;
		std::wstring dmg_hp;
		std::wstring dmg_obj;

		std::wstring separation_explosion;
		float mass;
		std::wstring type;
		float hit_pts;
		bool root_health_proxy;
	};

	struct Simple
	{
		std::wstring nickname;
		std::wstring DA_archetype;
		std::wstring material_library;

		float mass;

		std::vector<float> LODranges;
	};
} // namespace ShipArch