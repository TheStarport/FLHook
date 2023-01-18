/**
 * @date Feb, 2010
 * @author Cannon (Ported by Raikkonen 2022)
 * @defgroup MiscCommands Misc Commands
 * @brief
 * The "Misc Commands" plugin provides an array of useful commands that serve as general quality of life features or game enhancements.
 * They are commands that do not really fit into another plugin, or have the significance to belong in FLHook Core.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - lights - Activate optional ship lights, these usually default to ones on the docking light hardpoints.
 * - shields - De/Reactivate your shields
 * - pos - Prints the current absolute position of your ship
 * - stuck - Nudges the ship 10m away from where they currently are while stationary. Designed to prevent player capital ships from being wedged.
 * - droprep - Lowers the reputation of the current faction you are affiliated with. 
 * - coin - Toss a coin and print the result in local chat.
 * - dice [sides] - Tosses a dice with the specified number of sides, defaulting to 6.
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - smiteall [die] - Remove all shields of all players within 15k and plays music. If [die] is specified, then instead of lowering shields, it kills the players.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "coinMessage": "%player tosses %result",
 *     "diceMessage": "%player rolled %number of %max",
 *     "repDropCost": 0,
 *     "smiteMusicId": "music_danger",
 *     "stuckMessage": "Attention! Stand Clear. Towing %player"
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "Main.h"

namespace Plugins::MiscCommands
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// Load the configuration
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->smiteMusicHash = CreateID(config.smiteMusicId.c_str());
		global->config = std::make_unique<Config>(config);
	}

	/** Clean up when a client disconnects */
	void ClearClientInfo(ClientId& client) { global->mapInfo.erase(client); }

	/** One second timer */
	void OneSecondTimer()
	{
		// Drop player shields and keep them down.
		for (const auto& [id, info] : global->mapInfo)
		{
			if (info.bShieldsDown)
			{
				if (const auto playerInfo = Hk::Admin::GetPlayerInfo(Hk::Client::GetCharacterNameByID(id).value(), false); playerInfo.has_value() && playerInfo.value().ship)
				{
					pub::SpaceObj::DrainShields(playerInfo.value().ship);
				}
			}
		}
	}

	const std::vector<Timer> timers = {{OneSecondTimer, 1}};

	static void SetLights(ClientId client, bool bOn)
	{
		auto ship = Hk::Player::GetShip(client);
		if (ship.has_error())
		{
			PrintUserCmdText(client, L"ERR Not in space");
			return;
		}

		bool bLights = false;
		st6::list<EquipDesc> const& eqLst = Players[client].equipDescList.equip;
		for (const auto& eq : eqLst)
		{
			std::string hp = ToLower(eq.szHardPoint.value);
			if (hp.find("dock") != std::string::npos)
			{
				XActivateEquip ActivateEq;
				ActivateEq.bActivate = bOn;
				ActivateEq.iSpaceId = ship.value();
				ActivateEq.sId = eq.sId;
				Server.ActivateEquip(client, ActivateEq);
				bLights = true;
			}
		}

		if (bLights)
			PrintUserCmdText(client, L" Lights %s", bOn ? L"on" : L"off");
		else
			PrintUserCmdText(client, L"Light control not available");
	}

	/** @ingroup MiscCommands
	 * @brief Print the current location of your ship
	 */
	void UserCmdPos(ClientId& client, const std::wstring& wscParam)
	{
		const auto playerInfo = Hk::Admin::GetPlayerInfo(Hk::Client::GetCharacterNameByID(client).value(), false);
		if (playerInfo.has_error() || !playerInfo.value().ship)
		{
			PrintUserCmdText(client, L"ERR Not in space");
			return;
		}

		auto [pos, rot] = Hk::Solar::GetLocation(playerInfo.value().ship, IdType::Ship).value();
		
		Vector erot = Hk::Math::MatrixToEuler(rot);

		wchar_t buf[100];
		_snwprintf_s(buf, sizeof(buf), L"Position %0.0f %0.0f %0.0f Orient %0.0f %0.0f %0.0f", pos.x, pos.y, pos.z, erot.x, erot.y, erot.z);
		PrintUserCmdText(client, buf);
	}

	/** @ingroup MiscCommands
	 * @brief Nudge your ship 15 meters on all axis to try and dislodge a stuck ship.
	 */
	void UserCmdStuck(ClientId& client, const std::wstring& wscParam)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);

		const auto playerInfo = Hk::Admin::GetPlayerInfo(Hk::Client::GetCharacterNameByID(client).value(), false);
		if (playerInfo.has_error() || !playerInfo.value().ship)
		{
			PrintUserCmdText(client, L"ERR Not in space");
			return;
		}

		auto motion = Hk::Solar::GetMotion(playerInfo.value().ship);
		if (motion.has_error())
		{
			Console::ConWarn(Hk::Err::ErrGetText(motion.error()));
		}
		const auto & [dir1, dir2] = motion.value();

		if (dir1.x > 5 || dir1.y > 5 || dir1.z > 5)
		{
			PrintUserCmdText(client, L"ERR Ship is moving");
			return;
		}

		auto [pos, rot] = Hk::Solar::GetLocation(playerInfo.value().ship, IdType::Ship).value();
		pos.x += 15;
		pos.y += 15;
		pos.z += 15;
		Hk::Player::RelocateClient(client, pos, rot);

		std::wstring wscMsg = global->config->stuckMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
		PrintLocalUserCmdText(client, wscMsg, 6000.0f);
	}

	/** @ingroup MiscCommands
	 * @brief Command to remove your current affiliation if applicable.
	 */
	void UserCmdDropRep(ClientId& client, [[maybe_unused]] const std::wstring& wscParam)
	{
		if (!global->config->enableDropRep)
		{
			PrintUserCmdText(client, L"Command Disabled");
			return;
		}

		auto repGroupNick = Hk::Ini::GetFromPlayerFile(client, L"rep_group");
		if (repGroupNick.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(repGroupNick.error()));
			return;
		}

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		const auto iCash = Hk::Player::GetCash(client);
		if (iCash.has_error())
		{
			PrintUserCmdText(client, L"ERR %s", Hk::Err::ErrGetText(iCash.error()).c_str());
			return;
		}

		if (global->config->repDropCost > 0 && iCash < global->config->repDropCost)
		{
			PrintUserCmdText(client, L"ERR Insufficient credits");
			return;
		}

		if (const auto repValue = Hk::Player::GetRep(client, repGroupNick.value()); repValue.has_error())
		{
			PrintUserCmdText(client, L"ERR %s", Hk::Err::ErrGetText(repValue.error()).c_str());
			return;
		}

		Hk::Player::SetRep(client, repGroupNick.value(), 0.599f);
		PrintUserCmdText(client, L"OK Reputation dropped, logout for change to take effect.");

		// Remove cash if we're charging for it.
		if (global->config->repDropCost > 0)
		{
			Hk::Player::RemoveCash(client, global->config->repDropCost);
		}
	}

	/** @ingroup MiscCommands
	 * @brief Roll a dice with the specified number of sides, or 6 is not specified.
	 */
	void UserCmdDice(ClientId& client, const std::wstring& wscParam)
	{
		const std::wstring charName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		int max = ToInt(GetParam(wscParam, ' ', 0));
		if (max <= 1)
			max = 6;

		const uint number = rand() % max + 1;
		std::wstring wscMsg = global->config->diceMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", charName);
		wscMsg = ReplaceStr(wscMsg, L"%number", std::to_wstring(number));
		wscMsg = ReplaceStr(wscMsg, L"%max", std::to_wstring(max));
		PrintLocalUserCmdText(client, wscMsg, 6000.0f);
	}

	/** @ingroup MiscCommands
	 * @brief Throw the dice and tell all players within 6 km
	 */
	void UserCmdCoin(ClientId& client, const std::wstring& wscParam)
	{
		const std::wstring charName = (const wchar_t*)Players.GetActiveCharacterName(client);

		const uint number = (rand() % 2);
		std::wstring wscMsg = global->config->coinMessage;
		wscMsg = ReplaceStr(wscMsg, L"%player", charName);
		wscMsg = ReplaceStr(wscMsg, L"%result", (number == 1) ? L"heads" : L"tails");
		PrintLocalUserCmdText(client, wscMsg, 6000.0f);
	}

	void UserCmdValue(ClientId& client, const std::wstring& wscParam)
	{
		auto shipValue = Hk::Player::GetShipValue(client);
		if (shipValue.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(shipValue.error()));
		}
		else
		{
			PrintUserCmdText(client, stows(fmt::format("{}", shipValue.value())));
		}
	}

	/** @ingroup MiscCommands
	 * @brief Activate or deactivate docking lights on your ship.
	 */
	void UserCmdLights(ClientId& client, const std::wstring& wscParam)
	{
		global->mapInfo[client].bLightsOn = !global->mapInfo[client].bLightsOn;
		SetLights(client, global->mapInfo[client].bLightsOn);
	}

	/** @ingroup MiscCommands
	 * @brief Disable/Enable your shields at will.
	 */
	void UserCmdShields(ClientId& client, const std::wstring& wscParam)
	{
		global->mapInfo[client].bShieldsDown = !global->mapInfo[client].bShieldsDown;
		PrintUserCmdText(client, L"Shields %s", global->mapInfo[client].bShieldsDown ? L"Disabled" : L"Enabled");
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/togglelights", L"", UserCmdLights, L""),
	    CreateUserCommand(L"/shields", L"", UserCmdShields, L""),
	    CreateUserCommand(L"/pos", L"", UserCmdPos, L""),
	    CreateUserCommand(L"/stuck", L"", UserCmdStuck, L""),
	    CreateUserCommand(L"/droprep", L"", UserCmdDropRep, L""),
	    CreateUserCommand(L"/dice", L"", UserCmdDice, L""),
	    CreateUserCommand(L"/coin", L"", UserCmdCoin, L""),
	    CreateUserCommand(L"/value", L"", UserCmdValue, L""),
	}};

	/** @} */ // End of user commands

	//! Smite all players in radar range
	void AdminCmdSmiteAll(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		const auto playerInfo = Hk::Admin::GetPlayerInfo(cmds->GetAdminName(), false);
		if (playerInfo.has_error() || !playerInfo.value().ship)
		{
			cmds->Print(L"ERR Not in space");
			return;
		}

		const bool bKillAll = cmds->ArgStr(1) == L"die";

		auto [fromShipPos, _] = Hk::Solar::GetLocation(playerInfo.value().ship, IdType::Ship).value();

		pub::Audio::Tryptich music;
		music.iDunno = 0;
		music.iDunno2 = 0;
		music.iDunno3 = 0;
		music.iMusicId = global->smiteMusicHash;
		pub::Audio::SetMusic(playerInfo.value().client, music);

		// For all players in system...
		struct PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			// Get the this player's current system and location in the system.
			ClientId client = playerData->iOnlineId;
			if (client == playerInfo.value().client)
				continue;

			if (playerInfo.value().iSystem != Hk::Player::GetSystem(client).value())
				continue;

			uint ship = Hk::Player::GetShip(client).value();

			auto [playerPosition, _] = Hk::Solar::GetLocation(ship, IdType::Ship).value();
			// Is player within scanner range (15K) of the sending char.
			if (Hk::Math::Distance3D(playerPosition, fromShipPos) > 14999)
				continue;

			pub::Audio::SetMusic(client, music);

			global->mapInfo[client].bShieldsDown = true;

			if (bKillAll)
			{
				if (const auto obj = Hk::Client::GetInspect(client); obj.has_value())
				{
					Hk::Admin::LightFuse(reinterpret_cast<IObjRW*>(obj.value()), CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
				}
			}
		}

		cmds->Print(L"OK");
		return;
	}

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (!cmds->IsPlayer())
			return false;

		if (wscCmd == L"smiteall")
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmdSmiteAll(cmds);
			return true;
		}
		return false;
	}
} // namespace Plugins::MiscCommands

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::MiscCommands;

// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(repDropCost), field(stuckMessage), field(diceMessage), field(coinMessage), field(smiteMusicId), field(enableDropRep))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Misc Commands");
	pi->shortName("MiscCommands");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
