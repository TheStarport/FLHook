﻿#include "Global.hpp"

bool LoadBaseMarket()
{
	INI_Reader ini;

	if (!ini.open("..\\data\\equipment\\market_misc.ini", false))
		return false;

	while (ini.read_header())
	{
		if (!ini.is_header("BaseGood"))
			continue;
		if (!ini.read_value())
			continue;
		if (!ini.is_value("base"))
			continue;

		const char* BaseName = ini.get_value_string();
		BaseInfo* biBase = 0;
		for (auto& base : CoreGlobals::i()->allBases)
		{
			const char* BN = base.scBasename.c_str();
			if (!ToLower(base.scBasename).compare(ToLower(BaseName)))
			{
				biBase = &base;
				break;
			}
		}

		if (!biBase)
			continue; // base not found

		ini.read_value();

		biBase->MarketMisc.clear();
		if (!ini.is_value("MarketGood"))
			continue;

		do
		{
			DataMarketItem mi;
			const char* EquipName = ini.get_value_string(0);
			mi.iArchId = CreateID(EquipName);
			mi.fRep = ini.get_value_float(2);
			biBase->MarketMisc.push_back(mi);
		} while (ini.read_value());
	}

	ini.close();
	return true;
}
