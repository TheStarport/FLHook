// Kill Counter Plugin
// Originally by ||KOS||Acid
// https://sourceforge.net/projects/kosacid/files/

#include "Main.h"

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}

EXPORT void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{

	if (fdwReason == DLL_PROCESS_ATTACH)
		LoadSettings();

	return true;
}

EXPORT void UserCmd_Help(uint iClientID, const std::wstring& wscParam)
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
		for ( auto& str : lstLines)
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
	for ( auto& str : lstLines)
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

void __stdcall ShipDestroyed(DamageList* _dmg, char* szECX, uint iKill)
{
	returncode = DEFAULT_RETURNCODE;

	char* szP;
	memcpy(&szP, szECX + 0x10, 4);
	uint iClientID;
	memcpy(&iClientID, szP + 0xB4, 4);
	if (iClientID)
	{
		DamageList dmg;
		if (!dmg.get_cause())
			dmg = ClientInfo[iClientID].dmgLast;
		uint iClientIDKiller = HkGetClientIDByShip(dmg.get_inflictor_id());
		if (iClientIDKiller && (iClientID != iClientIDKiller))
		{
			int iNumKills;
			pub::Player::GetNumKills(iClientIDKiller, iNumKills);
			iNumKills++;
			pub::Player::SetNumKills(iClientIDKiller, iNumKills);
		}
	}
}

USERCMD UserCmds[] =
{
	{ L"/kills", UserCmd_Kills},
};

EXPORT bool UserCmd_Process(uint iClientID, const std::wstring& wscCmd)
{
	std::wstring wscCmdLower = ToLower(wscCmd);
	for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++)
	{
		if (wscCmdLower.find(ToLower(UserCmds[i].wszCmd)) == 0)
		{
			std::wstring wscParam = L"";
			if (wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
			}
			UserCmds[i].proc(iClientID, wscParam);
			returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command, return immediatly
			return true;
		}
	}
	returncode = DEFAULT_RETURNCODE; // we did not handle the command, so let other plugins or FLHook kick in
	return false;
}

EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "Kill Counter Plugin";
	p_PI->sShortName = "killcounter";
	p_PI->bMayPause = false;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Process, PLUGIN_UserCmd_Process, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Help, PLUGIN_UserCmd_Help, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&ShipDestroyed, PLUGIN_ShipDestroyed, 0));
	return p_PI;
}
