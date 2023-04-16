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
#include "PCH.hpp"
#include "AntiJumpDisconnect.hpp"
#include "Features/TempBan.hpp"
#include "FLHook.hpp"
#include "Helpers/Error.hpp"
#include "Helpers/Player.hpp"

constexpr auto TempBanDurationMinutes = 5;

namespace Plugins
{
	AntiJumpDisconnect::AntiJumpDisconnect(const PluginInfo& info) : Plugin(info)
	{
		EmplaceHook(HookedCall::FLHook__ClearClientInfo, &AntiJumpDisconnect::ClearClientInfo, HookStep::After);
		EmplaceHook(HookedCall::IServerImpl__DisConnect, &AntiJumpDisconnect::Disconnect);
		EmplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &AntiJumpDisconnect::CharacterInfoReq);
		EmplaceHook(HookedCall::IServerImpl__JumpInComplete, &AntiJumpDisconnect::JumpInComplete);
		EmplaceHook(HookedCall::IServerImpl__SystemSwitchOutComplete, &AntiJumpDisconnect::SystemSwitchOutComplete);
	}

	void AntiJumpDisconnect::ClearClientInfo(ClientId& client)
	{
		mapInfo[client] = false;
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Kills and possibly bans the player. This depends on if the Temp Ban plugin is active.
	 */
	void AntiJumpDisconnect::KillIfInJumpTunnel(ClientId& client)
	{
		if (mapInfo[client])
		{
			if (const auto ban = Hk::Player::Kill(client); ban.has_error())
			{
				PrintUserCmdText(client, Hk::Err::ErrGetText(ban.error()));
				return;
			}

			// tempban for 5 minutes
			TempBanManager::i()->AddTempBan(client, TempBanDurationMinutes);
		}
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on Disconnect. Calls KillBan.
	 */
	void AntiJumpDisconnect::Disconnect(ClientId& client, [[maybe_unused]] const EFLConnection& state)
	{
		KillIfInJumpTunnel(client);
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on CharacterInfoReq (Character Select screen). Calls KillBan.
	 */
	void AntiJumpDisconnect::CharacterInfoReq(ClientId& client, [[maybe_unused]] const bool& param2)
	{
		KillIfInJumpTunnel(client);
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on JumpInComplete. Sets the "In Gate" variable to false.
	 */
	void AntiJumpDisconnect::JumpInComplete([[maybe_unused]] const SystemId& system, [[maybe_unused]] const ShipId& ship, ClientId& client)
	{
		mapInfo[client] = false;
	}

	/** @ingroup AntiJumpDisconnect
	 * @brief Hook on SystemSwitchOutComplete. Sets the "In Gate" variable to true.
	 */
	void AntiJumpDisconnect::SystemSwitchOutComplete([[maybe_unused]] const ShipId& Ship, ClientId& client)
	{
		mapInfo[client] = true;
	}
} // namespace Plugins

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info("AFK", "afk", PluginMajorVersion::VERSION_04, PluginMinorVersion::VERSION_01);
SetupPlugin(AntiJumpDisconnect, Info);