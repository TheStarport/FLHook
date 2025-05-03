#include "PCH.hpp"

#include "LootTables.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/HttpServer.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/Random.hpp"

#include <bsoncxx/json.hpp>

namespace Plugins
{
    LootTablesPlugin::LootTablesPlugin(const PluginInfo& info) : Plugin(info) {}

    /// Load the configuration
    bool LootTablesPlugin::OnLoadSettings()
    {
        using RflType = std::unordered_map<std::string, rfl::Ref<LootTable>>;
        LoadJsonWithPrompt(RflType, lootTables, "config/loot_tables.json");

        // Extra validations and checks

        std::unordered_set<std::wstring> errors;

#define ADD_ERROR(key, msg) errors.insert(std::format(L"{}.{} - {}", StringUtils::stows(name), key, msg))

        if (lootTables.empty())
        {
            WARN(L"No lootables loaded, plugin inactive")
            return false;
        }

        constexpr auto allPositiveNonZero = [](const auto& p) { return p > 0; };
        for (const auto& [name, table] : lootTables)
        {
            if ((!table->points || table->points->empty()) && (!table->rollCount || table->rollCount->empty()))
            {
                ADD_ERROR(L"rollCount", L"'rollCount' is not defined or empty, and 'points' is also not defined or is empty. One or both is required.");
            }
            else if (table->points && !std::ranges::all_of(*table->points, allPositiveNonZero))
            {
                ADD_ERROR(L"points", L"All points must be positive and above zero.");
            }
            else if (table->rollCount && !std::ranges::all_of(*table->rollCount, allPositiveNonZero))
            {
                ADD_ERROR(L"rollCount", L"All roll counts must be positive and above zero.");
            }

            if (table->maxRollsWithoutAnItem == 0)
            {
                ADD_ERROR(L"maxRollsWithoutAnItem", L"There must be a cap of at minimum 1 failure before stopping. This is to prevent infinite loops");
            }

            if (table->rollItems.empty())
            {
                ADD_ERROR(L"rollItems", L"'rollItems' is not defined or empty.");
                continue;
            }

            int index = -1;
            bool hasNullItem = false;
            for (const auto& item : table->rollItems)
            {
                index++;
                std::wstring key = std::format(L"rollItems[{}].", index);
                if (!item->goodId)
                {
                    if (hasNullItem)
                    {
                        ADD_ERROR(key + L"item", L"You have multiple null/nothing items. You can only have one.");
                    }
                    else
                    {
                        hasNullItem = true;
                    }
                }
                else if (!*item->goodId)
                {
                    ADD_ERROR(key + L"item", L"The item could not be found");
                }

                if (table->points)
                {
                    if (!item->cost)
                    {
                        ADD_ERROR(key + L"cost", L"Cost has been set on the lootable, but not on this item.");
                    }
                    else if (item->cost.value() <= 0)
                    {
                        ADD_ERROR(key + L"cost", L"The item had a cost, but was either empty or contained a negative integer or zero");
                    }
                }

                if (item->weight <= 0)
                {
                    ADD_ERROR(key + L"weight", L"The item had a weight, but it contained a negative integer or zero");
                }

                if (item->count.empty() || !std::ranges::all_of(item->count, allPositiveNonZero))
                {
                    ADD_ERROR(key + L"weights", L"The item had a weights array, but was either empty or contained a negative integer or zero");
                }
            }
        }

#undef ADD_ERROR

        if (!errors.empty())
        {
            for (const auto& error : errors)
            {
                ERROR(error)
            };

            return false;
        }

        for (auto& table : lootTables | std::views::values)
        {
            auto& weights = table->itemWeights.value();

            for (auto& item : table->rollItems)
            {
                weights.emplace_back(item->weight);
            }
        }

        return true;
    }

    void LootTablesPlugin::LootTableRoll(rfl::Ref<LootTable> table, ClientId client)
    {
        auto& weights = table->itemWeights.value();
        unsigned pointsBudget = table->points.has_value() ? Random::Item(*table->points) : UINT_MAX;
        unsigned rollsLeft = table->rollCount.has_value() ? Random::Item(*table->rollCount) : UINT_MAX;

        std::vector<rfl::Ref<RollItem>> basket;

        DEBUG(std::format(L"Lootable Selection: Points: {}, Rolls: {}", pointsBudget, rollsLeft));

        uint rollsWithoutAnItem = 0;
        while (pointsBudget > 0 && rollsLeft-- > 0 && rollsWithoutAnItem < table->maxRollsWithoutAnItem)
        {
            const auto itemIndex = Random::Weighted(weights.begin(), weights.end());
            auto item = table->rollItems[itemIndex];

            if (item->cost.has_value())
            {
                if (pointsBudget < *item->cost)
                {
                    rollsWithoutAnItem++;

                    DEBUG(std::format(L"Rolled an item costing {}, but points left was {}", item->cost.value(), pointsBudget));
                    DEBUG(std::format(L"Rolls left: {}", std::clamp(rollsLeft, table->maxRollsWithoutAnItem - rollsWithoutAnItem, UINT_MAX)));
                    continue;
                }

                DEBUG(std::format(L"Consuming {} points, leaving {} left over", *item->cost, pointsBudget - *item->cost));
                pointsBudget -= *item->cost;
            }

            if (item->goodId.has_value())
            {
                basket.emplace_back(item);
            }
        }

        // Spawn our items!
        for (auto& item : basket)
        {
            pub::SpaceObj::LootInfo lootInfo;
            lootInfo.systemId = client.GetSystemId().Handle().GetValue();
            lootInfo.ownerId = 0;
            lootInfo.equipmentArchId = item->goodId.value().GetHash().Unwrap();
            lootInfo.itemCount = Random::Item(item->count);
            // Add some variance to the pos so they don't all spawn on top of each other
            lootInfo.pos = client.GetPosition().Unwrap();
            lootInfo.pos.x += Random::UniformFloat(-5.f, 5.f);
            lootInfo.pos.y += Random::UniformFloat(-5.f, 5.f);
            lootInfo.pos.z += Random::UniformFloat(-5.f, 5.f);
            lootInfo.rot = Matrix::Identity();
            lootInfo.isMissionLoot = false;
            lootInfo.canAITractor = false;
            lootInfo.hitPtsPercentage = 1.0f;
            // Randomise movement speed and angular velocity to make it feel more dynamic
            lootInfo.linearVelocity = { Random::UniformFloat(-5.f, 5.f), Random::UniformFloat(-5.f, 5.f), Random::UniformFloat(-5.f, 5.f) };
            lootInfo.angularVelocity = { Random::UniformFloat(-10.f, 10.f), Random::UniformFloat(-10.f, 10.f), Random::UniformFloat(-10.f, 10.f) };
            lootInfo.infocardOverride = 0;
            uint spaceObjId = 0;
            pub::SpaceObj::CreateLoot(spaceObjId, lootInfo);
        }
    }

    void LootTablesPlugin::OnShipDestroy(Ship* ship, DamageList* dmgList, const ShipId killerId)
    {
        if (killerId.IsNpc())
        {
            TRACE(L"Ship destroyed by NPC, ignoring possible lootable")
            return;
        }

        auto* params = ResourceManager::LookupShipCreationInfo(ship->cship()->id);
        if (!params)
        {
            DEBUG(L"Destroyed ship had no creation params");
            return;
        }

        const auto lootTable = std::ranges::find_if(lootTables, [params](const auto& table) { return table.second->loadout.GetValue() == params->loadout; });
        if (lootTable == lootTables.end())
        {
            DEBUG(L"Destroyed ship had no loot table associated with it");
            return;
        }

        DEBUG(std::format(L"Lootable found for {}", Id(params->loadout)));
        LootTableRoll(lootTable->second, killerId.GetPlayer().Handle());
    }

    void LootTablesPlugin::OnSolarDestroy(Solar* solar, bool isKill, const ShipId killerId)
    {
        if (killerId.IsNpc())
        {
            TRACE(L"Solar destroyed by NPC, ignoring possible lootable")
            return;
        }

        auto* params = ResourceManager::LookupSolarCreationInfo(solar->csolar()->id);
        if (!params)
        {
            DEBUG(L"Destroyed solar had no creation params");
            return;
        }

        const auto lootTable = std::ranges::find_if(lootTables, [params](const auto& table) { return table.second->loadout.GetValue() == params->loadoutId; });
        if (lootTable == lootTables.end())
        {
            DEBUG(L"Destroyed solar had no loot table associated with it");
            return;
        }

        DEBUG(std::format(L"Lootable found for {}", Id(params->loadoutId)));
        LootTableRoll(lootTable->second, killerId.GetPlayer().Handle());
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Loot Tables", L"loot_tables", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(LootTablesPlugin, Info);
