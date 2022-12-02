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

	void UserCmd_Help(ClientId& client, const std::wstring_view& wscParam)
	{
		PrintUserCmdText(client, L"/kills <player name>");
		PrintUserCmdText(client, L"/kills$ <player id>");
	}

	/** @ingroup KillCounter
	 * @brief Called when a player types "/kills".
	 */
	void UserCmd_Kills(ClientId& client, const std::wstring_view& wscParam)
	{
		std::wstring wscClientId = GetParam(wscParam, ' ', 0);
		int iNumKills;
		std::wstring mainrank;
		std::list<std::wstring> lstLines;
		int count;
		if (!wscClientId.length())
		{
			std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);
			ReadCharFile(wscCharname, lstLines);
			pub::Player::GetNumKills(client, iNumKills);
			PrintUserCmdText(client, L"PvP kills: %i", iNumKills);
			for (auto& str : lstLines)
			{
				if (!ToLower((str)).find(L"ship_type_killed"))
				{
					uint shipArchId = ToInt(GetParam(str, '=', 1).c_str());
					count = ToInt(GetParam(str, ',', 1).c_str());
					iNumKills += count;
					Archetype::Ship* ship = Archetype::GetShip(shipArchId);
					if (!ship)
						continue;
					PrintUserCmdText(client, L"NPC kills:  %s %i", GetWStringFromIdS(ship->iIdsName).c_str(), count);
				}
			}
			int rank;
			pub::Player::GetRank(client, rank);
			PrintUserCmdText(client, L"Total kills: %i", iNumKills);
			PrintUserCmdText(client, L"Level: %i", rank);
			return;
		}

		ClientId clientPlayer = GetClientIdFromCharname(wscClientId);
		if (clientPlayer == -1)
		{
			PrintUserCmdText(client, L"ERROR player not found");
			return;
		}
		ReadCharFile(wscClientId, lstLines);
		pub::Player::GetNumKills(clientPlayer, iNumKills);
		PrintUserCmdText(client, L"PvP kills: %i", iNumKills);
		for (auto& str : lstLines)
		{
			if (!ToLower((str)).find(L"ship_type_killed"))
			{
				uint shipArchId = ToInt(GetParam(str, '=', 1));
				count = ToInt(GetParam(str, ',', 1).c_str());
				iNumKills += count;
				Archetype::Ship* ship = Archetype::GetShip(shipArchId);
				if (!ship)
					continue;
				PrintUserCmdText(client, L"NPC kills:  %s %i", GetWStringFromIdS(ship->iIdsName).c_str(), count);
			}
		}
		int rank;
		pub::Player::GetRank(client, rank);
		PrintUserCmdText(client, L"Total kills: %i", iNumKills);
		PrintUserCmdText(client, L"Level: %i", rank);
	}

	/** @ingroup KillCounter
	 * @brief Hook on ShipDestroyed. Increments the number of kills of a player if there is one.
	 */
	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
	{
		if (iKill == 1)
		{
			const CShip* cShip = CShipFromShipDestroyed(ecx);

			if (ClientId client = cShip->GetOwnerPlayer())
			{
				const DamageList* dmg = *_dmg;

				if (const uint killerId = dmg->get_cause() == DamageCause::Unknown ? GetClientIdByShip(ClientInfo[client].dmgLast.get_inflictor_id())
					: GetClientIdByShip(dmg->get_inflictor_id()); killerId && (client != killerId))
				{
					int iNumKills;
					pub::Player::GetNumKills(killerId, iNumKills);
					iNumKills++;
					pub::Player::SetNumKills(killerId, iNumKills);
				}
			}
		}
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/kills", L"{client}", UserCmd_Kills, L"Displays how many pvp kills you have."),
	}};
} // namespace Plugins::KillCounter

using namespace Plugins::KillCounter;

DefaultDllMain()

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Kill Counter");
	pi->shortName("killcounter");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
}