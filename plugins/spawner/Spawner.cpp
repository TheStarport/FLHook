#include "PCH.hpp"

#include "Spawner.hpp"

#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/Random.hpp"

namespace Plugins
{
    bool SpawnerPlugin::OnLoadSettings()
    {
        auto npcResult = Json::Load<std::vector<Npc>>("config/spawner_npcs.json", false);
        auto solarResult = Json::Load<std::vector<Solar>>("config/spawner_solars.json", false);
        auto formationResult = Json::Load<std::vector<Formation>>("config/spawner_formationss.json", false);
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
                if (formation.name == scheduledSpawn.name)
                {
                    scheduledSpawn.object = &formation;
                    break;
                }
            }
            for (auto& solar : solars)
            {
                if (solar.common.value_.name == scheduledSpawn.name)
                {
                    scheduledSpawn.object = &solar;
                    break;
                }
            }
            for (auto& npc : npcs)
            {
                if (npc.common.value_.name == scheduledSpawn.name)
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
                Logger::Warn(std::format(L"Could not find specified object '{}' in scheduled spawns list", scheduledSpawn.name));
                return false;
            }
        }

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
        .WithLoadout(common.loadout.GetValue())
        .WithReputation(common.iff.GetValue())
        .WithPersonality(common.pilot)
        .WithSystem(system.GetValue())
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
            .head = common.head.GetValue(),
            .body = common.body.GetValue(),
            .leftHand = 0,
            .rightHand = 0,
        };
        costume.accessory[0] = common.helmet.GetValue();
        costume.accessories = 1;
        // clang-format on

        if (costume.head == 0 || costume.body == 0)
        {
            costume.head = CreateID("benchmark_male_head");
            costume.body = CreateID("benchmark_male_body");
        }

        builder.WithCostume(costume);
        builder.WithVoice(common.voice.empty() ? L"atc_leg_m01" : common.voice);

        if (npc)
        {
            builder.WithLevel(npc->rank).WithStateGraph(npc->graph);
        }

        else
        {
            builder.WithDockTo(solar->base.GetValue());
            builder.AsSolar();
        }
        // TODO: Store spawned object and keep track of it
        auto object = builder.Spawn();
        if (object.expired())
        {
            Logger::Warn(std::format(L"Could not spawn object '{}' in '{}' ({})", common.name, system.GetName().Unwrap(), system.GetValue()));
        }
        else
        {
            Logger::Debug(std::format(L"Spawned '{}' at {}, {}, {} in '{}'", common.name, position.x, position.y, position.z, system.GetName().Handle()));
        }
    }

    SpawnerPlugin::SpawnerPlugin(const PluginInfo& info) : Plugin(info) {}
    const std::vector<SpawnerPlugin::Npc>& SpawnerPlugin::GetNpcs() { return npcs; }
    const std::vector<SpawnerPlugin::Solar>& SpawnerPlugin::GetSolars() { return solars; }
    const std::vector<SpawnerPlugin::Formation>& SpawnerPlugin::GetFormations() { return formations; }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Spawner", L"spawner", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(SpawnerPlugin, Info);
