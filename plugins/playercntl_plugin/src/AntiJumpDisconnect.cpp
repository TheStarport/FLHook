// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// This code from somewhere, credit to motah, wodka and mc_horst.

#include <FLHook.h>
#include <float.h>
#include <list>
#include <math.h>
#include <plugin.h>
#include <plugin_comms.h>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#include "Main.h"

namespace AntiJumpDisconnect {
struct INFO {
  bool bInWrapGate;
};
static std::map<uint, INFO> mapInfo;

void AntiJumpDisconnect::ClearClientInfo(unsigned int iClientID) {
  mapInfo[iClientID].bInWrapGate = false;
}

void AntiJumpDisconnect::DisConnect(unsigned int iClientID,
                                    enum EFLConnection state) {
  if (mapInfo[iClientID].bInWrapGate) {
    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    pub::SpaceObj::SetInvincible(iShip, false, false, 0);
    IObjInspectImpl *obj = HkGetInspect(iClientID);
    if (obj) {
      HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
    }
    HkTempBan(iClientID, 5);
  }
}

void AntiJumpDisconnect::CharacterInfoReq(unsigned int iClientID, bool p2) {
  if (mapInfo[iClientID].bInWrapGate) {
    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    pub::SpaceObj::SetInvincible(iShip, false, false, 0);
    IObjInspectImpl *obj = HkGetInspect(iClientID);
    if (obj) {
      HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
    }
    HkTempBan(iClientID, 5);
  }
}

void AntiJumpDisconnect::JumpInComplete(unsigned int iSystem,
                                        unsigned int iShip,
                                        unsigned int iClientID) {
  mapInfo[iClientID].bInWrapGate = false;
}

void AntiJumpDisconnect::SystemSwitchOutComplete(unsigned int iShip,
                                                 unsigned int iClientID) {
  mapInfo[iClientID].bInWrapGate = true;
}
} // namespace AntiJumpDisconnect
