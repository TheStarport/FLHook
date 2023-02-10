/**
 * @date Unknown
 * @author ||KOS||Acid (Ported by Raikkonen)
 * @defgroup KillTracker Kill Tracker
 * @brief
 * This plugin is used to count pvp kills and save them in the player file. Vanilla doesn't do this by default.
 * Also keeps track of damage taken between players, prints greatest damage contributor.
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

IMPORT uint g_DmgTo;

namespace Plugins::KillTracker
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void UserCmd_Help(ClientId& client, [[maybe_unused]] const std::wstring& param)
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

	/** @ingroup KillTracker
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

	/** @ingroup KillTracker
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

	void AddDamageEntry(const DamageList** damageList, const ushort& subObjId, const float& newHitPoints, [[maybe_unused]] const enum DamageEntry::SubObjFate& fate)
	{
		if (global->config->enableDamageTracking && g_DmgTo && subObjId == 1)
		{
			if (const auto& inflictor = (*damageList)->inflictorPlayerId; 
				inflictor && g_DmgTo && inflictor != g_DmgTo)
			{
				float hpLost = global->lastPlayerHealth[g_DmgTo] - newHitPoints;
				global->damageArray[inflictor][g_DmgTo] += hpLost;
			}
			global->lastPlayerHealth[g_DmgTo] = newHitPoints;
		}
	}

	void clearDamageTaken(ClientId& victim)
	{
		for (auto& damageEntry : global->damageArray[victim])
			damageEntry = 0.0f;
	}

	void clearDamageDone(ClientId& inflictor)
	{
		for (int i = 1; i < MaxClientId + 1; i++)
			global->damageArray[i][inflictor] = 0.0f;
	}

	void SendDeathMessage([[maybe_unused]] const std::wstring& message, const SystemId& system, ClientId& clientVictim, ClientId& clientKiller)
	{
		if (global->config->enableDamageTracking && clientVictim && clientKiller)
		{
			uint greatestInflictorId = 0;
			float greatestDamageDealt = 0.0f;
			float totalDamageTaken = 0.0f;
			for (uint inflictorIndex = 1; inflictorIndex < global->damageArray[0].size(); inflictorIndex++)
			{
				float damageDealt = global->damageArray[inflictorIndex][clientVictim];
				totalDamageTaken += damageDealt;
				if (damageDealt > greatestDamageDealt)
				{
					greatestDamageDealt = damageDealt;
					greatestInflictorId = inflictorIndex;
				}
			}
			clearDamageTaken(clientVictim);
			if (totalDamageTaken == 0.0f || greatestInflictorId == 0)
				return;
			std::wstring victimName = Hk::Client::GetCharacterNameByID(clientVictim).value();
			std::wstring greatestInflictorName = Hk::Client::GetCharacterNameByID(greatestInflictorId).value();
			std::wformat_args templateArgs = std::make_wformat_args(victimName, greatestInflictorName, static_cast<uint>(ceil((greatestDamageDealt/totalDamageTaken) * 100)));
			std::wstring greatestDamageMessage = std::vformat(global->config->deathDamageTemplate, templateArgs);

			greatestDamageMessage = Hk::Message::FormatMsg(MessageColor::Orange, MessageFormat::Normal, greatestDamageMessage);
			Hk::Message::FMsgS(system, greatestDamageMessage);
		}
	}

	void Disconnect(ClientId& client, [[maybe_unused]] EFLConnection conn)
	{
		if (global->config->enableDamageTracking)
		{
			clearDamageTaken(client);
			clearDamageDone(client);
		}
	}

	void PlayerLaunch([[maybe_unused]]ShipId shipId, ClientId& client)
	{
		if (global->config->enableDamageTracking)
		{
			clearDamageTaken(client);
			clearDamageDone(client);
			if (Hk::Client::IsValidClientID(client))
			{
				float maxHp = Archetype::GetShip(Hk::Player::GetShipID(client).value())->fHitPoints;
				global->lastPlayerHealth[client] = maxHp * Players[client].fRelativeHealth;
			}
		}
	}

	void CharacterSelect([[maybe_unused]] CHARACTER_ID const& cid, ClientId& client)
	{
		if (global->config->enableDamageTracking)
		{
			clearDamageTaken(client);
			clearDamageDone(client);
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

		for (auto& subArray : global->damageArray)
			subArray.fill(0.0f);
	}
}

using namespace Plugins::KillTracker;

REFL_AUTO(type(Config), field(enableNPCKillOutput), field(deathDamageTemplate), field(enableDamageTracking))

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Kill Tracker");
	pi->shortName("killtracker");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IEngine__AddDamageEntry, &AddDamageEntry, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMessage);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &Disconnect);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect);
}