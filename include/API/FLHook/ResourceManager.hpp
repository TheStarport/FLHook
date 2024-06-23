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

                SpaceObjectBuilder& WithNpc(const std::wstring& npcNickname);
                SpaceObjectBuilder& WithArchetype(const std::wstring& archetype);
                SpaceObjectBuilder& WithArchetype(uint archetype);
                SpaceObjectBuilder& WithLoadout(const std::wstring& loadout);
                SpaceObjectBuilder& WithLoadout(uint loadout);
                SpaceObjectBuilder& WithCostume(const Costume& costume);
                SpaceObjectBuilder& WithPersonality(const std::wstring& personalityNickname);
                SpaceObjectBuilder& WithPersonality(pub::AI::Personality& personality);
                SpaceObjectBuilder& WithPosition(const Vector& positionVector, float variance = 0.0f);
                SpaceObjectBuilder& WithRotation(const Vector& euler);
                SpaceObjectBuilder& WithRotation(const Matrix& orientation);
                SpaceObjectBuilder& WithSystem(uint systemHash);
                SpaceObjectBuilder& WithSystem(const std::wstring& systemNick);
                SpaceObjectBuilder& WithAbsoluteHealth(float absoluteHealth);
                SpaceObjectBuilder& WithRelativeHealth(float relativeHealth);
                SpaceObjectBuilder& WithLevel(uint level);
                SpaceObjectBuilder& WithVoice(const std::wstring& voice);
                SpaceObjectBuilder& WithName(uint firstName, uint secondName = 0);
                SpaceObjectBuilder& WithName(FmtStr& scannerName, FmtStr& pilotName);
                SpaceObjectBuilder& WithReputation(const std::wstring& rep);
                SpaceObjectBuilder& WithReputation(uint affiliation);
                SpaceObjectBuilder& WithDockTo(uint base);
                SpaceObjectBuilder& WithFuse(const Fuse& fuse);
                SpaceObjectBuilder& WithRandomNpc();
                SpaceObjectBuilder& WithRandomReputation();
                SpaceObjectBuilder& WithRandomName(Region region = Region::Random);

                /**
                 * @brief Ensures that the ship/solar will remain valid in memory, even after being destroyed.
                 * The only way it will be cleaned up and disposed of, is if it is explicitly destroyed through the ResourceManager.
                 * @note This should be considered very dangerous and should you not keep hold of pointers to delete it,
                 * it will lead to potentially quite large memory leaks and performance degredation.
                 */
                SpaceObjectBuilder& WithMemoryProtection();

                SpaceObjectBuilder& AsSolar();
                SpaceObjectBuilder& AsNpc();
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

                friend ResourceManager;
                std::weak_ptr<CShip> SpawnNpc();
                std::weak_ptr<CSolar> SpawnSolar();
                bool ValidateSpawn() const;
                SpaceObjectBuilder() = default;
        };

        ResourceManager();
        ~ResourceManager();

        // ReSharper disable once CppMemberFunctionMayBeStatic
        SpaceObjectBuilder NewBuilder() { return {}; }

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
