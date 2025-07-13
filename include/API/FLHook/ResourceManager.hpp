#pragma once

#include "Utils/Detour.hpp"

class IEngineHook;
class DLL ResourceManager final
{
        friend IServerImplHook;
        friend IEngineHook;

        using DefaultNakedType = void (*)();
        using CObjAllocatorType = CObject*(__cdecl*)(CObject::Class objClass);
        inline static std::unique_ptr<FunctionDetour<DefaultNakedType>> gameObjectDestructorDetour;
        inline static std::unique_ptr<FunctionDetour<DefaultNakedType>> cSimpleInitDetour;
        inline static std::unique_ptr<FunctionDetour<DefaultNakedType>> cObjDestrDetour;
        inline static std::unique_ptr<FunctionDetour<DefaultNakedType>> cObjectFindDetourFunc;
        inline static std::unique_ptr<FunctionDetour<CObjAllocatorType>> cObjAllocatorDetour;
        inline static std::unique_ptr<FunctionDetour<DefaultNakedType>> findStarListNaked2;

        using SpaceObjCreateType = int (*)(unsigned int& id, const pub::SpaceObj::ShipInfo& info);
        using SpaceObjSolarCreateType = int (*)(unsigned int& id, const pub::SpaceObj::SolarInfo& info);
        using SpaceObjLootCreateType = int (*)(unsigned int& id, const pub::SpaceObj::LootInfo& info);
        inline static FunctionDetour<SpaceObjCreateType> spaceObjCreateDetour{ pub::SpaceObj::Create };
        inline static FunctionDetour<SpaceObjSolarCreateType> spaceObjCreateSolarDetour{ pub::SpaceObj::CreateSolar };
        inline static FunctionDetour<SpaceObjLootCreateType> spaceObjCreateLootDetour{ pub::SpaceObj::CreateLoot };

        struct StarSystemMock
        {
                uint systemId;
                StarSystem starSystem;
        };

        struct iobjCache
        {
                StarSystem* cacheStarSystem;
                CObject::Class objClass;
        };

        struct CObjNode
        {
                CObjNode* next;
                CObjNode* prev;
                CObject* cobj;
        };

        struct CObjEntryNode
        {
                CObjNode* first;
                CObjNode* last;
        };

        struct CObjList
        {
                uint dunno;
                CObjEntryNode* entry;
                uint size;
        };

        using CObjListFunc = CObjList*(__cdecl*)(CObject::Class);
        using FindIObjOnList = MetaListNode*(__thiscall*)(MetaList&, uint searchedId);
        using FindIObjInSystem = IObjRW*(__thiscall*)(StarSystemMock& starSystem, uint searchedId);
        using RemoveCobjFromVector = void(__thiscall*)(CObjList*, void*, CObjNode*);
        const inline static auto findIObjOnListFunc = reinterpret_cast<FindIObjOnList>(0x6CF4F00);
        const inline static auto findIObjFunc = reinterpret_cast<FindIObjInSystem>(0x6D0C840);
        const inline static auto CObjListFind = reinterpret_cast<CObjListFunc>(0x62AE690);
        const inline static auto removeCObjNode = reinterpret_cast<RemoveCobjFromVector>(0x62AF830);
        
        inline static std::unordered_map<CObject*, CObjNode*> cMineMap;
        inline static std::unordered_map<CObject*, CObjNode*> cCmMap;
        inline static std::unordered_map<CObject*, CObjNode*> cBeamMap;
        inline static std::unordered_map<CObject*, CObjNode*> cGuidedMap;
        inline static std::unordered_map<CObject*, CObjNode*> cSolarMap;
        inline static std::unordered_map<CObject*, CObjNode*> cShipMap;
        inline static std::unordered_map<CObject*, CObjNode*> cAsteroidMap;
        inline static std::unordered_map<CObject*, CObjNode*> cLootMap;
        inline static std::unordered_map<CObject*, CObjNode*> cEquipmentMap;
        inline static std::unordered_map<CObject*, CObjNode*> cObjectMap;
        inline static std::unordered_map<uint, ClientId> playerShips;

        inline static std::unordered_map<uint, std::shared_ptr<CAsteroid>> cAsteroidIdMap;
        inline static std::unordered_map<uint, std::shared_ptr<CSolar>> cSolarIdMap;
        inline static std::unordered_map<uint, std::shared_ptr<CShip>> cShipIdMap;
        inline static std::unordered_map<uint, std::shared_ptr<CLoot>> cLootIdMap;
        inline static std::unordered_map<uint, std::shared_ptr<CCounterMeasure>> cCmIdMap;
        inline static std::unordered_map<uint, std::shared_ptr<CMine>> cMineIdMap;
        inline static std::unordered_map<uint, std::shared_ptr<CGuided>> cGuidedIdMap;

        inline static std::unordered_map<uint, iobjCache> cacheSolarIObjs;
        inline static std::unordered_map<uint, iobjCache> cacheNonsolarIObjs;

        inline static std::unordered_map<uint, pub::SpaceObj::ShipInfo> shipCreationParams;
        inline static std::unordered_map<uint, pub::SpaceObj::SolarInfo> solarCreationParams;
        inline static std::unordered_map<uint, pub::SpaceObj::LootInfo> lootCreationParams;

        static GameObject* FindNonSolar(StarSystemMock* starSystem, uint searchedId);
        static GameObject* FindSolar(StarSystemMock* starSystem, uint searchedId);
        static GameObject* __stdcall FindInStarList(StarSystemMock* starSystem, uint searchedId);
        static void GameObjectDestructorNaked();
        static void CSimpleInitNaked();
        static void CObjDestrOrgNaked();
        static void FindInStarListNaked();
        static void FindInStarListNaked2();
        static void __stdcall GameObjectDestructor(uint id);
        static CObject* CObjectFindDetour(const uint& spaceObjId, CObject::Class objClass);
        static CObject* CObjAllocDetour(CObject::Class objClass);
        static void __fastcall CObjDestr(CObject* cobj);
        static void __fastcall CSimpleInit(CSimple* casteroid, void* edx, const CSimple::CreateParms& param);
        static int SpaceObjCreateDetour(unsigned int& id, const pub::SpaceObj::ShipInfo& info);
        static int SpaceObjSolarCreateDetour(unsigned int& id, const pub::SpaceObj::SolarInfo& info);
        static int SpaceObjLootCreateDetour(unsigned int& id, const pub::SpaceObj::LootInfo& info);

        friend IEngineHook;

        struct NpcTemplate
        {
                std::wstring nickname;
                Id hash;
                std::wstring loadout;
                Id loadoutHash;
                std::wstring archetype;
                Id archetypeHash;
                std::wstring pilot;
                Id pilotHash;
                uint level;
        };

        inline static std::unordered_map<Id, NpcTemplate> npcTemplates;
        std::vector<std::pair<std::shared_ptr<CShip>, bool>> spawnedShips;
        std::vector<std::pair<std::shared_ptr<CSolar>, bool>> spawnedSolars;

        void OnShipDestroyed(Ship* ship);
        void OnSolarDestroyed(Solar* solar);
        static void SendSolarPacket(uint spaceId, pub::SpaceObj::SolarInfo& si);

        inline static std::unordered_map<uint, ClientId> npcToLastAttackingPlayerMap;
        inline static std::unordered_map<Plugin*, std::vector<Id>> spawnedIdsPerPlugin;
    public:

        struct DLL SpaceObjectBuilder
        {
                struct Fuse
                {
                        std::variant<std::wstring, uint> fuse;
                        float radius = 5.0f;
                        float lifetime = 5.0f;
                        short equipmentId = 0;
                };

                enum class StateGraph
                {
                    Nothing,
                    Player,
                    Fighter,
                    Transport,
                    Gunboat,
                    Cruiser
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
                SpaceObjectBuilder& WithArchetype(Id archetype);

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
                 * @brief Set the state graph parameter for the spawned solar/npc
                 */
                SpaceObjectBuilder& WithStateGraph(StateGraph stateGraph);

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
                SpaceObjectBuilder& WithSystem(SystemId systemHash);

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
                SpaceObjectBuilder& WithReputation(RepGroupId affiliation);

                /**
                 * @brief Specify that the spawned solar should have a dock target
                 * @param base The hash of the base that should be a dock target for the spawned solar
                 */
                SpaceObjectBuilder& WithDockTo(BaseId base);

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
                std::optional<Id> voiceOverride{};
                std::optional<const pub::AI::Personality*> personalityOverride{};
                std::optional<StateGraph> stateGraphOverride{};
                std::optional<Matrix> rotation{};
                std::optional<Vector> position{};
                std::optional<float> positionVariance{};
                std::optional<SystemId> system{};
                std::optional<float> health{};
                std::optional<RepGroupId> affiliation{};
                std::optional<std::pair<uint, uint>> name{};
                std::optional<BaseId> dockTo{};
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

        [[nodiscard]]
        SpaceObjectBuilder NewBuilder()
        {
            return SpaceObjectBuilder{ *this };
        }

        /**
         * @brief Returns an optional of the last ClientId to hit an NPC with a given id.
         * @param id The Id of the NPC ship.
         */
        std::optional<ClientId> GetLastAttackingPlayer(uint id);

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

        template <typename T>
            requires std::is_base_of_v<CObject, T> && !std::is_same_v<CBeam, T>
                     std::weak_ptr<T> Get(uint id) = delete;

        template <>
        std::weak_ptr<CGuided> Get(uint id);

        template <>
        std::weak_ptr<CAsteroid> Get(uint id);

        template <>
        std::weak_ptr<CSolar> Get(uint id);

        template <>
        std::weak_ptr<CShip> Get(uint id);

        template <>
        std::weak_ptr<CSimple> Get(uint id);

        template <>
        std::weak_ptr<CLoot> Get(uint id);

        template <>
        std::weak_ptr<CCounterMeasure> Get(uint id);

        template <>
        std::weak_ptr<CMine> Get(uint id);

        static const pub::SpaceObj::ShipInfo* LookupShipCreationInfo(uint id);
        static const pub::SpaceObj::SolarInfo* LookupSolarCreationInfo(uint id);
        static const pub::SpaceObj::LootInfo* LookupLootCreationInfo(uint id);

        struct SolarSpawnStruct
        {
                std::string pluginName;
                Id solarArchetypeId;
                Id loadoutArchetypeId;
                std::string nickname;
                uint solarIds;
                std::wstring nameOverride;
                Vector pos;
                Matrix ori;
                SystemId systemId;
                Id spaceObjId;
                SystemId destSystem;
                Id destObj;
                Id affiliation;
                float percentageHp = 1.0f;
        };

        static Id CreateShipSimple(SystemId system, const Vector& pos, Matrix& rot, Plugin* callingPlugin);
        static Id CreateSolarSimple(SolarSpawnStruct& solarSpawnData, Plugin* callingPlugin);
        static Id CreateLootSimple(SystemId system, const Vector& pos, Id commodity, uint amount, ShipId owner, bool canAITractor, Plugin* callingPlugin);
};
