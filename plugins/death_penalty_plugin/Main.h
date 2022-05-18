#pragma once

#include <FLHook.h>
#include <plugin.h>

struct CLIENT_DATA
{
	bool bDisplayDPOnLaunch = true;
	int DeathPenaltyCredits = 0;
};

float set_fDeathPenalty = 0;
float set_fDeathPenaltyKiller = 0;
std::list<uint> ExcludedSystems;
std::map<uint, CLIENT_DATA> MapClients;
std::map<uint, float> FractionOverridesbyShip;

ReturnCode returncode = ReturnCode::Default;
