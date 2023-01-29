/**
 * @date Jan, 2023
 * @author Raikkonen
 * @defgroup Betting Betting
 * @brief
 * A plugin that allows players to place bets and then duel, the winner getting the pot.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - acceptduel - Accepts the current duel request.
 * - acceptffa - Accept the current ffa request.
 * - cancel - Cancel the current duel/ffa request.
 * - duel <amount> - Create a duel request to the targeted player. Winner gets the pot.
 * - ffa <amount> - Create an ffa and send an invite to everyone in the system. Winner gets the pot.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * This plugin has no configuration file.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "Main.h"

namespace Plugins::Betting
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup Betting
	 * @brief If the player who died is in an FreeForAll, mark them as a loser. Also handles payouts to winner.
	 */
	void processFFA(ClientId client)
	{
		for (const auto& [system, freeForAll] : global->freeForAlls)
		{
			if (global->freeForAlls[system].contestants[client].accepted && !global->freeForAlls[system].contestants[client].loser)
			{
				if (global->freeForAlls[system].contestants.contains(client))
				{
					global->freeForAlls[system].contestants[client].loser = true;
					PrintLocalUserCmdText(client, std::wstring(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client))) + L" has been knocked out the FFA.", 100000);
				}

				// Is the FreeForAll over?
				int count = 0;
				uint contestantId = 0;
				for (const auto& [id, contestant] : global->freeForAlls[system].contestants)
				{
					if (contestant.loser == false && contestant.accepted == true)
					{
						count++;
						contestantId = id;
					}
				}

				// Has the FreeForAll been won?
				if (count <= 1)
				{
					if (Hk::Client::IsValidClientID(contestantId))
					{
						// Announce and pay winner
						std::wstring winner = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(contestantId));
						Hk::Player::AddCash(winner, global->freeForAlls[system].pot);
						const std::wstring message = winner + L" has won the FFA and receives " + std::to_wstring(global->freeForAlls[system].pot) + L" credits.";
						PrintLocalUserCmdText(contestantId, message, 100000);
					}
					else
					{
						struct PlayerData* playerData = nullptr;
						while (playerData = Players.traverse_active(playerData))
						{
							ClientId localClient = playerData->iOnlineId;
							if (SystemId systemId = Hk::Player::GetSystem(localClient).value(); 
								system == systemId)
								PrintUserCmdText(localClient, L"No one has won the FFA.");
						}
					}
					// Delete event
					global->freeForAlls.erase(system);
					return;
				}
			}
		}
	}

	/** @ingroup Betting
	 * @brief This method is called when a player types /ffa in an attempt to start a pvp event
	 */
	void UserCmdStartFreeForAll(ClientId& client, const std::wstring& param)
	{
		// Get entryAmount amount
		std::wstring amountString = GetParam(param, ' ', 0);

		// Filter out any weird denotion/currency symbols
		amountString = ReplaceStr(amountString, L".", L"");
		amountString = ReplaceStr(amountString, L",", L"");
		amountString = ReplaceStr(amountString, L"$", L"");
		amountString = ReplaceStr(amountString, L"$", L"");

		// Convert string to uint
		uint amount = ToUInt(amountString);

		// Check its a valid amount of cash
		if (amount == 0)
		{
			PrintUserCmdText(client, L"Must specify a cash amount. Usage: /ffa <amount> e.g. /ffa 5000");
			return;
		}

		// Check the player can afford it
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
		uint cash = 0;
		if (const auto err =  Hk::Player::GetCash(client); err.has_error() )
		{
			PrintUserCmdText(client, L"ERR " + Hk::Err::ErrGetText(err.error()));
			return;
		}
		if (amount > 0 && cash < amount)
		{
			PrintUserCmdText(client, L"You don't have enough credits to create this FFA.");
			return;
		}

		// Get the player's current system and location in the system.
		SystemId systemId = Hk::Player::GetSystem(client).value();

		// Look in FreeForAll map, is an ffa happening in this system already?
		// If system doesn't have an ongoing ffa
		if (!global->freeForAlls.contains(systemId))
		{
			// Get a list of other players in the system
			// Add them and the player into the ffa map
			struct PlayerData* playerData = nullptr;
			while (playerData = Players.traverse_active(playerData))
			{
				// Get the this player's current system
				ClientId client2 = playerData->iOnlineId;
				if (SystemId clientSystemId = Hk::Player::GetSystem(client2).value(); 
					systemId != clientSystemId)
					continue;

				// Add them to the contestants freeForAlls
				global->freeForAlls[systemId].contestants[client2].loser = false;

				if (client == client2)
					global->freeForAlls[systemId].contestants[client2].accepted = true;
				else
				{
					global->freeForAlls[systemId].contestants[client2].accepted = false;
					PrintUserCmdText(client2, std::format(L"{} has started a Free-For-All tournament. Cost to enter is {} credits. Type \"/acceptffa\" to enter.",
					    characterName, amount));
				}
			}

			// Are there any other players in this system?
			if (!global->freeForAlls[systemId].contestants.empty())
			{
				PrintUserCmdText(client, L"Challenge issued. Waiting for others to accept.");
				global->freeForAlls[systemId].entryAmount = amount;
				global->freeForAlls[systemId].pot = amount;
				Hk::Player::RemoveCash(characterName, amount);
			}
			else
			{
				global->freeForAlls.erase(systemId);
				PrintUserCmdText(client, L"There are no other players in this system.");
			}
		}
		else
			PrintUserCmdText(client, L"There is an FFA already happening in this system.");
	}

	/** @ingroup Betting
	 * @brief This method is called when a player types /acceptffa
	 */
	void UserCmd_AcceptFFA(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		// Is player in space?
		if (uint ship = Hk::Player::GetShip(client).value(); !ship)
		{
			PrintUserCmdText(client, L"You must be in space to accept this.");
			return;
		}

		// Get the player's current system and location in the system.
		SystemId systemId = Hk::Player::GetSystem(client).value();

		if (!global->freeForAlls.contains(systemId))
		{
			PrintUserCmdText(client, L"There isn't an FFA in this system. Use /ffa to create one.");
		}
		else
		{
			std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

			// Check the player can afford it
			uint cash = 0;
			if (const auto err = Hk::Player::GetCash(client) ; err.has_error())
			{
				PrintUserCmdText(client, L"ERR " + Hk::Err::ErrGetText(err.error()));
				return;
			}
			if (global->freeForAlls[systemId].entryAmount > 0 && cash < global->freeForAlls[systemId].entryAmount)
			{
				PrintUserCmdText(client, L"You don't have enough credits to join this FFA.");
				return;
			}

			// Accept
			if (global->freeForAlls[systemId].contestants[client].accepted == false)
			{
				global->freeForAlls[systemId].contestants[client].accepted = true;
				global->freeForAlls[systemId].contestants[client].loser = false;
				global->freeForAlls[systemId].pot = global->freeForAlls[systemId].pot + global->freeForAlls[systemId].entryAmount;
				PrintUserCmdText(client,
				    std::to_wstring(global->freeForAlls[systemId].entryAmount) +
				        L" credits have been deducted from "
				        L"your Neural Net account.");
				const std::wstring msg = characterName + L" has joined the FFA. Pot is now at " + std::to_wstring(global->freeForAlls[systemId].pot) + L" credits.";
				PrintLocalUserCmdText(client, msg, 100000);

				// Deduct cash
				Hk::Player::RemoveCash(characterName, global->freeForAlls[systemId].entryAmount);
			}
			else
				PrintUserCmdText(client, L"You have already accepted the FFA.");
		}
	}

	/** @ingroup Betting
	 * @brief Removes any duels with this client and handles payouts.
	 */
	void ProcessDuel(ClientId client)
	{
		const auto duel = global->duels.begin();
		while (duel != global->duels.end())
		{
			uint clientKiller = 0;

			if (duel->client == client)
				clientKiller = duel->client2;

			if (duel->client2 == client)
				clientKiller = duel->client;

			if (clientKiller == 0)
				continue;

			if (duel->accepted)
			{
				// Get player names
				std::wstring victim = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
				std::wstring killer = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientKiller));

				// Prepare and send message
				const std::wstring msg = killer + L" has won a duel against " + victim + L" for " + std::to_wstring(duel->amount) + L" credits.";
				PrintLocalUserCmdText(clientKiller, msg, 10000);

				// Change cash
				Hk::Player::AddCash(killer, duel->amount);
				Hk::Player::RemoveCash(victim, duel->amount);
			}
			else
			{
				PrintUserCmdText(duel->client, L"Duel cancelled.");
				PrintUserCmdText(duel->client2, L"Duel cancelled.");
			}
			global->duels.erase(duel);
			return;
		}
	}

	/** @ingroup Betting
	 * @brief This method is called when a player types /duel in an attempt to start a duel
	 */
	void UserCmdDuel(ClientId& client, const std::wstring& param)
	{
		// Get the object the player is targetting
		uint ship = Hk::Player::GetShip(client).value();
		uint targetShip = Hk::Player::GetTarget(ship).value();
		if (!targetShip)
		{
			PrintUserCmdText(client, L"Target is not a ship.");
			return;
		}

		// Check ship is a player
		const auto clientTarget = Hk::Client::GetClientIdByShip(targetShip);
		if (!clientTarget.value())
		{
			PrintUserCmdText(client, L"Target is not a player.");
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
		const uint amount = ToUInt(amountString);

		// Check its a valid amount of cash
		if (amount == 0)
		{
			PrintUserCmdText(client,
			    L"Must specify a cash amount. Usage: /duel "
			    L"<amount> e.g. /duel 5000");
			return;
		}

		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		// Check the player can afford it
		uint uCash = 0;
		if (const auto err = Hk::Player::GetCash(client); err.has_error())
		{
			PrintUserCmdText(client, L"ERR " + Hk::Err::ErrGetText(err.error()));
			return;
		}
		if (amount > 0 && uCash < amount)
		{
			PrintUserCmdText(client, L"You don't have enough credits to issue this challenge.");
			return;
		}

		// Do either players already have a duel?
		for (const auto& duel : global->duels)
		{
			// Target already has a bet
			if (duel.client == clientTarget || duel.client2 == clientTarget)
			{
				PrintUserCmdText(client, L"This player already has an ongoing duel.");
				return;
			}
			// Player already has a bet
			if (duel.client == client || duel.client2 == client)
			{
				PrintUserCmdText(client, L"You already have an ongoing duel. Type /cancel");
				return;
			}
		}

		// Create duel
		Duel duel;
		duel.client = client;
		duel.client2 = clientTarget.value();
		duel.amount = amount;
		duel.accepted = false;
		global->duels.push_back(duel);

		// Message players
		const std::wstring characterName2 = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientTarget.value()));
		const std::wstring message = characterName + L" has challenged " + characterName2 + L" to a duel for " + std::to_wstring(amount) + L" credits.";
		PrintLocalUserCmdText(client, message, 10000);
		PrintUserCmdText(clientTarget.value(), L"Type \"/acceptduel\" to accept.");
	}

	/** @ingroup Betting
	 * @brief This method is called when a player types /acceptduel to accept a duel request.
	 */
	void UserCmdAcceptDuel(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		// Is player in space?
		if (uint ship = Hk::Player::GetShip(client).value(); !ship)
		{
			PrintUserCmdText(client, L"You must be in space to accept this.");
			return;
		}

		for (auto& duel : global->duels)
		{
			if (duel.client2 == client)
			{
				// Has player already accepted the bet?
				if (duel.accepted == true)
				{
					PrintUserCmdText(client, L"You have already accepted the challenge.");
					return;
				}

				// Check the player can afford it
				std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
				uint cash = 0;
				if (const auto err = Hk::Player::GetCash(client); err.has_error())
				{
					PrintUserCmdText(client, L"ERR " + Hk::Err::ErrGetText(err.error()));
					return;
				}

				if (cash < duel.amount)
				{
					PrintUserCmdText(client, L"You don't have enough credits to accept this challenge");
					return;
				}

				duel.accepted = true;
				const std::wstring message = characterName + L" has accepted the duel with " + reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(duel.client)) + L" for " +
				    std::to_wstring(duel.amount) + L" credits.";
				PrintLocalUserCmdText(client, message, 10000);
				return;
			}
		}
		PrintUserCmdText(client,
		    L"You have no duel requests. To challenge "
		    L"someone, target them and type /duel <amount>");
	}

	/** @ingroup Betting
	 * @brief This method is called when a player types /cancel to cancel a duel/ffa request.
	 */
	void UserCmd_Cancel(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		processFFA(client);
		ProcessDuel(client);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Client command processing
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::vector commands = {
		CreateUserCommand(L"/acceptduel", L"", UserCmdAcceptDuel, L"Accepts the current duel request."),
	    CreateUserCommand(L"/acceptffa", L"", UserCmd_AcceptFFA, L"Accept the current ffa request."),
	    CreateUserCommand(L"/cancel", L"", UserCmd_Cancel, L"Cancel the current duel/ffa request."),
	    CreateUserCommand(L"/duel", L"<amount>", UserCmdDuel, L"Create a duel request to the targeted player. Winner gets the pot."),
	    CreateUserCommand(L"/ffa", L"<amount>", UserCmdStartFreeForAll, L"Create an ffa and send an invite to everyone in the system. Winner gets the pot.")
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Hooks
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** @ingroup Betting
	 * @brief Hook for dock call. Treats a player as if they died if they were part of a duel
	 */
	int __cdecl DockCall(
	    unsigned int const& ship, [[maybe_unused]] unsigned int const& d, [[maybe_unused]]const int& cancel, [[maybe_unused]] const enum DOCK_HOST_RESPONSE& response)
	{
		if (const auto client = Hk::Client::GetClientIdByShip(ship); 
			client.has_value() && Hk::Client::IsValidClientID(client.value()))
		{
			processFFA(client.value());
			ProcessDuel(client.value());
		}
		return 0;
	}

	/** @ingroup Betting
	 * @brief Hook for disconnect. Treats a player as if they died if they were part of a duel
	 */
	void DisConnect(ClientId& client, [[maybe_unused]]const enum EFLConnection& state)
	{
		processFFA(client);
		ProcessDuel(client);
	}

	/** @ingroup Betting
	 * @brief Hook for char info request (F1). Treats a player as if they died if they were part of a duel
	 */
	void CharacterInfoReq(ClientId& client, [[maybe_unused]]const bool& p2)
	{
		processFFA(client);
		ProcessDuel(client);
	}

	/** @ingroup Betting
	 * @brief Hook for death to kick player out of duel
	 */
	void SendDeathMessage([[maybe_unused]] const std::wstring& message, [[maybe_unused]] const uint& system, ClientId& clientVictim,
	    [[maybe_unused]] const ClientId& clientKiller)
	{
		ProcessDuel(clientVictim);
		processFFA(clientVictim);
	}
} // namespace Plugins::Betting

using namespace Plugins::Betting;

DefaultDllMain();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Functions to hook
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Betting");
	pi->shortName("betting");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->commands(&commands);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMessage);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
	pi->emplaceHook(HookedCall::IEngine__DockCall, &DockCall);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}