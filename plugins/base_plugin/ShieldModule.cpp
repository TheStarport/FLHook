#include "Main.h"

static uint shield_fuse = 0;

ShieldModule::ShieldModule(PlayerBase* the_base) : Module(TYPE_SHIELDGEN), base(the_base), reset_needed(false)
{
	shield_fuse = CreateID("player_base_shield");
}

ShieldModule::~ShieldModule()
{
	base->shield_state = PlayerBase::SHIELD_STATE_OFFLINE;
}

std::wstring ShieldModule::GetInfo(bool xml)
{
	std::wstring info = L"Shield Generator Module Type 1 - ";
	switch (base->shield_state)
	{
		case PlayerBase::SHIELD_STATE_ACTIVE:
			info += L"Online - Active";
			break;
		case PlayerBase::SHIELD_STATE_ONLINE:
			info += L"Online - Not Active";
			break;
		case PlayerBase::SHIELD_STATE_OFFLINE:
			info += L"Offline";
			break;
	}
	return info;
}

// Load module state from ini file.
void ShieldModule::LoadState(INI_Reader& ini)
{
	while (ini.read_value())
	{
	}
}

// Append module state to the ini file.
void ShieldModule::SaveState(FILE* file)
{
	fprintf(file, "[ShieldModule]\n");
}

bool ShieldModule::HasShieldPower()
{
	for (std::map<uint, uint>::iterator i = shield_power_items.begin(); i != shield_power_items.end(); ++i)
	{
		uint good = i->first;
		uint quantity_consumed = i->second;
		if (base->HasMarketItem(good) >= quantity_consumed)
		{
			return true;
		}
	}
	return false;
}

bool ShieldModule::Timer(uint time)
{
	// Check to see if we have fuel for the shield
	int old_shield_state = base->shield_state;

	base->shield_active_time--;
	if (base->shield_active_time <= 0)
		base->shield_active_time = 0;

	base->shield_state = PlayerBase::SHIELD_STATE_OFFLINE;
	if (HasShieldPower())
	{
		// If there's a pending request for the shield then activate it
		if (base->shield_active_time)
		{
			base->shield_state = PlayerBase::SHIELD_STATE_ACTIVE;
		}
		// Otherwise turn off the shield.
		else
		{
			base->shield_state = PlayerBase::SHIELD_STATE_ONLINE;
		}
	}

	// If the shield is active then use fuel every 10 seconds
	if (base->shield_state == PlayerBase::SHIELD_STATE_ACTIVE && (time % set_tick_time) == 0)
	{
		for (std::map<uint, uint>::iterator i = shield_power_items.begin(); i != shield_power_items.end(); ++i)
		{
			uint good = i->first;
			uint quantity_consumed = i->second;
			if (base->HasMarketItem(good) >= quantity_consumed)
			{
				base->RemoveMarketGood(good, quantity_consumed);
				break;
			}
		}
	}

	// Reset fuse on change state or if player undocks/enters system.
	if (base->shield_state != old_shield_state || reset_needed)
	{
		reset_needed = false;
		CoreModule* core = (CoreModule*)base->modules[0];
		if (core && core->space_obj)
		{
			uint dunno;
			IObjInspectImpl* inspect;
			if (GetShipInspect(core->space_obj, inspect, dunno))
			{
				HkUnLightFuse((IObjRW*)inspect, shield_fuse);
				if (base->shield_state == PlayerBase::SHIELD_STATE_ACTIVE)
					HkLightFuse((IObjRW*)inspect, shield_fuse, 0.0f, 0.0f, 0.0f);
			}
		}
	}

	return false;
}

void ShieldModule::SetReputation(int player_rep, float attitude)
{
	// If a ship enters the system or undocks then reset the fuse. SetReputation
	// gets called when this happens so we use this.
	reset_needed = true;
}
