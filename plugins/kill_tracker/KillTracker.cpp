

#include "PCH.hpp"

#include "KillTracker.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Random.hpp"

namespace Plugins
{
    void KillTrackerPlugin::ClearDamageTaken(const ClientId client) { damageArray[client.GetValue()].fill({ 0.0f, 0.0f }); }
    void KillTrackerPlugin::ClearDamageDone(const ClientId client, const bool fullReset)
    {
        for (int i = 1; i <= MaxClientId; i++)
        {
            auto& [currDamage, lastUndockDamage, hasAttacked] = damageArray[i][client.GetValue()];
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
    bool KillTrackerPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/killtracker.json");

        return true;
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
    void KillTrackerPlugin::OnPlayerLaunchAfter(ClientId client, const ShipId& ship)
    {
        ClearDamageTaken(client);
        ClearDamageDone(client, false);
    }

    void KillTrackerPlugin::LogFirstStrike(Ship* ship, DamageList* dmgList)
    {
        uint targetClient = ship->cship()->ownerPlayer;

        damageArray[targetClient][dmgList->inflictorPlayerId].hasAttacked = true;

        std::wstring equipEntries;
        bool firstEntry = true;
        for (auto& equip : ClientId(targetClient).GetEquipCargo().Handle()->equip)
        {
            if (equip.mounted || !InternalApi::IsCommodity(equip.archId))
            {
                continue;
            }

            if (!firstEntry)
            {
                equipEntries += L", ";
            }
            else
            {
                firstEntry = false;
            }
            auto name = InternalApi::HashLookup(equip.archId.GetValue());
            equipEntries += StringUtils::stows(name);
        }

        INFO("{{attacker}} has attacked {{victim}} in {{sysName}}, {{pos}}",
             { "attacker", ClientId(dmgList->inflictorPlayerId) },
             { "victim", ClientId(targetClient) },
             { "sysName", ship->cship()->system.GetName().Handle() },
             { "pos", ship->get_position() },
             { "victim_cargo", equipEntries },
             LOG_CAT("first_strike"));
    }

    void KillTrackerPlugin::OnShipHullDmg(Ship* ship, float& damage, DamageList* dmgList)
    {
        uint targetClient = ship->cship()->ownerPlayer;
        if (!targetClient || !dmgList->inflictorPlayerId || targetClient == dmgList->inflictorPlayerId || damage <= 0.0f ||
            (Players[targetClient].playerGroup && Players[targetClient].playerGroup == Players[dmgList->inflictorPlayerId].playerGroup))
        {
            return;
        }

        damageArray[targetClient][dmgList->inflictorPlayerId].currDamage += damage;

        if (config.logPlayerWhoShotFirst && dmgList->damageCause != DamageCause::Collision && dmgList->damageCause != DamageCause::CruiseDisrupter)
        {
            LogFirstStrike(ship, dmgList);
        }
    }

    void KillTrackerPlugin::OnShipShieldDmg(Ship* ship, CEShield* shield, float& damage, DamageList* dmgList)
    {
        uint targetClient = ship->cship()->ownerPlayer;
        if (!targetClient || !dmgList->inflictorPlayerId || targetClient == dmgList->inflictorPlayerId || damage <= 0.0f ||
            (Players[targetClient].playerGroup && Players[targetClient].playerGroup == Players[dmgList->inflictorPlayerId].playerGroup))
        {
            return;
        }

        damageArray[targetClient][dmgList->inflictorPlayerId].currDamage += damage;

        if (config.logPlayerWhoShotFirst && dmgList->damageCause != DamageCause::Collision && dmgList->damageCause != DamageCause::CruiseDisrupter)
        {
            LogFirstStrike(ship, dmgList);
        }
    }

    float KillTrackerPlugin::GetDamageDone(const DamageDone& damageDone)
    {
        if (damageDone.currDamage != 0.0f)
        {
            return damageDone.currDamage;
        }
        if (damageDone.lastUndockDamage != 0.0f)
        {
            return damageDone.lastUndockDamage;
        }
        return 0.0f;
    }

    enum MessageType
    {
        MSGNONE,
        MSGBLUE,
        MSGRED,
        MSGDARKRED
    };

    MessageType GetMessageType(const ClientId victimId, const ClientId& recevingClient, const SystemId system, bool isGroupInvolved)
    {
        const auto dieMsgType = victimId.GetData().dieMsg;
        if (dieMsgType == DieMsgType::None)
        {
            return MSGNONE;
        }

        if (isGroupInvolved)
        {
            return MSGBLUE;
        }

        if (dieMsgType == DieMsgType::Self)
        {
            if (recevingClient.GetValue() == victimId.GetValue())
            {
                return MSGBLUE;
            }
        }
        else if (dieMsgType == DieMsgType::System)
        {
            if (recevingClient.GetValue() == victimId.GetValue())
            {
                return MSGBLUE;
            }
            if (recevingClient.GetSystemId().Handle().GetValue() == system.GetValue())
            {
                return MSGRED;
            }
        }
        else if (dieMsgType == DieMsgType::All)
        {
            if (recevingClient.GetValue() == victimId.GetValue())
            {
                return MSGBLUE;
            }
            if (recevingClient.GetSystemId().Handle().GetValue() == system.GetValue())
            {
                return MSGRED;
            }
            return MSGDARKRED;
        }
        return MSGNONE;
    }

    void ProcessDeath(ClientId victimId, const std::wstring_view message1, const std::wstring_view message2, const SystemId system, bool isPvP,
                      const std::set<uint>& involvedGroups, const std::set<ClientId>& involvedPlayers)
    {
        const std::wstring deathMessageBlue1 = std::format(L"<TRA data=\"0xFF000001" // Blue, Bold
                                                           L"\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                                           message1);
        const std::wstring deathMessageRed1 = std::format(L"<TRA data=\"0x0000CC01" // Red, Bold
                                                          L"\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                                          message1);
        const std::wstring deathMessageDarkRed = std::format(L"<TRA data=\"0x18188c01" // Dark Red, Bold
                                                             L"\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                                             message1);

        std::wstring deathMessageRed2;
        std::wstring deathMessageBlue2;
        if (!message2.empty())
        {
            deathMessageRed2 = std::format(L"<TRA data=\"0x0000CC01" // Red, Bold
                                           L"\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                           message2);

            deathMessageBlue2 = std::format(L"<TRA data=\"0xFF000001" // Blue, Bold
                                            L"\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                            message2);
        }

        for (const auto& clientData : FLHook::Clients())
        {
            const auto client = clientData.id;
            const bool isInvolved = involvedGroups.contains(client.GetGroup().Unwrap().GetValue()) || involvedPlayers.contains(client);

            if (const MessageType msgType = GetMessageType(victimId, client, system, isInvolved); msgType == MSGBLUE)
            {
                if (isPvP)
                {
                    (void)client.MessageCustomXml(deathMessageBlue1);
                    if (!message2.empty())
                    {
                        (void)client.MessageCustomXml(deathMessageBlue2);
                    }
                }
                else
                {
                    (void)client.MessageCustomXml(deathMessageRed1);
                    if (!message2.empty())
                    {
                        (void)client.MessageCustomXml(deathMessageRed2);
                    }
                }
            }
            else if (msgType == MSGRED)
            {
                (void)client.MessageCustomXml(deathMessageRed1);
                if (!message2.empty())
                {
                    (void)client.MessageCustomXml(deathMessageRed2);
                }
            }
            else if (msgType == MSGDARKRED)
            {
                (void)client.MessageCustomXml(deathMessageDarkRed);
            }
        }
    }

    std::wstring KillTrackerPlugin::SelectRandomDeathMessage(const ClientId client)
    {
        const uint shipClass = client.GetShipArch().Handle().Cast<Archetype::Ship>().Handle()->shipClass;
        if (const auto& deathMsgList = config.shipClassToDeathMsgMap.find(shipClass); deathMsgList == config.shipClassToDeathMsgMap.end())
        {
            return config.defaultDeathDamageTemplate;
        }
        else
        {
            const uint randomIndex = Random::Uniform(0u, deathMsgList->second.size() - 1);
            return deathMsgList->second.at(randomIndex);
        }
    }

    void KillTrackerPlugin::OnSendDeathMessage(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg)
    {
        returnCode = ReturnCode::SkipAll;

        std::map<float, ClientId> damageToInflictorMap; // damage is the key instead of value because keys are sorted, used to render top contributors in order
        std::set<uint> involvedGroups;
        std::set<ClientId> involvedPlayers;

        float totalDamageTaken = 0.0f;
        for (const auto& clientData : FLHook::Clients())
        {
            const auto client = clientData.id;
            auto& damageData = damageArray[victim.GetValue()][client.GetValue()];
            float damageToAdd = GetDamageDone(damageData);

            if (const auto playerGroup = client.GetGroup().Unwrap(); playerGroup && (client.GetValue() == victim.GetValue() || damageToAdd > 0.0f))
            {
                involvedGroups.insert(playerGroup.GetValue());
            }
            else if (damageToAdd > 0.0f)
            {
                involvedPlayers.insert(client);
            }
            if (damageToAdd == 0.0f)
            {
                continue;
            }

            damageToInflictorMap[damageToAdd] = client;
            totalDamageTaken += damageToAdd;
        }

        if (killer && totalDamageTaken == 0.0f)
        {
            return;
        }

        const Archetype::Ship* shipArch = victim.GetShipArch().Unwrap().Cast<Archetype::Ship>().Handle();

        if (totalDamageTaken < shipArch->hitPoints * 0.02 && (!killer || killer == victim))
        {
            ClearDamageTaken(victim);
            ProcessDeath(victim, StringUtils::XmlText(msg), L"", system, false, involvedGroups, involvedPlayers);
            return;
        }

        auto victimName = victim.GetCharacterId().Handle();

        std::wstring deathMessage = SelectRandomDeathMessage(victim);
        std::wstring assistMessage;

        uint killerCounter = 0;

        for (auto i = damageToInflictorMap.rbegin(); i != damageToInflictorMap.rend(); ++i) // top values at the end
        {
            if (killerCounter >= config.numberOfListedKillers)
            {
                break;
            }
            float contributionPercentage = round(i->first / totalDamageTaken);
            if (contributionPercentage < config.minimumAssistancePercentage)
            {
                break;
            }

            contributionPercentage *= 100;

            std::wstring_view inflictorName = ClientId(i->second).GetCharacterId().Handle().GetValue();
            if (killerCounter == 0)
            {
                deathMessage = std::vformat(deathMessage, std::make_wformat_args(victimName, inflictorName, contributionPercentage));
            }
            else if (killerCounter == 1)
            {
                assistMessage = std::vformat(L"Assisted by: {0} ({1:0.0f}%)", std::make_wformat_args(inflictorName, contributionPercentage));
            }
            else
            {
                assistMessage += std::vformat(assistMessage + L", {0} ({1:0.0f}%)", std::make_wformat_args(inflictorName, contributionPercentage));
            }
            killerCounter++;
        }

        if (assistMessage.empty())
        {
            ProcessDeath(victim, StringUtils::XmlText(deathMessage), L"", system, true, involvedGroups, involvedPlayers);
        }
        else
        {
            ProcessDeath(victim, StringUtils::XmlText(deathMessage), StringUtils::XmlText(assistMessage), system, true, involvedGroups, involvedPlayers);
        }

        if (config.logPlayerDeaths)
        {
            INFO("{{killer}} killed {{victim}} in {{sysName}} {{pos}}. {{fullMsg}}",
                 { "killer", killer },
                 { "victim", victim },
                 { "sysName", system },
                 { "fullMsg", deathMessage });
        }

        ClearDamageTaken(victim);
    }

    KillTrackerPlugin::KillTrackerPlugin(const PluginInfo& info) : Plugin(info) {}
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"KillTracker",
	    .shortName = L"killtracker",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(KillTrackerPlugin);
