/**
 * @date August, 2022
 * @author Ported from 88Flak by Raikkonen
 * @defgroup DeathPenalty Death Penalty
 * @brief
 * This plugin charges players credits for dying based on their ship worth. If the killer was a player it also rewards them.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - dp - Shows the credits you would be charged if you died.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "DeathPenaltyFraction": 1.0,
 *     "DeathPenaltyFractionKiller": 1.0,
 *     "ExcludedSystems": ["li01"],
 *     "FractionOverridesByShip": {"ge_fighter": 1.0}
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */
#include "PCH.hpp"

#include "DeathPenalty.hpp"

#include "API/FLHook/ClientList.hpp"

namespace Plugins
{
    // Load configuration file
    bool DeathPenaltyPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/deathpenalty.json");

        return true;
    }

    void DeathPenaltyPlugin::OnClearClientInfo(ClientId client) { ClientInfo.erase(client); }

    /** @ingroup DeathPenalty
     * @brief Is the player is a system that is excluded from death penalty?
     */
    bool DeathPenaltyPlugin::IsInExcludedSystem(ClientId client) const
    {
        // Get System Id
        const SystemId systemId = client.GetSystemId().Handle();
        // Search list for system
        return ExcludedSystemsIds.contains(systemId);
    }

    /** @ingroup DeathPenalty
     * @brief This returns the override for the specific ship as defined in the json file.
     * If there is not override it returns the default value defined as
     * "DeathPenaltyFraction" in the json file
     */
    float DeathPenaltyPlugin::GetShipFractionOverride(const ClientId client)
    {
        // Get ShipArchId
        const auto shipArchId = EquipmentId(client.GetShipArch().Unwrap().Cast<Archetype::Ship>().Handle()->archId);

        // Default return value is the default death penalty fraction
        float overrideValue = config.DeathPenaltyFraction;

        // See if the ship has an override fraction
        if (FractionOverridesByShipIds.contains(shipArchId))
        {
            overrideValue = FractionOverridesByShipIds[shipArchId];
        }

        return overrideValue;
    }

    /** @ingroup DeathPenalty
     * @brief Hook on Player Launch. Used to work out the death penalty and display a message to the player warning them of such
     */
    void DeathPenaltyPlugin::OnPlayerLaunchAfter(const ClientId client, const ShipId& ship)
    {
        // No point in processing anything if there is no death penalty
        if (config.DeathPenaltyFraction > 0.00001f)
        {
            // Check to see if the player is in a system that doesn't have death
            // penalty
            if (!IsInExcludedSystem(client))
            {
                // Get the players net worth
                const auto shipValue = client.GetValue();

                const auto cash = client.GetCash().Handle();

                auto penaltyCredits = static_cast<uint>(static_cast<float>(shipValue) * GetShipFractionOverride(client));
                if (cash < penaltyCredits)
                {
                    penaltyCredits = cash;
                }

                // Calculate what the death penalty would be upon death
                ClientInfo[client].deathPenaltyCredits = penaltyCredits;

                // Should we print a death penalty notice?
                if (ClientInfo[client].displayDPOnLaunch)
                {
                    client.Message(std::format(L"Notice: the death penalty for your ship will be {} credits. Type /dp for more information.",
                                               StringUtils::ToMoneyStr(ClientInfo[client].deathPenaltyCredits)));
                }
            }
            else
            {
                ClientInfo[client].deathPenaltyCredits = 0;
            }
        }
    }

    /** @ingroup DeathPenalty
     * @brief Load settings directly from the player's save directory
     */
    void DeathPenaltyPlugin::OnCharacterSelectAfter(const ClientId client)
    {
        const auto view = client.GetData().characterData->characterDocument;
        if (auto findResult = view.find("deathPenaltyDisplay"); findResult != view.end())
        {
            ClientInfo[client].displayDPOnLaunch = findResult->get_bool();
        }
    }
    void DeathPenaltyPlugin::OnCharacterSave(const ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document)
    {
        document.append(bsoncxx::builder::basic::kvp("deathPenaltyDisplay", ClientInfo[client].displayDPOnLaunch));
    }
    void DeathPenaltyPlugin::OnSendDeathMessageAfter(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg)
    {
        if(victim && (config.penalizePvpOnly || killer))
        {
            PenalizeDeath(victim, killer);
        }
    }

    /** @ingroup DeathPenalty
     * @brief Apply the death penalty on a player death
     */
    void DeathPenaltyPlugin::PenalizeDeath(const ClientId client, const ClientId killerId)
    {
        if (config.DeathPenaltyFraction < 0.00001f)
        {
            return;
        }

        // Valid client and the ShipArch or System isnt in the excluded list?
        if (IsInExcludedSystem(client))
        {
            return;
        }
        // Get the players cash
        const auto cash = client.GetCash().Handle();

        // Get how much the player owes
        uint cashOwed = ClientInfo[client].deathPenaltyCredits;

        // If the amount the player owes is more than they have, set the
        // amount to their total cash
        if (cashOwed > cash)
        {
            cashOwed = cash;
        }

        // If another player has killed the player
        if (killerId && killerId != client && config.DeathPenaltyFractionKiller > 0.0f)
        {
            if (const auto killerReward = static_cast<uint>(static_cast<float>(cashOwed) * config.DeathPenaltyFractionKiller))
            {
                // Reward the killer, print message to them
                (void)killerId.AddCash(killerReward);
                killerId.Message(std::format(L"Death penalty: given {} credits from {}'s death penalty.",
                                             StringUtils::ToMoneyStr(killerReward),
                                             client.GetCharacterName().Handle()));
            }
        }

        if (cashOwed)
        {
            // Print message to the player and remove cash
            (void)client.Message(L"Death penalty: charged " + StringUtils::ToMoneyStr(cashOwed) + L" credits.");
            (void)client.RemoveCash(cashOwed);
        }
    }

    void DeathPenaltyPlugin::OnJumpInComplete(const SystemId system, const ShipId& ship)
    {
        if (const auto player = ship.GetPlayer().Unwrap(); player)
        {
            ClientInfo[player].isJumping = false;
        }
    }

    void DeathPenaltyPlugin::KillIfInJumpTunnel(const ClientId client)
    {
        if (config.KillOnDisconnect && ClientInfo[client].isJumping)
        {
            client.GetShip().Handle().Destroy();
        }
    }

    void DeathPenaltyPlugin::OnDisconnect(const ClientId client, EFLConnection connection) { KillIfInJumpTunnel(client); }

    void DeathPenaltyPlugin::OnCharacterInfoRequest(const ClientId client, bool unk1) { KillIfInJumpTunnel(client); }

    bool DeathPenaltyPlugin::OnSystemSwitchOutPacket(const ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet)
    {
        if (const auto jumpingClient = ShipId(packet.shipId).GetPlayer().Unwrap(); jumpingClient)
        {
            ClientInfo[jumpingClient].isJumping = true;
        }
        return true;
    }

    DeathPenaltyPlugin::DeathPenaltyPlugin(const PluginInfo& info) : Plugin(info) {}

    /** @ingroup DeathPenalty
     * @brief /dp command. Shows information about death penalty
     */
    Task DeathPenaltyPlugin::UserCmdDeathPenalty(const ClientId client, const std::wstring_view param)
    {
        // If there is no death penalty, no point in having death penalty commands
        if (std::abs(config.DeathPenaltyFraction) < 0.0001f)
        {
            WARN(L"DP Plugin active, but no/too low death penalty fraction is set.");
            co_return TaskStatus::Finished;
        }

        if (!param.empty()) // Arguments passed
        {
            if (StringUtils::ToLower(param) == L"off")
            {
                ClientInfo[client].displayDPOnLaunch = false;
                (void)client.Message(L"Death penalty notices disabled.");
            }
            else if (StringUtils::ToLower(param) == L"on")
            {
                ClientInfo[client].displayDPOnLaunch = true;
                (void)client.Message(L"Death penalty notices enabled.");
            }
            else
            {
                (void)client.Message(L"ERR Invalid parameters");
                (void)client.Message(L"/dp on | /dp off");
            }
        }
        else
        {
            (void)client.Message(L"The death penalty is charged immediately when you die.");
            if (!IsInExcludedSystem(client))
            {
                const auto shipValue = client.GetValue();
                const auto cashOwed = static_cast<uint>(static_cast<float>(shipValue) * GetShipFractionOverride(client));
                const uint playerCash = client.GetCash().Handle();

                (void)client.Message(
                    std::format(L"The death penalty for your ship will be {} credits.", StringUtils::ToMoneyStr(std::min(cashOwed, playerCash))));
                (void)client.Message(L"If you would like to turn off the death penalty notices, run "
                                     L"this command with the argument \"off\".");
            }
            else
            {
                (void)client.Message(L"You don't have to pay the death penalty "
                                     L"because you are in a specific system.");
            }
        }

        co_return TaskStatus::Finished;
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Death Penalty", L"deathpenalty", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(DeathPenaltyPlugin, Info);
