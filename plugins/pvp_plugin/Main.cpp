// PvP Betting Plugin
// By Raikkonen

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Includes
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Main.h"

// Structure to hold a bet
struct BET
{
	uint iClientID;
	uint iClientID2;
	uint amount;
	bool accepted;
};

std::map<std::wstring, BET> duels;

// Structure to hold a contestant
struct contestant
{
	bool accepted;
	bool loser;
};

// Structure to hold an FFA event
struct FFA {
	std::map<uint, contestant> contestants;
	uint system;
	uint buyin;
};

std::map<uint, FFA> ffas; // uint is system

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Removes any bets with this iClientID

void removeBet(uint iClientID) {
	std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	for (std::map<std::wstring, BET>::iterator iter = duels.begin(); iter != duels.end();)
	{
		if (iter->second.iClientID == iClientID || iter->second.iClientID2 == iClientID) {
			PrintUserCmdText(iter->second.iClientID, L"Duel cancelled.");
			PrintUserCmdText(iter->second.iClientID2, L"Duel cancelled.");
			duels.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void handleFFA(uint iSystem, uint iClientIDVictim) {

	if (ffas.find(iSystem) != ffas.end())
	{
		std::wstring victim = (const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim);

		if (ffas[iSystem].contestants[iClientIDVictim].accepted) {
			if (!ffas[iSystem].contestants[iClientIDVictim].loser) {



				if (ffas[iSystem].contestants.find(iClientIDVictim) != ffas[iSystem].contestants.end()) {
					ffas[iSystem].contestants[iClientIDVictim].loser = true;
					std::wstring msg = victim + L" has been knocked out the FFA.";
					PrintLocalUserCmdText(iClientIDVictim, msg, 100000);
				}

				// Is the FFA over?
				int count = 0;
				uint contestantid;
				for (std::map<uint, contestant>::iterator iter = ffas[iSystem].contestants.begin();
					iter != ffas[iSystem].contestants.end();
					iter++)
				{
					if (iter->second.loser == false) {
						count++;
						contestantid = iter->first;
					}
				}
				// Has the FFA been won?
				if (count == 1) 
				{
					// Get winner
					std::wstring winner = (const wchar_t*)Players.GetActiveCharacterName(contestantid);

					// calculate winning credits
					uint winnings = ffas[iSystem].buyin * ffas[iSystem].contestants.size();

					// Pay winner
					HkAddCash(winner, winnings);

					// Announce winner
					std::wstring msg = winner + L" has won the FFA and receives " + stows(itos(winnings)) + L" credits.";
					PrintLocalUserCmdText(contestantid, msg, 100000);

					// Delete event
					ffas.erase(iSystem);
				}
			}
		}
	}
}

// This method is called when a player types /ffa in an attempt to start a pvp event

bool UserCmd_StartFFA(uint iClientID, const std::wstring& wscCmd, const std::wstring& wscParam, const wchar_t* usage)
{
	HK_ERROR err;

	// Get buyin cost
	int amount = ToInt(GetParam(wscParam, ' ', 0));

	// No amount check
	if (amount <= 0)
	{
		PrintUserCmdText(iClientID, usage);
		return true;
	}

	std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// Check the player can afford it
	int iCash = 0;
	if ((err = HkGetCash(charname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return true;
	}
	if (amount > 0 && iCash < amount)
	{
		PrintUserCmdText(iClientID, L"You don't have enough credits to create this FFA.");
		return true;
	}

	// Get the player's current system and location in the system.
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);

	// Look in FFA map, is an ffa happening in this system already?
	// If system doesn't have an ongoing ffa
	if (ffas.find(iSystemID) == ffas.end())
	{
		// Get a list of other players in the system
		// Add them and the player into the ffa map
		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			// Get the this player's current system
			uint iClientID2 = HkGetClientIdFromPD(pPD);
			uint iClientSystemID = 0;
			pub::Player::GetSystem(iClientID2, iClientSystemID);
			if (iSystemID != iClientSystemID)
				continue;
			// Add them to the contestants map
			ffas[iSystemID].contestants[iClientID2].loser = false;
			if (iClientID == iClientID2) {
				ffas[iSystemID].contestants[iClientID2].accepted = true;			
			}
			else {
				ffas[iSystemID].contestants[iClientID2].accepted = false;
				std::wstring msg = charname + L" has started a Free-For-All tournament. Cost to enter is " + std::to_wstring(amount) + L" credits. Type \"/acceptffa\" to enter.";
				PrintUserCmdText(iClientID2, msg);
			}
		}
		// Are there any other players in this system?
		if (ffas[iSystemID].contestants.size() > 0) {
			PrintUserCmdText(iClientID, L"Challenge issued. Waiting for others to accept.");
			ffas[iSystemID].buyin = amount;
		}
		else {
			ffas.erase(iSystemID);
			PrintUserCmdText(iClientID, L"There are no other players in this system.");
		}
	}
	// Already an FFA
	else
	{
		PrintUserCmdText(iClientID, L"There is an FFA already happening in this system.");
	}
	return true;
}

// This method is called when a player types /acceptffa

bool UserCmd_AcceptFFA(uint iClientID, const std::wstring& wscCmd, const std::wstring& wscParam, const wchar_t* usage)
{
	// Get the player's current system and location in the system.
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);

	if (ffas.find(iSystemID) == ffas.end())
	{
		PrintUserCmdText(iClientID, L"There isn't an FFA in this system. Use /ffa to create one.");
		return true;
	}
	else {
		HK_ERROR err;

		// Get charname
		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Check the player can afford it
		int iCash = 0;
		if ((err = HkGetCash(charname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return true;
		}
		if (ffas[iSystemID].buyin > 0 && iCash < ffas[iSystemID].buyin)
		{
			PrintUserCmdText(iClientID, L"You don't have enough credits to join this FFA.");
			return true;
		}

		// Accept
		if (ffas[iSystemID].contestants[iClientID].accepted == false) {
			ffas[iSystemID].contestants[iClientID].accepted = true;
			ffas[iSystemID].contestants[iClientID].loser = false;
			PrintUserCmdText(iClientID, stows(itos(ffas[iSystemID].buyin)) + L" has been deducted from your Neural Net account.");
			std::wstring msg = charname + L" has joined the FFA.";
			PrintLocalUserCmdText(iClientID, msg, 100000);

			// Deduct cash
			HkAddCash(charname, -(ffas[iSystemID].buyin));
			return true;
		}
		else {
			PrintUserCmdText(iClientID, L"You have already accepted the FFA.");
			return true;
		}
	}
}

// This method is called when a player types /duel in an attempt to start a duel

bool UserCmd_Duel(uint iClientID, const std::wstring& wscCmd, const std::wstring& wscParam, const wchar_t* usage)
{
	// Get char name
	std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
	std::wstring targetcharname = GetParam(wscParam, ' ', 0);
	uint amount = ToInt(GetParam(wscParam, ' ', 1));

	if (!targetcharname.length() || !amount)
	{
		PrintUserCmdText(iClientID, usage);
		return true;
	}

	// Target is player?
	if (charname == targetcharname) {
		PrintUserCmdText(iClientID, L"You can't duel yourself.");
		return true;
	}

	// Does the target exist?
	if (!HkGetAccountByCharname(targetcharname))
	{
		PrintUserCmdText(iClientID, L"Player doesn't exist.");
		return true;
	}

	// Is the target online?
	uint iToClientID = HkGetClientIdFromCharname(targetcharname);
	if (iToClientID == -1)
	{
		PrintUserCmdText(iClientID, L"Player is offline.");
		return true;
	}

	HK_ERROR err;

	// Check the player can afford it
	int iCash = 0;
	if ((err = HkGetCash(charname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return true;
	}
	if (amount > 0 && iCash < amount)
	{
		PrintUserCmdText(iClientID, L"You don't have enough credits to issue this challenge.");
		return true;
	}

	// Do either players already have a duel?
	for (std::map<std::wstring, BET>::iterator iter = duels.begin();
		iter != duels.end();
		iter++)
	{
		// Target already has a bet
		if ((iter->second.iClientID == iToClientID || iter->second.iClientID2 == iToClientID)) {
			PrintUserCmdText(iClientID, L"This player already has an ongoing duel.");
			return true;
		}
		// Player already has a bet
		if ((iter->second.iClientID == iClientID || iter->second.iClientID2 == iClientID)) {
			PrintUserCmdText(iClientID, L"You already have an ongoing duel. Type /cancel");
			return true;
		}
	}

	// Create duel
	duels[targetcharname].iClientID = iClientID;
	duels[targetcharname].iClientID2 = iToClientID;
	duels[targetcharname].amount = amount;
	duels[targetcharname].accepted = false;

	std::wstring msg = charname + L" has challenged " + targetcharname + L" to a duel for " + std::to_wstring(duels[targetcharname].amount) + L" credits.";
	PrintLocalUserCmdText(iClientID, msg, 10000);
	PrintUserCmdText(iToClientID, L"Type \"/acceptduel\" to accept.");

	return true;
}

bool UserCmd_AcceptDuel(uint iClientID, const std::wstring& wscCmd, const std::wstring& wscParam, const wchar_t* usage)
{
	std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// If player doesn't have an ongoing bet
	if (duels.find(charname) == duels.end())
	{
		PrintUserCmdText(iClientID, L"You don't have an ongoing duel.");
	}
	// Player has a bet
	else
	{
		if (duels[charname].accepted) {
			PrintUserCmdText(iClientID, L"You have already accepted the challenge.");
		}
		else {

			HK_ERROR err;

			// Check the player can afford it
			int iCash = 0;
			if ((err = HkGetCash(charname, iCash)) != HKE_OK)
			{
				PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
				return true;
			}
			if (duels[charname].amount > 0 && iCash < duels[charname].amount)
			{
				PrintUserCmdText(iClientID, L"You don't have enough credits to accept this challenge");
				return true;
			}

			duels[charname].accepted = true;
			std::wstring msg = charname + L" has accepted a duel with " + (const wchar_t*)Players.GetActiveCharacterName(duels[charname].iClientID2) + L" for " + std::to_wstring(duels[charname].amount) + L" credits.";
			PrintLocalUserCmdText(iClientID, msg, 10000);
		}
	}
	return true;
}

bool UserCmd_Cancel(uint iClientID, const std::wstring& wscCmd, const std::wstring& wscParam, const wchar_t* usage)
{
	removeBet(iClientID);
	return true;
}

/** Clean up when a client disconnects */
void ClearClientInfo(uint iClientID)
{
	returncode = DEFAULT_RETURNCODE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Client command processing
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

USERCMD UserCmds[] =
{
	{ L"/acceptduel", UserCmd_AcceptDuel, L"Usage: /acceptduel" },
	{ L"/acceptduel*", UserCmd_AcceptDuel, L"Usage: /acceptduel" },
	{ L"/acceptffa", UserCmd_AcceptFFA, L"Usage: /acceptffa" },
	{ L"/acceptffa*", UserCmd_AcceptFFA, L"Usage: /acceptffa" },
	{ L"/cancelbet", UserCmd_Cancel, L"Usage: /cancel" },
	{ L"/cancelbet*", UserCmd_Cancel, L"Usage: /cancel" },
	{ L"/duel", UserCmd_Duel, L"Usage: /duel <charname> <amount>" },
	{ L"/duel*", UserCmd_Duel, L"Usage: /duel <charname> <amount>" },
	{ L"/ffa", UserCmd_StartFFA, L"Usage: /ffa <amount>" },
	{ L"/ffa*", UserCmd_StartFFA, L"Usage: /ffa <amount>" }
};

/**
This function is called by FLHook when a user types a chat string. We look at the
string they've typed and see if it starts with one of the above commands. If it
does we try to process it.
*/
bool UserCmd_Process(uint iClientID, const std::wstring& wscCmd)
{
	returncode = DEFAULT_RETURNCODE;

	std::wstring wscCmdLineLower = ToLower(wscCmd);

	// If the chat string does not match the USER_CMD then we do not handle the
	// command, so let other plugins or FLHook kick in. We require an exact match
	for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++)
	{

		if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0)
		{
			// Extract the parameters string from the chat string. It should
			// be immediately after the command and a space.
			std::wstring wscParam = L"";
			if (wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
			}

			// Dispatch the command to the appropriate processing function.
			if (UserCmds[i].proc(iClientID, wscCmd, wscParam, UserCmds[i].usage))
			{
				// We handled the command tell FL hook to stop processing this
				// chat string.
				returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command, return immediatly
				return true;
			}
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hooks
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int __cdecl Dock_Call(unsigned int const& iShip, unsigned int const& iDockTarget, int iCancel, enum DOCK_HOST_RESPONSE response)
{
	returncode = DEFAULT_RETURNCODE;
	uint iClientID = HkGetClientIDByShip(iShip);
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	handleFFA(iSystemID, iClientID);
	removeBet(iClientID);
	return 0;
}

void __stdcall DisConnect(unsigned int iClientID, enum  EFLConnection state)
{
	returncode = DEFAULT_RETURNCODE;
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	handleFFA(iSystemID, iClientID);
	removeBet(iClientID);
}

void __stdcall CharacterSelect(struct CHARACTER_ID const& charId, unsigned int iClientID)
{
	returncode = DEFAULT_RETURNCODE;
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	handleFFA(iSystemID, iClientID);
	removeBet(iClientID);
}

// Hook for ship distruction. It's easier to hook this than the PlayerDeath one.
void SendDeathMsg(const std::wstring& wscMsg, uint iSystem, uint iClientIDVictim, uint iClientIDKiller)
{
	returncode = DEFAULT_RETURNCODE;

	// Bets
	for (std::map<std::wstring, BET>::iterator iter = duels.begin(); iter != duels.end();)
	{
		// If we get a match on the betting map
		if ((iter->second.iClientID == iClientIDKiller && iter->second.iClientID2 == iClientIDVictim) || (iter->second.iClientID == iClientIDVictim && iter->second.iClientID2 == iClientIDKiller)) {
			if (iter->second.accepted) {
				// Remove the bet
				duels.erase(iter);
				// Get player names
				std::wstring victim = (const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim);
				std::wstring killer = (const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller);
				// Prepare and send message
				std::wstring msg = killer + L" has won a duel against " + victim + L" for " + std::to_wstring(iter->second.amount) + L" credits.";
				PrintLocalUserCmdText(iClientIDKiller, msg, 10000);
				// Change cash
				HkAddCash(killer, iter->second.amount);
				HkAddCash(victim, -(iter->second.amount));
			}
			else
			{
				iter++;
			}
		}
		else
		{
			iter++;
		}
	}

	//FFA
	handleFFA(iSystem, iClientIDVictim);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Loading Settings
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;

}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));
	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		if (set_scCfgFile.length() > 0)
			LoadSettings();
	}
	else if (fdwReason == DLL_PROCESS_DETACH)
	{
	}
	return true;
}

/// Hook will call this function after calling a plugin function to see if we the
/// processing to continue
EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Functions to hook
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "PvP";
	p_PI->sShortName = "pvp";
	p_PI->bMayPause = true;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;

	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&ClearClientInfo, PLUGIN_ClearClientInfo, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Process, PLUGIN_UserCmd_Process, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&SendDeathMsg, PLUGIN_SendDeathMsg, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&CharacterSelect, PLUGIN_HkIServerImpl_CharacterSelect, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&Dock_Call, PLUGIN_HkCb_Dock_Call, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&DisConnect, PLUGIN_HkIServerImpl_DisConnect, 0));

	return p_PI;
}
