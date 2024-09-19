#pragma once

#include "API/FLHook/ResourceManager.hpp"
#include "Core/Commands/AbstractAdminCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date September, 2024
     * @author IrateRedKite
     * @brief
     * The Spawner plugin allows a server admin to define NPCs and solars that can be spawned by other plugins as well as spawn them on a schedule or startup.
     *
     * @par configuration Configuration
     * No configuration file is needed.
     *
     * @par Player Commands
     * There are no player commands in this plugin.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */

    class SpawnerPlugin final : public Plugin
    {
            struct Spawnable final
            {
                    std::wstring name;
                    EquipmentId arch;
                    Id loadout;
                    RepGroupId iff;
                    std::optional<std::pair<uint, uint>> nameOneIds;
                    std::optional<std::pair<uint, uint>> nameTwoIds;
                    std::wstring pilot;

                    std::optional<Id> head;
                    std::optional<Id> body;
                    std::optional<Id> helmet;
                    std::optional<std::wstring> voice;
            };

            struct Npc final
            {
                    rfl::Flatten<Spawnable> common;
                    // TODO: Convert to Enum
                    ResourceManager::SpaceObjectBuilder::StateGraph graph = ResourceManager::SpaceObjectBuilder::StateGraph::Fighter;
                    int rank = 19;
            };

            struct Solar final
            {
                    rfl::Flatten<Spawnable> common;
                    BaseId base;
            };

            struct FormationComponent final
            {
                    std::wstring name;
                    // If true, this randomly place stuff in a 1000m radius, generally for use with NPCs and multiple quantity parts
                    bool randomPositions = false;
                    int quantity = 1;
                    Vector relativePosition;
                    Vector rotation;
                    // Under the hood value to determine if this is an NPC or a Solar
                    rfl::Skip<rfl::Variant<Npc*, Solar*>> object;
            };

            struct Formation final
            {
                    std::wstring name;
                    std::vector<FormationComponent> components;
            };

            struct SpawnLocation final
            {
                    Vector position{};
                    SystemId system;
            };

            struct ScheduledSpawns final
            {
                    // This takes an NPC, Solar or Formation
                    std::wstring spawnTarget;
                    // TODO: Ensure quantity does not exceed the maximum number of possible spawnLocations
                    std::vector<SpawnLocation> spawnLocations;
                    rfl::Validator<float, rfl::Minimum<0.0f>, rfl::Maximum<1.0f>> spawnChance;
                    int spawnWeight = 1;
                    int quantity = 1;
                    bool spawnOnStart = true;
                    std::wstring spawnTime = L"0 10 * * 5";
                    //  TODO: @Laz Create validator
                    int lifetimeInSeconds = 6000; // 0 is infinite lifetime/until killed
                    //  Should only occur on successful spawn of a full scheduled spawn, empty string means no message
                    std::wstring spawnMessage = L"This special Solar/NPC has spawned in New York!";
                    //  TODO: Turn this into an enum
                    int messageTarget;
                    //  Under the hood value to determine if this is an NPC, Solar or Formation.
                    rfl::Skip<rfl::Variant<Npc*, Solar*, Formation*>> object;
            };

            bool OnLoadSettings() override;
            void OnLoginAfter(ClientId client, const SLoginInfo& li) override;

            void SpawnObject(const rfl::Variant<Npc*, Solar*>& objectData, SystemId system, const Vector& position, const Vector& orientation = {});

            std::vector<Npc> npcs;
            std::vector<Solar> solars;
            std::vector<Formation> formations;
            std::vector<ScheduledSpawns> scheduledSpawns;
            bool firstRun = true;

        public:
            explicit SpawnerPlugin(const PluginInfo& info);

            /**
             * @brief TODO
             * @returns const reference std::vector<Npc>
             */
            const std::vector<Npc>& GetNpcs();
            const std::vector<Solar>& GetSolars();
            const std::vector<Formation>& GetFormations();
    };
} // namespace Plugins
