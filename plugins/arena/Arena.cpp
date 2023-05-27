/**
 * @date August, 2022
 * @author MadHunter (Ported by Raikkonen 2022)
 * @defgroup Arena Arena
 * @brief
 * This plugin is used to beam players to/from an arena system for the purpose of pvp.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - arena (configurable) - This beams the player to the pvp system.
 * - return - This returns the player to their last docked base.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "command": "arena",
 *     "restrictedSystem": "Li01",
 *     "targetBase": "Li02_01_Base",
 *     "targetSystem": "Li02"
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin uses the "Base" plugin.
 */
#include "PCH.hpp"
#include "Arena.hpp"
#include "FLHook.hpp"
#include "Helpers/Player.hpp"
#include "Helpers/Client.hpp"
#include "Helpers/Error.hpp"
#include "Helpers/Ini.hpp"

namespace Plugins
{
	Arena::Arena(const PluginInfo& info) : Plugin(info)
	{
		EmplaceHook(HookedCall::FLHook__LoadSettings, &Arena::LoadSettings, HookStep::After);
		EmplaceHook(HookedCall::IServerImpl__CharacterSelect, &Arena::CharacterSelect);
		EmplaceHook(HookedCall::IServerImpl__PlayerLaunch, &Arena::PlayerLaunch_AFTER, HookStep::After);
		EmplaceHook(HookedCall::FLHook__ClearClientInfo, &Arena::ClearClientInfo, HookStep::After);
	
		AddCommand(L"/return", L"", CmdP(UserCmd_Return), L"Returns you to your last docked base.");
	}

	/// Clear client info when a client connects.
	void Arena::ClearClientInfo(ClientId& client)
	{
		transferFlags[client] = ClientState::None;
	}

	/// Load the configuration
	void Arena::LoadSettings()
	{
		config = Serializer::JsonToObject<Config>();
		command = L"/" + StringUtils::stows(config.command);
		restrictedSystemId = CreateID(config.restrictedSystem.c_str());
		targetBaseId = CreateID(config.targetBase.c_str());
		targetSystemId = CreateID(config.targetSystem.c_str());

		AddCommand(command, L"", CmdP(UserCmd_Conn), L"Sends you to the designated arena system.");
	}

	/** @ingroup Arena
	 * @brief Returns true if the client is docked, returns false otherwise.
	 */
	bool IsDockedClient(unsigned int client)
	{
		return Hk::Player::GetCurrentBase(client).has_value();
	}

	/** @ingroup Arena
	 * @brief Returns true if the client doesn't hold any commodities, returns false otherwise. This is to prevent people using the arena system as a trade
	 * shortcut.
	 */
	bool ValidateCargo(ClientId& client)
	{
		if (const auto playerName = Hk::Client::GetCharacterNameByID(client); playerName.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(playerName.error()));
			return false;
		}
		int holdSize = 0;

		const auto cargo = Hk::Player::EnumCargo(client, holdSize);
		if (cargo.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(cargo.error()));
			return false;
		}

		for (const auto& item : cargo.value())
		{
			bool flag = false;
			pub::IsCommodity(item.archId, flag);

			// Some commodity present.
			if (flag)
				return false;
		}

		return true;
	}

	/** @ingroup Arena
	 * @brief Stores the return point for the client in their save file (this should be changed).
	 */
	void StoreReturnPointForClient(unsigned int client)
	{
		// It's not docked at a custom base, check for a regular base
		auto base = Hk::Player::GetCurrentBase(client);
		if (base.has_error())
			return;

		Hk::Ini::SetCharacterIni(client, L"conn.retbase", std::to_wstring(base.value()));
	}

	/** @ingroup Arena
	 * @brief This returns the return base id that is stored in the client's save file.
	 */
	unsigned int ReadReturnPointForClient(unsigned int client)
	{
		return Hk::Ini::GetCharacterIniUint(client, L"conn.retbase");
	}

	/** @ingroup Arena
	 * @brief Move the specified client to the specified base.
	 */
	void MoveClient(unsigned int client, unsigned int targetBase)
	{
		// Ask that another plugin handle the beam.
		// if (global->baseCommunicator && global->baseCommunicator->CustomBaseBeam(client, targetBase))
		//	return;

		// No plugin handled it, do it ourselves.
		SystemId system = Hk::Player::GetSystem(client).value();
		const Universe::IBase* base = Universe::get_base(targetBase);

		Hk::Player::Beam(client, targetBase);
		// if not in the same system, emulate F1 charload
		if (base->systemId != system)
		{
			Server.BaseEnter(targetBase, client);
			Server.BaseExit(targetBase, client);
			const std::wstring CharFileName;
			const auto charFileName = Hk::Client::GetCharFileName(client);

			if (charFileName.has_error())
				return;

			auto fileName = charFileName.value() + L".fl";
			CHARACTER_ID charId;
			strcpy_s(charId.charFilename, StringUtils::wstos(CharFileName.substr(0, 14)).c_str());
			Server.CharacterSelect(charId, client);
		}
	}

	/** @ingroup Arena
	 * @brief Checks the client is in the specified base. Returns true is so, returns false otherwise.
	 */
	bool CheckReturnDock(unsigned int client, unsigned int target)
	{
		if (auto base = Hk::Player::GetCurrentBase(client); base.value() == target)
			return true;

		return false;
	}

	/** @ingroup Arena
	 * @brief Hook on CharacterSelect. Sets their transfer flag to "None".
	 */
	void Arena::CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client)
	{
		transferFlags[client] = ClientState::None;
	}

	/** @ingroup Arena
	 * @brief Hook on PlayerLaunch. If their transfer flags are set appropriately, redirect the undock to either the arena base or the return point
	 */
	void Arena::PlayerLaunch_AFTER([[maybe_unused]] const uint& ship, ClientId& client)
	{
		if (transferFlags[client] == ClientState::Transfer)
		{
			if (!ValidateCargo(client))
			{
				PrintUserCmdText(client, cargoErrorText);
				return;
			}

			transferFlags[client] = ClientState::None;
			MoveClient(client, targetBaseId);
			return;
		}

		if (transferFlags[client] == ClientState::Return)
		{
			if (!ValidateCargo(client))
			{
				PrintUserCmdText(client, cargoErrorText);
				return;
			}

			transferFlags[client] = ClientState::None;
			const unsigned int returnPoint = ReadReturnPointForClient(client);

			if (!returnPoint)
				return;

			MoveClient(client, returnPoint);
			Hk::Ini::SetCharacterIni(client, L"conn.retbase", L"0");
			return;
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// USER COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** @ingroup Arena
	 * @brief Used to switch to the arena system
	 */
	void Arena::UserCmd_Conn(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		// Prohibit jump if in a restricted system or in the target system
		if (SystemId system = Hk::Player::GetSystem(client).value(); system == restrictedSystemId || system == targetSystemId)
		{
			PrintUserCmdText(client, L"ERR Cannot use command in this system or base");
			return;
		}

		if (!IsDockedClient(client))
		{
			PrintUserCmdText(client, dockErrorText);
			return;
		}

		if (!ValidateCargo(client))
		{
			PrintUserCmdText(client, cargoErrorText);
			return;
		}

		StoreReturnPointForClient(client);
		PrintUserCmdText(client, L"Redirecting undock to Arena.");
		transferFlags[client] = ClientState::Transfer;
	}

	/** @ingroup Arena
	 * @brief Used to return from the arena system.
	 */
	void Arena::UserCmd_Return(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		if (!ReadReturnPointForClient(client))
		{
			PrintUserCmdText(client, L"No return possible");
			return;
		}

		if (!IsDockedClient(client))
		{
			PrintUserCmdText(client, dockErrorText);
			return;
		}

		if (!CheckReturnDock(client, targetBaseId))
		{
			PrintUserCmdText(client, L"Not in correct base");
			return;
		}

		if (!ValidateCargo(client))
		{
			PrintUserCmdText(client, cargoErrorText);
			return;
		}

		PrintUserCmdText(client, L"Redirecting undock to previous base");
		transferFlags[client] = ClientState::Return;
	}
} // namespace Plugins

using namespace Plugins;

// REFL_AUTO(type(Config), field(command), field(targetBase), field(targetSystem), field(restrictedSystem))

DefaultDllMain();

const PluginInfo Info("Arena", "arena", PluginMajorVersion::VERSION_04, PluginMinorVersion::VERSION_01);
SetupPlugin(Arena, Info);