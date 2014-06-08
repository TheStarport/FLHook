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


struct LOOTABLE_ZONE
{
	/** The zone nickname */
	string zoneNick;

	/** The id of the system for this lootable zone */
	uint systemID;

	/** The nickname and arch id of the loot dropped by the asteroids */
	string lootNick;
	uint iLootID;

	/** The arch id of the crate the loot is dropped in */
	uint iCrateID;

	/** The minimum number of loot items to drop */
	uint iMinLoot;

	/** The maximum number of loot items to drop */
	uint iMaxLoot;

	/** The drop difficultly */
	uint iLootDifficulty;

	/** The lootable zone ellipsoid size */
	Vector size;

	/** The lootable zone position */
	Vector pos;
};
typedef multimap<uint, LOOTABLE_ZONE, less<uint> >::value_type zone_map_pair_t;
typedef multimap<uint, LOOTABLE_ZONE, less<uint> >::iterator zone_map_iter_t;
typedef multimap<uint, LOOTABLE_ZONE, less<uint> > zone_map_t;


/**
Parse the specified ini file (usually in the data/solar/asteriods) and retrieve 
the lootable zone details. 
*/
void ReadLootableZone(zone_map_t &set_mmapZones, const string &systemNick, const string &defaultZoneNick, const string &file)
{
	string path="..\\data\\";
	path += file;

	INI_Reader ini;
	if (ini.open(path.c_str(), false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("LootableZone"))
			{
				string zoneNick=defaultZoneNick;
				string crateNick="";
				string lootNick="";
				int iMinLoot = 0;
				int iMaxLoot = 0;
				uint iLootDifficulty = 0;
				while (ini.read_value())
				{
					if (ini.is_value("zone"))
					{
						zoneNick=ToLower(ini.get_value_string());
					}
					else if (ini.is_value("dynamic_loot_container"))
					{
						crateNick=ToLower(ini.get_value_string());
					}
					else if (ini.is_value("dynamic_loot_commodity"))
					{
						lootNick=ToLower(ini.get_value_string());
					}
					else if (ini.is_value("dynamic_loot_count"))
					{
						iMinLoot=ini.get_value_int(0);
						iMaxLoot=ini.get_value_int(1);
					}
					else if (ini.is_value("dynamic_loot_difficulty"))
					{
						iLootDifficulty=ini.get_value_int(0);
					}
				}

				LOOTABLE_ZONE lz;
				lz.systemID = CreateID(systemNick.c_str());
				lz.zoneNick = zoneNick;
				lz.lootNick = lootNick;
				lz.iLootID = CreateID(lootNick.c_str());
				lz.iCrateID = CreateID(crateNick.c_str());
				lz.iMinLoot = iMinLoot;
				lz.iMaxLoot = iMaxLoot;
				lz.iLootDifficulty = iLootDifficulty;
				lz.pos.x = lz.pos.y = lz.pos.z = 0;
				lz.size.x = lz.size.y = lz.size.z = 0;

				bool exists = false;
				for (zone_map_iter_t i=set_mmapZones.begin(); i!=set_mmapZones.end(); i++)
				{
					if (i->second.zoneNick==zoneNick)
					{
						exists = true;
						break;
					}
				}
				if (!exists)
					set_mmapZones.insert(zone_map_pair_t(lz.systemID,lz));
			}
		}
		ini.close();
	}
}

/** Read the asteroid sections out of the system ini */
void ReadSystemLootableZones(zone_map_t &set_mmapZones, const string &systemNick, const string &file)
{
	string path="..\\data\\universe\\";
	path += file;

	INI_Reader ini;
	if (ini.open(path.c_str(), false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("Asteroids"))
			{
				string file="";
				string zoneNick="";
				while (ini.read_value())
				{
					if (ini.is_value("zone"))
						zoneNick=ToLower(ini.get_value_string());
					if (ini.is_value("file"))
						file=ini.get_value_string();
				}
				ReadLootableZone(set_mmapZones, systemNick,zoneNick, file);
			}
		}
		ini.close();
	}
}

/** Read the zone size/rotation and position information out of the
 specified file and calcuate the lootable zone transformation matrix */
void ReadSystemZones(zone_map_t &set_mmapZones, const string &systemNick, const string &file)
{
	string path="..\\data\\universe\\";
	path += file;

	INI_Reader ini;
	if (ini.open(path.c_str(), false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("zone"))
			{
				string zoneNick="";
				Vector size={0,0,0};
				Vector pos={0,0,0};
				Vector rotation={0,0,0};
				int idsName = 0;
				string idsNameTxt = "";
				int idsInfo = 0;
				string idsInfoTxt = "";

				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
					{
						zoneNick=ToLower(ini.get_value_string());
					}
					else if (ini.is_value("pos"))
					{
						pos.x = ini.get_value_float(0);
						pos.y = ini.get_value_float(1);
						pos.z = ini.get_value_float(2);
					}
					else if (ini.is_value("rotate"))
					{
						rotation.x = 180 + ini.get_value_float(0);
						rotation.y = 180 + ini.get_value_float(1);
						rotation.z = 180 + ini.get_value_float(2);
					}
					else if (ini.is_value("size"))
					{
						size.x = ini.get_value_float(0);
						size.y = ini.get_value_float(1);
						size.z = ini.get_value_float(2);
						if (size.y==0 || size.z==0)
						{
							size.y=size.x;
							size.z=size.x;
						}
					}
					else if (ini.is_value("ids_name"))
					{
						idsName = ini.get_value_int(0);
					}
					else if (ini.is_value("ids_info"))
					{
						idsInfo = ini.get_value_int(0);
					}
				}
				
				for (zone_map_iter_t i=set_mmapZones.begin(); i!=set_mmapZones.end(); i++)
				{
					if (i->second.zoneNick==zoneNick)
					{
						i->second.pos = pos;
						i->second.size = size;
						break;
					}
				}
			}
		}
		ini.close();
	}
}

/** Read all systems in the universe ini */
void ReadUniverse(zone_map_t &set_mmapZones)
{
	// Read all system ini files and build the lootable zone list.
	INI_Reader ini;
	if (ini.open("..\\data\\universe\\universe.ini", false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("System"))
			{
				string systemNick="";
				string file="";
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
						systemNick = ToLower(ini.get_value_string());
					if (ini.is_value("file"))
						file = ini.get_value_string();
				}
				ReadSystemLootableZones(set_mmapZones, systemNick,file);
			}
		}
		ini.close();
	}

	// Read all system ini files again this time extracting zone size/postion 
	// information for the lootable zone list.
	if (ini.open("..\\data\\universe\\universe.ini", false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("System"))
			{
				string systemNick="";
				string file="";
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
						systemNick = ini.get_value_string();
					if (ini.is_value("file"))
						file = ini.get_value_string();
				}									
				ReadSystemZones(set_mmapZones, systemNick,file);
			}
		}
		ini.close();
	}
}

void PrintZones()
{
	zone_map_t set_mmapZones;
	ReadUniverse(set_mmapZones);

	ConPrint(L"Zone, Commodity, MinLoot, MaxLoot, Difficultly, PosX, PosY, PosZ, SizeX, SizeY, SizeZ, IdsName, IdsInfo, Bonus\n");
	for (zone_map_iter_t i=set_mmapZones.begin(); i!=set_mmapZones.end(); i++)
	{
		ConPrint(L"%s, %s, %d, %d, %d, %0.0f, %0.0f, %0.0f, %0.0f, %0.0f, %0.0f, %d, %d, %2.2f\n", 
			stows(i->second.zoneNick).c_str(), stows(i->second.lootNick).c_str(),
			i->second.iMinLoot, i->second.iMaxLoot, i->second.iLootDifficulty,
			i->second.pos.x,i->second.pos.y,i->second.pos.z,
			i->second.size.x,i->second.size.y,i->second.size.z);
	}
	ConPrint(L"Zones=%d\n",set_mmapZones.size());
}