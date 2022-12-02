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

// Setup Doxygen Group

/** @defgroup SoundManager Sound Manager */

namespace Plugins::SoundManager
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		Config conf = Serializer::JsonToObject<Config>();

		for (auto& sound : conf.sounds)
			conf.sound_ids.push_back(CreateID(sound.c_str()));

		global->config = std::make_unique<Config>(std::move(conf));
	}

	void __stdcall Login(struct SLoginInfo const& li, ClientId& client)
	{
		// Player sound when player logs in

		if (global->config->sound_ids.size())
			pub::Audio::PlaySoundEffect(client, global->config->sound_ids[rand() % global->config->sound_ids.size()]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::SoundManager;
// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(sounds))

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Sound Manager");
	pi->shortName("sound_manager");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login);
}