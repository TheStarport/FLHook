#include "Main.h"
#include "../hookext_plugin/hookext_exports.h"

void LoadDockInfo(uint client) {
  CLIENT_DATA &cd = clients[client];

  // How many docking modules do we have?
  cd.iDockingModules = 0;
  cd.mapDockedShips.clear();

  for (auto item = Players[client].equipDescList.equip.begin();
       item != Players[client].equipDescList.equip.end(); item++) {
    if (item->bMounted && item->iArchID == 0xB85AB480) {
      cd.iDockingModules++;
    }
  }

  // Load docked ships until we run out of docking module space.
  uint count = HookExt::IniGetI(client, "dock.docked_ships_count");
  for (uint i = 1; i <= count && cd.mapDockedShips.size() <= cd.iDockingModules;
       i++) {
    char key[100];
    sprintf_s(key, "dock.docked_ship.%u", i);
    std::wstring charname = HookExt::IniGetWS(client, key);
    if (charname.length()) {
      if (HkGetAccountByCharname(charname)) {
        cd.mapDockedShips[charname] = charname;
      }
    }
  }

  cd.wscDockedWithCharname =
      HookExt::IniGetWS(client, "dock.docked_with_charname");
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

void SaveDockInfo(uint client) {
  CLIENT_DATA &cd = clients[client];

  if (cd.mobile_docked) {
    HookExt::IniSetWS(client, "dock.docked_with_charname",
                      cd.wscDockedWithCharname);
    HookExt::IniSetI(client, "dock.last_base", cd.iLastBaseID);
    HookExt::IniSetI(client, "dock.carrier_system", cd.iCarrierSystem);
    HookExt::IniSetF(client, "dock.carrier_pos.x", cd.vCarrierLocation.x);
    HookExt::IniSetF(client, "dock.carrier_pos.y", cd.vCarrierLocation.y);
    HookExt::IniSetF(client, "dock.carrier_pos.z", cd.vCarrierLocation.z);

    Vector vRot = MatrixToEuler(cd.mCarrierLocation);
    HookExt::IniSetF(client, "dock.carrier_rot.x", vRot.x);
    HookExt::IniSetF(client, "dock.carrier_rot.y", vRot.y);
    HookExt::IniSetF(client, "dock.carrier_rot.z", vRot.z);
  } else {
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

  if (cd.mapDockedShips.size() > 0) {
    int index = 1;
    for (map<std::wstring, std::wstring>::iterator i =
             cd.mapDockedShips.begin();
         i != cd.mapDockedShips.end(); ++i, ++index) {
      char key[100];
      sprintf_s(key, "dock.docked_ship.%u", index);
      HookExt::IniSetWS(client, key, i->second);
    }
    HookExt::IniSetI(client, "dock.docked_ships_count",
                     cd.mapDockedShips.size());
  } else {
    HookExt::IniSetI(client, "dock.docked_ships_count", 0);
  }
}

void UpdateDockInfo(const std::wstring &wscCharname, uint iSystem, Vector pos,
                    Matrix rot) {
  HookExt::IniSetI(wscCharname, "dock.carrier_system", iSystem);
  HookExt::IniSetF(wscCharname, "dock.carrier_pos.x", pos.x);
  HookExt::IniSetF(wscCharname, "dock.carrier_pos.y", pos.y);
  HookExt::IniSetF(wscCharname, "dock.carrier_pos.z", pos.z);

  Vector vRot = MatrixToEuler(rot);

  HookExt::IniSetF(wscCharname, "dock.carrier_rot.x", vRot.x);
  HookExt::IniSetF(wscCharname, "dock.carrier_rot.y", vRot.y);
  HookExt::IniSetF(wscCharname, "dock.carrier_rot.z", vRot.z);
}
