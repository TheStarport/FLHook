#ifndef __MAIN_H__
#define __MAIN_H__ 1

#include <FLHook.h>
#include <plugin.h>

struct CLIENT_DATA
{
	CLIENT_DATA()
	    : bSetup(false), reverse_sell(false), stop_buy(false), bAdmin(false), iDockingModules(0), mobile_docked(false)
	{
	}

	bool bSetup;

	bool reverse_sell;
	bool stop_buy;
	std::list<CARGO_INFO> cargo;

	bool bAdmin;

	uint iDockingModules;
	std::map<std::wstring, std::wstring> mapDockedShips;

	// True if currently docked on a carrier.
	bool mobile_docked;

	// The name of the carrier.
	std::wstring wscDockedWithCharname;

	// The last known location in space of the carrier
	uint iCarrierSystem;
	Vector vCarrierLocation;
	Matrix mCarrierLocation;

	// The last real base this ship was on.
	uint iLastBaseID;
};

extern std::map<uint, CLIENT_DATA> clients;

#endif
