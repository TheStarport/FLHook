#include "PCH.hpp"

#include "Arena.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/HttpServer.hpp"
#include <bsoncxx/json.hpp>

namespace Plugins
{
    ArenaPlugin::ArenaPlugin(const PluginInfo& info) : Plugin(info) {}

    /// Clear client info when a client connects.
    void ArenaPlugin::OnClearClientInfo(const ClientId client)
    {
        auto& [flag, returnBase] = clientData[client.GetValue()];
        flag = TransferFlag::None;
        returnBase = BaseId(0);
    }

    /// Load the configuration
    bool ArenaPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/arena.json");

        return true;
    }

    bool ArenaPlugin::ValidateCargo(const ClientId client)
    {
        for (const auto cargo = client.GetEquipCargo().Handle(); const auto& item : cargo->equip)
        {
            bool flag = false;
            pub::IsCommodity(item.archId.GetValue(), flag);

            // Some commodity present.
            if (flag)
            {
                return false;
            }
        }

        return true;
    }

    void ArenaPlugin::OnHttpServerRegister(const std::shared_ptr<httplib::Server> httpServer)
    {
        httpServer->Get("/plugins/arena/usage",
                        [&](const httplib::Request& req, httplib::Response& res)
                        {
                            const auto flHttp = FLHook::GetHttpServer();
                            std::scoped_lock lock(*flHttp);
                            return GetCurrentArenaUsers(req, res);
                        });
    }

    httplib::StatusCode ArenaPlugin::GetCurrentArenaUsers(const httplib::Request& request, httplib::Response& response)
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        bsoncxx::builder::basic::array players;
        for (auto& client : FLHook::Clients())
        {
            auto system = client.id.GetSystemId().Unwrap();
            if (client.characterName.empty() || system != config.targetSystem)
            {
                continue;
            }

            auto shipRaw = client.ship.GetShipArchetype().Raw();
            if (shipRaw.has_error())
            {
                continue;
            }

            auto ship = shipRaw.value();

            // clang-format off
            players.append(make_document(
                kvp("clientId", static_cast<int>(client.id.GetValue())),
                kvp("playerName", StringUtils::wstos(client.characterName)),
                kvp("shipNick", ship->name), // TODO: Replace name which is a CMP with the actual nickname
                kvp("shipName", StringUtils::wstos(FLHook::GetInfocardManager()->GetInfoName(ship->idsName)))
            ));

            // clang-format on
        }

        const auto payload = make_document(kvp("players", players));
        FLHook::GetHttpServer().get()->WriteHttpResponse(request, payload, response);

        return httplib::StatusCode::OK_200;
    }

    BaseId ArenaPlugin::ReadReturnPointForClient(const ClientId client)
    {
        const auto view = client.GetData().characterData->characterDocument;
        if (auto returnBase = view.find("arenaReturnBase"); returnBase != view.end())
        {
            return BaseId{ static_cast<uint>(returnBase->get_int32()) };
        }

        return {};
    }

    void ArenaPlugin::OnCharacterSelectAfter(const ClientId client)
    {
        auto& [flag, returnBase] = clientData[client.GetValue()];

        flag = TransferFlag::None;

        const auto view = client.GetData().characterData->characterDocument;
        if (auto findResult = view.find("arenaReturnBase"); findResult != view.end())
        {
            returnBase = BaseId(findResult->get_int32());
        }
        else
        {
            returnBase = BaseId(0);
        }
    }

    void ArenaPlugin::OnPlayerLaunchAfter(const ClientId client, const ShipId& ship)
    {
        const auto state = clientData[client.GetValue()].flag;
        if (state == TransferFlag::Transfer)
        {
            if (!ValidateCargo(client))
            {
                (void)client.Message(cargoErrorText);
                return;
            }

            clientData[client.GetValue()].flag = TransferFlag::None;
            (void)client.Beam(config.targetBase);
            return;
        }

        if (state == TransferFlag::Return)
        {
            if (!ValidateCargo(client))
            {
                (void)client.Message(cargoErrorText);
                return;
            }

            clientData[client.GetValue()].flag = TransferFlag::None;
            const BaseId returnPoint = ReadReturnPointForClient(client);

            if (!returnPoint)
            {
                return;
            }

            (void)client.Beam(returnPoint);
        }
    }

    void ArenaPlugin::OnCharacterSave(const ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document)
    {
        int value = 0;
        if (const auto data = clientData.find(client.GetValue()); data != clientData.end())
        {
            value = static_cast<int>(data->second.returnBase.GetValue());
        }
        document.append(bsoncxx::builder::basic::kvp("arenaReturnBase", value));
    }

    concurrencpp::result<void> ArenaPlugin::UserCmdArena(const ClientId client)
    {
        // Prohibit jump if in a restricted system or in the target system
        if (const SystemId system = client.GetSystemId().Unwrap();
            std::ranges::find(config.restrictedSystems, system) != config.restrictedSystems.end() || system == config.targetSystem)
        {
            (void)client.MessageErr(L"Cannot use command in this system or base");
            co_return;
        }

        const BaseId currBase = client.GetCurrentBase().Unwrap();
        if (!currBase)
        {
            (void)client.Message(dockErrorText);
            co_return;
        }

        if (!ValidateCargo(client))
        {
            (void)client.Message(cargoErrorText);
            co_return;
        }

        (void)client.Message(L"Redirecting undock to Arena.");
        auto& [flag, returnBase] = clientData[client.GetValue()];
        flag = TransferFlag::Transfer;
        returnBase = currBase;

        co_return;
    }

    concurrencpp::result<void> ArenaPlugin::UserCmdReturn(const ClientId client)
    {
        if (!ReadReturnPointForClient(client))
        {
            (void)client.Message(L"No return possible");
            co_return;
        }

        if (!client.IsDocked())
        {
            (void)client.Message(dockErrorText);
            co_return;
        }

        if (client.GetCurrentBase().Unwrap() != config.targetBase)
        {
            (void)client.Message(L"Not in correct base");
            co_return;
        }

        if (!ValidateCargo(client))
        {
            (void)client.Message(cargoErrorText);
            co_return;
        }

        (void)client.Message(L"Redirecting undock to previous base");
        clientData[client.GetValue()].flag = TransferFlag::Return;

        co_return;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Arena",
	    .shortName = L"arena",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(ArenaPlugin);
