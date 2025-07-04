﻿#include "PCH.hpp"

#include "Afk.hpp"

namespace Plugins
{

    AfkPlugin::AfkPlugin(const PluginInfo& info) : Plugin(info) {}

    concurrencpp::result<void> AfkPlugin::UserCmdAfk(const ClientId client)
    {
        awayClients.emplace_back(client);
        const auto playerName = client.GetCharacterName().Handle();
        const auto message = std::format(L"{} is now away from keyboard.", playerName);

        const auto system = client.GetSystemId().Handle();
        (void)system.Message(message, MessageColor::Red, MessageFormat::Normal);
        (void)client.Message(L"Use the /back command to stop sending automatic replies to PMs.");

        co_return;
    }

    concurrencpp::result<void> AfkPlugin::UserCmdBack(const ClientId client)
    {
        if (const auto it = awayClients.begin(); std::find(it, awayClients.end(), client) != awayClients.end())
        {
            const auto system = client.GetSystemId().Handle();

            awayClients.erase(it);

            auto playerName = client.GetCharacterName().Handle();

            system.Message(std::format(L"{} has returned.", playerName), MessageColor::Red);
        }

        co_return;
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
            UserCmdBack(fromClient);
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

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"AFK",
	    .shortName = L"afk",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(AfkPlugin);
