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
		std::string nick_name;

		std::vector<int> LODranges;

		std::string msg_id_prefix;
		std::string mission_property;
		std::string TYPE;

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

		std::string DA_archetype;

		// TODO: Ditto for material library;

		std::string envmap_material;
		std::string cockpit;
		std::string pilot_mesh;
		std::string explosion_arch;

		// Surface hit effects as well..

		float steering_torque[3];
		float angular_drag[3];
		float rotation_inertia[3];

		float nudge_force;
		float strafe_force;
		float strafe_power_usage;

		std::string HpTractor_Source;
		std::string shield_link;

		// TODO: hp_type struct.
		std::vector<int> hp_type;
	};

	struct CollisionGroup
	{
		std::string obj;

		float parent_impulse;
		float child_impulse;

		std::string debris_type;
		std::string dmg_hp;
		std::string dmg_obj;

		std::string separation_explosion;
		float mass;
		std::string type;
		float hit_pts;
		bool root_health_proxy;
	};

	struct Simple
	{
		std::string nickname;
		std::string DA_archetype;
		std::string material_library;

		float mass;

		std::vector<float> LODranges;
	};
} // namespace ShipArch