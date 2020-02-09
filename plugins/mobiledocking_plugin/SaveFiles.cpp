#include "Main.h"
#include "../hookext_plugin/hookext_exports.h"

#define PI 3.14159265f

// Convert radians to degrees.
float degrees( float rad )
{
  rad *= 180 / PI;

  // Prevent displaying -0 and prefer 180 to -180.
  if (rad < 0)
  {
    if (rad > -0.005f)
      rad = 0;
    else if (rad <= -179.995f)
      rad = 180;
  }

  // Round to two decimal places here, so %g can display it without decimals.
  float frac = modff( rad * 100, &rad );
  if (frac >= 0.5f)
    ++rad;
  else if (frac <= -0.5f)
    --rad;

  return rad / 100;
}

// Convert an orientation matrix to a pitch/yaw/roll vector.  Based on what
// Freelancer does for the save game.
void Matrix_to_Vector( const Matrix& mat, Vector& vec )
{
  Vector x = { mat.data[0][0], mat.data[1][0], mat.data[2][0] };
  Vector y = { mat.data[0][1], mat.data[1][1], mat.data[2][1] };
  Vector z = { mat.data[0][2], mat.data[1][2], mat.data[2][2] };

  float h = (float)_hypot( x.x, x.y );
  if (h > 1/524288.0f)
  {
    vec.x = degrees( atan2f(  y.z, z.z ) );
    vec.y = degrees( atan2f( -x.z, h   ) );
    vec.z = degrees( atan2f(  x.y, x.x ) );
  }
  else
  {
    vec.x = degrees( atan2f( -z.y, y.y ) );
    vec.y = degrees( atan2f( -x.z, h   ) );
    vec.z = 0;
  }
}

void ini_write_wstring(FILE *file, const std::string &parmname, const std::wstring &in)
{
	fprintf(file, "%s=", parmname.c_str()); 
	for (int i = 0; i < (int)in.size(); i++)
	{
		UINT v1 = in[i] >> 8;
		UINT v2 = in[i] & 0xFF;
		fprintf(file, "%02x%02x", v1, v2); 
	}
	fprintf(file, "\n");
}


void ini_get_wstring(INI_Reader &ini, std::wstring &wscValue)
{
	std::string scValue = ini.get_value_string();
	wscValue = L"";
	long lHiByte;
	long lLoByte;
	while(sscanf(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}
}

void LoadDockInfo(uint client)
{
	CLIENT_DATA &cd = clients[client];

	// How many docking modules do we have?
	cd.iDockingModules = 0;
	cd.mapDockedShips.clear();

	for (auto item = Players[client].equipDescList.equip.begin();
		item != Players[client].equipDescList.equip.end(); item++)
	{
		if (item->bMounted && item->iArchID == 0xB85AB480)
		{
			cd.iDockingModules++;
		}
	}

	// Load docked ships until we run out of docking module space.
	uint count = HookExt::IniGetI(client, "dock.docked_ships_count");
	for (uint i=1; i<=count && cd.mapDockedShips.size()<=cd.iDockingModules; i++)
	{
		char key[100];
		sprintf(key, "dock.docked_ship.%u", i);
		std::wstring charname = HookExt::IniGetWS(client, key);
		if (charname.length())
		{
			if (HkGetAccountByCharname(charname))
			{
				cd.mapDockedShips[charname] = charname;
			}
		}
	}

	cd.wscDockedWithCharname = HookExt::IniGetWS(client, "dock.docked_with_charname");
	if (cd.wscDockedWithCharname.length())
		cd.mobile_docked = true;
						
	cd.iLastBaseID = HookExt::IniGetI(client, "dock.last_base");
	cd.iCarrierSystem = HookExt::IniGetI(client, "dock.carrier_system");
	cd.vCarrierLocation.x = HookExt::IniGetF(client, "dock.carrier_pos.x");
	cd.vCarrierLocation.y = HookExt::IniGetF(client, "dock.carrier_pos.y");
	cd.vCarrierLocation.z = HookExt::IniGetF(client, "dock.carrier_pos.z");

	Vector vRot;
	vRot.x = HookExt::IniGetF(client, "dock.carrier_rot.x");
	vRot.y = HookExt::IniGetF(client, "dock.carrier_rot.y");
	vRot.z = HookExt::IniGetF(client, "dock.carrier_rot.z");
	cd.mCarrierLocation = EulerMatrix(vRot);
}

void SaveDockInfo(uint client)
{
	CLIENT_DATA &cd = clients[client];

	if (cd.mobile_docked)
	{		
		HookExt::IniSetWS(client, "dock.docked_with_charname", cd.wscDockedWithCharname);
		HookExt::IniSetI(client, "dock.last_base", cd.iLastBaseID);
		HookExt::IniSetI(client, "dock.carrier_system", cd.iCarrierSystem);
		HookExt::IniSetF(client, "dock.carrier_pos.x", cd.vCarrierLocation.x);
		HookExt::IniSetF(client, "dock.carrier_pos.y", cd.vCarrierLocation.y);
		HookExt::IniSetF(client, "dock.carrier_pos.z", cd.vCarrierLocation.z);
		
		Vector vRot;
		Matrix_to_Vector(cd.mCarrierLocation, vRot);
		HookExt::IniSetF(client, "dock.carrier_rot.x", vRot.x);
		HookExt::IniSetF(client, "dock.carrier_rot.y", vRot.y);
		HookExt::IniSetF(client, "dock.carrier_rot.z", vRot.z);
	}
	else
	{
		HookExt::IniSetWS(client, "dock.docked_with_charname", L"");
		HookExt::IniSetI(client, "dock.last_base", 0);
		HookExt::IniSetI(client, "dock.carrier_system", 0);
		HookExt::IniSetI(client, "dock.carrier_pos.x", 0);
		HookExt::IniSetI(client, "dock.carrier_pos.y", 0);
		HookExt::IniSetI(client, "dock.carrier_pos.z", 0);
		HookExt::IniSetI(client, "dock.carrier_rot.x", 0);
		HookExt::IniSetI(client, "dock.carrier_rot.y", 0);
		HookExt::IniSetI(client, "dock.carrier_rot.z", 0);
	}
	
	if (cd.mapDockedShips.size() > 0)
	{
		int index = 1;
		for (map<std::wstring, std::wstring>::iterator i = cd.mapDockedShips.begin();
			i != cd.mapDockedShips.end(); ++i, ++index)
		{
			char key[100];
			sprintf(key, "dock.docked_ship.%u", index);
			HookExt::IniSetWS(client, key, i->second);
		}
		HookExt::IniSetI(client, "dock.docked_ships_count", cd.mapDockedShips.size());
	}
	else
	{
		HookExt::IniSetI(client, "dock.docked_ships_count", 0);
	}
}

void UpdateDockInfo(const std::wstring &wscCharname, uint iSystem, Vector pos, Matrix rot)
{
	HookExt::IniSetI(wscCharname, "dock.carrier_system", iSystem);
	HookExt::IniSetF(wscCharname, "dock.carrier_pos.x", pos.x);
	HookExt::IniSetF(wscCharname, "dock.carrier_pos.y", pos.y);
	HookExt::IniSetF(wscCharname, "dock.carrier_pos.z", pos.z);
	
	Vector vRot;
	Matrix_to_Vector(rot, vRot);

	HookExt::IniSetF(wscCharname, "dock.carrier_rot.x", vRot.x);
	HookExt::IniSetF(wscCharname, "dock.carrier_rot.y", vRot.y);
	HookExt::IniSetF(wscCharname, "dock.carrier_rot.z", vRot.z);
}