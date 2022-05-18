#include "Main.h"

void LoadDockInfo(uint client)
{
	CLIENT_DATA& cd = clients[client];

	// How many docking modules do we have?
	cd.iDockingModules = 0;
	cd.mapDockedShips.clear();

	for (auto item = Players[client].equipDescList.equip.begin(); item != Players[client].equipDescList.equip.end();
	     item++)
	{
		if (item->bMounted && item->iArchID == 0xB85AB480)
		{
			cd.iDockingModules++;
		}
	}

	// Load docked ships until we run out of docking module space.
	uint count = HkGetCharacterIniUint(client, L"dock.docked_ships_count");
	for (uint i = 1; i <= count && cd.mapDockedShips.size() <= cd.iDockingModules; i++)
	{
		std::wstring key = L"dock.docked_ship." + std::to_wstring(i);
		std::wstring charname = HkGetCharacterIniString(client, key);
		if (charname.length())
		{
			if (HkGetAccountByCharname(charname))
			{
				cd.mapDockedShips[charname] = charname;
			}
		}
	}

	cd.wscDockedWithCharname = HkGetCharacterIniString(client, L"dock.docked_with_charname");
	if (cd.wscDockedWithCharname.length())
		cd.mobile_docked = true;

	cd.iLastBaseID = HkGetCharacterIniUint(client, L"dock.last_base");
	cd.iCarrierSystem = HkGetCharacterIniUint(client, L"dock.carrier_system");
	cd.vCarrierLocation.x = HkGetCharacterIniFloat(client, L"dock.carrier_pos.x");
	cd.vCarrierLocation.y = HkGetCharacterIniFloat(client, L"dock.carrier_pos.y");
	cd.vCarrierLocation.z = HkGetCharacterIniFloat(client, L"dock.carrier_pos.z");

	Vector vRot;
	vRot.x = HkGetCharacterIniFloat(client, L"dock.carrier_rot.x");
	vRot.y = HkGetCharacterIniFloat(client, L"dock.carrier_rot.y");
	vRot.z = HkGetCharacterIniFloat(client, L"dock.carrier_rot.z");
	cd.mCarrierLocation = EulerMatrix(vRot);
}

void SaveDockInfo(uint client)
{
	CLIENT_DATA& cd = clients[client];

	if (cd.mobile_docked)
	{
		HkSetCharacterIni(client, L"dock.docked_with_charname", cd.wscDockedWithCharname);
		HkSetCharacterIni(client, L"dock.last_base", std::to_wstring(cd.iLastBaseID));
		HkSetCharacterIni(client, L"dock.carrier_system", std::to_wstring(cd.iCarrierSystem));
		HkSetCharacterIni(client, L"dock.carrier_pos.x", std::to_wstring(cd.vCarrierLocation.x));
		HkSetCharacterIni(client, L"dock.carrier_pos.y", std::to_wstring(cd.vCarrierLocation.y));
		HkSetCharacterIni(client, L"dock.carrier_pos.z", std::to_wstring(cd.vCarrierLocation.z));

		Vector vRot = MatrixToEuler(cd.mCarrierLocation);
		HkSetCharacterIni(client, L"dock.carrier_rot.x", std::to_wstring(vRot.x));
		HkSetCharacterIni(client, L"dock.carrier_rot.y", std::to_wstring(vRot.y));
		HkSetCharacterIni(client, L"dock.carrier_rot.z", std::to_wstring(vRot.z));
	}
	else
	{
		HkSetCharacterIni(client, L"dock.docked_with_charname", L"");
		HkSetCharacterIni(client, L"dock.last_base", L"0");
		HkSetCharacterIni(client, L"dock.carrier_system", L"0");
		HkSetCharacterIni(client, L"dock.carrier_pos.x", L"0");
		HkSetCharacterIni(client, L"dock.carrier_pos.y", L"0");
		HkSetCharacterIni(client, L"dock.carrier_pos.z", L"0");
		HkSetCharacterIni(client, L"dock.carrier_rot.x", L"0");
		HkSetCharacterIni(client, L"dock.carrier_rot.y", L"0");
		HkSetCharacterIni(client, L"dock.carrier_rot.z", L"0");
	}

	if (cd.mapDockedShips.size() > 0)
	{
		int index = 1;
		for (std::map<std::wstring, std::wstring>::iterator i = cd.mapDockedShips.begin(); i != cd.mapDockedShips.end();
		     ++i, ++index)
		{
			std::wstring key = L"dock.docked_ship." + std::to_wstring(index);
			HkSetCharacterIni(client, key, i->second);
		}
		HkSetCharacterIni(client, L"dock.docked_ships_count", std::to_wstring(cd.mapDockedShips.size()));
	}
	else
	{
		HkSetCharacterIni(client, L"dock.docked_ships_count", L"0");
	}
}

void UpdateDockInfo(uint iClientID, uint iSystem, Vector pos, Matrix rot)
{
	HkSetCharacterIni(iClientID, L"dock.carrier_system", std::to_wstring(iSystem));
	HkSetCharacterIni(iClientID, L"dock.carrier_pos.x", std::to_wstring(pos.x));
	HkSetCharacterIni(iClientID, L"dock.carrier_pos.y", std::to_wstring(pos.y));
	HkSetCharacterIni(iClientID, L"dock.carrier_pos.z", std::to_wstring(pos.z));

	Vector vRot = MatrixToEuler(rot);

	HkSetCharacterIni(iClientID, L"dock.carrier_rot.x", std::to_wstring(vRot.x));
	HkSetCharacterIni(iClientID, L"dock.carrier_rot.y", std::to_wstring(vRot.y));
	HkSetCharacterIni(iClientID, L"dock.carrier_rot.z", std::to_wstring(vRot.z));
}
