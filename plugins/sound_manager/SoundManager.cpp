/**
 * @date 2022
 * @author Raikkonen
 * @defgroup SoundManager Sound Manager
 * @brief
 * The plugin plays a random sound upon player login. To be expanded upon.
 *
 * @paragraph cmds Player Commands
 * None
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *	"sounds": ["dock_not_allowed", "dock_granted"]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

// Includes
#include "SoundManager.h"

namespace Plugins::SoundManager
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		Config conf = Serializer::JsonToObject<Config>();

		for (const auto& sound : conf.sounds)
			conf.sound_ids.push_back(CreateID(sound.c_str()));

		global->config = std::make_unique<Config>(std::move(conf));
	}

	void Login([[maybe_unused]] SLoginInfo const& li, ClientId& client)
	{
		// Player sound when player logs in

		if (global->config->sound_ids.size())
			Hk::Client::PlaySoundEffect(client, global->config->sound_ids[rand() % global->config->sound_ids.size()]);
	}
} // namespace Plugins::SoundManager

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::SoundManager;
// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(sounds))

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Sound Manager");
	pi->shortName("sound_manager");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::V04);
	pi->versionMinor(PluginMinorVersion::V00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login, HookStep::After);
}