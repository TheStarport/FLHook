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
        auto& [flag, returnBase, cargo] = clientData[client];
        flag = TransferFlag::None;
        returnBase = BaseId(0);
        cargo.clear();
    }

    /// Load the configuration
    bool ArenaPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/arena.json");

        return true;
    }

    void ArenaPlugin::RestoreCargo(const ClientId client)
    {
        std::vector<EquipDesc> itemsToSell;
        uint creditsToReimburse = 0;
        for (const auto cargo = client.GetEquipCargo().Handle(); const auto& item : cargo->equip)
        {
            // Some commodity present.
            if (item.mounted || !InternalApi::IsCommodity(item.archId))
            {
                continue;
            }
            auto good = GoodId(Arch2Good(item.archId.GetValue()));
            auto baseData = BaseDataList_get()->get_base_data(client.GetCurrentBase().Handle().GetValue());
            bool found = false;
            if (baseData)
            {
                auto iter = baseData->marketMap.find(item.archId);
                if (iter != baseData->marketMap.end())
                {
                    found = true;
                    creditsToReimburse += iter->second.price;
                }
            }
            if (!found)
            {
                creditsToReimburse += static_cast<uint>(good.GetPrice().Unwrap());
            }
        }

        client.AddCash(creditsToReimburse);
        auto& cd = clientData[client];
        for (const auto& cargo : cd.storedCargo)
        {
            client.AddCargo(Id(cargo.archId), cargo.amount);
        }

        if (!cd.storedCargo.empty())
        {
            client.Message(std::format(L"Restoring {} items", cd.storedCargo.size()));
        }

        cd.storedCargo.clear();
    }

    void ArenaPlugin::StoreCargo(const ClientId client)
    {
        auto& cd = clientData[client];
        cd.storedCargo.clear();
        std::vector<EquipDesc> itemsToClear;
        for (const auto cargo = client.GetEquipCargo().Handle(); const auto& item : cargo->equip)
        {
            if (item.mounted || !InternalApi::IsCommodity(item.archId))
            {
                continue;
            }
            cd.storedCargo.push_back(item);
            itemsToClear.push_back(item);
        }

        for (auto& item : itemsToClear)
        {
            client.RemoveCargo(item.id, item.count);
        }

        client.Message(std::format(L"Beaming you to the arena system"));
        if (!itemsToClear.empty())
        {
            client.Message(std::format(L"{} commodity/ammo items were stored and will be returned upon returning from the arena system", itemsToClear.size()));
        }
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
        B_ARR players;
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
            players.append(B_MDOC(
                B_KVP("clientId", static_cast<int>(client.id.GetValue())),
                B_KVP("playerName", StringUtils::wstos(client.characterName)),
                B_KVP("shipNick", InternalApi::HashLookup(ship->archId.GetValue())),
                B_KVP("shipName", StringUtils::wstos(FLHook::GetInfocardManager()->GetInfoName(ship->idsName)))
            ));

            // clang-format on
        }

        const auto payload = B_MDOC(B_KVP("players", players));
        FLHook::GetHttpServer().get()->WriteHttpResponse(request, payload, response);

        return httplib::StatusCode::OK_200;
    }

    void ArenaPlugin::OnCharacterSelectAfter(const ClientId client)
    {
        auto& [flag, returnBase, cargo] = clientData[client];

        flag = TransferFlag::None;

        const auto view = client.GetData().characterData->characterDocument;
        if (auto findResult = view.find("arena"); findResult != view.end())
        {
            auto doc = findResult->get_document().view();
            returnBase = BaseId(doc.find("returnBase")->get_int32());
            for (auto& item : doc.find("cachedEquipment")->get_array().value)
            {
                auto itemDoc = item.get_document().view();
                cargo.push_back({ itemDoc.find("archId")->get_int32().value,
                                  static_cast<ushort>(itemDoc.find("amount")->get_int32().value),
                                  static_cast<float>(itemDoc.find("health")->get_double().value),
                                  itemDoc.find("isMissionCargo")->get_bool().value });
            }
        }
    }

    void ArenaPlugin::OnPlayerLaunchAfter(const ClientId client, const ShipId& ship)
    {
        const auto state = clientData[client].flag;
        if (state == TransferFlag::Transfer)
        {
            StoreCargo(client);

            clientData[client].flag = TransferFlag::None;
            client.Beam(config.targetBase);
            return;
        }

        if (state == TransferFlag::Return)
        {
            clientData[client].flag = TransferFlag::None;

            if (!clientData[client].returnBase)
            {
                return;
            }

            RestoreCargo(client);

            client.Beam(clientData[client].returnBase);
        }
    }

    void ArenaPlugin::OnCharacterSave(const ClientId client, std::wstring_view charName, B_DOC& document)
    {
        const auto data = clientData.find(client);
        if (data == clientData.end())
        {
            return;
        }
        
        int value = static_cast<int>(data->second.returnBase.GetValue());
        B_ARR commodityArray;
        for (auto& entry : data->second.storedCargo)
        {
            commodityArray.append(entry.ToBson());
        }

        document.append(B_KVP("arena", B_MDOC(B_KVP("returnBase", value), B_KVP("cachedEquipment", commodityArray))));
    }

    concurrencpp::result<void> ArenaPlugin::UserCmdArena(const ClientId client)
    {
        // Prohibit jump if in a restricted system or in the target system
        if (const SystemId system = client.GetSystemId().Unwrap();
            std::ranges::find(config.restrictedSystems, system) != config.restrictedSystems.end() || system == config.targetSystem)
        {
            client.MessageErr(L"Cannot use command in this system or base");
            co_return;
        }

        const BaseId currBase = client.GetCurrentBase().Unwrap();
        if (!currBase)
        {
            client.Message(dockErrorText);
            co_return;
        }

        client.Message(L"Redirecting undock to Arena.");
        auto& [flag, returnBase, _] = clientData[client];
        flag = TransferFlag::Transfer;
        returnBase = currBase;

        co_return;
    }

    concurrencpp::result<void> ArenaPlugin::UserCmdReturn(const ClientId client)
    {
        if (!clientData[client].returnBase)
        {
            client.Message(L"No return possible");
            co_return;
        }

        if (!client.IsDocked())
        {
            client.Message(dockErrorText);
            co_return;
        }

        if (client.GetCurrentBase().Unwrap() != config.targetBase)
        {
            client.Message(L"Not in correct base");
            co_return;
        }

        client.Message(L"Redirecting undock to previous base");
        clientData[client].flag = TransferFlag::Return;

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
