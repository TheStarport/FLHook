#include "PCH.hpp"

#include "Afk.hpp"

namespace Plugins
{

    AfkPlugin::AfkPlugin(const PluginInfo& info) : Plugin(info) {}

    void AfkPlugin::UserCmdAfk()
    {
        awayClients.emplace_back(userCmdClient);
        const auto playerName = userCmdClient.GetCharacterName().Handle();
        const auto message = std::format(L"{} is now away from keyboard.", playerName);

        const auto system = userCmdClient.GetSystemId().Handle();
        (void)system.Message(message, MessageColor::Red, MessageFormat::Normal);
        (void)userCmdClient.Message(L"Use the /back command to stop sending automatic replies to PMs.");
    }

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
