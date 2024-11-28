#pragma once

namespace Plugins
{
    class KillTrackerPlugin final : public Plugin
    {
            struct Config final
            {
                    uint numberOfListedKillers = 3;
                    float deathBroadcastRange = 15000;
                    float minimumAssistancePercentage = 5;

                    std::unordered_map<uint, std::vector<std::wstring>> shipClassToDeathMsgMap;
                    std::wstring defaultDeathDamageTemplate = L"{0} has been slain by {1} ({2:0.0f}%)";
            };

            struct DamageDone
            {
                    float currDamage = 0.0f;
                    float lastUndockDamage = 0.0f;
            };

            Config config;
            std::array<std::array<DamageDone, MaxClientId + 1>, MaxClientId + 1> damageArray;

            void ClearDamageTaken(ClientId client);
            void ClearDamageDone(ClientId client, bool fullReset);
            bool OnLoadSettings() override;
            void OnCharacterSelectAfter(ClientId client) override;
            void OnDisconnectAfter(ClientId client, EFLConnection connection) override;
            void OnClearClientInfo(ClientId client) override;
            void OnShipHullDmg(Ship* ship, float& damage, DamageList* dmgList) override;
            static float GetDamageDone(const DamageDone& damageDone);
            std::wstring SelectRandomDeathMessage(ClientId client);
            void OnSendDeathMessage(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg) override;
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;

        public:
            explicit KillTrackerPlugin(const PluginInfo& info);
    };
} // namespace Plugins
