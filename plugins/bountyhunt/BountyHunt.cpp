/**
 * @date unknown
 * @author ||KOS||Acid (Ported by Raikkonen 2022)
 * @defgroup BountyHunt Bounty Hunt
 * @brief
 * The "Bounty Hunt" plugin allows players to put bounties on each other that can be collected by destroying that player.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - bountyhunt <player> <amount> [timelimit] - Places a bounty on the specified player. When another player kills them, they gain <credits>.
 * - bountyhuntid <id> <amount> [timelimit] - Same as above but with an id instead of a player name. Use /ids
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "enableBountyHunt": true,
 *     "levelProtect": 0,
 *     "minimalHuntTime": 1,
 *     "maximumHuntTime": 240,
 *     "defaultHuntTime": 30
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * None
 */

#include "PCH.hpp"

#include "BountyHunt.hpp"

namespace Plugins
{
    BountyHunt::BountyHunt(const PluginInfo& info) : Plugin(info) {}

    /** @ingroup BountyHunt
     * @brief Removed an active bounty hunt
     */
    std::vector<std::pair<ClientId, uint>> BountyHunt::ClearPlayerOfBounties(const ClientId client)
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
    void BountyHunt::PrintBountyHunts(const ClientId client)
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
    void BountyHunt::UserCmdBountyHunt(std::wstring_view target, const uint prize, uint time)
    {
        if (target.empty() || prize == 0)
        {
            (void)userCmdClient.Message(L"Usage: /bountyhunt <playername> <credits> <time>");
            PrintBountyHunts(userCmdClient);
            return;
        }

        const auto targetClient = ClientId(target);
        if (!targetClient || targetClient.InCharacterSelect())
        {
            (void)userCmdClient.Message(std::format(L"{} is not online.", target));
            return;
        }

        if (const uint rankTarget = targetClient.GetRank().Handle(); rankTarget < config->levelProtect)
        {
            (void)userCmdClient.Message(L"Low level players may not be hunted.");
            return;
        }

        // clamp the hunting time to configured range, or set default if not specified
        if (time)
        {
            time = std::min(config->maximumHuntTime, std::max(config->minimalHuntTime, time));
        }
        else
        {
            time = config->defaultHuntTime;
        }

        if (const uint clientCash = userCmdClient.GetCash().Unwrap(); clientCash < prize)
        {
            (void)userCmdClient.Message(L"You do not possess enough credits.");
            return;
        }

        auto vec = bountiesOnPlayers[targetClient.GetValue()];

        if (std::ranges::find_if(vec, [this](const Bounty& b) { return b.issuer == userCmdClient; }) != vec.end())
        {
            (void)userCmdClient.Message(L"You already have a bounty on this player.");
            return;
        }

        (void)userCmdClient.RemoveCash(prize);

        Bounty bounty;
        bounty.issuer = userCmdClient;
        bounty.end = TimeUtils::UnixTime<std::chrono::milliseconds>() + (static_cast<mstime>(time) * 60000);
        bounty.cash = prize;

        vec.push_back(bounty);

        FLHook::MessageUniverse(std::format(L"{} offers {} credits for killing {} in {} minutes.", bounty.issuer, std::to_wstring(bounty.cash), target, time));
    }

    /** @ingroup BountyHunt
     * @brief User Command for /bountyhuntid. Creates a bounty against a specified player.
     */
    void BountyHunt::UserCmdBountyHuntByClientID(const ClientId target, const uint credits, const uint time)
    {
        UserCmdBountyHunt(target.GetCharacterName().Handle(), credits, time);
    }

    /** @ingroup BountyHunt
     * @brief Checks for expired bounties.
     */
    void BountyHunt::TimeOutCheck()
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
    void BountyHunt::BillCheck(const ClientId client, const ClientId killer)
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

    /** @ingroup BountyHunt
     * @brief Hook for SendDeathMsg to call BillCheck
     */
    void BountyHunt::OnSendDeathMessageAfter(const ClientId killer, const ClientId victim, const SystemId system, std::wstring_view msg)
    {
        BillCheck(victim, killer);
    }

    void BountyHunt::CheckIfPlayerFled(const ClientId client)
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

    /** @ingroup BountyHunt
     * @brief Hook for Disconnect to see if the player had a bounty on them
     */
    void BountyHunt::OnDisconnect(const ClientId client, EFLConnection connection) { CheckIfPlayerFled(client); }

    /** @ingroup BountyHunt
     * @brief Hook for CharacterSelect to see if the player had a bounty on them
     */
    void BountyHunt::OnCharacterSelectAfter(const ClientId client) { CheckIfPlayerFled(client); }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"bounty hunt", L"bountyhunt", PluginMajorVersion::V05, PluginMinorVersion::V01);

SetupPlugin(BountyHunt, Info);
