// Sound Manager Plugin by Raikkonen 2022
//
// This plugin plays a random sound loaded from a config file on login. It's a pretty simple plugin
// and could be expanded to play sounds on other hooks.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

void LoadSettings() {

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\sound_manager.cfg";

    INI_Reader ini;
    if (ini.open(scPluginCfgFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("Sounds")) {
                while (ini.read_value()) {
                    if (ini.is_value("sound")) {
                        sounds.push_back(CreateID(ini.get_value_string(0)));
                    }
                }
            }
        }
        ini.close();
    }
}

void __stdcall Login(struct SLoginInfo const &li, uint& iClientID) {
    // Player sound when player logs in
    if (sounds.size() > 0)
        pub::Audio::PlaySoundEffect(iClientID, sounds[rand() % sounds.size()]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Sound Manager");
    pi->shortName("sound_manager");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->versionMajor(PluginMajorVersion::VERSION_04);
    pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
    pi->emplaceHook(HookedCall::IServerImpl__Login, &Login);
}