// Anti Jump Disconnect Plugin by Cannon
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

TempBanCommunicator *tempBanCommunicator = nullptr;

struct INFO {
    bool bInWrapGate;
};
static std::map<uint, INFO> mapInfo;

void ClearClientInfo(uint& iClientID) {
    mapInfo[iClientID].bInWrapGate = false;
}

void DisConnect(uint& iClientID, enum EFLConnection& state) {
    if (mapInfo[iClientID].bInWrapGate) {
        uint iShip;
        pub::Player::GetShip(iClientID, iShip);
        pub::SpaceObj::SetInvincible(iShip, false, false, 0);
        IObjInspectImpl *obj = HkGetInspect(iClientID);
        if (obj) {
            HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f,
                        0.0f);
        }
        if (tempBanCommunicator) {
            std::wstring wscCharname =
                (const wchar_t *)Players.GetActiveCharacterName(iClientID);
            tempBanCommunicator->TempBan(wscCharname, 5);
        }
    }
}

void CharacterInfoReq(uint& iClientID, bool& p2) {
    if (mapInfo[iClientID].bInWrapGate) {
        uint iShip;
        pub::Player::GetShip(iClientID, iShip);
        pub::SpaceObj::SetInvincible(iShip, false, false, 0);
        IObjInspectImpl *obj = HkGetInspect(iClientID);
        if (obj) {
            HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f,
                        0.0f);
        }
        if (tempBanCommunicator) {
            std::wstring wscCharname =
                (const wchar_t *)Players.GetActiveCharacterName(iClientID);
            tempBanCommunicator->TempBan(wscCharname, 5);
        }
    }
}

void JumpInComplete(uint& iSystem, uint& iShip,
                    uint& iClientID) {
    mapInfo[iClientID].bInWrapGate = false;
}

void SystemSwitchOutComplete(uint& iShip, uint& iClientID) {
    mapInfo[iClientID].bInWrapGate = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Anti Jump Disconnect Plugin by Cannon");
    pi->shortName("anti_jump_disconnect");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
    pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq,
                    &CharacterInfoReq);
    pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
    pi->emplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete,
                    &SystemSwitchOutComplete);

    // We import the definitions for TempBan Communicator so we can talk to it
    tempBanCommunicator = static_cast<TempBanCommunicator *>(PluginCommunicator::ImportPluginCommunicator(TempBanCommunicator::pluginName));
}