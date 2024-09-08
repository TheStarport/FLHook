

#include "PCH.hpp"

#include "KillTracker.hpp"

namespace Plugins
{
    void KillTrackerPlugin::ClearDamageTaken(const ClientId client) { damageArray[client.GetValue()].fill({ 0.0f, 0.0f }); }
    void KillTrackerPlugin::ClearDamageDone(const ClientId client, const bool fullReset)
    {
        for (int i = 1; i <= MaxClientId; i++)
        {
            auto& [currDamage, lastUndockDamage] = damageArray[i][client.GetValue()];
            if (fullReset)
            {
                lastUndockDamage = 0.0f;
            }
            else
            {
                lastUndockDamage = currDamage;
            }
            currDamage = 0.0f;
        }
    }

    void KillTrackerPlugin::OnCharacterSelectAfter(ClientId client)
    {
        ClearDamageTaken(client);
        ClearDamageDone(client, true);
    }
    void KillTrackerPlugin::OnDisconnectAfter(ClientId client, EFLConnection connection)
    {
        ClearDamageTaken(client);
        ClearDamageDone(client, true);
    }
    void KillTrackerPlugin::OnClearClientInfo(ClientId client)
    {
        ClearDamageTaken(client);
        ClearDamageDone(client, true);
    }
    void KillTrackerPlugin::OnPlayerLaunchAfter(ClientId client, ShipId ship)
    {
        ClearDamageTaken(client);
        ClearDamageDone(client, false);
    }

    void KillTrackerPlugin::OnShipHullDmg(Ship* ship, float& damage, DamageList* dmgList)
    {
        if (!dmgList->inflictorPlayerId)
        {
            return;
        }

        if (const auto shipOwnerPlayer = ship->cship()->ownerPlayer; shipOwnerPlayer && shipOwnerPlayer != dmgList->inflictorPlayerId && damage > 0.0f)
        {
            damageArray[shipOwnerPlayer][dmgList->inflictorPlayerId].currDamage += damage;
        }
    }
    void KillTrackerPlugin::OnSendDeathMessage(ClientId killer, ClientId victim, SystemId system, std::wstring_view msg)
    {
        // TODO: Print death message, supress core death message printing.
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"KillTracker", L"killtracker", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(KillTrackerPlugin, Info);
