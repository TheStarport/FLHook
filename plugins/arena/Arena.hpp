#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    class ArenaPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            //! This plugin can communicate with the base plugin if loaded.
            enum class TransferFlag
            {
                None,
                Transfer,
                Return
            };

            const std::wstring dockErrorText = L"Please dock at nearest base";
            const std::wstring cargoErrorText = L"Cargo hold is not empty";

            //! Config data for this plugin
            struct Config final
            {
                    std::string targetBase;
                    std::string targetSystem;
                    std::vector<std::string> restrictedSystems;
            };

            Config config;

            // Non-reflectable fields
            BaseId targetBaseId;
            SystemId targetSystemId;
            std::vector<SystemId> restrictedSystems;

            struct ArenaClientData
            {
                TransferFlag flag;
                BaseId returnBase;
            };

            std::unordered_map<ClientId, ArenaClientData> clientData;

            void UserCmdArena();
            void UserCmdReturn();

            // clang-format off
            constexpr static std::array<CommandInfo<ArenaPlugin>, 2> commands = {
            {
                    AddCommand(ArenaPlugin, L"/arena", UserCmdArena, L"/arena", L" Sends you to the designated arena system."),
                    AddCommand(ArenaPlugin, L"/return", UserCmdReturn, L"/return", L" Returns you from the arena system to where you last docked.")}
            };
            // clang-format on

            SetupUserCommandHandler(ArenaPlugin, commands);

            void OnClearClientInfo(ClientId client) override;
            void OnLoadSettings() override;
            void OnCharacterSelect(ClientId client, std::wstring_view charFilename) override;
            void OnPlayerLaunchAfter(ClientId client, [[maybe_unused]] ShipId ship) override;
            void OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document) override;

            static BaseId ReadReturnPointForClient(ClientId client);
            static bool ValidateCargo(ClientId client);
            static void StoreReturnPointForClient(ClientId client);

        public:
            explicit ArenaPlugin(const PluginInfo& info);
    };
} // namespace Plugins
