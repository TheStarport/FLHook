/**
 * @date Feb 2010
 * @author Cannon (Ported by Raikkonen 2022)
 * @defgroup AntiJumpDisconnect Anti Jump Disconnect
 * @brief
 * The "Anti Jump Disconnect" plugin will kill a player if they disconnect during the jump animation.
 * If tempban is loaded then they will also be banned for 5 minutes.
 *
 * @paragraph cmds Player Commands
 * There are no player commands in this plugin.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin uses the "Temp Ban" plugin.
 */

// Includes
#include "Main.h"

namespace Plugins::AntiJumpDisconnect
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		global->tempBanCommunicator = static_cast<Tempban::TempBanCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Tempban::TempBanCommunicator::pluginName));
	}

	void ClearClientInfo(ClientId& client) { global->mapInfo[client].bInWrapGate = false; }

	/** @ingroup AntiJumpDisconnect
	 * @brief Kills and possibly bans the player. This depends on if the Temp Ban plugin is active.
	 */
	void KillBan(ClientId& client)
	{
		if (global->mapInfo[client].bInWrapGate)
		{

			if (const auto ban = Hk::Player::Kill(client); ban.has_error())
			{
				PrintUserCmdText(client, Hk::Err::ErrGetText(ban.error()));
				return;
			}

			if (global->tempBanCommunicator)
			{
				const auto charName = Hk::Client::GetCharacterNameByID(client);
				global->tempBanCommunicator->TempBan(charName.value(), static_cast<uint>(5));
			}
		}
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on Disconnect. Calls KillBan.
	 */
	void DisConnect(ClientId& client, [[maybe_unused]] const enum EFLConnection& state)
	{
		KillBan(client);
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on CharacterInfoReq (Character Select screen). Calls KillBan.
	 */
	void CharacterInfoReq(ClientId& client, [[maybe_unused]] const bool& p2)
	{
		KillBan(client);
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on JumpInComplete. Sets the "In Gate" variable to false.
	 */
	void JumpInComplete([[maybe_unused]] const SystemId& System, [[maybe_unused]] const ShipId& Ship, ClientId& client)
	{
		global->mapInfo[client].bInWrapGate = false;
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on SystemSwitchOutComplete. Sets the "In Gate" variable to true.
	 */
	void SystemSwitchOutComplete([[maybe_unused]]const ShipId& Ship, ClientId& client) { global->mapInfo[client].bInWrapGate = true; }
} // namespace Plugins::AntiJumpDisconnect

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::AntiJumpDisconnect;

DefaultDllMain()

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Anti Jump Disconnect Plugin by Cannon");
	pi->shortName("anti_jump_disconnect");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
	pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
	pi->emplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete, &SystemSwitchOutComplete);
}