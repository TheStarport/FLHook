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
            WARN("No lootables loaded, plugin inactive");
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
                ERROR("{{error}}", { "error", error });
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

        DEBUG("Lootable Selection: Points: {{points}}, Rolls: {{rollsLeft}}", { "points", pointsBudget }, { "rollsLeft", rollsLeft });

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

                    DEBUG("Rolled an item costing {{value}}, but points left was {{budget}}", { "value", item->cost.value() }, { "budget", pointsBudget });
                    DEBUG("Rolls left: {{rollsLeft}}", { "rollsLeft", std::clamp(rollsLeft, table->maxRollsWithoutAnItem - rollsWithoutAnItem, UINT_MAX) });
                    continue;
                }

                DEBUG("Consuming {{cost}} points, leaving {{budget}} left over",
                      {
                          "cost",
                          *item->cost,
                      },
                      { "budget", pointsBudget - *item->cost });
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
            lootInfo.systemId = client.GetSystemId().Handle();
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
            lootInfo.infocardOverride = Id();
            uint spaceObjId = 0;
            pub::SpaceObj::CreateLoot(spaceObjId, lootInfo);
        }
    }

    void LootTablesPlugin::OnShipDestroy(Ship* ship, DamageList* dmgList, const ShipId killerId)
    {
        if (killerId.IsNpc())
        {
            TRACE("Ship destroyed by NPC, ignoring possible lootable");
            return;
        }

        auto* params = ResourceManager::LookupShipCreationInfo(ship->cship()->id.GetValue());
        if (!params)
        {
            DEBUG("Destroyed ship had no creation params");
            return;
        }

        const auto lootTable = std::ranges::find_if(lootTables, [params](const auto& table) { return table.second->loadout.GetValue() == params->loadout; });
        if (lootTable == lootTables.end())
        {
            DEBUG("Destroyed ship had no loot table associated with it");
            return;
        }

        DEBUG("Lootable found for {{itemId}}", { "itemId", Id(params->loadout) });
        LootTableRoll(lootTable->second, killerId.GetPlayer().Handle());
    }

    void LootTablesPlugin::OnSolarDestroy(Solar* solar, DestroyType& isKill, const ShipId killerId)
    {
        if (killerId.IsNpc())
        {
            TRACE("Solar destroyed by NPC, ignoring possible lootable");
            return;
        }

        auto* params = ResourceManager::LookupSolarCreationInfo(solar->csolar()->id.GetValue());
        if (!params)
        {
            DEBUG("Destroyed solar had no creation params");
            return;
        }

        const auto lootTable = std::ranges::find_if(lootTables, [params](const auto& table) { return table.second->loadout == params->loadoutId; });
        if (lootTable == lootTables.end())
        {
            DEBUG("Destroyed solar had no loot table associated with it");
            return;
        }

        DEBUG("Lootable found for {{item}}", { "item", Id(params->loadoutId) });
        LootTableRoll(lootTable->second, killerId.GetPlayer().Handle());
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Loot Tables",
	    .shortName = L"loot_tables",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(LootTablesPlugin);
