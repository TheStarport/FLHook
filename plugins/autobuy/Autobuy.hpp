#pragma once

namespace Plugins
{
    struct AutobuyInfo
    {
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
            struct Config
            {
                    // Reflectable fields
                    //! Nickname of the nanobot item being used when performing the automatic purchase
                    std::string nanobot_nickname = "ge_s_repair_01";
                    //! Nickname of the shield battery item being used when performing the automatic purchase
                    std::string shield_battery_nickname = "ge_s_battery_01";
            };

            void UserCmdAutobuy(std::wstring_view autobuyType, std::wstring_view newState);

            constexpr inline static std::array<CommandInfo<Autobuy>, 1> commands = {
                {
                 AddCommand(Autobuy, L"/autobuy", UserCmdAutobuy, L"/autobuy <consumable type/info> <on/off>",
                 L"Sets up automatic purchases for consumables."),
                 }
            };

            SetupUserCommandHandler(Autobuy, commands)

                std::unique_ptr<Config> config;
            std::map<uint, AutobuyInfo> autobuyInfo;
            void LoadSettings();
            void OnBaseEnter(BaseId& baseId, ClientId& client);
            void ClearClientInfo(ClientId& client);
            void LoadPlayerAutobuy(ClientId client);
            void AddEquipToCart(const Archetype::Launcher* launcher, const std::list<CargoInfo>& cargo, std::list<AutobuyCartItem>& cart, AutobuyCartItem& item,
                                const std::wstring_view& desc);
            AutobuyInfo& LoadAutobuyInfo(ClientId& client);

        public:
            explicit Autobuy(const PluginInfo info);
    };
} // namespace Plugins
