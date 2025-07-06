#pragma once

#include "API/FLHook/Plugin.hpp"
#include "API/Types/GoodId.hpp"
#include "API/Types/Id.hpp"
#include "rfl/Ref.hpp"

#include <httplib.h>

namespace Plugins
{
    /**
     * @date May 2025
     * @author Laz
     * @brief
     * This plugin is to allow when NPCs/Solars are destroyed randomised items are dropped from them
     */
    class LootTablesPlugin final : public Plugin
    {
            struct RollItem
            {
                    std::optional<GoodId> goodId;
                    int weight;
                    std::optional<int> cost;
                    std::vector<int> count;
            };

            struct LootTable
            {
                    Id loadout;
                    unsigned maxRollsWithoutAnItem = 3;
                    std::optional<std::vector<int>> points;
                    std::optional<std::vector<int>> rollCount;
                    std::vector<rfl::Ref<RollItem>> rollItems;
                    rfl::Skip<std::vector<unsigned>> itemWeights;
            };

            std::unordered_map<std::string, rfl::Ref<LootTable>> lootTables;

            bool OnLoadSettings() override;
            static void LootTableRoll(rfl::Ref<LootTable> table, ClientId client);

            void OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) override;
            void OnSolarDestroy(Solar* solar, DestroyType& isKill, ShipId killerId) override;

        public:
            explicit LootTablesPlugin(const PluginInfo& info);
    };
} // namespace Plugins
