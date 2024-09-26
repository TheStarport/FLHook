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
            /**
             * @brief The common struct for a spawnable object, this is inherited by Npc and Solar.
             */
            struct Spawnable final
            {
                    std::wstring name;                               //! The internal identifier for a Spawnable.
                    EquipmentId arch;                                //! The Freelancer archetype nickname used for a Spawnable.
                    std::optional<Id> loadout;                       //! Optional, the Freelancer loadout nickname used for a Spawnable.
                    std::optional<RepGroupId> iff;                   //! Optional, the Freelancer IFF nickname used for a Spawnable.
                    std::optional<std::pair<uint, uint>> nameOneIds; //! Optional, the Freelancer IDS range to use for a Spawnable's 'First Name'.
                    std::optional<std::pair<uint, uint>> nameTwoIds; //! Optional, the Freelancer IDS range to use for a Spawnable's 'Second Name'.
                    std::optional<std::wstring> pilot;               //! Optional, the Freelancer pilot to use for a Spawnable.

                    std::optional<Id> head;            //! Optional, the Freelancer head nickname to use for a Spawnable comm.
                    std::optional<Id> body;            //! Optional, the Freelancer body nickname to use for a Spawnable comm.
                    std::optional<Id> helmet;          //! Optional, the Freelancer helmet nickname to use for a Spawnable comm.
                    std::optional<std::wstring> voice; //! Optional, the Freelancer voice nickname to use for a Spawnable comm.
            };

            /**
             * @brief A spawnable Npc, inherits from Spawnable.
             */
            struct Npc final
            {
                    rfl::Flatten<Spawnable> common; // The inherited struct for the rest of the Npc Data.

                    ResourceManager::SpaceObjectBuilder::StateGraph graph =
                        ResourceManager::SpaceObjectBuilder::StateGraph::Fighter; //! The State Graph used for an Npc. Options are Player, Fighter, Transport,
                                                                                  //! Gunboat or Cruiser.
                    int rank = 19;                                                //! The rank an Npc is spawned with.
            };

            /**
             * @brief A spawnable Solar, inherits from Spawnable.
             */
            struct Solar final
            {
                    rfl::Flatten<Spawnable> common; // The inherited struct for the rest of the Solar Data.
                    std::optional<BaseId> base;     //! Optional, the base nickname used for a Solar if it is dockable.
            };

            /**
             * @brief An individual formation component, used in the Formation struct. Can contain Npc or Solar object and associated positonal data.
             */
            struct FormationComponent final
            {
                    std::wstring name;            //! The name of a Solar or Npc object.
                    bool randomPositions = false; //! If true, randomly vary the position of the FormationComponent within a 1000m radius.
                    int quantity = 1;             //! The quantity of FormationComponents that will be spawned.
                    Vector relativePosition;      //! The position of a FormationComponent relative to the Formation's spawn location.
                    Vector rotation;              //! The orientation of the FormationComponent.

                    rfl::Skip<rfl::Variant<Npc*, Solar*>>
                        object; //! Internal runtime variable for easy access to the object. Not part of the configuration file.
            };

            /**
             * @brief A group of FormationComponent objects that comprise a complete formation of Npcs and/or Solars.
             */
            struct Formation final
            {
                    std::wstring name;                          //! The internal identifier for a Formation.
                    std::vector<FormationComponent> components; //! A list of FormationComponents that make up the Formation.
            };

            /**
             * @brief A group of complete formations with attached spawn weight metadata. Used in SpawnPool.
             */
            struct PoolFormation final
            {
                    std::wstring formation; //! The name of a existing Formation to spawn.
                    int spawnWeight;        //! The weight with which a formation will be selected.
            };

            /**
             * @brief Positional data containing coordinates and a system for the purposes of spawning an object.
             */
            struct SpawnLocation final
            {
                    Vector position{}; //! An XYZ position with which to spawn a Formation.
                    SystemId system;   //! The Freelancer nickname for a system in which to spawn a formation.
            };

            enum class MessageTarget
            {
                None,    //! No message will be sent on a spawn event.
                System,  //! A message will be broadcast to the system in which an object is spawned on the spawn event.
                Universe //! A message will be broadcast to the entire server when an object is spawned on the spawn event.
            };

            /**
             * @brief A pool of possible spawns and their associated data and metadata. Used in ScheduledSpawns.
             */
            struct SpawnPool final
            {
                    std::wstring name; //! The internal identifier for a SpawnPool.
                    rfl::Validator<float, rfl::Minimum<0.0f>, rfl::Maximum<1.0f>>
                        spawnChance;                       //! The percentile chance between 0.0 and 1.0 that a SpawnPool will be spawned at all.
                    int quantity;                          //! The quantity of objects to create in the SpawnPool.
                    bool spawnOnStart = true;              //! Determines whether or not a SpawnPool should be spawned on server startup.
                    std::optional<std::wstring> spawnTime; //! Cron-format schedule timer that determines when a SpawnPool should be spawned.
                    int lifetimeInSeconds; //! The lifetime of a spawned object from the SpawnPool in seconds. A 0 value will cause spawned objects in this pool
                                           //! to persist until server restart or until destroyed.
                    std::optional<std::wstring> spawnMessage; //! Optional, the message broadcast when an object in the SpawnPool is created.
                    MessageTarget messageTarget;              //! The broadcast level of the sent message. Options are None, System or Universe.
                    std::vector<PoolFormation> formatations;  //! A list of possible Formations that can be spawned from the SpawnPool.
                    std::vector<SpawnLocation> locations;     //! A list of possible Locations that Formations can be spawned into.
            };

            bool OnLoadSettings() override;
            void OnLoginAfter(ClientId client, const SLoginInfo& li) override;

            void SpawnObject(const rfl::Variant<Npc*, Solar*>& objectData, SystemId system, const Vector& position, const Vector& orientation = {});

            std::vector<Npc> npcs;               //! A list of Npcs that can be spawned.
            std::vector<Solar> solars;           //! A list of Solars that can be spawned.
            std::vector<Formation> formations;   //! A list of formations that can be spawned, consisting of groups of Npcs and/or Solars.
            std::vector<SpawnPool> spawnPools{}; //! A list of possible SpawnPools, to be spawned on server start or on a defined cron schedule.
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
