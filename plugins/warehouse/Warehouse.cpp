#include "PCH.hpp"

#include "Warehouse.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"

namespace Plugins
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_array;
    using bsoncxx::builder::basic::make_document;

#define VALIDATE_ACCOUNT                                                                                   \
    if (const auto error = account.error(); error.has_value())                                             \
    {                                                                                                      \
        Logger::Err(std::format(L"Error when accessing database: {}", StringUtils::stows(error->what()))); \
        if (client && client.GetData().account->_id == accountId)                                          \
        {                                                                                                  \
            (void)client.Message(L"Something went wrong while accessing the data. Please try again.");     \
        }                                                                                                  \
        co_return TaskStatus::Finished;                                                                    \
    }

    WarehousePlugin::WarehousePlugin(const PluginInfo& info) : Plugin(info) {}

    void WarehousePlugin::OnLoadSettings()
    {
        // TODO: If config exists and is faulty, prevent loading
        if (const auto conf = Json::Load<Config>("config/warehouse.json"); !conf.has_value())
        {
            Json::Save(config, "config/warehouse.json");
        }
        else
        {
            config = conf.value();
        }
    }

    rfl::Result<WarehousePlugin::PlayerWarehouse> WarehousePlugin::GetOrCreateAccount(const std::string& accountId) const
    {
        auto collection = Database::GetCollection(FLHook::GetDbClient(), config.collectionName);
        const auto filter = make_document(kvp("_id", accountId));

        if (const auto document = collection.find_one(filter.view()); document.has_value())
        {
            if (const auto& value = document.value(); value.find("_id") != value.end())
            {
                return rfl::bson::read<PlayerWarehouse>(document->data(), document->length());
            }
        }

        // not found, must create
        const auto newDoc = make_document(kvp("_id", accountId), kvp("equipmentMap", make_document()));
        collection.insert_one(newDoc.view());

        return rfl::bson::read<PlayerWarehouse>(newDoc.view().data(), newDoc.view().length());
    }

    Task WarehousePlugin::UserCmdListItems(const ClientId client)
    {
        if (!client.IsDocked())
        {
            (void)client.Message(L"Not docked on a station!");
            co_yield TaskStatus::Finished;
        }

        int counter = 1;
        for (const auto& equip : *client.GetEquipCargo().Handle())
        {
            auto good = GoodId(equip.archId);
            if (!config.allowCommoditiesToBeStored && good.GetType().Handle() == GoodType::Commodity)
            {
                continue;
            }
            (void)client.Message(std::format(L"{}. {} x{}", counter++, good.GetName().Handle(), equip.count));
        }

        co_return TaskStatus::Finished;
    }

    Task WarehousePlugin::UserCmdDeposit(const ClientId client, uint itemNr, uint count)
    {

        if (!client.IsDocked())
        {
            (void)client.Message(L"Not docked on a station!");
            co_yield TaskStatus::Finished;
        }

        if (!itemNr)
        {
            (void)client.Message(L"Invalid item number!");
            co_yield TaskStatus::Finished;
        }

        if (!count)
        {
            (void)client.Message(L"Invalid item count!");
            co_yield TaskStatus::Finished;
        }

        if (config.bannedBases.contains(client.GetCurrentBase().Handle()) || config.bannedSystems.contains(client.GetSystemId().Handle()))
        {
            (void)client.Message(L"Command unavailable");
            co_yield TaskStatus::Finished;
        }

        int counter = 0;
        const auto equipList = client.GetEquipCargo().Handle();

        if (itemNr > equipList->size())
        {
            (void)client.Message(L"You don't have that much of this item!");
            co_yield TaskStatus::Finished;
        }

        GoodId good;
        for (const auto& equip : *equipList)
        {
            counter++;
            if (counter != itemNr)
            {
                continue;
            }

            good = GoodId(equip.archId);
            if (!config.allowCommoditiesToBeStored && good.GetType().Handle() == GoodType::Commodity)
            {
                continue;
            }
            break;
        }

        // TODO: Implement clientId.RemoveCargo

        co_yield TaskStatus::DatabaseAwait;
        // TODO: Call DB to add the item
        co_yield TaskStatus::FLHookAwait;

        (void)client.Message(std::format(L"Deposited {} of {}!", count, good.GetName().Handle()));
        co_return TaskStatus::Finished;
    }

    Task WarehousePlugin::UserCmdWithdraw(const ClientId client, const uint itemNr, const uint count)
    {

        if (!client.IsDocked())
        {
            (void)client.Message(L"Not docked on a station!");
            co_yield TaskStatus::Finished;
        }

        if (!itemNr)
        {
            (void)client.Message(L"Invalid item number!");
            co_yield TaskStatus::Finished;
        }

        if (!count)
        {
            (void)client.Message(L"Invalid item count!");
            co_yield TaskStatus::Finished;
        }

        if (config.bannedBases.contains(client.GetCurrentBase().Handle()) || config.bannedSystems.contains(client.GetSystemId().Handle()))
        {
            (void)client.Message(L"Command unavailable");
            co_yield TaskStatus::Finished;
        }

        const std::string accountId = client.GetData().account->_id;

        co_yield TaskStatus::DatabaseAwait;

        auto account = GetOrCreateAccount(accountId);

        co_yield TaskStatus::FLHookAwait;
        VALIDATE_ACCOUNT

        if (config.allowWithdrawAndStoreFromAnywhere)
        {
            uint counter = 0;
            for (const auto& equipMap : account.value().baseEquipmentMap | std::views::values)
            {
                for (const auto& [itemId, amount] : equipMap)
                {
                    counter++;
                    if (counter != itemNr)
                    {
                        continue;
                    }
                    if (const auto itemVolume = EquipmentId(itemId).GetVolume().Handle();
                        itemVolume * static_cast<float>(amount) > client.GetRemainingCargo().Handle())
                    {
                        (void)client.Message(L"Insufficient cargo space!");
                        co_return TaskStatus::Finished;
                    }

                    client.AddCargo(itemId.GetValue()->archId, amount, false);
                    co_yield TaskStatus::DatabaseAwait;
                    // TODO: Call DB to remove the item
                    co_yield TaskStatus::FLHookAwait;
                    (void)client.Message(std::format(L"Withdrew {} of {]!", amount, itemId.GetName().Handle()));
                    co_return TaskStatus::Finished;
                }
            }
            (void)client.Message(L"Invalid item number!");
            co_return TaskStatus::Finished;
        }

        const auto equipMap = account.value().baseEquipmentMap.find(client.GetCurrentBase().Handle());
        if (equipMap == account.value().baseEquipmentMap.end())
        {
            (void)client.Message(L"No items on current base!");
            co_return TaskStatus::Finished;
        }

        if (itemNr > equipMap->second.size())
        {
            (void)client.Message(L"Invalid item number!");
            co_return TaskStatus::Finished;
        }

        uint counter = 0;
        for (const auto& [itemId, amount] : equipMap->second)
        {
            counter++;
            if (counter != itemNr)
            {
                continue;
            }

            if (const auto itemVolume = EquipmentId(itemId).GetVolume().Handle(); itemVolume * static_cast<float>(amount) > client.GetRemainingCargo().Handle())
            {
                (void)client.Message(L"Insufficient cargo space!");
                co_return TaskStatus::Finished;
            }

            client.AddCargo(itemId.GetValue()->archId, amount, false);
            co_yield TaskStatus::DatabaseAwait;
            // TODO: Call DB to remove the item
            co_yield TaskStatus::FLHookAwait;
            (void)client.Message(std::format(L"Withdrew {} of {]!", amount, itemId.GetName().Handle()));
            co_return TaskStatus::Finished;
        }

        (void)client.Message(L"Invalid item number!");
        co_return TaskStatus::Finished;
    }

    Task WarehousePlugin::UserCmdListBasesWithItems(const ClientId client)
    {
        if (config.allowWithdrawAndStoreFromAnywhere)
        {
            (void)client.Message(L"Items can be withdrawn or deposited regardless of location on this server.");
            co_return TaskStatus::Finished;
        }

        const std::string accountId = client.GetData().account->_id;

        co_yield TaskStatus::DatabaseAwait;

        auto account = GetOrCreateAccount(accountId);

        co_yield TaskStatus::FLHookAwait;

        VALIDATE_ACCOUNT

        int counter = 1;
        for (const auto& [base, items] : account.value().baseEquipmentMap)
        {
            client.Message(std::format(L"{}. {} - {} items", counter++, base.GetName().Handle(), items.size()));
        }

        co_return TaskStatus::Finished;
    }

    Task WarehousePlugin::UserCmdListStored(const ClientId client, BaseId base)
    {
        // TODO: Transform base into an argument

        const std::string accountId = client.GetData().account->_id;

        co_yield TaskStatus::DatabaseAwait;

        auto account = GetOrCreateAccount(accountId);

        co_yield TaskStatus::FLHookAwait;

        VALIDATE_ACCOUNT

        if (config.allowWithdrawAndStoreFromAnywhere)
        {
            int counter = 1;
            for (const auto& equipMap : account.value().baseEquipmentMap | std::views::values)
            {
                for (const auto& [itemId, amount] : equipMap)
                {
                    client.Message(std::format(L"{}. {} x{}", counter++, EquipmentId(itemId).GetName().Handle(), amount));
                }
            }
            co_yield TaskStatus::Finished;
        }

        const auto& equipMap = account.value().baseEquipmentMap.find(base);
        if (equipMap == account.value().baseEquipmentMap.end())
        {
            (void)client.Message(L"No items on selected base!");
            co_return TaskStatus::Finished;
        }

        int counter = 1;
        for (const auto& [itemId, amount] : equipMap->second)
        {
            client.Message(std::format(L"{}. {} x{}", counter++, EquipmentId(itemId).GetName().Handle(), amount));
        }

        co_return TaskStatus::Finished;
    }

    // Clean up when a client disconnects

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Warehouse", L"warehouse", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(WarehousePlugin, Info);
