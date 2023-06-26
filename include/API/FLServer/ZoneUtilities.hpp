#pragma once

namespace Hk::ZoneUtilities
{
	DLL void ReadUniverse(std::multimap<uint, LootableZone, std::less<>>* zones);
	DLL void ReadLootableZone(
	    std::multimap<uint, LootableZone, std::less<>>& zones, const std::string& systemNick, const std::string& defaultZoneNick, const std::string& file);
	DLL void ReadSystemLootableZones(std::multimap<uint, LootableZone, std::less<>>& zones, const std::string& systemNick, const std::string& file);
	DLL void ReadSystemZones(std::multimap<uint, LootableZone, std::less<>>& zones, const std::string& systemNick, const std::string& file);
	DLL bool InZone(uint systemId, const Vector& pos, Zone& rlz);
	DLL bool InDeathZone(uint systemId, const Vector& pos, Zone& rlz);
	DLL SystemInfo* GetSystemInfo(uint systemId);
	DLL void PrintZones();
}