#include "PCH.hpp"

#include "Spawner.hpp"

#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/Random.hpp"

namespace Plugins
{
    bool SpawnerPlugin::OnLoadSettings()
    {
        /*auto npcResult = Json::Load<std::vector<Npc>>("config/spawner_npcs.json", false);
        auto solarResult = Json::Load<std::vector<Solar>>("config/spawner_solars.json", false);
        auto formationResult = Json::Load<std::vector<Formation>>("config/spawner_formations.json", false);
        auto scheduledSpawnsResult = Json::Load<std::vector<ScheduledSpawns>>("config/spawner_scheduled_spawns.json", false);

        if (npcResult.first == Json::LoadState::DoesNotExist || solarResult.first == Json::LoadState::DoesNotExist ||
            formationResult.first == Json::LoadState::DoesNotExist || scheduledSpawnsResult.first == Json::LoadState::DoesNotExist)
        {
            Logger::Err(L"One or more configs for spawner.dll do not exist. The following files should exist in the config folder: spawner_npcs.json, "
                        L"spawner_solars.json, spawner_formations.json and spawner_scheduled_spawns.json.");
            return false;
        }

        if (npcResult.first == Json::LoadState::FailedToValidate || solarResult.first == Json::LoadState::FailedToValidate ||
            formationResult.first == Json::LoadState::FailedToValidate || scheduledSpawnsResult.first == Json::LoadState::FailedToValidate)
        {
            return false;
        }

        npcs = npcResult.second.value();
        solars = solarResult.second.value();
        formations = formationResult.second.value();
        scheduledSpawns = scheduledSpawnsResult.second.value();

        // TODO: Validate for duplicate names between objects
        for (auto& formation : formations)
        {
            for (auto& component : formation.components)
            {
                for (auto& npc : npcs)
                {
                    if (npc.common.value_.name == component.name)
                    {
                        component.object = &npc;
                        break;
                    }
                }

                for (auto& solar : solars)
                {
                    if (solar.common.value_.name == component.name)
                    {
                        component.object = &solar;
                        break;
                    }
                }

                auto variant = component.object.get();

                if (const auto obj =
                        (variant.index() == 1 ? reinterpret_cast<void*>(rfl::get<Solar*>(variant)) : reinterpret_cast<void*>(rfl::get<Npc*>(variant)));
                    !obj)
                {
                    Logger::Warn(std::format(L"Invalid formation component '{}' specified in formation '{}'", component.name, formation.name));
                    return false;
                }
            }
        }
        for (auto& scheduledSpawn : scheduledSpawns)
        {
            for (auto& formation : formations)
            {
                if (formation.name == scheduledSpawn.spawnTarget)
                {
                    scheduledSpawn.object = &formation;
                    break;
                }
            }
            for (auto& solar : solars)
            {
                if (solar.common.value_.name == scheduledSpawn.spawnTarget)
                {
                    scheduledSpawn.object = &solar;
                    break;
                }
            }
            for (auto& npc : npcs)
            {
                if (npc.common.value_.name == scheduledSpawn.spawnTarget)
                {
                    scheduledSpawn.object = &npc;
                    break;
                }
            }
            auto variant = scheduledSpawn.object.get();
            void* obj = nullptr;

            if (variant.index() == 0)
            {
                obj = reinterpret_cast<void*>(rfl::get<Npc*>(variant));
            }
            else if (variant.index() == 1)
            {
                obj = reinterpret_cast<void*>(rfl::get<Solar*>(variant));
            }
            else
            {
                obj = reinterpret_cast<void*>(rfl::get<Formation*>(variant));
            }
            if (!obj)
            {
                Logger::Warn(std::format(L"Could not find specified object '{}' in scheduled spawns list", scheduledSpawn.spawnTarget));
                return false;
            }
        }
*/
        return true;
    }

    void SpawnerPlugin::SpawnObject(const rfl::Variant<Npc*, Solar*>& objectData, SystemId system, const Vector& position, const Vector& orientation)
    {
        Npc* npc = nullptr;
        Solar* solar = nullptr;

        if (objectData.index() == 1)
        {
            solar = rfl::get<Solar*>(objectData);
        }
        else
        {
            npc = rfl::get<Npc*>(objectData);
        }

        if (!npc && !solar)
        {
            // TODO: Better error message
            throw GameException(L"Something has gone very wrong");
        }

        // ReSharper disable once CppDFANullDereference
        Spawnable& common = npc ? npc->common.value_ : solar->common.value_;

        auto builder = FLHook::GetResourceManager()->NewBuilder();
        // clang-format off
        builder
        .WithArchetype(common.arch.GetId())
        //.WithLoadout(common.loadout.GetValue())
        //.WithReputation(common.iff.GetValue())
        //.WithPersonality(common.pilot)
        .WithSystem(system)
        .WithPosition(position)
        .WithRotation(orientation);
        // clang-format on

        if (!common.nameOneIds.has_value())
        {
            builder.WithRandomName();
        }
        else
        {
            auto [oneIdsLower, oneIdsUpper] = common.nameOneIds.value();
            auto [twoIdsLower, twoIdsUpper] = common.nameTwoIds.value_or(std::make_pair(0u, 0u));
            builder.WithName(Random::Uniform(oneIdsLower, oneIdsUpper), Random::Uniform(twoIdsLower, twoIdsUpper));
        }
        // clang-format off
        Costume costume{
            .head = common.head.value_or(Id(CreateID("benchmark_male_head"))).GetValue(),
            .body = common.body.value_or(Id(CreateID("benchmark_male_body"))).GetValue(),
            .leftHand = 0,
            .rightHand = 0,
        };
        costume.accessory[0] = common.helmet.value_or(Id()).GetValue();
        costume.accessories = 1;
        // clang-format on

        builder.WithCostume(costume);
        builder.WithVoice(common.voice.value_or(L"atc_leg_m01"));

        if (npc)
        {
            builder.WithLevel(npc->rank).WithStateGraph(npc->graph);
        }

        else
        {
            // builder.WithDockTo(solar->base.GetValue());
            builder.AsSolar();
        }
        // TODO: Store spawned object and keep track of it
        auto object = builder.Spawn();
        if (object.expired())
        {
            // Logger::Warn(std::format(L"Could not spawn object '{}' in '{}' ({})", common.name, system.GetName().Unwrap(), system.GetValue()));
        }
        else
        {
            // Logger::Debug(std::format(L"Spawned '{}' at {}, {}, {} in '{}'", common.name, position.x, position.y, position.z, system.GetName().Handle()));
        }
    }

    void SpawnerPlugin::OnLoginAfter(ClientId client, const SLoginInfo& li)
    {
        if (!firstRun)
        {
            return;
        }

        // for (auto& spawn : scheduledSpawns)
        //{
        //     if (!spawn.spawnOnStart)
        //     {
        //         continue;
        //     }
        //     auto variant = spawn.object.value();
        //     if (variant.index() == 2)
        //     {
        //         // TODO: Formation
        //     }
        //     else if (variant.index() == 1)
        //     {
        //         // TODO: Solar
        //     }
        //     else
        //     {
        //         SpawnObject(rfl::get<Npc*>(variant), SystemId(L"li01"), { -32977.f, 0.f, -27417.f });
        //     }
        //
        //    firstRun = false;
        //}
    }
    SpawnerPlugin::SpawnerPlugin(const PluginInfo& info) : Plugin(info) {}
    const std::vector<SpawnerPlugin::Npc>& SpawnerPlugin::GetNpcs() { return npcs; }
    const std::vector<SpawnerPlugin::Solar>& SpawnerPlugin::GetSolars() { return solars; }
    const std::vector<SpawnerPlugin::Formation>& SpawnerPlugin::GetFormations() { return formations; }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Spawner",
	    .shortName = L"spawner",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(SpawnerPlugin);
