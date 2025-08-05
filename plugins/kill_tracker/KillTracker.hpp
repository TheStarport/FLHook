#pragma once
#include "API/FLHook/Plugin.hpp"

namespace Plugins
{
    /**
     * @author Aingar 2024
     * @brief
     * Tracks player damage and overrides death message to provide percentage contribution to hull damage.
     *
     * @par configuration Configuration
     * @code
     * {
     *     "numberOfListedKillers": 3,
     *     "deathBroadcastRange": 15000,
     *     "minimumAssistancePercentage": 5
     * }
     * @endcode
     *
     * @par Player Commands
     * There are no player commands in this plugin.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     */
    class KillTrackerPlugin final : public Plugin
    {
            struct Config final
            {
                    bool logPlayerDeaths = false;
                    bool logPlayerWhoShotFirst = false;
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
                    bool hasAttacked = false;
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
            void OnShipShieldDmg(Ship* ship, CEShield* shield, float& damage, DamageList* dmgList) override;
            static float GetDamageDone(const DamageDone& damageDone);
            std::wstring SelectRandomDeathMessage(ClientId client);
            /// @brief Suppresses the core FLHook death message and prints its own which shows the damage contribution percentages.
            void OnSendDeathMessage(ClientId& killer, ClientId victim, SystemId system, std::wstring_view msg) override;
            /// @brief Reset damage taken/dealt upon launch
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;

            void LogFirstStrike(Ship* ship, DamageList* dmgList);

        public:
            explicit KillTrackerPlugin(const PluginInfo& info);
    };
} // namespace Plugins
