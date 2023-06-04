#pragma once

//struct definitions for objects defined in weapon_equip.ini
namespace WeaponEquip
{
	struct Gun
	{
		std::wstring nickname;
		int ids_name;
		int ids_info;

		std::wstring DA_archetype;
		std::wstring material_library;

		std::wstring Hp_child;

		float hit_pts;
		float explosion_resistance;

		std::wstring debris_type;

		float parent_impulse;
		float child_impulse;

		float volume;
		float mass;

		std::wstring hp_gun_type;

		int damage_per_fire;
		int power_usage;
		float refire_delay;
		float muzzle_velocity;

		std::wstring use_animation;

		float toughness;
		std::wstring flash_particle_name;
		float flash_radius;
		std::wstring light_anim;

		std::wstring projectile_archetype;
		std::wstring seperation_exploson;

		bool auto_turret;
		float turn_rate;

		bool lootable;
		std::vector<float> LODranges;
	};

	struct Munition
	{
		std::wstring nickname;
		std::wstring hp_type;

		bool requires_ammo;
		float hit_pts;

		float hull_damage;
		int energy_damage;

		std::wstring weapon_type;
		std::wstring one_shot_sound;
		std::wstring munition_hit_effect;
		std::wstring const_effect;

		float lifetime;
		bool force_gun_ori;
		float mass;
		float volume;
	};
}