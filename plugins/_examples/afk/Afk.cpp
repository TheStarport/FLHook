/**
 * @date August, 2022
 * @author Raikkonen
 * @defgroup AwayFromKeyboard Away from Keyboard
 * @brief
 * The AFK plugin allows you to set yourself as Away from Keyboard.
 * This will notify other players if they try and speak to you, that you are not at your desk.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - afk - Sets your status to Away from Keyboard. Other players will notified if they try to speak to you.
 * - back - Removes the AFK status.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "PCH.hpp"

#include "Afk.hpp"

namespace Plugins
{

    AfkPlugin::AfkPlugin(const PluginInfo& info) : Plugin(info) {}

    /** @ingroup AwayFromKeyboard
     * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let anyone
     * who messages them know too.
     */
    void AfkPlugin::UserCmdAfk()
    {
        awayClients.emplace_back(userCmdClient);
        const auto playerName = userCmdClient.GetCharacterName().Handle();
        const auto message = std::format(L"{} is now away from keyboard.", playerName);

        const auto system = userCmdClient.GetSystemId().Handle();
        (void)system.Message(message, MessageColor::Red, MessageFormat::Normal);
        (void)userCmdClient.Message(L"Use the /back command to stop sending automatic replies to PMs.");
    }

    /** @ingroup AwayFromKeyboard
     * @brief This command is called when a player types /back. It removes the afk status and welcomes the player back.
     * who messages them know too.
     */
    void AfkPlugin::UserCmdBack()
    {
        if (const auto it = awayClients.begin(); std::find(it, awayClients.end(), userCmdClient) != awayClients.end())
        {
            const auto system = userCmdClient.GetSystemId().Handle();

            awayClients.erase(it);

            auto playerName = userCmdClient.GetCharacterName().Handle();

            system.Message(std::format(L"{} has returned.", playerName), MessageColor::Red);
        }
    }

    // Clean up when a client disconnects

    // Hook on chat being sent (This gets called twice with the client and to swapped
    void AfkPlugin::OnSendChat(const ClientId fromClient, const ClientId targetClient, [[maybe_unused]] const uint size, [[maybe_unused]] void* rdl)
    {
        if (std::ranges::find(awayClients, targetClient) != awayClients.end())
        {
            (void)fromClient.Message(L"This user is away from keyboard.");
        }
    }

    // Hooks on chat being submitted
    void AfkPlugin::OnSubmitChat(const ClientId fromClient, [[maybe_unused]] const unsigned long lP1, [[maybe_unused]] const void* rdlReader,
                                 [[maybe_unused]] ClientId to, [[maybe_unused]] const int dunno)
    {
        if (const auto it = awayClients.begin(); fromClient && std::find(it, awayClients.end(), fromClient) != awayClients.end())
        {
            userCmdClient = fromClient;
            UserCmdBack();
        }
    }

    void AfkPlugin::OnClearClientInfo(const ClientId client)
    {
        auto [first, last] = std::ranges::remove(awayClients, client);
        awayClients.erase(first, last);
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"AFK", L"afk", PluginMajorVersion::V05, PluginMinorVersion::V01);
SetupPlugin(AfkPlugin, Info);
