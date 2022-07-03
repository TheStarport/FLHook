// Kill Counter Plugin
// Originally by ||KOS||Acid
// https://sourceforge.net/projects/kosacid/files/

#include "Main.h"

namespace Plugins::KillCounter
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void UserCmd_Help(const uint& iClientID, const std::wstring_view& wscParam)
	{
		PrintUserCmdText(iClientID, L"/kills <player name>");
		PrintUserCmdText(iClientID, L"/kills$ <player id>");
	}

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
			HkReadCharFile(wscCharname, lstLines);
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
					PrintUserCmdText(iClientID, L"NPC kills:  %s %i", HkGetWStringFromIDS(ship->iIdsName).c_str(), count);
				}
			}
			int rank;
			pub::Player::GetRank(iClientID, rank);
			PrintUserCmdText(iClientID, L"Total kills: %i", iNumKills);
			PrintUserCmdText(iClientID, L"Level: %i", rank);
			return;
		}

		uint iClientIDPlayer = HkGetClientIdFromCharname(wscClientID);
		if (iClientIDPlayer == -1)
		{
			PrintUserCmdText(iClientID, L"ERROR player not found");
			return;
		}
		HkReadCharFile(wscClientID, lstLines);
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
				PrintUserCmdText(iClientID, L"NPC kills:  %s %i", HkGetWStringFromIDS(ship->iIdsName).c_str(), count);
			}
		}
		int rank;
		pub::Player::GetRank(iClientID, rank);
		PrintUserCmdText(iClientID, L"Total kills: %i", iNumKills);
		PrintUserCmdText(iClientID, L"Level: %i", rank);
	}

	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
	{
		if (iKill == 1)
		{
			const CShip* cShip = HkCShipFromShipDestroyed(ecx);

			if (const uint iClientID = cShip->GetOwnerPlayer())
			{
				const DamageList* dmg = *_dmg;

				if (const uint killerId = dmg->get_cause() == DamageCause::Unknown ? HkGetClientIDByShip(ClientInfo[iClientID].dmgLast.get_inflictor_id())
					: HkGetClientIDByShip(dmg->get_inflictor_id()); killerId && (iClientID != killerId))
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
	    CreateUserCommand(L"/kills", L"", UserCmd_Kills, L""),
	}};
} // namespace Plugins::KillCounter

using namespace Plugins::KillCounter;

DefaultDllMain()

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Kill Counter");
	pi->shortName("killcounter");
	pi->mayPause(false);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
}