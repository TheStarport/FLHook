#pragma once

#include "API/FLHook/Plugin.hpp"
#include "API/Types/GoodId.hpp"
#include "API/Types/Id.hpp"
#include "API/Types/RepGroupId.hpp"
#include "rfl/Ref.hpp"

#include <httplib.h>

namespace Plugins
{
    /**
     * @date May 2025
     * @author Laz
     * @brief
     * This plugin is to enable spawning groups of Solars in specific locations on startup, with extra conditions to prevent or weight spawning counts/odds
     */
    class StartupSolarsPlugin final : public Plugin
    {
            struct SpawnedSolar
            {
                    Id archetype;
                    Id loadout;
                    std::wstring pilot;
                    int idsName;
                    int idsInfo;
                    std::optional<Vector> pos;
                    std::optional<Vector> rotation;
                    std::optional<RepGroupId> repGroup;

                    rfl::Skip<std::weak_ptr<CEqObj>> spawnedEntity;
            };

            using Formation = std::vector<rfl::Ref<SpawnedSolar>>;

            struct Location
            {
                    Vector pos;
                    Vector rotation;
                    SystemId system;
                    rfl::Skip<bool> inUse;
            };

            struct Group
            {
                    RepGroupId repGroup{};
                    std::vector<rfl::Ref<Location>> locations{};
                    std::optional<float> spawnChance{};
                    std::optional<int> respawnTime{};
                    std::optional<int> hullRegenTime{};
                    std::optional<float> hullRegen{};
                    std::vector<int> spawnCount{};
                    rfl::Rename<"formations", std::unordered_map<std::string, uint>> possibleFormationsRaw{};
                    rfl::Skip<std::vector<uint>> possibleFormationWeights{};
                    rfl::Skip<std::vector<Formation>> possibleFormations{};

                    rfl::Skip<int64> timeSinceLastHeal = 0;
                    rfl::Skip<std::vector<int64>> timeWhenLastSpawned{};
                    rfl::Skip<std::vector<Formation>> spawnedFormations{};
                    rfl::Skip<std::vector<int>> spawnedLocations{};
            };

            struct Config
            {
                    std::unordered_map<std::string, std::vector<rfl::Ref<SpawnedSolar>>> formations;
                    std::unordered_map<std::string, rfl::Ref<Group>> groups;
            };

            Config config;

            bool OnLoadSettings() override;
            void OnServerStartupAfter() override;

            static void SpawnFormation(const Formation& formation, const Location& location, RepGroupId group);
            void OnSpawnTimerElapsed();

        public:
            explicit StartupSolarsPlugin(const PluginInfo& info);
    };
} // namespace Plugins
