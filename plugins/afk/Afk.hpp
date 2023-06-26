#pragma once

// Includes

namespace Plugins
{
    class AfkPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            std::vector<uint> awayClients;

            /**
             * @ingroup AwayFromKeyboard
             * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let
             * anyone who messages them know too.
             */
            void UserCmdAfk();

            /**
             * @ingroup AwayFromKeyboard
             * @brief This command is called when a player types /back. It removes the afk status and welcomes the player back.
             * who messages them know too.
             */
            void UserCmdBack();

            void ClearClientInfo(ClientId& client);
            void SendChat(ClientId& client, ClientId& targetClient, [[maybe_unused]] const uint& size, [[maybe_unused]] void** rdl);
            void SubmitChat(ClientId& client, [[maybe_unused]] const unsigned long& lP1, [[maybe_unused]] const void** rdlReader, [[maybe_unused]] ClientId& to,
                            [[maybe_unused]] const int& dunno);

            constexpr inline static std::array<CommandInfo<AfkPlugin>, 2> commands = {
                {
                 AddCommand(AfkPlugin, L"/afk", UserCmdAfk, L"/afk",
                 L"Sets your status to \"Away from Keyboard\". Other players will notified if they try to speak to you."),
                 AddCommand(AfkPlugin, L"/back", UserCmdBack, L"/back", L"Removes the AFK status."),
                 }
            }; // namespace Plugins

            SetupUserCommandHandler(AfkPlugin, commands);

        public:
            explicit AfkPlugin(const PluginInfo& info);
    };
} // namespace Plugins
