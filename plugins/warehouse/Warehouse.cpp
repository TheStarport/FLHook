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
        ERROR(L"Error when accessing database: {0}", {L"Error", StringUtils::stows(error->what())}); \
        if (client && client.GetData().account->_id == accountId)                                          \
        {                                                                                                  \
            (void)client.Message(L"Something went wrong while accessing the data. Please try again.");     \
        }                                                                                                  \
        co_return TaskStatus::Finished;                                                                    \
    }

    WarehousePlugin::WarehousePlugin(const PluginInfo& info) : Plugin(info) {}

    bool WarehousePlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/warehouse.json");

        const auto& flhookConfig = FLHook::GetConfig();
        const auto dbClient = FLHook::GetDbClient();
        if (auto db = dbClient->database(flhookConfig->database.dbName); !db.has_collection(config.collectionName))
        {
            db.create_collection(config.collectionName);
        }

        return true;
    }

    rfl::Result<WarehousePlugin::PlayerWarehouse> WarehousePlugin::GetOrCreateAccount(const std::string& accountId) const
    {
        auto dbQuery = FLHook::GetDatabase()->BeginDatabaseQuery();

        const auto filter = make_document(kvp("_id", accountId));
        auto findResult = dbQuery.FindFromCollection(config.collectionName, filter.view());
        if (!findResult.has_value())
        {
            auto warehouse = PlayerWarehouse{ ._id = accountId };

            auto bsonWarehouse = rfl::bson::write(warehouse);
            auto insertDoc = bsoncxx::document::view{ reinterpret_cast<const uint8_t*>(bsonWarehouse.data()), bsonWarehouse.size() };
            auto insertResult = std::get<0>(dbQuery.InsertIntoCollection(config.collectionName, { insertDoc }));
            dbQuery.ConcludeQuery(true);

            return { warehouse };
        }

        dbQuery.ConcludeQuery(false);
        return rfl::bson::read<PlayerWarehouse>(findResult->view().data(), findResult->length());
    }

    void WarehousePlugin::UpdatePlayerWarehouse(const PlayerWarehouse& warehouse) const
    {
        assert(!warehouse._id.empty());

        auto dbQuery = FLHook::GetDatabase()->BeginDatabaseQuery();

        const auto filterDoc = make_document(kvp("_id", warehouse._id));
        auto warehouseRaw = rfl::bson::write(warehouse);
        const auto warehouseDoc = bsoncxx::document::view{ reinterpret_cast<uint8_t*>(warehouseRaw.data()), warehouseRaw.size() };

        dbQuery.FindAndUpdate(config.collectionName, filterDoc.view(), warehouseDoc, {}, true, true, true);
        dbQuery.ConcludeQuery(true);
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
            if (equip.mounted)
            {
                continue;
            }
            auto good = GoodId(Arch2Good(equip.archId));
            if (!good || (!config.allowCommoditiesToBeStored && good.GetType().Handle() == GoodType::Commodity))
            {
                continue;
            }
            (void)client.Message(std::format(L"{}. {} x{}", counter++, good.GetName().Handle(), equip.count));
        }

        co_return TaskStatus::Finished;
    }

    Task WarehousePlugin::UserCmdDeposit(const ClientId client, uint itemNr, int count)
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
        EquipmentId equipment;
        for (const auto& equip : *equipList)
        {
            if (equip.mounted)
            {
                continue;
            }

            good = GoodId(Arch2Good(equip.archId));
            if (!good || (!config.allowCommoditiesToBeStored && good.GetType().Handle() == GoodType::Commodity))
            {
                continue;
            }

            counter++;
            if (counter != itemNr)
            {
                continue;
            }

            if (equip.count < count)
            {
                (void)client.Message(
                    std::format(L"You tried to deposit {} of {}, but you only have {}!", count, EquipmentId(equip.archId).GetName().Unwrap(), equip.count));
                co_return TaskStatus::Finished;
            }

            equipment = EquipmentId(equip.archId);
            break;
        }

        if (!equipment || !equipment.GetValue()) {}

        if (good.GetValue() == nullptr)
        {
            (void)client.Message(L"This item no longer exists!");
            co_return TaskStatus::Finished;
        }

        (void)client.RemoveCargo(good, count);

        const std::string accountId = client.GetData().account->_id;
        const BaseId currBase = client.GetCurrentBase().Handle();

        co_yield TaskStatus::DatabaseAwait;

        auto result = GetOrCreateAccount(accountId).value();
        result.baseEquipmentMap[currBase][equipment] += count;
        UpdatePlayerWarehouse(result);

        co_yield TaskStatus::FLHookAwait;

        (void)client.Message(std::format(L"Deposited {} of {}!", count, good.GetName().Handle()));
        co_return TaskStatus::Finished;
    }

    Task WarehousePlugin::UserCmdWithdraw(const ClientId client, const uint itemNr, const int requestedAmount)
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

        if (!requestedAmount)
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
        const BaseId currBase = client.GetCurrentBase().Handle();

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

                    const auto equipId = EquipmentId(itemId);
                    bool databaseProblem = false;

                    co_yield TaskStatus::DatabaseAwait;

                    if (const int currentCount = account.value().baseEquipmentMap[currBase][equipId]; currentCount < requestedAmount)
                    {
                        databaseProblem = true;
                    }
                    else if (currentCount == requestedAmount)
                    {
                        account.value().baseEquipmentMap[currBase].erase(equipId);
                        if(account.value().baseEquipmentMap[currBase].empty())
                        {
                            account.value().baseEquipmentMap.erase(currBase);
                        }
                        UpdatePlayerWarehouse(account.value());
                    }
                    else
                    {
                        account.value().baseEquipmentMap[currBase][equipId] -= requestedAmount;
                        UpdatePlayerWarehouse(account.value());
                    }

                    co_yield TaskStatus::FLHookAwait;
                    if (databaseProblem)
                    {
                        (void)client.Message(L"Attempted to withdraw more than is stored!");
                    }
                    else
                    {
                        client.AddCargo(equipId.GetId(), requestedAmount, false);
                        (void)client.Message(std::format(L"Withdrew {} of {}!", requestedAmount, equipId.GetName().Handle()));
                    }
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

            const auto equipId = EquipmentId(itemId);
            bool databaseProblem = false;
            uint storedAmount = amount;

            co_yield TaskStatus::DatabaseAwait;

            if (const int currentCount = account.value().baseEquipmentMap[currBase][equipId]; currentCount < requestedAmount)
            {
                databaseProblem = true;
            }
            else if (currentCount == requestedAmount)
            {
                account.value().baseEquipmentMap[currBase].erase(equipId);
                if(account.value().baseEquipmentMap[currBase].empty())
                {
                    account.value().baseEquipmentMap.erase(currBase);
                }
                UpdatePlayerWarehouse(account.value());
            }
            else
            {
                account.value().baseEquipmentMap[currBase][equipId] -= requestedAmount;
                UpdatePlayerWarehouse(account.value());
            }

            co_yield TaskStatus::FLHookAwait;
            if (databaseProblem)
            {
                (void)client.Message(L"Attempted to withdraw more than is stored!");
                co_return TaskStatus::Finished;
            }

            client.AddCargo(equipId.GetId(), requestedAmount, false);
            (void)client.Message(std::format(L"Withdrew {} of {}!", requestedAmount, equipId.GetName().Handle()));
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

    Task WarehousePlugin::UserCmdListStored(const ClientId client, std::wstring_view baseName)
    {
        const auto currBase = client.GetCurrentBase().Handle();

        BaseId base;
        if (baseName.empty())
        {
            base = currBase;
        }
        else
        {
            base = TransformArg<BaseId>(baseName, 0);
        }

        const std::string accountId = client.GetData().account->_id;

        co_yield TaskStatus::DatabaseAwait;

        auto account = GetOrCreateAccount(accountId);

        co_yield TaskStatus::FLHookAwait;

        VALIDATE_ACCOUNT

        if (config.allowWithdrawAndStoreFromAnywhere)
        {
            if (account.value().baseEquipmentMap.empty())
            {
                client.Message(L"You don't have anything stored!");
                co_yield TaskStatus::Finished;
            }
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

        if (equipMap->second.empty())
        {
            client.Message(L"You don't have anything stored on this base!");
            co_yield TaskStatus::Finished;
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
