#pragma once
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include "API/FLHook/ClientList.hpp"

namespace Plugins
{
    struct AutobuyInfo
    {
            bool updated;
            bool ammo;
            bool mines;
            bool cm;
            bool bb;
            bool repairs;
    };

    /**
     * @author Cannon (Ported by Aingar 2023)
     * @brief
     * The "Autobuy" plugin allows players to set up automatic purchases of various munition/consumable type items.
     *
     * @par configuration Configuration
     * @code
     * {
     *     "nanobot_nickname": "ge_s_repair_01";
     *     "shield_battery_nickname": "ge_s_battery_01";
     * }
     * @endcode
     *
     * @par Player Commands
     * All commands are prefixed with '/' unless explicitly specified.
     * - autobuy info - Lists status of autobuy features for this character.
     * - autobuy <all/munition type> <on/off> - enables or disables autobuy feature for selected munition types on this character.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    class AutobuyPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            //! A struct to represent each client

            struct AutobuyCartItem
            {
                    Id archId;
                    uint count = 0;
                    std::wstring description;
            };

            struct PlayerAmmoData
            {
                    int ammoAdjustment;
                    int ammoCount;
                    ushort sid;
                    int launcherCount;
                    int ammoLimit;
            };

            struct AmmoData
            {
                    int ammoLimit;
                    int launcherStackingLimit;
            };

            struct Config final
            {
                    // Reflectable fields
                    EquipmentId nanobot;
                    EquipmentId shieldBattery;
            };

            concurrencpp::result<void> UserCmdAutobuy(ClientId client, std::wstring_view autobuyType, std::wstring_view newState);

            int __fastcall GetAmmoCapacityDetourHash(CShip* cship, void* edx, Id ammoArch);

            int __fastcall GetAmmoCapacityDetourEq(CShip* cship, void* edx, Archetype::Equipment* ammoType);

            // clang-format off
            const inline static std::array<CommandInfo<AutobuyPlugin>, 1> commands =
            {
                {
                    AddCommand(AutobuyPlugin, Cmds(L"/autobuy"), UserCmdAutobuy, L"/autobuy <consumable type/info> <on/off>",
                        L"Sets up automatic purchases for consumables."),
                }
            };
            // clang-format on

            SetupUserCommandHandler(AutobuyPlugin, commands);

            Config config;
            float hullRepairFactor;
            float equipmentRepairFactor;

            std::array<AutobuyInfo, MaxClientId + 1> autobuyInfo;
            std::unordered_map<ClientId, std::unordered_map<Id, PlayerAmmoData>> playerAmmoLimits;
            std::unordered_map<Id, AmmoData> ammoLimits;
            bool OnLoadSettings() override;
            /**
             * @brief Hook on BaseEnter. Triggers the autobuy/repair.
             */
            void OnBaseEnterAfter(BaseId baseId, ClientId client) override;
            /**
             * @brief Hook on CharacterSelect. Loads autobuy settings.
             */
            void OnCharacterSelectAfter(ClientId client) override;
            /**
             * @brief Hook on CharacterSave. Saves the autobuy settings on character db entry.
             */
            void OnCharacterSave(ClientId client, std::wstring_view charName, B_DOC& document) override;
            /**
             * @brief Hook on ClearClientInfo. Resets the player data.
             */
            void OnClearClientInfo(ClientId client) override;
            int OnGetAmmoCapacity(CShip* cship, Id ammoArch) override;

            void CheckforStackables(ClientId client);
            void LoadPlayerAutobuy(ClientId client);
            std::unordered_map<Id, PlayerAmmoData> GetAmmoLimits(ClientId client);
            void AddEquipToCart(const Archetype::Launcher* launcher, const EquipDescList* cargo, std::map<Id, AutobuyCartItem>& cart,
                                AutobuyCartItem& item, const std::wstring_view& desc);

        public:
            explicit AutobuyPlugin(const PluginInfo& info);
    };
} // namespace Plugins
