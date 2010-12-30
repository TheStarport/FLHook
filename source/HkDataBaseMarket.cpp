#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkLoadBaseMarket()
{
	INI_Reader ini;

	if(!ini.open("..\\data\\equipment\\market_misc.ini", false))
		return false;

	while(ini.read_header())
	{
		try {
			if(!ini.is_header("BaseGood"))
				throw "";
			if(!ini.read_value())
				throw "";
			if(!ini.is_value("base"))
				throw "";

			const char *szBaseName = ini.get_value_string();
			BASE_INFO *biBase = 0;
			foreach(lstBases, BASE_INFO, it)
			{
				const char *szBN = it->scBasename.c_str();
				if(!ToLower(it->scBasename).compare(ToLower(szBaseName)))
				{
					biBase = &(*it);
					break;
				}
			}

			if(!biBase)
				throw ""; // base not found

			ini.read_value();

			biBase->lstMarketMisc.clear();
			if(!ini.is_value("MarketGood"))
				throw "";

			do {
				DATA_MARKETITEM mi;
				const char *szEquipName = ini.get_value_string(0);
				mi.iArchID = CreateID(szEquipName);
				mi.fRep = ini.get_value_float(2);
				biBase->lstMarketMisc.push_back(mi);
			} while(ini.read_value());

		} catch(char*) {}
	}

	ini.close();
	return true;
}