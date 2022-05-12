// Spin Protection plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h" 

// Load configuration file
void LoadSettings() {
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\spin_protection.cfg";

	set_fSpinProtectMass =
        IniGetF(scPluginCfgFile, "General", "SpinProtectionMass", 180.0f);
    set_fSpinImpulseMultiplier =
        IniGetF(scPluginCfgFile, "General", "SpinProtectionMultiplier", -1.0f);
}

void __stdcall SPObjCollision(struct SSPObjCollisionInfo const &ci,
                              uint& iClientID) {

    // If spin protection is off, do nothing.
    if (set_fSpinProtectMass == -1.0f)
        return;

    // If the target is not a player, do nothing.
    // uint iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip);
    // if (iClientIDTarget<=0)
    //	return;

    float target_mass;
    pub::SpaceObj::GetMass(ci.iColliderObjectID, target_mass);

    uint client_ship;
    pub::Player::GetShip(iClientID, client_ship);

    float client_mass;
    pub::SpaceObj::GetMass(client_ship, client_mass);

    // Don't do spin protect unless the hit ship is big
    if (target_mass < set_fSpinProtectMass)
        return;

    // Don't do spin protect unless the hit ship is 2 times larger than the
    // hitter
    if (target_mass < client_mass * 2)
        return;

    Vector V1, V2;
    pub::SpaceObj::GetMotion(ci.iColliderObjectID, V1, V2);
    V1.x *= set_fSpinImpulseMultiplier * client_mass;
    V1.y *= set_fSpinImpulseMultiplier * client_mass;
    V1.z *= set_fSpinImpulseMultiplier * client_mass;
    V2.x *= set_fSpinImpulseMultiplier * client_mass;
    V2.y *= set_fSpinImpulseMultiplier * client_mass;
    V2.z *= set_fSpinImpulseMultiplier * client_mass;
    pub::SpaceObj::AddImpulse(ci.iColliderObjectID, V1, V2);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        LoadSettings();

    return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Spin Protection Plugin");
    pi->shortName("spin_protection");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->versionMajor(PluginMajorVersion::VERSION_04);
    pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::IServerImpl__SPObjCollision, &SPObjCollision);
}
