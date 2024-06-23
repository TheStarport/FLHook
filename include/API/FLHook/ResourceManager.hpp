#pragma once

#include "ResourcePtr.hpp"

class DLL ResourceManager final
{
        friend IEngineHook;

        struct NpcTemplate
        {
                std::wstring nickname;
                uint hash;
                std::wstring loadout;
                uint loadoutHash;
                std::wstring archetype;
                uint archetypeHash;
                std::wstring pilot;
                uint pilotHash;
                uint level;
        };

        inline static std::unordered_map<uint, NpcTemplate> npcTemplates;
        std::vector<std::pair<std::shared_ptr<CShip>, bool>> spawnedShips;
        std::vector<std::pair<std::shared_ptr<CSolar>, bool>> spawnedSolars;

        void OnShipDestroyed(Ship* ship);
        void OnSolarDestroyed(Solar* solar);
        static void SendSolarPacket(uint spaceId, pub::SpaceObj::SolarInfo& si);

    public:
        struct SpaceObjectBuilder
        {
                struct Fuse
                {
                        std::variant<std::wstring, uint> fuse;
                        float radius = 5.0f;
                        float lifetime = 5.0f;
                        short equipmentId = 0;
                };

                enum class Region
                {
                    Random,
                    Bretonia,
                    Hispania,
                    Kusari,
                    Liberty,
                    Rheinland
                };

                /**
                 * Set the pilot, level, loadout, and ship archetype values from an NPC entry.
                 * These can be overriden with subsequent calls.
                 * @param npcNickname The internal nickname of an NPC template (specified in any npcships*.ini file)
                 */
                SpaceObjectBuilder& WithNpc(const std::wstring& npcNickname);

                /**
                 * Set the archetype of the ship/solar via nickname. It will be validated on spawn.
                 * @param archetype The internal nickname of a ship/solar archetype
                 */
                SpaceObjectBuilder& WithArchetype(const std::wstring& archetype);

                /**
                 * Set the archetype of the ship/solar via hash. It will be validated on spawn.
                 * @param archetype The hash of a ship/solar archetype
                 */
                SpaceObjectBuilder& WithArchetype(uint archetype);

                /**
                 * Set the loadout for the ship/solar.
                 * It will be validated on spawn, but will not validate that the loadout fits the archetype provided.
                 * @param loadout The internal nickname of a ship/solar loadout
                 */
                SpaceObjectBuilder& WithLoadout(const std::wstring& loadout);

                /**
                 * Set the loadout for the ship/solar.
                 * It will be validated on spawn, but will not validate that the loadout fits the archetype provided.
                 * @param loadout The internal hash of a ship/solar loadout
                 */
                SpaceObjectBuilder& WithLoadout(uint loadout);

                /**
                 * @brief Set the costume for the solar/NPC to be spawned.
                 * @note The costume provided should remain in scope until Spawn() has been called.
                 * To not do so would be undefined behaviour, and would likely lead to crashes.
                 */
                SpaceObjectBuilder& WithCostume(const Costume& costume);

                /**
                 * @brief Set the pilot block for the spawned solar/npc
                 * @param personalityNickname The nickname of a pilot entry
                 */
                SpaceObjectBuilder& WithPersonality(const std::wstring& personalityNickname);

                /**
                 * @brief A personality that has been manually constructed or selected via FLHook's personality manager.
                 * @note The personality provided should remain in-scope until Spawn() has been called.
                 * To not do so would be undefined behaviour, and would likely lead to crashes.
                 */
                SpaceObjectBuilder& WithPersonality(pub::AI::Personality& personality);

                /**
                 * @brief Set the spawn position for the solar/npc. Variance in provided value can optionally be provided.
                 * @param positionVector The X, Y, Z coordinates of where the entity should be spawned.
                 * @param variance The amount of random variance that should be applied to the position.
                 * It adds a random value between -variance and variance to each of the axis specified.
                 * - For example, if you specified 500.0f as your variance, every axis would pick random values between -500.0
                 * and 500.0, potentially leading to something like pos + { -123.f, 123.f, 400.f }; and everything in-between.
                 */
                SpaceObjectBuilder& WithPosition(const Vector& positionVector, float variance = 0.0f);

                /**
                 * @brief Specify the rotation of the object via euler angles (degrees)
                 * @param euler A vector representing the PYR rotation of the object in degrees
                 */
                SpaceObjectBuilder& WithRotation(const Vector& euler);

                /**
                 * @brief Specify the rotation of the object via a rotation matrix
                 * @param orientation A rotational matrix. Will not be validated.
                 */
                SpaceObjectBuilder& WithRotation(const Matrix& orientation);

                /**
                 * @brief Specify the hash of the system to be spawned into.
                 * @param systemHash The system hash. Will be validated on spawn.
                 */
                SpaceObjectBuilder& WithSystem(uint systemHash);

                /**
                 * @brief Specify the nickname of the system to be spawned into.
                 * @param systemNick The system nickname. Will be validated on spawn.
                 */
                SpaceObjectBuilder& WithSystem(const std::wstring& systemNick);

                /**
                 * @brief Set the absolute health of the target
                 * @param absoluteHealth Can be any positive, non-zero, value.
                 */
                SpaceObjectBuilder& WithAbsoluteHealth(float absoluteHealth);

                /**
                 * @brief Set the relative health of the target on spawn.
                 * @param relativeHealth Must be a value between 0 and 1 (exclusive, inclusive)
                 */
                SpaceObjectBuilder& WithRelativeHealth(float relativeHealth);

                /**
                 * @brief Set the level for the spawned NPC. Used
                 * @param level A non-zero number to use for NPC level. This can affect toughness and other AI modifiers.
                 */
                SpaceObjectBuilder& WithLevel(uint level);

                /**
                 * @brief Set the pilot voice for the entity. The value provided here is not validated.
                 * @param voice The nickname of the desired voice for this spawned entity
                 */
                SpaceObjectBuilder& WithVoice(const std::wstring& voice);

                /**
                 * @brief Specify the name that should be used for this spawned entity.
                 * @param firstName The IDS name that should be used for first name of the pilot
                 * @param secondName The IDS name that should be used for the second name of the pilot. This value is optional,
                 * provide 0 to ignore.
                 */
                SpaceObjectBuilder& WithName(uint firstName, uint secondName = 0);

                /**
                 * @brief Specify the names that should be used for spawning the desired entity.
                 * @note Once specified, the FmtStrs specified should remain in scope until Spawn() has been called.
                 * To not do so is undefined behaviour, and will likely lead to a crash.
                 * @param scannerName The format structure for what name should appear in the scanner (contact window) for this npc/solar
                 * @param pilotName The format structure for what name should appear in all other contexts for this npc/solar
                 */
                SpaceObjectBuilder& WithName(FmtStr& scannerName, FmtStr& pilotName);

                /**
                 * @brief Specify the affiliation that the NPC/Solar should be spawned with
                 * @param rep A string representing the nickname of the desired faction
                 */
                SpaceObjectBuilder& WithReputation(const std::wstring& rep);

                /**
                 * @brief Specify the affiliation that the NPC/Solar should be spawned with
                 * @param affiliation The rep group id that should be used.
                 */
                SpaceObjectBuilder& WithReputation(uint affiliation);

                /**
                 * @brief Specify that the spawned solar should have a dock target
                 * @param base The hash of the base that should be a dock target for the spawned solar
                 */
                SpaceObjectBuilder& WithDockTo(uint base);

                /**
                 * @brief Indicates that a certain fuse should be played when the ship is spawned.
                 * @param fuse Information about the fuse that should be played.
                 */
                SpaceObjectBuilder& WithFuse(const Fuse& fuse);

                /**
                 * @brief Select a random NPC template from any of the ships loaded in from the various npcships ini files.
                 * This will set the archetype, loadout, level and pilot for the builder, but these can be overidden.
                 */
                SpaceObjectBuilder& WithRandomNpc();

                /**
                 * @brief Randomise the reputation that the target will spawn with
                 * @param excludeStoryFactions By default this function will exclude 'special' factions that do not appear in the reputation sheet.
                 * Things like the story Rheinland Millitary, Q's Men, etc
                 */
                SpaceObjectBuilder& WithRandomReputation(bool excludeStoryFactions = true);

                /**
                 * @brief Give the NPC a random name from one of the game's regions. By default, if this function is not used,
                 * names are set to [Colour/Colour] (e.g. Red Blue or Silver Black)
                 * @param region Whether a regional name should be selected. By default it picks a name from any of the regions.
                 */
                SpaceObjectBuilder& WithRandomName(Region region = Region::Random);

                /**
                 * @brief Ensures that the ship/solar will remain valid in memory, even after being destroyed.
                 * The only way it will be cleaned up and disposed of, is if it is explicitly destroyed through the ResourceManager.
                 * @note This should be considered very dangerous and should you not keep hold of pointers to delete it,
                 * it will lead to potentially quite large memory leaks and performance degredation.
                 */
                SpaceObjectBuilder& WithMemoryProtection();

                /**
                 * @brief Set the spawn target as a solar rather than an NPC.
                 */
                SpaceObjectBuilder& AsSolar();

                /**
                 * @brief Set the spawn target as an NPC rather than a solar.
                 */
                SpaceObjectBuilder& AsNpc();

                /**
                 * @brief Create the desired NPC/Solar with the values provided.
                 * @note For any spawn to occur: position, system, and archetype must be set. To not do so would be a logic error.
                 * In some cases, not providing a loadout or providing one that is invalid, can also cause crashes.
                 * @return A weak pointer to a CEqObj that represents a spawned ship or solar. If there was any kind of validation error
                 * or issue spawning the target, warnings will be logged to the console and a nullptr will be returned.
                 */
                std::weak_ptr<CEqObj> Spawn();

            private:
                NpcTemplate npcTemplate{};
                std::optional<Costume> costumeOverride{};
                std::optional<uint> voiceOverride{};
                std::optional<const pub::AI::Personality*> personalityOverride{};
                std::optional<Matrix> rotation{};
                std::optional<Vector> position{};
                std::optional<float> positionVariance{};
                std::optional<uint> system{};
                std::optional<float> health{};
                std::optional<std::wstring> reputation{};
                std::optional<uint> affiliation{};
                std::optional<std::pair<uint, uint>> name{};
                std::optional<uint> dockTo{};
                std::optional<std::pair<FmtStr&, FmtStr&>> names{};
                std::optional<Fuse> fuse{};

                std::array<uint, 21> defaultNames = { 197808u, 197809u, 197810u, 197811u, 197812u, 197813u, 197814u, 197815u, 197816u, 197817u, 197818u,
                                                      197819u, 197820u, 197821u, 197822u, 197823u, 197824u, 197825u, 197826u, 197827u, 197828 };

                bool isNpc = true;
                bool healthRelative = true;
                bool protectMemory = false;
                ResourceManager& resourceManager;

                friend ResourceManager;
                std::weak_ptr<CShip> SpawnNpc();
                std::weak_ptr<CSolar> SpawnSolar();
                bool ValidateSpawn() const;
                explicit SpaceObjectBuilder(ResourceManager& resourceManager) : resourceManager(resourceManager) {};
        };

        ResourceManager();
        ~ResourceManager();

        SpaceObjectBuilder NewBuilder() { return SpaceObjectBuilder{*this}; }

        /**
         * @brief Cause an object to be destroyed, but not despawned.
         * Calling this will call the death fuse and release control over the object, using the underlying pointer after this
         * will be valid for the duration of the death fuse, but invalid after. For this reason, using the pointer after calling this function
         * is undefined behaviour.
         * @param object The object that should be destroyed.
         * @param instantly If false, set health to 0 causing any death animations the ship may have.
         * Otherwise instantly destroy the ship.
         */
        void Destroy(std::weak_ptr<CEqObj> object, const bool instantly = false);

        /**
         * @brief Instantly destroy the object without a fuse and dispose of the underlying object. Using the provided pointer after this is
         * to be considered invalid.
         * @param object The object that should be disposed.
         */
        void Despawn(std::weak_ptr<CEqObj> object);
};
