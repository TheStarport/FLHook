// PvP Betting Plugin
// By Raikkonen

// Includes
#include <FLHook.h>
#include <plugin.h>

// Global variables and structures
ReturnCode returncode = ReturnCode::Default;

struct BET
{
	uint iClientID;
	uint iClientID2;
	int iAmount;
	bool bAccepted;
};

struct contestant
{
	bool bAccepted;
	bool bLoser;
};

struct FFA
{
	std::map<uint, contestant> contestants;
	int buyin;
	int pot;
};

std::list<BET> bets;
std::map<uint, FFA> ffas; // uint is iSystemID

// If the player who died is in an FFA, mark them as a loser. Also handles
// payouts to winner.
void processFFA(uint iClientIDVictim)
{
	for (auto& [iSystem, ffa] : ffas)
	{
		if (ffas[iSystem].contestants[iClientIDVictim].bAccepted && !ffas[iSystem].contestants[iClientIDVictim].bLoser)
		{
			if (ffas[iSystem].contestants.find(iClientIDVictim) != ffas[iSystem].contestants.end())
			{
				ffas[iSystem].contestants[iClientIDVictim].bLoser = true;
				std::wstring wscVictim = (const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim);
				std::wstring msg = wscVictim + L" has been knocked out the FFA.";
				PrintLocalUserCmdText(iClientIDVictim, msg, 100000);
			}

			// Is the FFA over?
			int count = 0;
			uint iContestantID = 0;
			for (auto& [id, contestant] : ffas[iSystem].contestants)
			{
				if (contestant.bLoser == false && contestant.bAccepted == true)
				{
					count++;
					iContestantID = id;
				}
			}

			// Has the FFA been won?
			if (count <= 1)
			{
				if (HkIsValidClientID(iContestantID))
				{
					// Announce and pay winner
					std::wstring wscWinner = (const wchar_t*)Players.GetActiveCharacterName(iContestantID);
					HkAddCash(wscWinner, ffas[iSystem].pot);
					std::wstring msg = wscWinner + L" has won the FFA and receives " +
					    std::to_wstring(ffas[iSystem].pot) + L" credits.";
					PrintLocalUserCmdText(iContestantID, msg, 100000);
				}
				else
				{
					struct PlayerData* pPD = 0;
					while (pPD = Players.traverse_active(pPD))
					{
						uint iClientID = HkGetClientIdFromPD(pPD);
						uint iClientSystemID = 0;
						pub::Player::GetSystem(iClientID, iClientSystemID);
						if (iSystem == iClientSystemID)
							PrintUserCmdText(iClientID, L"No one has won the FFA.");
					}
				}
				// Delete event
				ffas.erase(iSystem);
				return;
			}
		}
	}
}

// This method is called when a player types /ffa in an attempt to start a pvp
// event
void UserCmd_StartFFA(uint iClientID, const std::wstring& wscParam)
{
	HK_ERROR err;

	// Get buyin amount
	std::wstring wscAmount = GetParam(wscParam, ' ', 0);

	// Filter out any weird denotion/currency symbols
	wscAmount = ReplaceStr(wscAmount, L".", L"");
	wscAmount = ReplaceStr(wscAmount, L",", L"");
	wscAmount = ReplaceStr(wscAmount, L"$", L"");
	wscAmount = ReplaceStr(wscAmount, L"$", L"");

	// Convert string to uint
	int iAmount = ToInt(wscAmount);

	// Check its a valid amount of cash
	if (iAmount <= 0)
	{
		PrintUserCmdText(iClientID, L"Must specify a cash amount. Usage: /ffa <amount> e.g. /ffa 5000");
		return;
	}

	// Check the player can afford it
	std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
	int iCash = 0;
	if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	if (iAmount > 0 && iCash < iAmount)
	{
		PrintUserCmdText(iClientID, L"You don't have enough credits to create this FFA.");
		return;
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

			// Add them to the contestants ffas
			ffas[iSystemID].contestants[iClientID2].bLoser = false;

			if (iClientID == iClientID2)
				ffas[iSystemID].contestants[iClientID2].bAccepted = true;
			else
			{
				ffas[iSystemID].contestants[iClientID2].bAccepted = false;
				std::wstring msg = wscCharname +
				    L" has started a Free-For-All tournament. "
				    L"Cost to enter is " +
				    std::to_wstring(iAmount) + L" credits. Type \"/acceptffa\" to enter.";
				PrintUserCmdText(iClientID2, msg);
			}
		}

		// Are there any other players in this system?
		if (ffas[iSystemID].contestants.size() > 0)
		{
			PrintUserCmdText(iClientID, L"Challenge issued. Waiting for others to accept.");
			ffas[iSystemID].buyin = iAmount;
			ffas[iSystemID].pot = iAmount;
			HkAddCash(wscCharname, -(iAmount));
		}
		else
		{
			ffas.erase(iSystemID);
			PrintUserCmdText(iClientID, L"There are no other players in this system.");
		}
	}
	else
		PrintUserCmdText(iClientID, L"There is an FFA already happening in this system.");
}

// This method is called when a player types /acceptffa
void UserCmd_AcceptFFA(uint iClientID, const std::wstring& wscParam)
{
	// Is player in space?
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	if (!iShip)
	{
		PrintUserCmdText(iClientID, L"You must be in space to accept this.");
		return;
	}

	// Get the player's current system and location in the system.
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);

	if (ffas.find(iSystemID) == ffas.end())
	{
		PrintUserCmdText(iClientID, L"There isn't an FFA in this system. Use /ffa to create one.");
		return;
	}
	else
	{
		HK_ERROR err;

		// Get charname
		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Check the player can afford it
		int iCash = 0;
		if ((err = HkGetCash(charname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		if (ffas[iSystemID].buyin > 0 && iCash < ffas[iSystemID].buyin)
		{
			PrintUserCmdText(iClientID, L"You don't have enough credits to join this FFA.");
			return;
		}

		// Accept
		if (ffas[iSystemID].contestants[iClientID].bAccepted == false)
		{
			ffas[iSystemID].contestants[iClientID].bAccepted = true;
			ffas[iSystemID].contestants[iClientID].bLoser = false;
			ffas[iSystemID].pot = ffas[iSystemID].pot + ffas[iSystemID].buyin;
			PrintUserCmdText(
			    iClientID,
			    std::to_wstring(ffas[iSystemID].buyin) +
			        L" credits have been deducted from "
			        L"your Neural Net account.");
			std::wstring msg =
			    charname + L" has joined the FFA. Pot is now at " + std::to_wstring(ffas[iSystemID].pot) + L" credits.";
			PrintLocalUserCmdText(iClientID, msg, 100000);

			// Deduct cash
			HkAddCash(charname, -(ffas[iSystemID].buyin));
		}
		else
			PrintUserCmdText(iClientID, L"You have already accepted the FFA.");
	}
}

// Removes any bets with this iClientID and handles payouts.
void processDuel(uint iClientID)
{
	for (std::list<BET>::iterator iter = bets.begin(); iter != bets.end();)
	{
		uint iClientIDKiller = 0;

		if (iter->iClientID == iClientID)
			iClientIDKiller = iter->iClientID2;

		if (iter->iClientID2 == iClientID)
			iClientIDKiller = iter->iClientID;

		if (iClientIDKiller == 0)
			return;

		if (iter->bAccepted)
		{
			// Get player names
			std::wstring victim = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			std::wstring killer = (const wchar_t*)Players.GetActiveCharacterName(iClientIDKiller);

			// Prepare and send message
			std::wstring msg = killer + L" has won a duel against " + victim + L" for " +
			    std::to_wstring(iter->iAmount) + L" credits.";
			PrintLocalUserCmdText(iClientIDKiller, msg, 10000);

			// Change cash
			HkAddCash(killer, iter->iAmount);
			HkAddCash(victim, -(iter->iAmount));
		}
		else
		{
			PrintUserCmdText(iter->iClientID, L"Duel cancelled.");
			PrintUserCmdText(iter->iClientID2, L"Duel cancelled.");
		}
		bets.erase(iter);
		return;
	}
}

// This method is called when a player types /duel in an attempt to start a duel
void UserCmd_Duel(uint iClientID, const std::wstring& wscParam)
{
	// Get the object the player is targetting
	uint iShip, iTargetShip;
	pub::Player::GetShip(iClientID, iShip);
	pub::SpaceObj::GetTarget(iShip, iTargetShip);
	if (!iTargetShip)
	{
		PrintUserCmdText(iClientID, L"Target is not a ship.");
		return;
	}

	// Check ship is a player
	uint iClientIDTarget = HkGetClientIDByShip(iTargetShip);
	if (!iClientIDTarget)
	{
		PrintUserCmdText(iClientID, L"Target is not a player.");
		return;
	}

	// Get bet amount
	std::wstring wscAmount = GetParam(wscParam, ' ', 0);

	// Filter out any weird denotion/currency symbols
	wscAmount = ReplaceStr(wscAmount, L".", L"");
	wscAmount = ReplaceStr(wscAmount, L",", L"");
	wscAmount = ReplaceStr(wscAmount, L"$", L"");
	wscAmount = ReplaceStr(wscAmount, L"$", L"");

	// Convert string to uint
	int iAmount = ToInt(wscAmount);

	// Check its a valid amount of cash
	if (iAmount <= 0)
	{
		PrintUserCmdText(
		    iClientID,
		    L"Must specify a cash amount. Usage: /duel "
		    L"<amount> e.g. /duel 5000");
		return;
	}

	HK_ERROR err;
	std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// Check the player can afford it
	int iCash = 0;
	if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	if (iAmount > 0 && iCash < iAmount)
	{
		PrintUserCmdText(iClientID, L"You don't have enough credits to issue this challenge.");
		return;
	}

	// Do either players already have a duel?
	for (auto& bet : bets)
	{
		// Target already has a bet
		if ((bet.iClientID == iClientIDTarget || bet.iClientID2 == iClientIDTarget))
		{
			PrintUserCmdText(iClientID, L"This player already has an ongoing duel.");
			return;
		}
		// Player already has a bet
		if ((bet.iClientID == iClientID || bet.iClientID2 == iClientID))
		{
			PrintUserCmdText(iClientID, L"You already have an ongoing duel. Type /cancel");
			return;
		}
	}

	// Create duel
	BET bet;
	bet.iClientID = iClientID;
	bet.iClientID2 = iClientIDTarget;
	bet.iAmount = iAmount;
	bet.bAccepted = false;
	bets.push_back(bet);

	// Message players
	std::wstring wscCharname2 = (const wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);
	std::wstring msg =
	    wscCharname + L" has challenged " + wscCharname2 + L" to a duel for " + std::to_wstring(iAmount) + L" credits.";
	PrintLocalUserCmdText(iClientID, msg, 10000);
	PrintUserCmdText(iClientIDTarget, L"Type \"/acceptduel\" to accept.");
}

void UserCmd_AcceptDuel(uint iClientID, const std::wstring& wscParam)
{
	// Is player in space?
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	if (!iShip)
	{
		PrintUserCmdText(iClientID, L"You must be in space to accept this.");
		return;
	}

	for (auto& bet : bets)
	{
		if (bet.iClientID2 == iClientID)
		{
			// Has player already accepted the bet?
			if (bet.bAccepted == true)
			{
				PrintUserCmdText(iClientID, L"You have already accepted the challenge.");
				return;
			}

			// Check the player can afford it
			HK_ERROR err;
			std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			int iCash = 0;
			if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
			{
				PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
				return;
			}

			if (iCash < bet.iAmount)
			{
				PrintUserCmdText(iClientID, L"You don't have enough credits to accept this challenge");
				return;
			}

			bet.bAccepted = true;
			std::wstring msg = wscCharname + L" has accepted the duel with " +
			    (const wchar_t*)Players.GetActiveCharacterName(bet.iClientID) + L" for " +
			    std::to_wstring(bet.iAmount) + L" credits.";
			PrintLocalUserCmdText(iClientID, msg, 10000);
			return;
		}
	}
	PrintUserCmdText(
	    iClientID,
	    L"You have no duel requests. To challenge "
	    L"someone, target them and type /duel <amount>");
}

void UserCmd_Cancel(uint iClientID, const std::wstring& wscParam)
{
	processFFA(iClientID);
	processDuel(iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client command processing
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

USERCMD UserCmds[] = {
	{ L"/acceptduel", UserCmd_AcceptDuel }, { L"/acceptffa", UserCmd_AcceptFFA },
	{ L"/cancel", UserCmd_Cancel },         { L"/duel", UserCmd_Duel },
	{ L"/ffa", UserCmd_StartFFA },
};

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd)
{
	DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hooks
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int __cdecl Dock_Call(
    unsigned int const& iShip, unsigned int const& iDockTarget, int& iCancel, enum DOCK_HOST_RESPONSE& response)
{
	uint iClientID = HkGetClientIDByShip(iShip);
	if (HkIsValidClientID(iClientID))
	{
		processFFA(iClientID);
		processDuel(iClientID);
	}
	return 0;
}

void __stdcall DisConnect(uint& iClientID, enum EFLConnection& state)
{
	processFFA(iClientID);
	processDuel(iClientID);
}

void __stdcall CharacterInfoReq(uint& iClientID, bool& p2)
{
	processFFA(iClientID);
	processDuel(iClientID);
}

void SendDeathMessage(const std::wstring& wscMsg, uint& iSystem, uint& iClientIDVictim, uint& iClientIDKiller)
{
	processDuel(iClientIDVictim);
	processFFA(iClientIDVictim);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions to hook
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("PvP by Raikkonen");
	pi->shortName("pvp");
	pi->mayPause(false);
	pi->mayUnload(true);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMessage);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
	pi->emplaceHook(HookedCall::IEngine__DockCall, &Dock_Call);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}