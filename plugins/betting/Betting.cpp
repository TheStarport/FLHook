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

#include "PCH.hpp"

#include "Betting.hpp"

namespace Plugins
{

    /** @ingroup Betting
     * @brief If the player who died is in an FreeForAll, mark them as a loser. Also handles payouts to winner.
     */
    void Betting::ProcessFFA(ClientId client)
    {

        for (const auto& [system, freeForAll] : freeForAlls)
        {
            if (freeForAlls[system].contestants[client].accepted && !freeForAlls[system].contestants[client].loser)
            {
                if (freeForAlls[system].contestants.contains(client))
                {
                    freeForAlls[system].contestants[client].loser = true;
                    PrintLocalUserCmdText(client,
                                          std::wstring(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client))) +
                                              L" has been knocked out the FFA.",
                                          100000);
                }

                // Is the FreeForAll over?
                int count = 0;
                uint contestantId = 0;
                for (const auto& [id, contestant] : freeForAlls[system].contestants)
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
                        Hk::Player::AddCash(winner, freeForAlls[system].pot);
                        const std::wstring message = winner + L" has won the FFA and receives " + std::to_wstring(freeForAlls[system].pot) + L" credits.";
                        PrintLocalUserCmdText(contestantId, message, 100000);
                    }
                    else
                    {
                        PlayerData* playerData = nullptr;
                        while ((playerData = Players.traverse_active(playerData)))
                        {
                            ClientId localClient = playerData->onlineId;
                            if (SystemId systemId = Hk::Player::GetSystem(localClient).Handle(); system == systemId)
                            {
                                PrintUserCmdText(localClient, L"No one has won the FFA.");
                            }
                        }
                    }
                    // Delete event
                    freeForAlls.erase(system);
                    return;
                }
            }
        }
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /ffa in an attempt to start a pvp event
     */
    void Betting::UserCmdStartFreeForAll(uint amount)
    {

        // Check its a valid amount of cash
        if (amount == 0)
        {
            client.Message(L"Must specify a cash amount. Usage: /ffa <amount> e.g. /ffa 5000");
            return;
        }

        // Check the player can afford it
        std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
        const auto cash = Hk::Player::GetCash(client).Unwrap();
        if (amount > 0 && cash < amount)
        {
            client.Message(L"You don't have enough credits to create this FFA.");
            return;
        }

        // Get the player's current system and location in the system.
        SystemId systemId = Hk::Player::GetSystem(client).Handle();

        // Look in FreeForAll map, is an ffa happening in this system already?
        // If system doesn't have an ongoing ffa
        if (!freeForAlls.contains(systemId))
        {
            // Get a list of other players in the system
            // Add them and the player into the ffa map
            PlayerData* playerData = nullptr;
            while ((playerData = Players.traverse_active(playerData)))
            {
                // Get the this player's current system
                ClientId client2 = playerData->onlineId;
                if (SystemId clientSystemId = Hk::Player::GetSystem(client2).Handle(); systemId != clientSystemId)
                {
                    continue;
                }

                // Add them to the contestants freeForAlls
                freeForAlls[systemId].contestants[client2].loser = false;

                if (client == client2)
                {
                    freeForAlls[systemId].contestants[client2].accepted = true;
                }
                else
                {
                    freeForAlls[systemId].contestants[client2].accepted = false;
                    PrintUserCmdText(client2,
                                     std::format(L"{} has started a Free-For-All tournament. Cost to enter is {} credits. Type \"/acceptffa\" to enter.",
                                                 characterName,
                                                 amount));
                }
            }

            // Are there any other players in this system?
            if (!freeForAlls[systemId].contestants.empty())
            {
                client.Message(L"Challenge issued. Waiting for others to accept.");
                freeForAlls[systemId].entryAmount = amount;
                freeForAlls[systemId].pot = amount;
                Hk::Player::RemoveCash(characterName, amount);
            }
            else
            {
                freeForAlls.erase(systemId);
                client.Message(L"There are no other players in this system.");
            }
        }
        else
        {
            client.Message(L"There is an FFA already happening in this system.");
        }
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /acceptffa
     */
    void Betting::UserCmdAcceptFFA()
    {
        // Is player in space?
        if (const uint ship = Hk::Player::GetShip(client).Unwrap(); !ship)
        {
            client.Message(L"You must be in space to accept this.");
            return;
        }

        // Get the player's current system and location in the system.
        SystemId systemId = Hk::Player::GetSystem(client).Handle();

        if (!freeForAlls.contains(systemId))
        {
            client.Message(L"There isn't an FFA in this system. Use /ffa to create one.");
        }
        else
        {
            std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

            // Check the player can afford it
            const auto cash = Hk::Player::GetCash(client).Unwrap();
            if (freeForAlls[systemId].entryAmount > 0 && cash < freeForAlls[systemId].entryAmount)
            {
                client.Message(L"You don't have enough credits to join this FFA.");
                return;
            }

            // Accept
            if (freeForAlls[systemId].contestants[client].accepted == false)
            {
                freeForAlls[systemId].contestants[client].accepted = true;
                freeForAlls[systemId].contestants[client].loser = false;
                freeForAlls[systemId].pot = freeForAlls[systemId].pot + freeForAlls[systemId].entryAmount;
                PrintUserCmdText(client,
                                 std::to_wstring(freeForAlls[systemId].entryAmount) + L" credits have been deducted from "
                                                                                      L"your Neural Net account.");
                const std::wstring msg = characterName + L" has joined the FFA. Pot is now at " + std::to_wstring(freeForAlls[systemId].pot) + L" credits.";
                PrintLocalUserCmdText(client, msg, 100000);

                // Deduct cash
                Hk::Player::RemoveCash(characterName, freeForAlls[systemId].entryAmount);
            }
            else
            {
                client.Message(L"You have already accepted the FFA.");
            }
        }
    }

    /** @ingroup Betting
     * @brief Removes any duels with this client and handles payouts.
     */
    void Betting::ProcessDuel(ClientId client)
    {
        auto duel = duels.begin();
        while (duel != duels.end())
        {
            uint clientKiller = 0;

            if (duel->client == client)
            {
                clientKiller = duel->client2;
            }

            if (duel->client2 == client)
            {
                clientKiller = duel->client;
            }

            if (clientKiller == 0)
            {
                duel++;
                continue;
            }

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
            duel = duels.erase(duel);
            return;
        }
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /duel in an attempt to start a duel
     */
    void Betting::UserCmdDuel(uint amount)
    {
        // Get the object the player is targetting
        const auto targetShip = Hk::Player::GetTarget(client).Handle();

        // Check ship is a player
        const auto clientTarget = Hk::Client::GetClientIdByShip(targetShip).Handle();

        // Check its a valid amount of cash
        if (amount == 0)
        {
            PrintUserCmdText(client,
                             L"Must specify a cash amount. Usage: /duel "
                             L"<amount> e.g. /duel 5000");
            return;
        }

        const std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

        // Check the player can afford it
        const auto cash = Hk::Player::GetCash(client).Handle();

        if (amount > 0 && cash < amount)
        {
            client.Message(L"You don't have enough credits to issue this challenge.");
            return;
        }

        // Do either players already have a duel?
        for (const auto& duel : duels)
        {
            // Target already has a bet
            if (duel.client == clientTarget || duel.client2 == clientTarget)
            {
                client.Message(L"This player already has an ongoing duel.");
                return;
            }
            // Player already has a bet
            if (duel.client == client || duel.client2 == client)
            {
                client.Message(L"You already have an ongoing duel. Type /cancel");
                return;
            }
        }

        // Create duel
        Duel duel;
        duel.client = client;
        duel.client2 = clientTarget;
        duel.amount = amount;
        duel.accepted = false;
        duels.push_back(duel);

        // Message players
        const std::wstring characterName2 = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientTarget));
        const std::wstring message = characterName + L" has challenged " + characterName2 + L" to a duel for " + std::to_wstring(amount) + L" credits.";
        PrintLocalUserCmdText(client, message, 10000);
        PrintUserCmdText(clientTarget, L"Type \"/acceptduel\" to accept.");
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /acceptduel to accept a duel request.
     */
    void Betting::UserCmdAcceptDuel()
    {
        // Is player in space?
        if (const uint ship = Hk::Player::GetShip(client).Unwrap(); !ship)
        {
            client.Message(L"You must be in space to accept this.");
            return;
        }

        for (auto& duel : duels)
        {
            if (duel.client2 == client)
            {
                // Has player already accepted the bet?
                if (duel.accepted == true)
                {
                    client.Message(L"You have already accepted the challenge.");
                    return;
                }

                // Check the player can afford it
                const std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
                const auto cash = Hk::Player::GetCash(client).Unwrap();

                if (cash < duel.amount)
                {
                    client.Message(L"You don't have enough credits to accept this challenge");
                    return;
                }

                duel.accepted = true;
                const std::wstring message = characterName + L" has accepted the duel with " +
                                             reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(duel.client)) + L" for " +
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
    void Betting::UserCmdCancel()
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Hooks
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** @ingroup Betting
     * @brief Hook for dock call. Treats a player as if they died if they were part of a duel
     */
    int Betting::DockCall(const unsigned int& ship, [[maybe_unused]] const unsigned int& dock, [[maybe_unused]] const int& cancel,
                          [[maybe_unused]] const DOCK_HOST_RESPONSE& response)
    {
        if (const auto client = Hk::Client::GetClientIdByShip(ship).Unwrap(); client && Hk::Client::IsValidClientID(client))
        {
            ProcessFFA(client);
            ProcessDuel(client);
        }
        return 0;
    }

    /** @ingroup Betting
     * @brief Hook for disconnect. Treats a player as if they died if they were part of a duel
     */
    void Betting::DisConnect(ClientId& client, [[maybe_unused]] const EFLConnection& state)
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    /** @ingroup Betting
     * @brief Hook for char info request (F1). Treats a player as if they died if they were part of a duel
     */
    void Betting::CharacterInfoReq(ClientId& client, [[maybe_unused]] const bool& param2)
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    /** @ingroup Betting
     * @brief Hook for death to kick player out of duel
     */
    void Betting::SendDeathMessage([[maybe_unused]] const std::wstring& message, [[maybe_unused]] const uint& system, ClientId& clientVictim,
                                   [[maybe_unused]] const ClientId& clientKiller)
    {
        ProcessDuel(clientVictim);
        ProcessFFA(clientVictim);
    }
} // namespace Plugins


using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Betting", L"betting", PluginMajorVersion::V04, PluginMinorVersion::V01);


Betting::Betting(const PluginInfo& info) : Plugin(info)
{
    EmplaceHook(HookedCall::IEngine__SendDeathMessage, &Betting::SendDeathMessage, HookStep::After);
    EmplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &Betting::CharacterInfoReq, HookStep::After);
    EmplaceHook(HookedCall::IEngine__DockCall, &Betting::DockCall, HookStep::After);
    EmplaceHook(HookedCall::IServerImpl__DisConnect, &Betting::DisConnect, HookStep::After);
}