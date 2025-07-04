#include "PCH.hpp"

#include "BountyHuntPlugin.hpp"

namespace Plugins
{
    BountyHuntPlugin::BountyHuntPlugin(const PluginInfo& info) : Plugin(info) {}

    /** @ingroup BountyHunt
     * @brief Removed an active bounty hunt
     */
    std::vector<std::pair<ClientId, uint>> BountyHuntPlugin::ClearPlayerOfBounties(const ClientId client)
    {
        std::vector<std::pair<ClientId, uint>> rewards;

        for (const auto& [issuer, target, cash, end] : bountiesOnPlayers[client.GetValue()])
        {
            rewards.emplace_back(issuer, cash);
        }
        bountiesOnPlayers[client.GetValue()].clear();

        return rewards;
    }

    /** @ingroup BountyHunt
     * @brief Print all the active bounty hunts to the player
     */
    void BountyHuntPlugin::PrintBountyHunts(const ClientId client)
    {
        for (const auto& bounties : bountiesOnPlayers)
        {
            for (const auto& [issuer, target, cash, end] : bounties)
            {
                (void)client.Message(std::format(L"Kill {} and earn {} credits ({} minutes left)",
                                                 issuer.GetCharacterName().Unwrap(),
                                                 cash,
                                                 (end - TimeUtils::UnixTime<std::chrono::seconds>()) / 60));
            }
        }
    }

    /** @ingroup BountyHunt
     * @brief User Command for /bountyhunt. Creates a bounty against a specified player.
     */
    concurrencpp::result<void> BountyHuntPlugin::UserCmdBountyHunt(ClientId client, ClientId target, const uint prize, uint time)
    {
        if (!prize)
        {
            (void)client.Message(L"Usage: /bountyhunt <playername> <credits> [time]");
            PrintBountyHunts(client);
            co_return;
        }

        if (!target || target.InCharacterSelect())
        {
            (void)client.Message(std::format(L"{} is not online.", target));
            co_return;
        }

        if (const uint rankTarget = target.GetRank().Handle(); rankTarget < config.levelProtect)
        {
            (void)client.Message(L"Low level players may not be hunted.");
            co_return;
        }

        // clamp the hunting time to configured range, or set default if not specified
        if (time)
        {
            time = std::min(config.maximumHuntTime, std::max(config.minimalHuntTime, time));
        }
        else
        {
            time = config.defaultHuntTime;
        }

        if (const uint clientCash = client.GetCash().Unwrap(); clientCash < prize)
        {
            (void)client.Message(L"You do not possess enough credits.");
            co_return;
        }

        auto vec = bountiesOnPlayers[target.GetValue()];

        if (std::ranges::find_if(vec, [client](const Bounty& b) { return b.issuer == client; }) != vec.end())
        {
            (void)client.Message(L"You already have a bounty on this player.");
            co_return;
        }

        (void)client.RemoveCash(prize);

        Bounty bounty;
        bounty.issuer = client;
        bounty.end = TimeUtils::UnixTime<std::chrono::milliseconds>() + (static_cast<mstime>(time) * 60000);
        bounty.cash = prize;

        vec.push_back(bounty);

        FLHook::MessageUniverse(std::format(L"{} offers {} credits for killing {} in {} minutes.", bounty.issuer, std::to_wstring(bounty.cash), target, time));

        co_return;
    }

    /**
     * @brief Checks for expired bounties.
     */
    void BountyHuntPlugin::TimeOutCheck()
    {
        const auto time = TimeUtils::UnixTime<std::chrono::milliseconds>();

        for (auto& bounty : bountiesOnPlayers)
        {
            for (auto bountyIter = bounty.begin(); bountyIter != bounty.end();)
            {
                if (bountyIter->end < time)
                {
                    (void)bountyIter->issuer.AddCash(bountyIter->cash);

                    (void)FLHook::MessageUniverse(
                        std::format(L"{} was not hunted down and earned {} credits.", bountyIter->target.GetCharacterName().Handle(), bountyIter->cash));
                    bountyIter = bounty.erase(bountyIter);
                }
                else
                {
                    ++bountyIter;
                }
            }
        }
    }

    /** @ingroup BountyHunt
     * @brief Processes a ship death to see if it was part of a bounty.
     */
    void BountyHuntPlugin::BillCheck(const ClientId client, const ClientId killer)
    {
        auto victimName = client.GetCharacterName().Unwrap();
        if (!killer || client == killer)
        {
            (void)FLHook::MessageUniverse(std::format(L"The hunt for {} still goes on.", client.GetCharacterName().Unwrap()));
            return;
        }
        const auto bountyRewards = ClearPlayerOfBounties(client);
        uint reward = 0;

        for (const auto& [client, payout] : bountyRewards)
        {
            reward += payout;
        }
        (void)killer.AddCash(reward);
        auto killerName = killer.GetCharacterName().Unwrap();
        (void)FLHook::MessageUniverse(std::format(L"{} has killed {} and earned {} credits", killerName, victimName, reward));
    }

    // Timer Hook
    //  const std::vector<Timer> timers = {{TimeOutCheck, 60}};

    void BountyHuntPlugin::OnSendDeathMessageAfter(ClientId& killer, const ClientId victim, const SystemId system, std::wstring_view msg)
    {
        BillCheck(victim, killer);
    }

    void BountyHuntPlugin::CheckIfPlayerFled(const ClientId client)
    {
        if (bountiesOnPlayers[client.GetValue()].empty())
        {
            return;
        }

        const auto refunds = ClearPlayerOfBounties(client);
        (void)FLHook::MessageUniverse(std::format(L"The coward {} has fled. Issuers of this bounty have been refunded.", client.GetCharacterName().Unwrap()));

        for (const auto& [client, refund] : refunds)
        {
            // We use Handle's lambda feature and always return true to prevent any throws from canceling the traversal of the vector.
            (void)client.AddCash(refund);
        }
    }

    void BountyHuntPlugin::OnDisconnect(const ClientId client, EFLConnection connection) { CheckIfPlayerFled(client); }

    void BountyHuntPlugin::OnCharacterSelectAfter(const ClientId client) { CheckIfPlayerFled(client); }
    void BountyHuntPlugin::OnCharacterInfoRequest(ClientId client, bool dunno) { CheckIfPlayerFled(client); }
    bool BountyHuntPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/bountyhunt.json");
        return true;
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"Bounty Hunting",
        .shortName = L"bounty_hunting",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00
    };
};

SetupPlugin(BountyHuntPlugin);
