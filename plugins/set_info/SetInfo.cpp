#include "PCH.hpp"

#include "SetInfo.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/InfocardManager.hpp"

namespace Plugins
{
    constexpr std::string_view characterInfocardKey = "playerInfocard";

    SetInfoPlugin::SetInfoPlugin(const PluginInfo& info) : Plugin(info) {}

    void SetInfoPlugin::PropagatePlayerInfo(const ClientId client) const
    {
        const auto& info = playersInfo[client.GetValue()];

        if (info.infocard.empty())
        {
            return;
        }

        SetInfoFlufPayload flufPayload;
        flufPayload[client.GetValue()] = info.infocard;

        if (info.changedSinceLastLaunch)
        {
            for (auto& player : FLHook::Clients())
            {
                if (player.id == client || !player.usingFlufClientHook)
                {
                    continue;
                }

                FlufPayload::ToPayload(flufPayload, "set_info");
            }
        }
    }

    void SetInfoPlugin::FetchPlayerInfo(const ClientId client)
    {
        if (auto& info = playersInfo[client.GetValue()]; !info.pulledInfos)
        {
            SetInfoFlufPayload flufPayload;
            for (int i = 1; i < MaxClientId; i++)
            {
                if (info.initialised)
                {
                    flufPayload[i] = playersInfo[i].infocard;
                }
            }

            FlufPayload::ToPayload(flufPayload, "set_info");
            info.pulledInfos = true;
        }
    }

    void SetInfoPlugin::InitializePlayerInfo(ClientId client)
    {
        auto& playerInfo = playersInfo[client.GetValue()];
        if (playerInfo.initialised)
        {
            return;
        }
        const auto view = client.GetData().characterData->characterDocument;
        auto playerInfocard = view.find(characterInfocardKey);
        if (playerInfocard == view.end())
        {
            return;
        }

        playerInfo.infocard = playerInfocard->get_string();
        playerInfo.initialised = true;
    }

    concurrencpp::result<void> SetInfoPlugin::UserCmdSetInfo(ClientId client, std::wstring_view newInfo)
    {
        auto& info = playersInfo[client.GetValue()];
        info.infocard = StringUtils::wstos(newInfo);
        info.initialised = true;

        client.SaveChar();

        PropagatePlayerInfo(client);
        co_return;
    }

    concurrencpp::result<void> SetInfoPlugin::UserCmdShowInfo(ClientId client)
    {
        InitializePlayerInfo(client);

        const auto info = StringUtils::stows(playersInfo[client.GetValue()].infocard);
        if (info.empty())
        {
            client.Message(L"You have no infocard set");
            co_return;
        }

        for (auto split = StringUtils::GetParams(info, '\n'); const auto line : split)
        {
            client.Message(line);
        }
    }

    void SetInfoPlugin::OnPlayerLaunchAfter(const ClientId client, const ShipId& ship)
    {
        if (playersInfo[client.GetValue()].initialised)
        {
            return;
        }

        FetchPlayerInfo(client);
        InitializePlayerInfo(client);
        PropagatePlayerInfo(client);
    }

    void SetInfoPlugin::OnClearClientInfo(ClientId client) {
        playersInfo[client.GetValue()].initialised = false;
        playersInfo[client.GetValue()].infocard = "";
    }

    void SetInfoPlugin::OnCharacterSave(ClientId client, std::wstring_view charName, B_DOC& document)
    {
        auto& info = playersInfo[client.GetValue()];
        document.append(B_KVP(characterInfocardKey, info.infocard));
    }

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
