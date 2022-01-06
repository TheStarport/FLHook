// Anti Jump Disconnect Plugin by Cannon
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

struct INFO {
    bool bInWrapGate;
};
static std::map<uint, INFO> mapInfo;

void ClearClientInfo(unsigned int iClientID) {
    mapInfo[iClientID].bInWrapGate = false;
}

void DisConnect(unsigned int iClientID, enum EFLConnection state) {
    if (mapInfo[iClientID].bInWrapGate) {
        uint iShip;
        pub::Player::GetShip(iClientID, iShip);
        pub::SpaceObj::SetInvincible(iShip, false, false, 0);
        IObjInspectImpl *obj = HkGetInspect(iClientID);
        if (obj) {
            HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f,
                        0.0f);
        }
        HkTempBan(iClientID, 5);
    }
}

void CharacterInfoReq(unsigned int iClientID, bool p2) {
    if (mapInfo[iClientID].bInWrapGate) {
        uint iShip;
        pub::Player::GetShip(iClientID, iShip);
        pub::SpaceObj::SetInvincible(iShip, false, false, 0);
        IObjInspectImpl *obj = HkGetInspect(iClientID);
        if (obj) {
            HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f,
                        0.0f);
        }
        HkTempBan(iClientID, 5);
    }
}

void JumpInComplete(unsigned int iSystem, unsigned int iShip,
                    unsigned int iClientID) {
    mapInfo[iClientID].bInWrapGate = false;
}

void SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID) {
    mapInfo[iClientID].bInWrapGate = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Anti Jump Disconnect Plugin by Cannon");
    pi->shortName("anti_jump_disconnect");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
    pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq,
                    &CharacterInfoReq);
    pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
    pi->emplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete,
                    &SystemSwitchOutComplete);
}