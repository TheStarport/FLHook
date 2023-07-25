#pragma once

#include <API/API.hpp>

namespace Plugins
{
    class Arena final : public Plugin, public AbstractUserCommandProcessor
    {
            //! This plugin can communicate with the base plugin if loaded.
            enum class ClientState
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
                    std::string restrictedSystem;
            };
            std::unique_ptr<Config> config;
            // Non-reflectable fields
            uint targetBaseId = 0;
            uint targetSystemId = 0;
            uint restrictedSystemId = 0;
            std::wstring command = L"arena";

            std::array<ClientState, MaxClientId + 1> transferFlags = {};

            void UserCmdArena();
            void UserCmdReturn();

            constexpr inline static std::array<CommandInfo<Arena>, 2> commands = {
                {AddCommand(Arena, L"/arena", UserCmdArena, L"/arena", L" Sends you to the designated arena system."),
                 AddCommand(Arena, L"/return", UserCmdArena, L"/return", L" Returns you from the arena system to where you last docked.")}
            };

            SetupUserCommandHandler(Arena, commands);

            void ClearClientInfo(ClientId& client);
            void LoadSettings();
            void CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client);
            void PlayerLaunch_AFTER([[maybe_unused]] const uint& ship, ClientId& client);

           

        public:
            explicit Arena(const PluginInfo& info);
    };
} // namespace Plugins
