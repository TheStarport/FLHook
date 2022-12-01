/**
 * @date Unknown
 * @author ||KOS||Acid (Ported by Raikkonen)
 * @defgroup KillCounter Kill Counter
 * @brief
 * This plugin is used to count pvp kills and save them in the player file. Vanilla doesn't do this by default.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - kills {clientId} - Shows the pvp kills for a player if a client id is specified, or if not, the player who typed it.
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

	void UserCmd_Help(const uint& iClientID, const std::wstring_view& wscParam)
	{
		PrintUserCmdText(iClientID, L"/kills <player name>");
		PrintUserCmdText(iClientID, L"/kills$ <player id>");
	}

	/** @ingroup KillCounter
	 * @brief Called when a player types "/kills".
	 */
	void UserCmd_Kills(const uint& iClientID, const std::wstring_view& wscParam)
	{
		std::wstring wscClientID = GetParam(wscParam, ' ', 0);
		int iNumKills;
		std::wstring mainrank;
		std::list<std::wstring> lstLines;
		int count;
		if (!wscClientID.length())
		{
			std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			ReadCharFile(wscCharname, lstLines);
			pub::Player::GetNumKills(iClientID, iNumKills);
			PrintUserCmdText(iClientID, L"PvP kills: %i", iNumKills);
			for (auto& str : lstLines)
			{
				if (!ToLower((str)).find(L"ship_type_killed"))
				{
					uint iShipArchID = ToInt(GetParam(str, '=', 1).c_str());
					count = ToInt(GetParam(str, ',', 1).c_str());
					iNumKills += count;
					Archetype::Ship* ship = Archetype::GetShip(iShipArchID);
					if (!ship)
						continue;
					PrintUserCmdText(iClientID, L"NPC kills:  %s %i", GetWStringFromIDS(ship->iIdsName).c_str(), count);
				}
			}
			int rank;
			pub::Player::GetRank(iClientID, rank);
			PrintUserCmdText(iClientID, L"Total kills: %i", iNumKills);
			PrintUserCmdText(iClientID, L"Level: %i", rank);
			return;
		}

		uint iClientIDPlayer = GetClientIdFromCharname(wscClientID);
		if (iClientIDPlayer == -1)
		{
			PrintUserCmdText(iClientID, L"ERROR player not found");
			return;
		}
		ReadCharFile(wscClientID, lstLines);
		pub::Player::GetNumKills(iClientIDPlayer, iNumKills);
		PrintUserCmdText(iClientID, L"PvP kills: %i", iNumKills);
		for (auto& str : lstLines)
		{
			if (!ToLower((str)).find(L"ship_type_killed"))
			{
				uint iShipArchID = ToInt(GetParam(str, '=', 1));
				count = ToInt(GetParam(str, ',', 1).c_str());
				iNumKills += count;
				Archetype::Ship* ship = Archetype::GetShip(iShipArchID);
				if (!ship)
					continue;
				PrintUserCmdText(iClientID, L"NPC kills:  %s %i", GetWStringFromIDS(ship->iIdsName).c_str(), count);
			}
		}
		int rank;
		pub::Player::GetRank(iClientID, rank);
		PrintUserCmdText(iClientID, L"Total kills: %i", iNumKills);
		PrintUserCmdText(iClientID, L"Level: %i", rank);
	}

	/** @ingroup KillCounter
	 * @brief Hook on ShipDestroyed. Increments the number of kills of a player if there is one.
	 */
	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
	{
		if (iKill == 1)
		{
			const CShip* cShip = CShipFromShipDestroyed(ecx);

			if (const uint iClientID = cShip->GetOwnerPlayer())
			{
				const DamageList* dmg = *_dmg;

				if (const uint killerId = dmg->get_cause() == DamageCause::Unknown ? GetClientIDByShip(ClientInfo[iClientID].dmgLast.get_inflictor_id())
					: GetClientIDByShip(dmg->get_inflictor_id()); killerId && (iClientID != killerId))
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
	    CreateUserCommand(L"/kills", L"{clientId}", UserCmd_Kills, L"Displays how many pvp kills you have."),
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