#include "Main.h"

const char* MODULE_TYPE_NICKNAMES[] = {
	"module_build",     "module_coreupgrade", "module_shieldgen",    "module_storage",
	"module_defense_1", "module_m_docking",   "module_m_jumpdrives", "module_m_hyperspace_scanner",
	"module_m_cloak",   "module_defense_2",   "module_defense_3",    0
};

BuildModule::BuildModule(PlayerBase* the_base) : Module(TYPE_BUILD), base(the_base), build_type(0)
{
}

// Find the recipe for this building_type and start construction.
BuildModule::BuildModule(PlayerBase* the_base, uint the_build_type)
    : Module(TYPE_BUILD), base(the_base), build_type(the_build_type)
{
	uint module_nickname = CreateID(MODULE_TYPE_NICKNAMES[build_type]);
	active_recipe = recipes[module_nickname];
}

std::wstring BuildModule::GetInfo(bool xml)
{
	std::wstring info;
	if (xml)
	{
		info = L"<TEXT>Constructing " + active_recipe.infotext + L". Waiting for:</TEXT>";

		for (std::map<uint, uint>::iterator i = active_recipe.consumed_items.begin();
		     i != active_recipe.consumed_items.end(); ++i)
		{
			uint good = i->first;
			uint quantity = i->second;

			const GoodInfo* gi = GoodList::find_by_id(good);
			if (gi)
			{
				info +=
				    L"<PARA/><TEXT>      - " + std::to_wstring(quantity) + L"x " + HkGetWStringFromIDS(gi->iIDSName);
				if (base->HasMarketItem(good) < quantity)
					info += L" [Out of stock]";
				info += L"</TEXT>";
			}
		}
	}
	else
	{
		info = L"Constructing " + active_recipe.infotext + L". Waiting for: ";

		for (std::map<uint, uint>::iterator i = active_recipe.consumed_items.begin();
		     i != active_recipe.consumed_items.end(); ++i)
		{
			uint good = i->first;
			uint quantity = i->second;

			const GoodInfo* gi = GoodList::find_by_id(good);
			if (gi)
			{
				info += std::to_wstring(quantity) + L"x" + HkGetWStringFromIDS(gi->iIDSName) + L" ";
			}
		}
	}

	return info;
}

// Every 10 seconds we consume goods for the active recipe at the cooking rate
// and if every consumed item has been used then declare the the cooking
// complete and convert this module into the specified type.
bool BuildModule::Timer(uint time)
{
	if ((time % set_tick_time) != 0)
		return false;

	bool cooked = true;

	// Consume goods at the cooking rate.
	for (std::map<uint, uint>::iterator i = active_recipe.consumed_items.begin();
	     i != active_recipe.consumed_items.end(); ++i)
	{
		uint good = i->first;
		uint quantity = i->second > active_recipe.cooking_rate ? active_recipe.cooking_rate : i->second;
		if (quantity)
		{
			cooked = false;
			std::map<uint, MARKET_ITEM>::iterator market_item = base->market_items.find(good);
			if (market_item != base->market_items.end())
			{
				if (market_item->second.quantity >= quantity)
				{
					i->second -= quantity;
					base->RemoveMarketGood(good, quantity);
					return false;
				}
			}
		}
	}

	// Once cooked turn this into the build type
	if (cooked)
	{
		for (uint i = 0; i < base->modules.size(); i++)
		{
			if (base->modules[i] == this)
			{
				switch (build_type)
				{
					case Module::TYPE_CORE:
						base->base_level++;
						if (base->base_level > 4)
							base->base_level = 4;
						base->SetupDefaults();

						// Clear the build module slot.
						base->modules[i] = 0;

						// Delete and respawn the old core module
						delete base->modules[0];
						base->modules[0] = new CoreModule(base);
						base->modules[0]->Spawn();

						break;
					case Module::TYPE_SHIELDGEN:
						base->modules[i] = new ShieldModule(base);
						break;
					case Module::TYPE_STORAGE:
						base->modules[i] = new StorageModule(base);
						break;
					case Module::TYPE_DEFENSE_1:
						base->modules[i] = new DefenseModule(base, Module::TYPE_DEFENSE_1);
						break;
					case Module::TYPE_M_DOCKING:
						base->modules[i] = new FactoryModule(base, Module::TYPE_M_DOCKING);
						break;
					case Module::TYPE_M_JUMPDRIVES:
						base->modules[i] = new FactoryModule(base, Module::TYPE_M_JUMPDRIVES);
						break;
					case Module::TYPE_M_HYPERSPACE_SCANNER:
						base->modules[i] = new FactoryModule(base, Module::TYPE_M_HYPERSPACE_SCANNER);
						break;
					case Module::TYPE_M_CLOAK:
						base->modules[i] = new FactoryModule(base, Module::TYPE_M_CLOAK);
						break;
					case Module::TYPE_DEFENSE_2:
						base->modules[i] = new DefenseModule(base, Module::TYPE_DEFENSE_2);
						break;
					case Module::TYPE_DEFENSE_3:
						base->modules[i] = new DefenseModule(base, Module::TYPE_DEFENSE_3);
						break;
					default:
						base->modules[i] = 0;
						break;
				}
				base->Save();
				delete this;
				return false;
			}
		}
	}

	return false;
}

void BuildModule::LoadState(INI_Reader& ini)
{
	while (ini.read_value())
	{
		if (ini.is_value("build_type"))
		{
			build_type = ini.get_value_int(0);
		}
		else if (ini.is_value("produced_item"))
		{
			active_recipe.produced_item = ini.get_value_int(0);
		}
		else if (ini.is_value("cooking_rate"))
		{
			active_recipe.cooking_rate = ini.get_value_int(0);
		}
		else if (ini.is_value("infotext"))
		{
			active_recipe.infotext = stows(ini.get_value_string());
		}
		else if (ini.is_value("consumed"))
		{
			active_recipe.consumed_items[ini.get_value_int(0)] = ini.get_value_int(1);
		}
	}
}

void BuildModule::SaveState(FILE* file)
{
	fprintf(file, "[BuildModule]\n");
	fprintf(file, "build_type = %u\n", build_type);
	fprintf(file, "produced_item = %u\n", active_recipe.produced_item);
	fprintf(file, "cooking_rate = %u\n", active_recipe.cooking_rate);
	fprintf(file, "infotext = %s\n", wstos(active_recipe.infotext).c_str());
	for (std::map<uint, uint>::iterator i = active_recipe.consumed_items.begin();
	     i != active_recipe.consumed_items.end(); ++i)
	{
		fprintf(file, "consumed = %u, %u\n", i->first, i->second);
	}
}
