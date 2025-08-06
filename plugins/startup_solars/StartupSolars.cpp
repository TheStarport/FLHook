#include "PCH.hpp"

#include "StartupSolars.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/HttpServer.hpp"
#include "API/FLHook/PersonalityHelper.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/Random.hpp"

#include <bsoncxx/json.hpp>

namespace Plugins
{
    StartupSolarsPlugin::StartupSolarsPlugin(const PluginInfo& info) : Plugin(info) {}

    /// Load the configuration
    bool StartupSolarsPlugin::OnLoadSettings()
    {
        LoadJsonWithPrompt(Config, config, "config/startup_solars.json");

        std::unordered_set<std::wstring> errors;

#define ADD_ERROR(key, msg) errors.insert(std::format(L"{}.{} - {}", StringUtils::stows(name), key, msg))

        for (const auto& [name, formation] : config.formations)
        {
            const auto key = std::format(L"formations.{}", StringUtils::stows(name));
            if (formation.empty())
            {
                ADD_ERROR(key, L"Formation arrays should not be empty!");
                continue;
            }

            int solarIndex = 0;
            for (const auto& solar : formation)
            {
                const auto arrKey = std::format(L"{}[{}]", key, solarIndex);
                solarIndex++;
                if (!Archetype::GetSolar(solar->archetype.GetValue()))
                {
                    ADD_ERROR(std::format(L"{}.archetype", arrKey), L"Archetype specified was not a valid solararch");
                }

                if (!Loadout::Get(solar->loadout.GetValue()))
                {
                    ADD_ERROR(std::format(L"{}.loadout", arrKey), L"Loadout specified was not a valid loadout");
                }

                if (FLHook::GetPersonality(solar->pilot).Raw().has_error())
                {
                    ADD_ERROR(std::format(L"{}.pilot", arrKey), L"Pilot specified was not a valid pilot");
                }

                if (solar->repGroup.has_value() && !solar->repGroup.value())
                {
                    ADD_ERROR(std::format(L"{}.repGroup", arrKey), L"Rep group specified was not a valid faction");
                }

                if (solar->rotation.has_value() && (solar->rotation->x > 180.0f || solar->rotation->y > 180.0f || solar->rotation->z > 180.0f ||
                                                    solar->rotation->x < -180.0f || solar->rotation->y < -180.0f || solar->rotation->z < -180.0f))
                {
                    ADD_ERROR(std::format(L"{}.rotation", arrKey), L"Rotation array had a value that was greater than 180, or less than -180.");
                }
            }
        }

        for (const auto& [name, group] : config.groups)
        {
            const auto key = std::format(L"groups.{}", StringUtils::stows(name));
            if (!group->repGroup)
            {
                ADD_ERROR(std::format(L"{}.repGroup", key), L"Rep group specified was not a valid faction");
            }

            if (!group->respawnTime.has_value() && group->respawnTime.value() <= 0)
            {
                ADD_ERROR(std::format(L"{}.respawnTime", key), L"Respawn time was given a zero or a negative value.");
            }

            if (group->hullRegen.has_value() && group->hullRegen.value() <= 0.f)
            {
                ADD_ERROR(std::format(L"{}.hullRegen", key), L"Hull regen was given a negative value.");
            }

            if (group->hullRegenTime.has_value() && group->hullRegenTime.value() <= 0)
            {
                ADD_ERROR(std::format(L"{}.hullRegenTime", key), L"Hull regen time was zero or negative");
            }

            if (group->spawnChance.has_value())
            {
                if (group->spawnChance.value() <= 0.f)
                {
                    ADD_ERROR(std::format(L"{}.spawnChance", key), L"Spawn chance was less than or equal to zero");
                }
                else if (group->spawnChance.value() >= 1)
                {
                    ADD_ERROR(std::format(L"{}.spawnChance", key), L"Spawn chance was greater than or equal to one");
                }
            }

            if (group->spawnCount.empty())
            {
                ADD_ERROR(std::format(L"{}.spawnCount", key), L"Spawn count was empty");
            }
            else
            {
                int spawnIndex = 0;
                for (const auto& spawnCount : group->spawnCount)
                {
                    if (spawnCount <= 0)
                    {
                        ADD_ERROR(std::format(L"{}.spawnCount[{}]", key, spawnIndex), L"Spawn count was less than or equal to zero.");
                    }

                    spawnIndex++;
                }
            }

            if (group->possibleFormationsRaw.value().empty())
            {
                ADD_ERROR(std::format(L"{}.formations", key), L"Formations was empty");
            }

            for (auto& [name, weight] : group->possibleFormationsRaw.value())
            {
                const auto formation = config.formations.find(name);
                if (formation == config.formations.end())
                {
                    ADD_ERROR(std::format(L"{}.formations.{}", key, StringUtils::stows(name)), L"Formations was not found");
                    continue;
                }

                group->possibleFormations.value().emplace_back(formation->second);
                group->possibleFormationWeights.value().emplace_back(weight);
            }

            if (group->locations.size() < group->possibleFormations.value().size())
            {
                ADD_ERROR(std::format(L"{}.locations", key), L"You must have at least as many locations as you have formations.");
            }

            int index = 0;
            for (const auto& location : group->locations)
            {
                if (location->rotation.x > 180.0f || location->rotation.y > 180.0f || location->rotation.z > 180.0f || location->rotation.x < -180.0f ||
                    location->rotation.y < -180.0f || location->rotation.z < -180.0f)
                {
                    ADD_ERROR(std::format(L"{}.locations[{}].rotation", key, index),
                              L"Rotation array had a value that was greater than 180, or less than -180.");
                }

                if (!location->system)
                {
                    ADD_ERROR(std::format(L"{}.locations[{}].rotation", key, index), L"System specified (nickname) does not exist");
                }

                index++;
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

        return true;
    }

    void StartupSolarsPlugin::OnServerStartupAfter()
    {
        AddTimer([this] { OnSpawnTimerElapsed(); }, 1min); // Once a minute

        for (const auto& [name, group] : config.groups)
        {
            if (group->spawnChance.has_value() && Random::UniformFloat(0.f, 1.f) > group->spawnChance.value())
            {
                DEBUG("Skipped solar group spawn {{name}}", { "name", name });
                continue;
            }

            const auto count = Random::Item(group->spawnCount);
            for (int i = 0; i < count; i++)
            {
                const auto selectedFormationIndex =
                    Random::Weighted(group->possibleFormationWeights.value().begin(), group->possibleFormationWeights.value().end());
                auto& formation = group->possibleFormations.value()[selectedFormationIndex];

                // Select a random available location
                while (true)
                {
                    const auto locationIndex = Random::Uniform(0u, group->locations.size() - 1);
                    const auto location = group->locations[locationIndex];
                    if (location->inUse.value())
                    {
                        continue;
                    }

                    location->inUse = true;
                    group->spawnedLocations.value().emplace_back(locationIndex);
                    break;
                }

                Formation spawnedFormation;

                // Make copy of the spawned formation
                for (auto& solar : formation)
                {
                    spawnedFormation.emplace_back(rfl::Ref<SpawnedSolar>::make(*solar.get()));
                }

                group->timeWhenLastSpawned.value().emplace_back(0);
                group->spawnedFormations.value().emplace_back(spawnedFormation);
            }
        }
    }

    void StartupSolarsPlugin::SpawnFormation(const Formation& formation, const Location& location, RepGroupId group)
    {
        const auto resourceManager = FLHook::GetResourceManager();

        for (const auto& solar : formation)
        {
            if (!solar->spawnedEntity.value().expired())
            {
                continue;
            }

            // clang-format off
            solar->spawnedEntity = resourceManager->NewBuilder()
                .AsSolar()
                .WithArchetype(solar->archetype)
                .WithPersonality(solar->pilot)
                .WithLoadout(solar->loadout.GetValue())
                .WithReputation(solar->repGroup.value_or(group))
                .WithName(solar->idsName, solar->idsInfo)
                .WithPosition(location.pos + solar->pos.value_or(Vector{}))
                .WithRotation(location.rotation + solar->rotation.value_or(Vector{}))
                .WithSystem(location.system)
                .Spawn();
            // clang-format on

            if (solar->spawnedEntity.value().expired())
            {
                DEBUG("Failed to spawn solar within formation");
                continue;
            }

            DEBUG("Spawned solar within formation");
        }
    }

    void StartupSolarsPlugin::OnSpawnTimerElapsed()
    {
        const auto currentTime = TimeUtils::UnixTime<std::chrono::seconds>();
        for (const auto& group : config.groups | std::views::values)
        {
            if (group->hullRegenTime && group->hullRegenTime.value() + currentTime > group->timeSinceLastHeal.value())
            {
                for (auto formation : group->spawnedFormations.value())
                {
                    for (auto solar : formation)
                    {
                        if (solar->spawnedEntity.value().expired())
                        {
                            continue;
                        }

                        const auto entity = solar->spawnedEntity.value().lock();
                        float health;
                        pub::SpaceObj::GetRelativeHealth(entity->id.GetValue(), health);
                        health += group->hullRegen.value() + health;
                        pub::SpaceObj::SetRelativeHealth(entity->id.GetValue(), health);
                    }
                }
            }

            if (!group->respawnTime)
            {
                continue;
            }

            for (size_t i = 0; i < group->spawnedFormations.value().size(); i++)
            {
                if (group->respawnTime.value() + currentTime < group->timeWhenLastSpawned.value()[i])
                {
                    continue;
                }

                SpawnFormation(group->spawnedFormations.value()[i], *group->locations[group->spawnedLocations.value()[i]], group->repGroup);
                group->timeWhenLastSpawned.value()[i] = currentTime;
            }
        }
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Startup Solars",
	    .shortName = L"startup_solars",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(StartupSolarsPlugin);
