#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <list>
#include <map>
#include <algorithm>
#include "./headers/FLHook.h"
#include "./headers/plugin.h"
#include "PluginUtilities.h"
#include <math.h>
#include "Main.h"

CoreModule::CoreModule(PlayerBase *the_base) : Module(TYPE_CORE), base(the_base), space_obj(0)
{
}

CoreModule::~CoreModule()
{
	if (space_obj)
	{
		pub::SpaceObj::Destroy(space_obj, (pub::SpaceObj::DestroyType)0); // use type 1 for fuses
		spaceobj_modules.erase(space_obj);
		space_obj = 0;
	}
}

void CoreModule::Spawn()
{
	if (!space_obj)
	{
		pub::SpaceObj::SolarInfo si;
		memset(&si, 0, sizeof(si));
		si.iFlag = 4;

		char archname[100];
		_snprintf(archname, sizeof(archname), "dsy_playerbase_%02u", base->base_level);
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
		strncpy_s(si.cNickName, sizeof(si.cNickName), base->nickname.c_str(), base->nickname.size());

		// Check to see if the hook IDS limit has been reached
		static uint solar_ids = 526000;
		if (++solar_ids > 526999)
		{
			solar_ids = 0;
			return;
		}
		
		// Send the base name to all players that are online
		base->solar_ids = solar_ids;

		wstring basename = base->basename;
		if (base->affiliation)
		{
			basename = HkGetWStringFromIDS(Reputation::get_name(base->affiliation)) + L" - " + base->basename;
		}

		struct PlayerData *pd = 0;
		while (pd = Players.traverse_active(pd))
		{
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

		pub::SpaceObj::CreateSolar(space_obj, si);
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
		pub::SpaceObj::SetRelativeHealth(space_obj, base->base_health/base->max_base_health);

		base->SyncReputationForBaseObject(space_obj);
		if (set_plugin_debug>1)
			ConPrint(L"CoreModule::created space_obj=%u health=%f\n", space_obj, base->base_health);
	}
}

wstring CoreModule::GetInfo(bool xml)
{
	return L"Core";
}

void CoreModule::LoadState(INI_Reader &ini)
{
	while (ini.read_value())
	{
	}
}

void CoreModule::SaveState(FILE *file)
{
	fprintf(file, "[CoreModule]\n");
}

void CoreModule::RepairDamage(float max_base_health)
{
	// The bigger the base the more damage can be repaired.
	for (uint repair_cycles = 0; repair_cycles < base->base_level; ++repair_cycles)
	{
		foreach (set_base_repair_items, REPAIR_ITEM, item)
		{
			if (base->base_health >= max_base_health)
				return;

			if (base->HasMarketItem(item->good) >= item->quantity)
			{
				base->RemoveMarketGood(item->good, item->quantity);
				base->base_health += 60000;
				base->repairing = true;
			}
		}
	}
}

bool CoreModule::Timer(uint time)
{
	if ((time%set_tick_time) != 0)
		return false;

	if (space_obj)
	{
		pub::SpaceObj::GetHealth(space_obj, base->base_health, base->max_base_health);

		// Reduce hitpoints to reflect wear and tear. This will eventually
		// destroy the base unless it is able to repair itself.
		base->base_health -= set_damage_per_tick + (set_damage_per_tick * base->base_level);

		// Repair damage if we have sufficient crew on the base.
		base->repairing = false;
		uint number_of_crew = base->HasMarketItem(set_base_crew_type);
		if (number_of_crew >= (base->base_level * 200))
			RepairDamage(base->max_base_health);

		if (base->base_health > base->max_base_health)
			base->base_health = base->max_base_health;
		else if (base->base_health <= 0)
			base->base_health = 0;

		// Humans use commodity_oxygen, commodity_water. Consume these for
		// the crew or kill 10 crew off and repeat this every 8 hours.
		if (time%28800 == 0)
		{
			for (map<uint, uint>::iterator i = set_base_crew_consumption_items.begin();
				i != set_base_crew_consumption_items.end(); ++i)
			{
				// Use water and oxygen.
				if (base->HasMarketItem(i->first) >= number_of_crew)
				{
					base->RemoveMarketGood(i->first, number_of_crew);
				}
				// Insufficient water and oxygen, kill crew.
				else
				{
					base->RemoveMarketGood(set_base_crew_type, (number_of_crew >= 10) ? 10 : number_of_crew);
				}
			}

			// Humans use food but may eat one of a number of types.
			uint crew_to_feed = number_of_crew;
			for (map<uint, uint>::iterator i = set_base_crew_food_items.begin();
				i != set_base_crew_food_items.end(); ++i)
			{
				if (!crew_to_feed)
					break;

				uint food_available = base->HasMarketItem(i->first);
				if (food_available)
				{
					uint food_to_use = (food_available >= crew_to_feed) ? crew_to_feed : food_available;
					base->RemoveMarketGood(i->first, food_to_use);
					crew_to_feed -= food_to_use;
				}
			}

			// Insufficent food so kill crew.
			if (crew_to_feed)
			{
				base->RemoveMarketGood(set_base_crew_type, (crew_to_feed >= 10) ? 10 : crew_to_feed);
			}
		}

		// Save the new base health
		float rhealth = base->base_health/base->max_base_health;
		pub::SpaceObj::SetRelativeHealth(space_obj, rhealth);
		if (set_plugin_debug>1)
			ConPrint(L"CoreModule::timer space_obj=%u health=%f\n", space_obj, base->base_health);

		// if health is 0 then the object will be destroyed but we won't
		// receive a notification of this so emulate it.
		if (base->base_health < 1)
			return SpaceObjDestroyed(space_obj);
	}

	return false;
}

float CoreModule::SpaceObjDamaged(uint space_obj, uint attacking_space_obj, float curr_hitpoints, float damage)
{
	base->SpaceObjDamaged(space_obj, attacking_space_obj, curr_hitpoints, damage);
	
	// Reduce the damage to 10% if the shield is or will be online.
	if (base->shield_state != PlayerBase::SHIELD_STATE_OFFLINE)
	{
		return curr_hitpoints - ((curr_hitpoints - damage) * set_shield_damage_multiplier);
	}

	return 0.0f;
}


bool CoreModule::SpaceObjDestroyed(uint space_obj)
{
	if (this->space_obj == space_obj)
	{
		if (set_plugin_debug>1)
			ConPrint(L"CoreModule::destroyed space_obj=%u\n", space_obj);
		pub::SpaceObj::LightFuse(space_obj, "player_base_explode_fuse", 0);
		spaceobj_modules.erase(space_obj);
		this->space_obj = 0;

		struct PlayerData *pd = 0;
		while (pd = Players.traverse_active(pd))
		{
			PrintUserCmdText(pd->iOnlineID, L"Base %s destroyed", base->basename.c_str());
		}

		// Unspawn, delete base and save file.
		DeleteBase(base);
		
		// Careful not to access this as this object will have been deleted by now.
		return true;
	}
	return false;
}

void CoreModule::SetReputation(int player_rep, float attitude)
{
	if (space_obj)
	{
		int obj_rep;
		pub::SpaceObj::GetRep(this->space_obj, obj_rep);
		if (set_plugin_debug>1)
			ConPrint(L"CoreModule::SetReputation player_rep=%u obj_rep=%u attitude=%f base=%08x\n",
				player_rep, obj_rep, attitude, base->base);
		pub::Reputation::SetAttitude(obj_rep, player_rep, attitude);
	}
}
