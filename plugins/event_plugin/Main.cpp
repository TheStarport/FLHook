/**
 Event Plugin for FLHook-Plugin
 by Cannon.

1.0:
*/

// includes 
#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <list>
#include <map>
#include <algorithm>
#include <FLHook.h>
#include <plugin.h>

static int set_iPluginDebug = 0;

struct CARGO_MISSION
{
	std::string nickname;
	uint base;
	uint item;
	int required_amount;

	int curr_amount;
};

struct NPC_MISSION
{
	std::string nickname;
	uint system;
	std::string sector;
	uint reputation;
	int required_amount;

	int curr_amount;
};

// Map of base ID to mission structure
std::multimap<UINT, CARGO_MISSION> set_mapCargoMissions;

// Map of repgroup ID to mission structure
std::multimap<UINT, NPC_MISSION> set_mapNpcMissions;

// A return code to indicate to FLHook if we want the hook processing to continue.
PLUGIN_RETURNCODE returncode;

std::string VectorToSectorCoord(uint iSystemID, Vector vPos)
{
	float scale = 1.0;
	const Universe::ISystem *iSystem = Universe::get_system(iSystemID);
	if (iSystem)
		scale = iSystem->NavMapScale;

	float fGridsize = 34000.0f / scale;
	int gridRefX = (int)((vPos.x + (fGridsize * 5)) / fGridsize) - 1;
	int gridRefZ = (int)((vPos.z + (fGridsize * 5)) / fGridsize) - 1;

	std::string scXPos = "X";
	if (gridRefX >= 0 && gridRefX < 8)
	{
		char* gridXLabel[] = {"A", "B", "C", "D", "E", "F", "G", "H"};
		scXPos = gridXLabel[gridRefX];
	}

	std::string scZPos = "X";
	if (gridRefZ >= 0 && gridRefZ < 8)
	{
		char* gridZLabel[] = {"1", "2", "3", "4", "5", "6", "7", "8"};
		scZPos = gridZLabel[gridRefZ];
	}

	char szCurrentLocation[100];
	_snprintf(szCurrentLocation, sizeof(szCurrentLocation), "%s-%s", scXPos.c_str(), scZPos.c_str());
	return szCurrentLocation;
}

void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\event.cfg";

	INI_Reader ini;
	set_mapCargoMissions.clear();
	set_mapNpcMissions.clear();
	if (ini.open(scPluginCfgFile.c_str(), false))
	{
		while (ini.read_header())
		{	
			if (ini.is_header("General"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("debug"))
					{ 
						set_iPluginDebug = ini.get_value_int(0);
					}
					else if (ini.is_value("cargo"))
					{
						CARGO_MISSION mis;
						mis.nickname = ini.get_value_string(0);
						mis.base = CreateID(ini.get_value_string(1));
						mis.item = CreateID(ini.get_value_string(2));
						mis.required_amount = ini.get_value_int(3);
						set_mapCargoMissions.insert(std::multimap<uint, CARGO_MISSION>::value_type(mis.base, mis));
					}
					else if (ini.is_value("npc"))
					{
						NPC_MISSION mis;
						mis.nickname = ini.get_value_string(0);
						mis.system = CreateID(ini.get_value_string(1));
						mis.sector = ini.get_value_string(2);
						pub::Reputation::GetReputationGroup(mis.reputation, ini.get_value_string(3));
						mis.required_amount = ini.get_value_int(4);
						set_mapNpcMissions.insert(std::multimap<uint, NPC_MISSION>::value_type(mis.reputation, mis));
					}
				}
			}
			ini.close();
		}

		if (set_iPluginDebug&1)
		{
			ConPrint(L"CargoMissionSettings loaded [%d]\n",set_mapCargoMissions.size());
			ConPrint(L"NpcMissionSettings loaded [%d]\n",set_mapNpcMissions.size());
		}
		ini.close();
	}
	
	// Read the last saved event status
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	std::string scStatsPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\event_stats.txt";	
	if (ini.open(scStatsPath.c_str(), false))
	{
		while (ini.read_header())
		{	
			if (ini.is_header("Missions"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("cargo"))
					{
						std::string nickname = ini.get_value_string(0);
						int curr_amount = ini.get_value_int(2);

						for (std::multimap<uint, CARGO_MISSION>::iterator i = set_mapCargoMissions.begin();
							i != set_mapCargoMissions.end(); ++i)
						{
							if (i->second.nickname == nickname)
							{
								i->second.curr_amount = curr_amount;
							}
						}
					}
					else if (ini.is_value("npc"))
					{
						NPC_MISSION mis;
						std::string nickname = ini.get_value_string(0);
						int curr_amount = ini.get_value_int(2);

						for (auto& i : set_mapNpcMissions)
						{
							if (i.second.nickname == nickname)
							{
								i.second.curr_amount = curr_amount;
							}
						}
					}
				}
			}
		}
		ini.close();
	}
}

// Save mission status every 100 seconds
void HkTimerCheckKick()
{
	if ((time(0) % 100) == 0)
	{
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		std::string scStatsPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\event_stats.txt";	

		FILE *file = fopen(scStatsPath.c_str(), "w");
		if (file)
		{
			fprintf(file, "[Missions]\n");
			for (std::multimap<uint, NPC_MISSION>::iterator i = set_mapNpcMissions.begin(); i != set_mapNpcMissions.end(); ++i)
			{
				fprintf(file, "npc = %s, %d, %d\n", i->second.nickname.c_str(), i->second.required_amount, i->second.curr_amount);
			}
			for (std::multimap<uint, CARGO_MISSION>::iterator i = set_mapCargoMissions.begin(); i != set_mapCargoMissions.end(); ++i)
			{
				fprintf(file, "cargo = %s, %d, %d\n", i->second.nickname.c_str(), i->second.required_amount, i->second.curr_amount);
			}
			fclose(file);
		}
	}
}

void __stdcall ShipDestroyed(DamageList *_dmg, DWORD *ecx, uint iKill)
{
	returncode = DEFAULT_RETURNCODE;

	if (iKill)
	{
		CShip *cship = (CShip*)ecx[4];

		int iRep;
		pub::SpaceObj::GetRep(cship->get_id(), iRep);
		
		uint iAff;
		pub::Reputation::GetAffiliation(iRep, iAff);

		uint iSystem;
		pub::SpaceObj::GetSystem(cship->get_id(), iSystem);

		Vector vPos = cship->get_position();
		std::string scSector = VectorToSectorCoord(iSystem, vPos);

		auto start = set_mapNpcMissions.lower_bound(iAff);
		auto end = set_mapNpcMissions.upper_bound(iAff);
		for (; start != end; ++start)
		{
			if (start->second.system == iSystem)
			{
				if (start->second.sector.length() && start->second.sector != scSector)
					continue;

				if (start->second.curr_amount < start->second.required_amount)
				{
					start->second.curr_amount++;
					// PrintUserCmdText(iClientID, L"%d ships remaining to destroy to complete mission objective", needed);
				}
			}
		}
	}
}

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID)
{
	uint iBase;
	pub::Player::GetBase(iClientID, iBase);

	auto start = set_mapCargoMissions.lower_bound(iBase);
	auto end = set_mapCargoMissions.upper_bound(iBase);
	for (; start != end; ++start)
	{
		if (start->second.item == gbi.iGoodID)
		{
			start->second.curr_amount -= gbi.iCount;
			if (start->second.curr_amount < 0)
			{
				start->second.curr_amount = 0;
			}
		}
	}
}

void __stdcall GFGoodSell(const struct SGFGoodSellInfo &gsi, unsigned int iClientID)
{
	returncode = DEFAULT_RETURNCODE;

	uint iBase;
	pub::Player::GetBase(iClientID, iBase);

	auto start = set_mapCargoMissions.lower_bound(iBase);
	auto end = set_mapCargoMissions.upper_bound(iBase);
	for (; start != end; ++start)
	{
		if (start->second.item == gsi.iArchID)
		{
			if (start->second.curr_amount < start->second.required_amount)
			{
				int needed = start->second.required_amount - start->second.curr_amount;
				if (needed > gsi.iCount)
				{
					start->second.curr_amount += gsi.iCount;
					needed = start->second.required_amount - start->second.curr_amount;
					PrintUserCmdText(iClientID, L"%d units remaining to complete mission objective", needed);
				}
				else
				{
					PrintUserCmdText(iClientID, L"Mission objective completed",needed);
				}
			}
		}
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));
	
	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
	if(fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length()>0)
		LoadSettings();

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Functions to hook */
EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "Event Plugin by cannon";
	p_PI->sShortName = "event";
	p_PI->bMayPause = true;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkTimerCheckKick, PLUGIN_HkTimerCheckKick, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&ShipDestroyed, PLUGIN_ShipDestroyed, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&GFGoodBuy, PLUGIN_HkIServerImpl_GFGoodBuy, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&GFGoodSell, PLUGIN_HkIServerImpl_GFGoodSell, 0));
	return p_PI;
}
