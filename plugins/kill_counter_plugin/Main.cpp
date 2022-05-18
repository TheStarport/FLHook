// Kill Counter Plugin
// Originally by ||KOS||Acid
// https://sourceforge.net/projects/kosacid/files/

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;
std::list<INISECTIONVALUE> lstRanks;

EXPORT void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, L"/kills <player name>");
	PrintUserCmdText(iClientID, L"/kills$ <player id>");
}

void UserCmd_Kills(uint iClientID, const std::wstring& wscParam)
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

void __stdcall ShipDestroyed(DamageList** _dmg, DWORD** ecx, uint& iKill)
{
	if (iKill == 1)
	{
		CShip* cship = (CShip*)(*ecx)[4];
		uint iClientID = cship->GetOwnerPlayer();
		DamageList* dmg = *_dmg;
		if (iClientID)
		{
			if (!dmg->get_cause())
				dmg = &ClientInfo[iClientID].dmgLast;
			uint iClientIDKiller = HkGetClientIDByShip(dmg->get_inflictor_id());
			if (iClientIDKiller && (iClientID != iClientIDKiller))
			{
				int iNumKills;
				pub::Player::GetNumKills(iClientIDKiller, iNumKills);
				iNumKills++;
				pub::Player::SetNumKills(iClientIDKiller, iNumKills);
			}
		}
	}
}

USERCMD UserCmds[] = {
	{ L"/kills", UserCmd_Kills },
};

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd)
{
	DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Kill Counter");
	pi->shortName("killcounter");
	pi->mayPause(false);
	pi->mayUnload(true);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
}