/**
 * @date Unknown
 * @author ||KOS||Acid (Ported by Raikkonen)
 * @defgroup KillCounter Kill Counter
 * @brief
 * This plugin is used to count pvp kills and save them in the player file. Vanilla doesn't do this by default.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - kills {client} - Shows the pvp kills for a player if a client id is specified, or if not, the player who typed it.
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
 * This plugin has no dependencies.
 */

#include "Main.h"

namespace Plugins::KillCounter
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void UserCmd_Help(ClientId& client, const std::wstring& wscParam)
	{
		PrintUserCmdText(client, L"/kills <player name>");
	}

	void PrintNPCKills(uint client, std::wstring& charFile, int& numKills)
	{
		for (const auto lines = Hk::Player::ReadCharFile(charFile); auto& str : lines.value())
		{
			if (std::wstring lineName = Trim(GetParam(str, '=', 0)); lineName == L"ship_type_killed")
			{
				uint shipArchId = ToUInt(GetParam(str, '=', 1));
				int count = ToInt(GetParam(str, ',', 1).c_str());
				numKills += count;
				const Archetype::Ship* ship = Archetype::GetShip(shipArchId);
				if (!ship)
					continue;
				PrintUserCmdText(client, std::format(L"NPC kills:  {} {}", Hk::Message::GetWStringFromIdS(ship->iIdsName), count));
			}
		}
		PrintUserCmdText(client, std::format(L"Total kills: {}", numKills));
	}

	/** @ingroup KillCounter
	 * @brief Called when a player types "/kills".
	 */
	void UserCmd_Kills(ClientId& client, const std::wstring& wscParam)
	{
		std::wstring targetCharName = GetParam(wscParam, ' ', 0);
		uint clientId;
		
		if (!targetCharName.empty())
		{
			const auto clientPlayer = Hk::Client::GetClientIdFromCharName(targetCharName);
			if (clientPlayer.has_error())
			{
				PrintUserCmdText(client, Hk::Err::ErrGetText(clientPlayer.error()));
				return;
			}
			clientId = clientPlayer.value();
		}
		else
		{
			clientId = client;
		}

		int numKills = Hk::Player::GetPvpKills(client).value();
		PrintUserCmdText(client, std::format(L"PvP kills: {}", numKills));
		if (global->config->enableNPCKillOutput)
		{
			std::wstring printCharname = Hk::Client::GetCharacterNameByID(clientId).value();
			PrintNPCKills(client, printCharname, numKills);
		}
		int rank = Hk::Player::GetRank(client).value();
		PrintUserCmdText(client, std::format(L"Level: {}", rank));

	}

	/** @ingroup KillCounter
	 * @brief Hook on ShipDestroyed. Increments the number of kills of a player if there is one.
	 */
	void ShipDestroyed(DamageList** _dmg, const DWORD** ecx, const uint& kill)
	{
		if (kill == 1)
		{
			const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);

			if (ClientId client = cShip->GetOwnerPlayer())
			{
				const DamageList* dmg = *_dmg;
				const auto killerId = Hk::Client::GetClientIdByShip(dmg->get_cause() == DamageCause::Unknown ? ClientInfo[client].dmgLast.get_inflictor_id() : dmg->get_inflictor_id());

				if (killerId.has_value() && killerId.value() != client)
				{
					Hk::Player::IncrementPvpKills(killerId.value());
				}
			}
		}
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/kills", L"[playerName]", UserCmd_Kills, L"Displays how many pvp kills you (or player you named) have."),
	}};

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
	}
}

using namespace Plugins::KillCounter;

REFL_AUTO(type(Config), field(enableNPCKillOutput))

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Kill Counter");
	pi->shortName("killcounter");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
}