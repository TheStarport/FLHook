#include "PCH.hpp"

#include "BettingPlugin.hpp"

namespace Plugins
{

    BettingPlugin::BettingPlugin(const PluginInfo& info) : Plugin(info) {}

    /** @ingroup Betting
     * @brief If the player who died is in an FreeForAll, mark them as a loser. Also handles payouts to winner.
     */
    void BettingPlugin::ProcessFFA(const ClientId client)
    {
        for (auto& [key, val] : freeForAlls)
        {
            auto& [contestants, entryAmount, pot] = val;
            if (!contestants[client].accepted || contestants[client].loser)
            {
                continue;
            }
            if (contestants.contains(client))
            {
                contestants[client].loser = true;
                (void)client.MessageLocal(std::format(L"{} has been knocked out the FFA.", client.GetCharacterName().Handle()), 100000);
            }

            // Is the FreeForAll over?
            int count = 0;
            ClientId contestantId;
            for (const auto& [id, contestant] : contestants)
            {
                if (contestant.loser == false && contestant.accepted == true)
                {
                    count++;
                    contestantId = id;
                }
            }

            // Has the FreeForAll been won?
            if (count > 1)
            {
                continue;
            }

            if (contestantId.IsValidClientId())
            {
                // Announce and pay winner
                (void)contestantId.AddCash(pot);
                contestantId.MessageLocal(std::format(L"{} has won the FFA and receives {} credits", contestantId.GetCharacterName().Handle(), pot), 100000);
            }
            else
            {
                PlayerData* playerData = nullptr;
                while ((playerData = Players.traverse_active(playerData)))
                {
                    if (const auto localClient = ClientId(playerData->clientId); key == localClient.GetSystemId().Handle())
                    {
                        (void)localClient.Message(L"No one has won the FFA.");
                    }
                }
            }
            // Delete event
            freeForAlls.erase(key);
            return;
        }
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /ffa in an attempt to start a pvp event
     */
    void BettingPlugin::UserCmdStartFreeForAll(const uint amount)
    {
        // Check its a valid amount of cash
        if (amount == 0)
        {
            (void)userCmdClient.Message(L"Must specify a cash amount. Usage: /ffa <amount> e.g. /ffa 5000");
            return;
        }

        // Check the player can afford it
        if (const auto cash = userCmdClient.GetCash().Handle(); amount > 0 && cash < amount)
        {
            (void)userCmdClient.Message(L"You don't have enough credits to create this FFA.");
            return;
        }

        // Get the player's current system and location in the system.
        const SystemId systemId = userCmdClient.GetSystemId().Handle();

        // Look in FreeForAll map, is an ffa happening in this system already?
        // If system doesn't have an ongoing ffa
        const auto freeForAllIter = freeForAlls.find(systemId);
        if (freeForAllIter == freeForAlls.end())
        {
            (void)userCmdClient.Message(L"There is an FFA already happening in this system.");
            return;
        }
        auto [contestants, entryAmount, pot] = freeForAllIter->second;
        // Get a list of other players in the system
        // Add them and the player into the ffa map
        PlayerData* playerData = nullptr;
        while ((playerData = Players.traverse_active(playerData)))
        {
            // Get the this player's current system

            auto client2 = ClientId(playerData->clientId);
            if (systemId != ClientId(playerData->clientId).GetSystemId().Handle())
            {
                continue;
            }

            // Add them to the contestants freeForAlls
            contestants[client2].loser = false;

            if (userCmdClient == client2)
            {
                contestants[client2].accepted = true;
            }
            else
            {
                contestants[client2].accepted = false;
                (void)client2.Message(std::format(L"{} has started a Free-For-All tournament. Cost to enter is {} credits. Type \"/acceptffa\" to enter.",
                                                  client2.GetCharacterName().Handle(),
                                                  amount));
            }
        }

        // Are there any other players in this system?
        if (!contestants.empty())
        {
            (void)userCmdClient.Message(L"Challenge issued. Waiting for others to accept.");
            entryAmount = amount;
            pot = amount;
            (void)userCmdClient.RemoveCash(amount);
        }
        else
        {
            freeForAlls.erase(systemId);
            (void)userCmdClient.Message(L"There are no other players in this system.");
        }
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /acceptffa
     */
    void BettingPlugin::UserCmdAcceptFFA()
    {
        // Is player in space?
        if (!userCmdClient.InSpace())
        {
            (void)userCmdClient.Message(L"You must be in space to accept this.");
            return;
        }

        const auto freeForAllIter = freeForAlls.find(userCmdClient.GetSystemId().Handle());
        if (freeForAllIter == freeForAlls.end())
        {
            (void)userCmdClient.Message(L"There isn't an FFA in this system. Use /ffa to create one.");
            return;
        }

        auto& [contestants, entryAmount, pot] = freeForAllIter->second;

        // Check the player can afford it
        if (const auto cash = userCmdClient.GetCash().Handle(); entryAmount > 0 && cash < entryAmount)
        {
            (void)userCmdClient.Message(L"You don't have enough credits to join this FFA.");
            return;
        }

        // Accept
        if (auto& [accepted, loser] = contestants[userCmdClient]; accepted == false)
        {
            accepted = true;
            loser = false;
            pot = pot + entryAmount;
            userCmdClient.Message(std::format(L"{} credits have been deducted from your Neural Net account.", entryAmount));
            userCmdClient.MessageLocal(std::format(L"{} has joined the FFA. Pot is now at {}", userCmdClient.GetCharacterName().Handle(), pot), 100000);

            // Deduct cash
            (void)userCmdClient.RemoveCash(entryAmount);
        }
        else
        {
            (void)userCmdClient.Message(L"You have already accepted the FFA.");
        }
    }

    /** @ingroup Betting
     * @brief Removes any duels with this client and handles payouts.
     */
    void BettingPlugin::ProcessDuel(const ClientId client)
    {
        auto duel = duels.begin();
        while (duel != duels.end())
        {
            ClientId clientKiller;

            if (duel->client == client)
            {
                clientKiller = duel->client2;
            }
            else if (duel->client2 == client)
            {
                clientKiller = duel->client;
            }

            if (!clientKiller)
            {
                ++duel;
                continue;
            }

            if (duel->accepted)
            {
                // Prepare and send message
                clientKiller.MessageLocal(
                    std::format(L"{} has won a duel against {}", clientKiller.GetCharacterName().Handle(), client.GetCharacterName().Handle()), 10000);

                // Change cash
                clientKiller.AddCash(duel->betAmount);
                client.RemoveCash(duel->betAmount);
            }
            else
            {
                (void)duel->client.Message(L"Duel cancelled.");
                (void)duel->client2.Message(L"Duel cancelled.");
            }
            duel = duels.erase(duel);
            return;
        }
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /duel in an attempt to start a duel
     */
    void BettingPlugin::UserCmdDuel(uint amount)
    {
        // Get the object the player is targetting
        if (!userCmdClient.InSpace())
        {
            (void)userCmdClient.Message(L"Must be in space");
            return;
        }

        const auto target = userCmdClient.GetShipId().Unwrap().GetTarget();
        if (!target.has_value())
        {
            (void)userCmdClient.Message(L"Must target a player");
            return;
        }

        // Check ship is a player
        const auto clientTarget = target->GetPlayer();
        if (!clientTarget.has_value())
        {
            (void)userCmdClient.Message(L"Must target a player");
            return;
        }

        // Check its a valid amount of cash
        if (amount == 0)
        {
            (void)userCmdClient.Message(L"Must specify a cash amount. Usage: /duel <amount> e.g. /duel 5000");
            return;
        }

        // Check the player can afford it
        if (const uint cash = userCmdClient.GetCash().Handle(); amount > 0 && cash < amount)
        {
            (void)userCmdClient.Message(L"You don't have enough credits to issue this challenge.");
            return;
        }

        // Do either players already have a duel?
        for (const auto& [client, client2, betAmount, accepted] : duels)
        {
            // Target already has a bet
            if (client == clientTarget || client2 == clientTarget)
            {
                (void)userCmdClient.Message(L"This player already has an ongoing duel.");
                return;
            }
            // Player already has a bet
            if (client == userCmdClient || client2 == userCmdClient)
            {
                (void)userCmdClient.Message(L"You already have an ongoing duel. Type /cancel");
                return;
            }
        }

        // Create duel
        Duel duel;
        duel.client = userCmdClient;
        duel.client2 = clientTarget.value();
        duel.betAmount = amount;
        duel.accepted = false;
        duels.push_back(duel);

        // Message players
        (void)userCmdClient.MessageLocal(
            std::format(
                L"{} has challenged {} to a duel for {} credits", userCmdClient.GetCharacterName().Handle(), clientTarget->GetCharacterName().Handle(), amount),
            10000);
        (void)clientTarget->Message(L"Type \"/acceptduel\" to accept.");
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /acceptduel to accept a duel request.
     */
    void BettingPlugin::UserCmdAcceptDuel()
    {
        // Is player in space?
        if (!userCmdClient.InSpace())
        {
            (void)userCmdClient.Message(L"You must be in space to accept this.");
            return;
        }

        for (auto& [client, client2, betAmount, accepted] : duels)
        {
            if (client2 != userCmdClient)
            {
                continue;
            }

            // Has player already accepted the bet?
            if (accepted == true)
            {
                (void)userCmdClient.Message(L"You have already accepted the challenge.");
                return;
            }

            // Check the player can afford it

            if (const uint cash = userCmdClient.GetCash().Handle(); cash < betAmount)
            {
                (void)userCmdClient.Message(L"You don't have enough credits to accept this challenge");
                return;
            }

            accepted = true;
            (void)userCmdClient.MessageLocal(std::format(L"{} has accepted the duel with {} for {} credits.",
                                                         userCmdClient.GetCharacterName().Handle(),
                                                         client.GetCharacterName().Handle(),
                                                         betAmount),
                                             10000);
            return;
        }
        (void)userCmdClient.Message(L"You have no duel requests. To challenge "
                                    L"someone, target them and type /duel <amount>");
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /cancel to cancel a duel/ffa request.
     */
    void BettingPlugin::UserCmdCancel()
    {
        ProcessFFA(userCmdClient);
        ProcessDuel(userCmdClient);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Hooks
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** @ingroup Betting
     * @brief Hook for dock call. Treats a player as if they died if they were part of a duel
     */
    void BettingPlugin::OnDockCallAfter(const ShipId shipId, ObjectId spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
    {
        if (const auto client = shipId.GetPlayer(); client.has_value())
        {
            ProcessFFA(client.value());
            ProcessDuel(client.value());
        }
    }

    /** @ingroup Betting
     * @brief Hook for disconnect. Treats a player as if they died if they were part of a duel
     */
    void BettingPlugin::OnDisconnect(const ClientId client, EFLConnection connection)
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    /** @ingroup Betting
     * @brief Hook for char info request (F1). Treats a player as if they died if they were part of a duel
     */
    void BettingPlugin::OnCharacterInfoRequestAfter(const ClientId client, [[maybe_unused]] bool unk1)
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    /** @ingroup Betting
     * @brief Hook for death to kick player out of duel
     */
    void BettingPlugin::OnSendDeathMessageAfter(const ClientId killer, const ClientId victim, SystemId system, std::wstring_view msg)
    {
        ProcessDuel(victim);
        ProcessFFA(victim);
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Betting", L"betting", PluginMajorVersion::V05, PluginMinorVersion::V01);

SetupPlugin(BettingPlugin, Info);
