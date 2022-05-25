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

struct Config final : Reflectable
{
	// This json file should contain a "sounds" array containing a list of sounds for e.g. dx_s032a_01a01_hvis_xtr_1
	std::string File() override { return "flhook_plugins/soundmanager.json"; }

	// Reflectable fields
	std::vector<std::string> sounds;

	// Non-reflectable fields
	std::vector<uint> sound_ids;
};

REFL_AUTO(type(Config), field(sounds))
std::unique_ptr<Config> config = nullptr;

void LoadSettings()
{
	Config conf = Serializer::JsonToObject<Config>();

	for (auto& sound : conf.sounds)
	    conf.sound_ids.push_back(CreateID(sound.c_str()));

	config = std::make_unique<Config>(std::move(conf));
}

void __stdcall Login(struct SLoginInfo const& li, uint& iClientID)
{
	// Player sound when player logs in
	
	if (config->sound_ids.size())
		pub::Audio::PlaySoundEffect(iClientID, config->sound_ids[rand() % config->sound_ids.size()]);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
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