#pragma once

#include "ResourcePtr.hpp"

class DLL ResourceManager final
{
    public:
        struct SpawnedObject
        {
                CSimple* obj;
                uint spaceObj;
        };

    private:
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
        inline static std::vector<std::shared_ptr<SpawnedObject>> spawnedObjects;

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
                SpaceObjectBuilder& WithRandomName();

                SpaceObjectBuilder& AsSolar();
                SpaceObjectBuilder& AsNpc();
                ResourcePtr<SpawnedObject> Spawn();

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

                friend ResourceManager;
                std::weak_ptr<SpawnedObject> SpawnNpc();
                std::weak_ptr<SpawnedObject> SpawnSolar();
                bool ValidateSpawn() const;
                SpaceObjectBuilder() = default;
        };

        ResourceManager();
        ~ResourceManager();

        static SpaceObjectBuilder NewBuilder() { return {}; }

        void Destroy(ResourcePtr<SpawnedObject> object);
        void Despawn(ResourcePtr<SpawnedObject> object);
};
