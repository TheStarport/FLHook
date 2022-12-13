#include "Main.h"

void LoadDockInfo(ClientId client)
{
	CLIENT_DATA& cd = clients[client];

	// How many docking modules do we have?
	cd.iDockingModules = 0;
	cd.mapDockedShips.clear();

	for (auto item = Players[client].equipDescList.equip.begin(); item != Players[client].equipDescList.equip.end();
	     item++)
	{
		if (item->bMounted && item->iArchId == 0xB85AB480)
		{
			cd.iDockingModules++;
		}
	}

	// Load docked ships until we run out of docking module space.
	uint count = GetCharacterIniUint(client, L"dock.docked_ships_count");
	for (uint i = 1; i <= count && cd.mapDockedShips.size() <= cd.iDockingModules; i++)
	{
		std::wstring key = L"dock.docked_ship." + std::to_wstring(i);
		std::wstring charname = GetCharacterIniString(client, key);
		if (charname.length())
		{
			if (GetAccountByCharname(charname))
			{
				cd.mapDockedShips[charname] = charname;
			}
		}
	}

	cd.wscDockedWithCharname = GetCharacterIniString(client, L"dock.docked_with_charname");
	if (cd.wscDockedWithCharname.length())
		cd.mobile_docked = true;

	cd.iLastBaseId = GetCharacterIniUint(client, L"dock.last_base");
	cd.iCarrierSystem = GetCharacterIniUint(client, L"dock.carrier_system");
	cd.vCarrierLocation.x = GetCharacterIniFloat(client, L"dock.carrier_pos.x");
	cd.vCarrierLocation.y = GetCharacterIniFloat(client, L"dock.carrier_pos.y");
	cd.vCarrierLocation.z = GetCharacterIniFloat(client, L"dock.carrier_pos.z");

	Vector vRot;
	vRot.x = GetCharacterIniFloat(client, L"dock.carrier_rot.x");
	vRot.y = GetCharacterIniFloat(client, L"dock.carrier_rot.y");
	vRot.z = GetCharacterIniFloat(client, L"dock.carrier_rot.z");
	cd.mCarrierLocation = EulerMatrix(vRot);
}

void SaveDockInfo(ClientId client)
{
	CLIENT_DATA& cd = clients[client];

	if (cd.mobile_docked)
	{
		SetCharacterIni(client, L"dock.docked_with_charname", cd.wscDockedWithCharname);
		SetCharacterIni(client, L"dock.last_base", std::to_wstring(cd.iLastBaseId));
		SetCharacterIni(client, L"dock.carrier_system", std::to_wstring(cd.iCarrierSystem));
		SetCharacterIni(client, L"dock.carrier_pos.x", std::to_wstring(cd.vCarrierLocation.x));
		SetCharacterIni(client, L"dock.carrier_pos.y", std::to_wstring(cd.vCarrierLocation.y));
		SetCharacterIni(client, L"dock.carrier_pos.z", std::to_wstring(cd.vCarrierLocation.z));

		Vector vRot = MatrixToEuler(cd.mCarrierLocation);
		SetCharacterIni(client, L"dock.carrier_rot.x", std::to_wstring(vRot.x));
		SetCharacterIni(client, L"dock.carrier_rot.y", std::to_wstring(vRot.y));
		SetCharacterIni(client, L"dock.carrier_rot.z", std::to_wstring(vRot.z));
	}
	else
	{
		SetCharacterIni(client, L"dock.docked_with_charname", L"");
		SetCharacterIni(client, L"dock.last_base", L"0");
		SetCharacterIni(client, L"dock.carrier_system", L"0");
		SetCharacterIni(client, L"dock.carrier_pos.x", L"0");
		SetCharacterIni(client, L"dock.carrier_pos.y", L"0");
		SetCharacterIni(client, L"dock.carrier_pos.z", L"0");
		SetCharacterIni(client, L"dock.carrier_rot.x", L"0");
		SetCharacterIni(client, L"dock.carrier_rot.y", L"0");
		SetCharacterIni(client, L"dock.carrier_rot.z", L"0");
	}

	if (cd.mapDockedShips.size() > 0)
	{
		int index = 1;
		for (std::map<std::wstring, std::wstring>::iterator i = cd.mapDockedShips.begin(); i != cd.mapDockedShips.end();
		     ++i, ++index)
		{
			std::wstring key = L"dock.docked_ship." + std::to_wstring(index);
			SetCharacterIni(client, key, i->second);
		}
		SetCharacterIni(client, L"dock.docked_ships_count", std::to_wstring(cd.mapDockedShips.size()));
	}
	else
	{
		SetCharacterIni(client, L"dock.docked_ships_count", L"0");
	}
}

void UpdateDockInfo(ClientId client, uint iSystem, Vector pos, Matrix rot)
{
	SetCharacterIni(client, L"dock.carrier_system", std::to_wstring(iSystem));
	SetCharacterIni(client, L"dock.carrier_pos.x", std::to_wstring(pos.x));
	SetCharacterIni(client, L"dock.carrier_pos.y", std::to_wstring(pos.y));
	SetCharacterIni(client, L"dock.carrier_pos.z", std::to_wstring(pos.z));

	Vector vRot = MatrixToEuler(rot);

	SetCharacterIni(client, L"dock.carrier_rot.x", std::to_wstring(vRot.x));
	SetCharacterIni(client, L"dock.carrier_rot.y", std::to_wstring(vRot.y));
	SetCharacterIni(client, L"dock.carrier_rot.z", std::to_wstring(vRot.z));
}
