#pragma once
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include "API/FLHook/ClientList.hpp"

namespace Plugins
{
    struct AutobuyInfo
    {
            bool updated;
            bool missiles;
            bool mines;
            bool torps;
            bool cd;
            bool cm;
            bool bb;
            bool repairs;
    };

    class Autobuy final : public Plugin, public AbstractUserCommandProcessor
    {
            //! A struct to represent each client

            struct AutobuyCartItem
            {
                    uint archId = 0;
                    uint count = 0;
                    std::wstring description;
            };

            //! Configurable fields for this plugin
            struct Config final
            {
                    // Reflectable fields
                    //! Nickname of the nanobot item being used when performing the automatic purchase
                    EquipmentId nanobot;
                    //! Nickname of the shield battery item being used when performing the automatic purchase
                    EquipmentId shieldBattery;
            };

            void UserCmdAutobuy(std::wstring_view autobuyType, std::wstring_view newState);

            // clang-format off
            constexpr inline static std::array<CommandInfo<Autobuy>, 1> commands =
            {
                {
                    AddCommand(Autobuy, L"/autobuy", UserCmdAutobuy, L"/autobuy <consumable type/info> <on/off>",
                        L"Sets up automatic purchases for consumables."),
                }
            };
            // clang-format on

            SetupUserCommandHandler(Autobuy, commands);

            Config config;
            std::array<AutobuyInfo, MaxClientId + 1> autobuyInfo;
            void OnLoadSettings() override;
            void OnBaseEnterAfter(BaseId baseId, ClientId client) override;
            void OnCharacterSelectAfter(ClientId client) override;
            void OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document) override;
            void OnClearClientInfo(ClientId client) override;
            void LoadPlayerAutobuy(ClientId client);
            void AddEquipToCart(const Archetype::Launcher* launcher, const st6::list<EquipDesc>* cargo, std::map<uint, AutobuyCartItem>& cart, AutobuyCartItem& item,
                                const std::wstring_view& desc);

        public:
            explicit Autobuy(const PluginInfo& info);
    };
} // namespace Plugins
