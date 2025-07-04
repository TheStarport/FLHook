#include "PCH.hpp"

#include "SetInfo.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/HttpServer.hpp"
#include "API/FLHook/InfocardManager.hpp"

#include <bsoncxx/json.hpp>

namespace Plugins
{
    SetInfoPlugin::SetInfoPlugin(const PluginInfo& info) : Plugin(info) {}

    concurrencpp::result<void> SetInfoPlugin::UserCmdSetInfo(ClientId client, std::wstring_view newInfo) { co_return; }

    concurrencpp::result<void> SetInfoPlugin::UserCmdShowInfo(ClientId client) { co_return; }

    void SetInfoPlugin::OnPlayerLaunchAfter(ClientId client, const ShipId& ship) {}

    void SetInfoPlugin::OnClearClientInfo(ClientId client) {}

    void SetInfoPlugin::OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document) {}

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"Set Info",
        .shortName = L"set_info",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00,
        .requiresFluf = true
    };
};

SetupPlugin(SetInfoPlugin);
