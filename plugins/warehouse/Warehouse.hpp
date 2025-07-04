#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    class WarehousePlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            struct Config final
            {
                    std::string collectionName = "warehouse";
                    bool allowCommoditiesToBeStored = true;
                    bool allowWithdrawAndStoreFromAnywhere = false;
                    rfl::Validator<int, rfl::Minimum<0>> costToStore = 0;
                    rfl::Validator<int, rfl::Minimum<0>> costToWithdraw = 0;
                    std::unordered_set<EquipmentId> bannedEquipment;
                    std::unordered_set<SystemId> bannedSystems;
                    std::unordered_set<BaseId> bannedBases;
            };

            struct PlayerWarehouse final
            {
                    std::string _id;
                    std::unordered_map<BaseId, std::unordered_map<EquipmentId, int>> baseEquipmentMap;
            };

            /*
             * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let
             * anyone who messages them know too.
             */
            concurrencpp::result<void>UserCmdListItems(ClientId);
            concurrencpp::result<void>UserCmdDeposit(ClientId, uint itemNr, int count);
            concurrencpp::result<void>UserCmdWithdraw(ClientId, uint itemNr, int requestedAmount);
            concurrencpp::result<void>UserCmdListBasesWithItems(ClientId);
            concurrencpp::result<void>UserCmdListStored(const ClientId client, std::wstring_view baseName);

            void UpdatePlayerWarehouse(const PlayerWarehouse& warehouse) const;

            [[nodiscard]]
            rfl::Result<PlayerWarehouse> GetOrCreateAccount(const std::string&) const;

            // clang-format off
            inline static const std::array<CommandInfo<WarehousePlugin>, 5> commands =
            {
                {
                    AddCommand(WarehousePlugin, Cmds(L"/wh listitems"), UserCmdListItems, L"/wh listitems", L""),
                    AddCommand(WarehousePlugin, Cmds(L"/wh deposit"), UserCmdDeposit, L"/wh deposit <itemNr>", L""),
                    AddCommand(WarehousePlugin, Cmds(L"/wh withdraw"), UserCmdWithdraw, L"/wh withdraw <itemNr>", L""),
                    AddCommand(WarehousePlugin, Cmds(L"/wh listbases"), UserCmdListBasesWithItems, L"/wh listbases", L""),
                    AddCommand(WarehousePlugin, Cmds(L"/wh liststored"), UserCmdListStored, L"/wh liststored [base]", L""),
                 }
            };
            // clang-format on

            SetupUserCommandHandler(WarehousePlugin, commands);

            bool OnLoadSettings() override;
            Config config;

        public:
            explicit WarehousePlugin(const PluginInfo& info);
    };
} // namespace Plugins
