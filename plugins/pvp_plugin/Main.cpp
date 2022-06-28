// PvP Betting Plugin
// By Raikkonen

#include "Main.h"

namespace Plugins::Pvp
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// If the player who died is in an FreeForAll, mark them as a loser. Also handles
	// payouts to winner.
	void processFFA(uint clientId)
	{
		for (auto& [system, freeForAll] : global->freeForAlls)
		{
			if (global->freeForAlls[system].contestants[clientId].accepted && !global->freeForAlls[system].contestants[clientId].loser)
			{
				if (global->freeForAlls[system].contestants.find(clientId) != global->freeForAlls[system].contestants.end())
				{
					global->freeForAlls[system].contestants[clientId].loser = true;
					PrintLocalUserCmdText(clientId, std::wstring(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId))) + L" has been knocked out the FFA.", 100000);
				}

				// Is the FreeForAll over?
				int count = 0;
				uint contestantID = 0;
				for (auto& [id, contestant] : global->freeForAlls[system].contestants)
				{
					if (contestant.loser == false && contestant.accepted == true)
					{
						count++;
						contestantID = id;
					}
				}

				// Has the FreeForAll been won?
				if (count <= 1)
				{
					if (HkIsValidClientID(contestantID))
					{
						// Announce and pay winner
						std::wstring winner = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(contestantID));
						HkAddCash(winner, global->freeForAlls[system].pot);
						const std::wstring message = winner + L" has won the FFA and receives " + std::to_wstring(global->freeForAlls[system].pot) + L" credits.";
						PrintLocalUserCmdText(contestantID, message, 100000);
					}
					else
					{
						struct PlayerData* playerData = nullptr;
						while (playerData = Players.traverse_active(playerData))
						{
							uint clientID = HkGetClientIdFromPD(playerData);
							uint systemID = 0;
							pub::Player::GetSystem(clientID, systemID);
							if (system == systemID)
								PrintUserCmdText(clientID, L"No one has won the FFA.");
						}
					}
					// Delete event
					global->freeForAlls.erase(system);
					return;
				}
			}
		}
	}

	// This method is called when a player types /ffa in an attempt to start a pvp
	// event
	void UserCmdStartFreeForAll(const uint& clientId, const std::wstring_view& param)
	{
		HK_ERROR error;

		// Get entryAmount amount
		std::wstring amountString = GetParam(param, ' ', 0);

		// Filter out any weird denotion/currency symbols
		amountString = ReplaceStr(amountString, L".", L"");
		amountString = ReplaceStr(amountString, L",", L"");
		amountString = ReplaceStr(amountString, L"$", L"");
		amountString = ReplaceStr(amountString, L"$", L"");

		// Convert string to uint
		int amount = ToInt(amountString);

		// Check its a valid amount of cash
		if (amount <= 0)
		{
			PrintUserCmdText(clientId, L"Must specify a cash amount. Usage: /ffa <amount> e.g. /ffa 5000");
			return;
		}

		// Check the player can afford it
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
		int cash = 0;
		if ((error = HkGetCash(characterName, cash)) != HKE_OK)
		{
			PrintUserCmdText(clientId, L"ERR " + HkErrGetText(error));
			return;
		}
		if (amount > 0 && cash < amount)
		{
			PrintUserCmdText(clientId, L"You don't have enough credits to create this FFA.");
			return;
		}

		// Get the player's current system and location in the system.
		uint systemId;
		pub::Player::GetSystem(clientId, systemId);

		// Look in FreeForAll map, is an ffa happening in this system already?
		// If system doesn't have an ongoing ffa
		if (global->freeForAlls.find(systemId) == global->freeForAlls.end())
		{
			// Get a list of other players in the system
			// Add them and the player into the ffa map
			struct PlayerData* playerData = nullptr;
			while (playerData = Players.traverse_active(playerData))
			{
				// Get the this player's current system
				uint clientID2 = HkGetClientIdFromPD(playerData);
				uint clientSystemID = 0;
				pub::Player::GetSystem(clientID2, clientSystemID);
				if (systemId != clientSystemID)
					continue;

				// Add them to the contestants freeForAlls
				global->freeForAlls[systemId].contestants[clientID2].loser = false;

				if (clientId == clientID2)
					global->freeForAlls[systemId].contestants[clientID2].accepted = true;
				else
				{
					global->freeForAlls[systemId].contestants[clientID2].accepted = false;
					PrintUserCmdText(clientID2,
					    L"%s has started a Free-For-All tournament. "
					    L"Cost to enter is %i credits. Type \"/acceptffa\" to enter.",
					    characterName.c_str(),
					    amount);
				}
			}

			// Are there any other players in this system?
			if (!global->freeForAlls[systemId].contestants.empty())
			{
				PrintUserCmdText(clientId, L"Challenge issued. Waiting for others to accept.");
				global->freeForAlls[systemId].entryAmount = amount;
				global->freeForAlls[systemId].pot = amount;
				HkAddCash(characterName, -(amount));
			}
			else
			{
				global->freeForAlls.erase(systemId);
				PrintUserCmdText(clientId, L"There are no other players in this system.");
			}
		}
		else
			PrintUserCmdText(clientId, L"There is an FFA already happening in this system.");
	}

	// This method is called when a player types /acceptffa
	void UserCmd_AcceptFFA(const uint& clientId, const std::wstring_view& param)
	{
		// Is player in space?
		uint ship;
		pub::Player::GetShip(clientId, ship);
		if (!ship)
		{
			PrintUserCmdText(clientId, L"You must be in space to accept this.");
			return;
		}

		// Get the player's current system and location in the system.
		uint systemId;
		pub::Player::GetSystem(clientId, systemId);

		if (global->freeForAlls.find(systemId) == global->freeForAlls.end())
		{
			PrintUserCmdText(clientId, L"There isn't an FFA in this system. Use /ffa to create one.");
		}
		else
		{
			HK_ERROR error;

			std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));

			// Check the player can afford it
			int cash = 0;
			if ((error = HkGetCash(characterName, cash)) != HKE_OK)
			{
				PrintUserCmdText(clientId, L"ERR " + HkErrGetText(error));
				return;
			}
			if (global->freeForAlls[systemId].entryAmount > 0 && cash < global->freeForAlls[systemId].entryAmount)
			{
				PrintUserCmdText(clientId, L"You don't have enough credits to join this FFA.");
				return;
			}

			// Accept
			if (global->freeForAlls[systemId].contestants[clientId].accepted == false)
			{
				global->freeForAlls[systemId].contestants[clientId].accepted = true;
				global->freeForAlls[systemId].contestants[clientId].loser = false;
				global->freeForAlls[systemId].pot = global->freeForAlls[systemId].pot + global->freeForAlls[systemId].entryAmount;
				PrintUserCmdText(clientId,
				    std::to_wstring(global->freeForAlls[systemId].entryAmount) +
				        L" credits have been deducted from "
				        L"your Neural Net account.");
				const std::wstring msg = characterName + L" has joined the FFA. Pot is now at " + std::to_wstring(global->freeForAlls[systemId].pot) + L" credits.";
				PrintLocalUserCmdText(clientId, msg, 100000);

				// Deduct cash
				HkAddCash(characterName, -(global->freeForAlls[systemId].entryAmount));
			}
			else
				PrintUserCmdText(clientId, L"You have already accepted the FFA.");
		}
	}

	// Removes any duels with this clientId and handles payouts.
	void processDuel(uint clientId)
	{
		const auto duel = global->duels.begin();
		while (duel != global->duels.end())
		{
			uint clientIDKiller = 0;

			if (duel->clientId == clientId)
				clientIDKiller = duel->clientId2;

			if (duel->clientId2 == clientId)
				clientIDKiller = duel->clientId;

			if (clientIDKiller == 0)
				return;

			if (duel->accepted)
			{
				// Get player names
				std::wstring victim = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
				std::wstring killer = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientIDKiller));

				// Prepare and send message
				const std::wstring msg = killer + L" has won a duel against " + victim + L" for " + std::to_wstring(duel->amount) + L" credits.";
				PrintLocalUserCmdText(clientIDKiller, msg, 10000);

				// Change cash
				HkAddCash(killer, duel->amount);
				HkAddCash(victim, -(duel->amount));
			}
			else
			{
				PrintUserCmdText(duel->clientId, L"Duel cancelled.");
				PrintUserCmdText(duel->clientId2, L"Duel cancelled.");
			}
			global->duels.erase(duel);
			return;
		}
	}

	// This method is called when a player types /duel in an attempt to start a duel
	void UserCmdDuel(const uint& clientId, const std::wstring_view& param)
	{
		// Get the object the player is targetting
		uint iShip;
		uint targetShip;
		pub::Player::GetShip(clientId, iShip);
		pub::SpaceObj::GetTarget(iShip, targetShip);
		if (!targetShip)
		{
			PrintUserCmdText(clientId, L"Target is not a ship.");
			return;
		}

		// Check ship is a player
		uint clientIDTarget = HkGetClientIDByShip(targetShip);
		if (!clientIDTarget)
		{
			PrintUserCmdText(clientId, L"Target is not a player.");
			return;
		}

		// Get bet amount
		std::wstring amountString = GetParam(param, ' ', 0);

		// Filter out any weird denotion/currency symbols
		amountString = ReplaceStr(amountString, L".", L"");
		amountString = ReplaceStr(amountString, L",", L"");
		amountString = ReplaceStr(amountString, L"$", L"");
		amountString = ReplaceStr(amountString, L"$", L"");

		// Convert string to uint
		const int amount = ToInt(amountString);

		// Check its a valid amount of cash
		if (amount <= 0)
		{
			PrintUserCmdText(clientId,
			    L"Must specify a cash amount. Usage: /duel "
			    L"<amount> e.g. /duel 5000");
			return;
		}

		HK_ERROR error;
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));

		// Check the player can afford it
		int iCash = 0;
		if ((error = HkGetCash(characterName, iCash)) != HKE_OK)
		{
			PrintUserCmdText(clientId, L"ERR " + HkErrGetText(error));
			return;
		}
		if (amount > 0 && iCash < amount)
		{
			PrintUserCmdText(clientId, L"You don't have enough credits to issue this challenge.");
			return;
		}

		// Do either players already have a duel?
		for (auto& duel : global->duels)
		{
			// Target already has a bet
			if ((duel.clientId == clientIDTarget || duel.clientId2 == clientIDTarget))
			{
				PrintUserCmdText(clientId, L"This player already has an ongoing duel.");
				return;
			}
			// Player already has a bet
			if ((duel.clientId == clientId || duel.clientId2 == clientId))
			{
				PrintUserCmdText(clientId, L"You already have an ongoing duel. Type /cancel");
				return;
			}
		}

		// Create duel
		Duel duel;
		duel.clientId = clientId;
		duel.clientId2 = clientIDTarget;
		duel.amount = amount;
		duel.accepted = false;
		global->duels.push_back(duel);

		// Message players
		const std::wstring characterName2 = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientIDTarget));
		const std::wstring message = characterName + L" has challenged " + characterName2 + L" to a duel for " + std::to_wstring(amount) + L" credits.";
		PrintLocalUserCmdText(clientId, message, 10000);
		PrintUserCmdText(clientIDTarget, L"Type \"/acceptduel\" to accept.");
	}

	void UserCmdAcceptDuel(const uint& clientId, const std::wstring_view& param)
	{
		// Is player in space?
		uint ship;
		pub::Player::GetShip(clientId, ship);
		if (!ship)
		{
			PrintUserCmdText(clientId, L"You must be in space to accept this.");
			return;
		}

		for (auto& duel : global->duels)
		{
			if (duel.clientId2 == clientId)
			{
				// Has player already accepted the bet?
				if (duel.accepted == true)
				{
					PrintUserCmdText(clientId, L"You have already accepted the challenge.");
					return;
				}

				// Check the player can afford it
				HK_ERROR error;
				std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
				int cash = 0;
				if ((error = HkGetCash(characterName, cash)) != HKE_OK)
				{
					PrintUserCmdText(clientId, L"ERR " + HkErrGetText(error));
					return;
				}

				if (cash < duel.amount)
				{
					PrintUserCmdText(clientId, L"You don't have enough credits to accept this challenge");
					return;
				}

				duel.accepted = true;
				const std::wstring message = characterName + L" has accepted the duel with " + reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(duel.clientId)) + L" for " +
				    std::to_wstring(duel.amount) + L" credits.";
				PrintLocalUserCmdText(clientId, message, 10000);
				return;
			}
		}
		PrintUserCmdText(clientId,
		    L"You have no duel requests. To challenge "
		    L"someone, target them and type /duel <amount>");
	}

	void UserCmd_Cancel(const uint& clientId, const std::wstring_view& param)
	{
		processFFA(clientId);
		processDuel(clientId);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Client command processing
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector commands = {
	    CreateUserCommand(L"/acceptduel", L"", UserCmdAcceptDuel, L""),
	    CreateUserCommand(L"/acceptffa", L"", UserCmd_AcceptFFA, L""),
	    CreateUserCommand(L"/cancel", L"", UserCmd_Cancel, L""),
	    CreateUserCommand(L"/duel", L"", UserCmdDuel, L""),
	    CreateUserCommand(L"/ffa", L"", UserCmdStartFreeForAll, L"")
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Hooks
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int __cdecl DockCall(unsigned int const& ship, unsigned int const& d, int& cancel, enum DOCK_HOST_RESPONSE& response)
	{
		const uint clientId = HkGetClientIDByShip(ship);
		if (HkIsValidClientID(clientId))
		{
			processFFA(clientId);
			processDuel(clientId);
		}
		return 0;
	}

	void __stdcall DisConnect(uint& clientID, enum EFLConnection& state)
	{
		processFFA(clientID);
		processDuel(clientID);
	}

	void __stdcall CharacterInfoReq(uint& clientID, bool& p2)
	{
		processFFA(clientID);
		processDuel(clientID);
	}

	void SendDeathMessage(const std::wstring& message, uint& system, uint& clientIdVictim, uint& clientIdKiller)
	{
		processDuel(clientIdVictim);
		processFFA(clientIdVictim);
	}
} // namespace Plugins::Pvp

using namespace Plugins::Pvp;

DefaultDllMain()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions to hook
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("PvP");
	pi->shortName("pvp");
	pi->mayPause(false);
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->commands(commands);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMessage);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
	pi->emplaceHook(HookedCall::IEngine__DockCall, &DockCall);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}