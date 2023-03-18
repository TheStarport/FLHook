#pragma once

//struct definitions for objects defined in weapon_equip.ini
namespace WeaponEquip
{
	struct Gun
	{
		std::string nickname;
		int ids_name;
		int ids_info;

		std::string DA_archetype;
		std::string material_library;

		std::string Hp_child;

		float hit_pts;
		float explosion_resistance;

		std::string debris_type;

		float parent_impulse;
		float child_impulse;

		float volume;
		float mass;

		std::string hp_gun_type;

		int damage_per_fire;
		int power_usage;
		float refire_delay;
		float muzzle_velocity;

		std::string use_animation;

		float toughness;
		std::string flash_particle_name;
		float flash_radius;
		std::string light_anim;

		std::string projectile_archetype;
		std::string seperation_exploson;

		bool auto_turret;
		float turn_rate;

		bool lootable;
		std::vector<float> LODranges;
	};

	struct Munition
	{
		std::string nickname;
		std::string hp_type;

		bool requires_ammo;
		float hit_pts;

		float hull_damage;
		int energy_damage;

		std::string weapon_type;
		std::string one_shot_sound;
		std::string munition_hit_effect;
		std::string const_effect;

		float lifetime;
		bool force_gun_ori;
		float mass;
		float volume;
	};
}