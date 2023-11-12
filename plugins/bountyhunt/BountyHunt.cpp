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

    /** @ingroup BountyHunt
     * @brief Removed an active bounty hunt
     */
    std::vector<std::pair<std::wstring, uint>> BountyHunt::ClearPlayerOfBounties(ClientId client)
    {
        std::vector<std::pair<std::wstring, uint>> rewards;

        for (auto bounty : bountiesOnPlayers[client])
        {
            rewards.emplace_back(bounty.issuer, bounty.cash);
        }
        bountiesOnPlayers[client].clear();

        return rewards;
    }

    /** @ingroup BountyHunt
     * @brief Print all the active bounty hunts to the player
     */
    void BountyHunt::PrintBountyHunts(ClientId client)
    {
        for (const auto i : bountiesOnPlayers)
        {
            for (const auto j : i.second)
            {
                PrintUserCmdText(client,
                                 std::format(L"Kill {} and earn {} credits ({} minutes left)",
                                             i.first.GetCharacterName().Unwrap(),
                                             j.cash,
                                             ((j.end - TimeUtils::UnixSeconds()) / 60)));
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
            client.Message(L"Usage: /bountyhunt <playername> <credits> <time>");
            PrintBountyHunts(client);
            return;
        }

        
      
        const auto targetId = Hk::Client::GetClientIdFromCharName(target).Unwrap();

        const int rankTarget = Hk::Player::GetRank(targetId).Unwrap();
        if (targetId == UINT_MAX || Hk::Client::IsInCharSelectMenu(targetId))
        {
            client.Message(std::format(L"{} is not online.", target));
            return;
        }

        if (rankTarget < config->levelProtect)
        {
            client.Message(L"Low level players may not be hunted.");
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

        if (const uint clientCash = Hk::Player::GetCash(client).Unwrap(); clientCash < prize)
        {
            client.Message(L"You do not possess enough credits.");
            return;
        }

        auto vec = bountiesOnPlayers[targetId];

        const auto clientName = client.GetCharacterName().Handle();

        if (std::ranges::find_if(vec, [&clientName](const Bounty b) { return b.issuer == clientName; }) != vec.end())
        {
            client.Message(L"You already have a bounty on this player.");
            return;
        }

        Hk::Player::RemoveCash(client, prize);
        const std::wstring initiator = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

        Bounty bounty;
        bounty.issuer = client.GetCharacterName().Unwrap();
        bounty.end = TimeUtils::UnixMilliseconds() + (static_cast<mstime>(time) * 60000);
        bounty.cash = prize;

        vec.push_back(bounty);

        Hk::Chat::MsgU(std::format(L"{} offers {} credits for killing {} in {} minutes.", bounty.issuer, std::to_wstring(bounty.cash), target, time));
    }

    /** @ingroup BountyHunt
     * @brief User Command for /bountyhuntid. Creates a bounty against a specified player.
     */
    void BountyHunt::UserCmdBountyHuntByClientID( const ClientId target, uint credits, uint time)
    {
        const std::wstring charName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(target));
        UserCmdBountyHunt(charName, credits, time);
    }

    /** @ingroup BountyHunt
     * @brief Checks for expired bounties.
     */
    void BountyHunt::TimeOutCheck()
    {
        const auto time = TimeUtils::UnixMilliseconds();

        for (auto i : bountiesOnPlayers)
        {
            std::ranges::remove_if(i.second,
                                   [&time, &i](const Bounty b)
                                   {
                                       if (b.end < time)
                                       {
                                           Hk::Player::AddCash(b.issuer, b.cash);
                                           auto targetName = i.first.GetCharacterName().Unwrap();
                                           Hk::Chat::MsgU(std::format(L"{} was not hunted down and earned {} credits.", targetName, b.cash));
                                           return true;
                                       }
                                       return false;
                                   });
        }
    }

    /** @ingroup BountyHunt
     * @brief Processes a ship death to see if it was part of a bounty.
     */
    void BountyHunt::BillCheck(ClientId& client, ClientId& killer)
    {
        auto victimName = client.GetCharacterName().Unwrap();
        if (killer == 0 || client == killer)
        {
            Hk::Chat::MsgU(std::format(L"The hunt for {} still goes on.", client.GetCharacterName().Unwrap()));
            return;
        }
        auto bountyRewards = ClearPlayerOfBounties(client);
        uint reward = 0;

        for (const auto i : bountyRewards)
        {
            reward += i.second;
        }
        Hk::Player::AddCash(killer, reward);
        auto killerName = killer.GetCharacterName().Unwrap();
        Hk::Chat::MsgU(std::format(L"{} has killed {} and earned {} credits", killerName, victimName, reward));
    }

    // Timer Hook
  //  const std::vector<Timer> timers = {{TimeOutCheck, 60}};

    /** @ingroup BountyHunt
     * @brief Hook for SendDeathMsg to call BillCheck
     */
    void BountyHunt::SendDeathMsg([[maybe_unused]] const std::wstring& msg, [[maybe_unused]] const SystemId& system, ClientId& clientVictim,
                                  ClientId& clientKiller)
    {
        BillCheck(clientVictim, clientKiller);
    }

    void BountyHunt::CheckIfPlayerFled(ClientId& client)
    {
        if (bountiesOnPlayers[client].empty())
        {
            return;
        }

        const auto refunds = ClearPlayerOfBounties(client);
        Hk::Chat::MsgU(std::format(L"The coward {} has fled. Issuers of this bounty have been refunded.", client.GetCharacterName().Unwrap()));

        for (const auto& i : refunds)
        {
            // We use Handle's lambda feature and always return true to prevent any throws from canceling the traversal of the vector.
            Hk::Player::AddCash(i.first, i.second).Handle([](Error err, std::wstring_view msg) { return true; });
        }
    }

    /** @ingroup BountyHunt
     * @brief Hook for Disconnect to see if the player had a bounty on them
     */
    void BountyHunt::DisConnect(ClientId& client, [[maybe_unused]] const EFLConnection& state) { CheckIfPlayerFled(client); }

    /** @ingroup BountyHunt
     * @brief Hook for CharacterSelect to see if the player had a bounty on them
     */
    void BountyHunt::CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client) { CheckIfPlayerFled(client); }


    // Load Settings
    void BountyHunt::LoadSettings() { config = Serializer::LoadFromJson<Config>(L"config/bountyHunt.json"); }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"bounty hunt", L"bountyhunt", PluginMajorVersion::V04, PluginMinorVersion::V01);

BountyHunt::BountyHunt(PluginInfo& info) : Plugin(info)
{
    EmplaceHook(HookedCall::IEngine__SendDeathMessage, &BountyHunt::SendDeathMsg, HookStep::After);
    EmplaceHook(HookedCall::FLHook__LoadSettings, &BountyHunt::LoadSettings, HookStep::After);
    EmplaceHook(HookedCall::IServerImpl__DisConnect, &BountyHunt::DisConnect);
    EmplaceHook(HookedCall::IServerImpl__CharacterSelect, &BountyHunt::CharacterSelect, HookStep::After);
}


/*
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
    pi->name("Bounty Hunt");
    pi->shortName("bountyhunt");
    pi->mayUnload(false);
    pi->commands(&commands);
    pi->timers(&timers);
    pi->returnCode(&returnCode);
    pi->versionMajor(PluginMajorVersion::V04);
    pi->versionMinor(PluginMinorVersion::V00);
    pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
    pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect, HookStep::After);
}
*/