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
                (void)client.MessageLocal(std::format(L"{} has been knocked out the FFA.", client.GetCharacterId().Handle()), 100000);
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
                contestantId.MessageLocal(std::format(L"{} has won the FFA and receives {} credits", contestantId.GetCharacterId().Handle(), pot), 100000);
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
    concurrencpp::result<void> BettingPlugin::UserCmdStartFreeForAll(const ClientId client, const uint amount)
    {
        // Check it's a valid amount of cash
        if (amount == 0)
        {
            (void)client.Message(L"Must specify a cash amount. Usage: /ffa <amount> e.g. /ffa 5000");
            co_return;
        }

        // Check the player can afford it
        if (const auto cash = client.GetCash().Handle(); amount > 0 && cash < amount)
        {
            (void)client.Message(L"You don't have enough credits to create this FFA.");
            co_return;
        }

        // Get the player's current system and location in the system.
        const SystemId systemId = client.GetSystemId().Handle();

        // Look in FreeForAll map, is an ffa happening in this system already?
        // If system doesn't have an ongoing ffa
        const auto freeForAllIter = freeForAlls.find(systemId);
        if (freeForAllIter == freeForAlls.end())
        {
            (void)client.Message(L"There is an FFA already happening in this system.");
            co_return;
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

            if (client == client2)
            {
                contestants[client2].accepted = true;
            }
            else
            {
                contestants[client2].accepted = false;
                (void)client2.Message(std::format(L"{} has started a Free-For-All tournament. Cost to enter is {} credits. Type \"/acceptffa\" to enter.",
                                                  client2.GetCharacterId().Handle(),
                                                  amount));
            }
        }

        // Are there any other players in this system?
        if (!contestants.empty())
        {
            (void)client.Message(L"Challenge issued. Waiting for others to accept.");
            entryAmount = amount;
            pot = amount;
            (void)client.RemoveCash(amount);
        }
        else
        {
            freeForAlls.erase(systemId);
            (void)client.Message(L"There are no other players in this system.");
        }

        co_return;
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /acceptffa
     */
    concurrencpp::result<void> BettingPlugin::UserCmdAcceptFFA(const ClientId client)
    {
        // Is player in space?
        if (!client.InSpace())
        {
            (void)client.Message(L"You must be in space to accept this.");
            co_return;
        }

        const auto freeForAllIter = freeForAlls.find(client.GetSystemId().Handle());
        if (freeForAllIter == freeForAlls.end())
        {
            (void)client.Message(L"There isn't an FFA in this system. Use /ffa to create one.");
            co_return;
        }

        auto& [contestants, entryAmount, pot] = freeForAllIter->second;

        // Check the player can afford it
        if (const auto cash = client.GetCash().Handle(); entryAmount > 0 && cash < entryAmount)
        {
            (void)client.Message(L"You don't have enough credits to join this FFA.");
            co_return;
        }

        // Accept
        if (auto& [accepted, loser] = contestants[client]; accepted == false)
        {
            accepted = true;
            loser = false;
            pot = pot + entryAmount;
            client.Message(std::format(L"{} credits have been deducted from your Neural Net account.", entryAmount));
            client.MessageLocal(std::format(L"{} has joined the FFA. Pot is now at {}", client.GetCharacterId().Handle(), pot), 100000);

            // Deduct cash
            (void)client.RemoveCash(entryAmount);
        }
        else
        {
            (void)client.Message(L"You have already accepted the FFA.");
        }

        co_return;
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
                    std::format(L"{} has won a duel against {}", clientKiller.GetCharacterId().Handle(), client.GetCharacterId().Handle()), 10000);

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
    concurrencpp::result<void> BettingPlugin::UserCmdDuel(const ClientId client, uint amount)
    {
        const auto target = client.GetShip().Handle().GetTarget().Handle();
        const auto targetClient = target.GetPlayer().Unwrap();

        if (!targetClient)
        {
            (void)client.MessageErr(L"Target must be a player");
            co_return;
        }

        // Check its a valid amount of cash
        if (amount == 0)
        {
            (void)client.Message(L"Must specify a cash amount. Usage: /duel <amount> e.g. /duel 5000");
            co_return;
        }

        // Check the player can afford it
        if (const uint cash = client.GetCash().Handle(); amount > 0 && cash < amount)
        {
            (void)client.Message(L"You don't have enough credits to issue this challenge.");
            co_return;
        }

        // Do either players already have a duel?
        for (const auto& [client1, client2, betAmount, accepted] : duels)
        {
            // Target already has a bet
            if (client1 == targetClient || client2 == targetClient)
            {
                (void)client.Message(L"This player already has an ongoing duel.");
                co_return;
            }

            // Player already has a bet
            if (client1 == client || client2 == client)
            {
                (void)client.Message(L"You already have an ongoing duel. Type /cancel");
                co_return;
            }
        }

        // Create duel
        Duel duel;
        duel.client = client;
        duel.client2 = targetClient;
        duel.betAmount = amount;
        duel.accepted = false;
        duels.push_back(duel);

        // Message players
        client.MessageLocal(
            std::format(L"{} has challenged {} to a duel for {} credits", client.GetCharacterId().Handle(), targetClient.GetCharacterId().Handle(), amount),
            10000);
        targetClient.Message(L"Type \"/acceptduel\" to accept.");
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /acceptduel to accept a duel request.
     */
    concurrencpp::result<void> BettingPlugin::UserCmdAcceptDuel(const ClientId client)
    {
        // Is player in space?
        if (!client.InSpace())
        {
            (void)client.Message(L"You must be in space to accept this.");
            co_return;
        }

        for (auto& [client1, client2, betAmount, accepted] : duels)
        {
            if (client2 != client)
            {
                continue;
            }

            // Has player already accepted the bet?
            if (accepted == true)
            {
                (void)client.Message(L"You have already accepted the challenge.");
                co_return;
            }

            // Check the player can afford it

            if (const uint cash = client.GetCash().Handle(); cash < betAmount)
            {
                (void)client.Message(L"You don't have enough credits to accept this challenge");
                co_return;
            }

            accepted = true;
            (void)client.MessageLocal(
                std::format(
                    L"{} has accepted the duel with {} for {} credits.", client.GetCharacterId().Handle(), client1.GetCharacterId().Handle(), betAmount),
                10000);
            co_return;
        }
        (void)client.Message(L"You have no duel requests. To challenge "
                             L"someone, target them and type /duel <amount>");

        co_return;
    }

    /** @ingroup Betting
     * @brief This method is called when a player types /cancel to cancel a duel/ffa request.
     */
    concurrencpp::result<void> BettingPlugin::UserCmdCancel(const ClientId client)
    {
        ProcessFFA(client);
        ProcessDuel(client);

        co_return;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Hooks
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void BettingPlugin::OnDockCallAfter(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
    {
        if (const auto client = shipId.GetPlayer().Unwrap(); client)
        {
            ProcessFFA(client);
            ProcessDuel(client);
        }
    }

    void BettingPlugin::OnDisconnect(const ClientId client, EFLConnection connection)
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    void BettingPlugin::OnCharacterInfoRequestAfter(const ClientId client, [[maybe_unused]] bool unk1)
    {
        ProcessFFA(client);
        ProcessDuel(client);
    }

    void BettingPlugin::OnSendDeathMessageAfter(ClientId& killer, const ClientId victim, SystemId system, std::wstring_view msg)
    {
        ProcessDuel(victim);
        ProcessFFA(victim);
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"Betting",
        .shortName = L"betting",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00
    };
};
SetupPlugin(BettingPlugin);
