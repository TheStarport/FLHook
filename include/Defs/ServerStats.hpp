#pragma once

struct ServerStats : Reflectable
{
	struct Player : Reflectable
	{
		std::wstring playerName;
		std::wstring systemName;
		std::wstring systemNick;
		std::wstring ipAddress;
		uint clientId;
	};

	std::vector<Player> players;
	uint serverLoad;
	uint memoryUsage;
	bool npcsEnabled;
};

REFL_AUTO(type(ServerStats::Player), field(playerName), field(systemName), field(systemNick), field(ipAddress));
REFL_AUTO(type(ServerStats), field(players), field(serverLoad), field(memoryUsage));