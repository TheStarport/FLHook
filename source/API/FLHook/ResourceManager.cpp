#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/PersonalityHelper.hpp"
#include "API/FLHook/ResourceManager.hpp"

#include "API/InternalApi.hpp"
#include "API/Utils/Random.hpp"
#include "FLCore/Common/Globals.hpp"

template <typename T>
void NoOp(T*)
{}

void ResourceManager::SendSolarPacket(uint spaceId, pub::SpaceObj::SolarInfo& si)
{
    if (GameObject * inspect; inspect = FLHook::GetObjInspect(spaceId))
    {
        const auto* solar = reinterpret_cast<const CSolar*>(inspect->cobject());

        solar->launch_pos(si.pos, si.orientation, 1);

        struct SolarStruct
        {
                std::byte unknown[0x100];
        };

        SolarStruct solarPacket{};

        DWORD server = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));
        // ReSharper disable twice CppDFAUnusedValue
        // ReSharper disable twice CppDFAUnreadVariable
        const std::byte* address1 = reinterpret_cast<std::byte*>(server) + 0x163F0;
        const std::byte* address2 = reinterpret_cast<std::byte*>(server) + 0x27950;

        // fill struct
        // clang-format off
        __asm
        {
            lea ecx, solarPacket
            mov eax, address1
            call eax
            push solar
            lea ecx, solarPacket
            push ecx
            mov eax, address2
            call eax
            add esp, 8
        }

        // clang-format on

        // Send packet to every client in the system
        for (auto& client : FLHook::Clients())
        {
            if (client.playerData->systemId == si.systemId)
            {
                GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(client.playerData->clientId, reinterpret_cast<FLPACKET_CREATESOLAR&>(solarPacket));
            }
        }
    }
}

void ResourceManager::OnSolarDestroyed(Solar* solar)
{
    for (auto& [spawnedSolar, protectMemory] : spawnedSolars)
    {
        if (spawnedSolar->id.GetValue() != solar->get_id())
        {
            continue;
        }

        if (protectMemory)
        {
            return;
        }

        solar->csolar()->Release();
    }
}

void ResourceManager::OnShipDestroyed(Ship* ship)
{
    for (auto& [spawnedShip, protectMemory] : spawnedShips)
    {
        if (spawnedShip->id.GetValue() != ship->get_id())
        {
            continue;
        }

        if (protectMemory)
        {
            return;
        }

        ship->cship()->Release();
    }
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithNpc(const std::wstring& npcNickname)
{
    const auto found = npcTemplates.find(Id(npcNickname));

    if (found == npcTemplates.end())
    {
        WARN("invalid npc nickname: {{npcName}}", { "npcName", npcNickname });
        return *this;
    }

    isNpc = true;
    npcTemplate = found->second;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithArchetype(const std::wstring& archetype)
{
    npcTemplate.archetype = archetype;
    npcTemplate.archetypeHash = Id(archetype);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithArchetype(const Id archetype)
{
    npcTemplate.archetypeHash = archetype;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithLoadout(const std::wstring& loadout)
{
    npcTemplate.loadout = loadout;
    npcTemplate.loadoutHash = Id(loadout);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithLoadout(const uint loadout)
{
    npcTemplate.loadoutHash = Id(loadout);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithCostume(const Costume& costume)
{
    costumeOverride.emplace(costume);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithPersonality(const std::wstring& personalityNickname)
{
    if (const auto personality = FLHook::GetPersonality(personalityNickname).Raw(); personality.has_value())
    {
        personalityOverride = personality.value();
    }
    else
    {
        WARN("called with invalid personality {{personality}}", { "personality", personalityNickname });
    }

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithPersonality(pub::AI::Personality& personality)
{
    personalityOverride = &personality;
    return *this;
}
ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithStateGraph(StateGraph stateGraph)
{
    stateGraphOverride = stateGraph;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithPosition(const Vector& positionVector, float variance)
{
    position = positionVector;
    if (variance > 1.0f)
    {
        positionVariance = variance;
    }

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRotation(const Vector& euler)
{
    rotation = EulerMatrix(euler);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRotation(const Matrix& orientation)
{
    rotation = orientation;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithSystem(SystemId systemHash)
{
    if (systemHash)
    {
        system = systemHash;
    }
    else
    {
        WARN("Called with invalid nickname");
    }

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithSystem(const std::wstring& systemNick)
{
    if (auto id = Universe::get_system_id(StringUtils::wstos(systemNick).c_str()))
    {
        system = SystemId(id);
    }
    else
    {
        WARN("Called with invalid nickname");
    }

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithAbsoluteHealth(float absoluteHealth)
{
    healthRelative = false;
    health = absoluteHealth;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRelativeHealth(float relativeHealth)
{
    healthRelative = true;
    health = relativeHealth;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithLevel(const uint level)
{
    npcTemplate.level = level;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithVoice(const std::wstring& voice)
{
    voiceOverride = Id(voice);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithName(uint firstName, uint secondName)
{
    name = std::make_pair(firstName, secondName);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithName(FmtStr& scannerName, FmtStr& pilotName)
{
    names = { scannerName, pilotName };
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithReputation(const std::wstring& rep)
{
    this->affiliation = RepGroupId(rep);
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithReputation(RepGroupId affiliation)
{
    this->affiliation = affiliation;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithDockTo(BaseId base)
{
    dockTo = base;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithFuse(const Fuse& fuse)
{
    if (fuse.lifetime < 0.0f)
    {
        WARN("Called with negative lifetime.");
    }

    if (fuse.radius < 0.0f)
    {
        WARN("Called with negative radius.");
    }

    this->fuse = fuse;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRandomNpc()
{
    auto el = npcTemplates.begin();
    std::advance(el, Random::Uniform(0u, npcTemplates.size() - 1));

    npcTemplate = el->second;

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRandomReputation(const bool excludeStoryFactions)
{
    auto& repGroups = GameData::repGroups;

    std::string rep;
    while (rep.empty())
    {
        const uint randomGroupIndex = Random::Uniform(0u, repGroups.size() - 1);
        uint currentIndex = 0;
        for (auto listItem = repGroups.begin(); listItem != repGroups.end(); ++listItem, ++currentIndex)
        {
            if (currentIndex == randomGroupIndex)
            {
                if (!excludeStoryFactions || std::ranges::find(GameData::storyFactions, listItem.key()) == GameData::storyFactions.end())
                {
                    rep = std::string(listItem.value()->name);
                }

                break;
            }
        }
    }

    affiliation = RepGroupId(MakeId(rep.c_str()));
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRandomName(Region region)
{
    struct NameRange
    {
            std::pair<uint, uint> firstName;
            std::pair<uint, uint> lastName;
    };

    // clang-format off
    constexpr std::array<NameRange, 5> nameLists = {
        {
            // Bretonia
            {
                { 227308, 227575 },
                { 227708, 228007 }
            },
            // Hispania
            {
                { 229208, 229340 },
                { 229408, 229460 }
            },
            // Kusari
        {
            { 228708, 228890 },
            { 228908, 229207 }
            },
            // Liberty
            {
                    { 226608, 226952 },
                    { 227008, 227307 }
            },
            // Rheinland
            {
            { 228008, 228407 },
            { 228408, 228663 }
            },
        }
    };

    // clang-format on

    const auto& [firstName, lastName] =
        region == Region::Random ? nameLists[Random::Uniform(0u, nameLists.size() - 1)] : nameLists[static_cast<size_t>(region) - 1u];

    name = {
        Random::Uniform(firstName.first, firstName.second),
        Random::Uniform(lastName.first, lastName.second),
    };

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithMemoryProtection()
{
    protectMemory = true;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::AsSolar()
{
    isNpc = false;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::AsNpc()
{
    isNpc = true;
    return *this;
}

std::weak_ptr<CEqObj> ResourceManager::SpaceObjectBuilder::Spawn()
{
    if (!ValidateSpawn())
    {
        WARN("Attempting to spawn NPC/Solar with invalid data within the builder.");
        return {};
    }

    std::shared_ptr<CEqObj> obj = nullptr;
    if (isNpc)
    {
        const auto ship = SpawnNpc().lock();
        if (!ship)
        {
            return {};
        }

        obj = std::dynamic_pointer_cast<CEqObj>(ship);
    }
    else
    {
        const auto solar = SpawnSolar().lock();
        if (!solar)
        {
            return {};
        }

        obj = std::dynamic_pointer_cast<CEqObj>(solar);
    }

    if (fuse.has_value() && obj)
    {
        if (EqObj* inspect = nullptr; inspect = reinterpret_cast<EqObj*>(FLHook::GetObjInspect(obj->id)))
        {
            inspect->light_fuse(0,
                                fuse->fuse.index() == 0 ? Id(InternalApi::CreateID(std::get<std::wstring>(fuse->fuse))) : Id(std::get<uint>(fuse->fuse)),
                                fuse->equipmentId,
                                fuse->radius,
                                fuse->lifetime);
        }
    }

    return obj;
}

std::weak_ptr<CShip> ResourceManager::SpaceObjectBuilder::SpawnNpc()
{
    const auto shipArch = Archetype::GetShip(npcTemplate.archetypeHash.GetValue());

    pub::SpaceObj::ShipInfo si{};
    std::memset(&si, 0x0, sizeof(pub::SpaceObj::ShipInfo)); // NOLINT

    si.flag = 1;

    si.system = system.value();
    si.pos = position.value();
    if (positionVariance.has_value())
    {
        si.pos.x += Random::UniformFloat(-positionVariance.value(), positionVariance.value());
        si.pos.y += Random::UniformFloat(-positionVariance.value(), positionVariance.value());
        si.pos.z += Random::UniformFloat(-positionVariance.value(), positionVariance.value());
    }

    si.shipArchetype = npcTemplate.archetypeHash;
    si.orientation = rotation.value_or(Matrix::Identity());
    si.loadout = npcTemplate.loadoutHash.GetValue();

    if (costumeOverride.has_value())
    {
        auto costume = costumeOverride.value();
        si.look1 = costume.head;
        si.look2 = costume.body;
        if (costume.accessories == 1)
        {
            si.comm = costume.accessory[0];
        }
    }
    else
    {
        si.look1 = CreateID("li_newscaster_head_gen_hat"); // Head
        si.look2 = CreateID("pl_female1_journeyman_body"); // Body
        si.comm = CreateID("comm_br_darcy_female");        // Hat
    }

    si.pilotVoice = voiceOverride.value_or(Id("pilot_f_leg_f01a"));

    if (health.has_value())
    {
        if (healthRelative)
        {
            if (health.value() == 1.0f || health.value() <= 0.0f)
            {
                si.health = -1;
            }
            else
            {
                si.health = static_cast<uint>(shipArch->hitPoints / health.value());
            }
        }
        else
        {
            si.health = static_cast<uint>(health.value());
        }
    }
    else
    {
        si.health = -1;
    }

    si.level = npcTemplate.level;

    FmtStr scannerName(0, nullptr);
    scannerName.begin_mad_lib(0);
    scannerName.end_mad_lib();

    // Define the string used for the pilot name. The example
    // below shows the use of multiple part names.
    FmtStr pilotName(0, nullptr);
    pilotName.begin_mad_lib(16163); // ids of "%s0 %s1"
    if (names.has_value())
    {
        auto [scanner, pilot] = names.value();
        scannerName = scanner;
        pilotName = pilot;
    }
    else if (name.has_value())
    {
        const auto [firstName, secondName] = name.value();
        pilotName.append_string(firstName);
        scannerName.append_string(firstName);

        if (secondName != 0)
        {
            pilotName.append_string(secondName);
            scannerName.append_string(secondName);
        }
    }
    else
    {
        uint firstName = defaultNames[Random::Uniform(0u, defaultNames.size() - 1)];
        uint secondName = defaultNames[Random::Uniform(0u, defaultNames.size() - 1)];
        pilotName.append_string(firstName);  // ids that replaces %s0
        pilotName.append_string(secondName); // ids that replaces %s1
    }
    pilotName.end_mad_lib();

    pub::AI::SetPersonalityParams personalityParams;
    if (npcTemplate.pilotHash)
    {
        if (auto personality = FLHook::GetPersonality(npcTemplate.pilot).Unwrap(); personality)
        {
            personalityOverride.emplace(personality);
        }
    }

    if (personalityOverride.has_value())
    {
        personalityParams.personality = *personalityOverride.value();
    }

    personalityParams.stateGraph = pub::StateGraph::get_state_graph(
        StringUtils::wstos(magic_enum::enum_name(stateGraphOverride.value_or(StateGraph::Fighter))).c_str(), pub::StateGraph::TYPE_STANDARD);

    personalityParams.stateId = true;

    pub::Reputation::Alloc(si.rep, scannerName, pilotName);

    if (affiliation.has_value())
    {
        pub::Reputation::SetAffiliation(si.rep, affiliation.value().GetValue());
    }

    uint spaceObj = 0;
    pub::SpaceObj::Create(spaceObj, si);

    if (!spaceObj)
    {
        return {};
    }

    // Add the personality to the space obj
    pub::AI::SubmitState(spaceObj, &personalityParams);

    // Check that the spawn was actually successful, sometimes it can spawn correctly but instantly die
    auto ptr = FLHook::GetResourceManager()->Get<CShip>(spaceObj);
    if (ptr.expired())
    {
        return {};
    }

    resourceManager.spawnedShips.emplace_back(ptr.lock(), protectMemory);

    return ptr;
}

std::weak_ptr<CSolar> ResourceManager::SpaceObjectBuilder::SpawnSolar()
{
    const auto solarArch = Archetype::GetSolar(npcTemplate.archetypeHash.GetValue());

    pub::SpaceObj::SolarInfo si{};
    std::memset(&si, 0, sizeof(si));

    si.flag = 4;

    // Prepare the settings for the space object
    si.archId = npcTemplate.archetypeHash;
    si.loadoutId = npcTemplate.loadoutHash;
    si.hitPointsLeft = -1;
    si.systemId = system.value();
    si.orientation = rotation.value_or(Matrix::Identity());

    si.costume =
        costumeOverride.value_or(Costume(CreateID("benchmark_male_head"), CreateID("benchmark_male_body"))); // NOLINT(clang-diagnostic-c++20-extensions)
    si.voiceId = voiceOverride.value_or(Id("atc_leg_m01"));

    auto nickname = Random::UniformString<std::string>(32);
    strncpy_s(si.nickName, sizeof(si.nickName), nickname.c_str(), nickname.size());

    // Do we need to vary the starting position slightly? Useful when spawning multiple objects
    si.pos = position.value();
    if (positionVariance.has_value())
    {
        si.pos.x += Random::UniformFloat(-positionVariance.value(), positionVariance.value());
        si.pos.y += Random::UniformFloat(-positionVariance.value(), positionVariance.value());
        si.pos.z += Random::UniformFloat(-positionVariance.value(), positionVariance.value());
    }

    si.dockWith = dockTo.value_or(BaseId());

    // Define the string used for the solar name.
    FmtStr solarName(0, nullptr);

    if (name.has_value())
    {
        auto [firstName, secondName] = name.value();
        solarName.begin_mad_lib(firstName); // ids of "%s0 %s1"
        solarName.end_mad_lib();
    }

    FmtStr scannerName = solarName;

    // Set Reputation
    pub::Reputation::Alloc(si.rep, scannerName, solarName);

    if (affiliation.has_value())
    {
        pub::Reputation::SetAffiliation(si.rep, affiliation.value().GetValue());
    }

    // prevent the game from sending the solar creation packet (we need to do it ourselves)
    auto address = FLHook::Offset(FLHook::BinaryType::Server, AddressList::CreateSolar);
    constexpr byte bypassPacketSending = '\xEB';
    MemUtils::WriteProcMem(address, &bypassPacketSending, 1);

    // Spawn the solar object
    uint spaceId;
    CreateSolar(spaceId, si);

    // Send the packet ourselves
    SendSolarPacket(spaceId, si);

    // Restore
    constexpr byte bypassRemoval = '\x74';
    MemUtils::WriteProcMem(address, &bypassRemoval, 1);

    pub::AI::SetPersonalityParams personalityParams;
    if (npcTemplate.pilotHash)
    {
        if (auto personality = FLHook::GetPersonality(npcTemplate.pilot).Raw(); personality.has_value())
        {
            personalityOverride.emplace(personality.value());
        }
    }

    if (personalityOverride.has_value())
    {
        personalityParams.personality = *personalityOverride.value();
    }

    personalityParams.stateGraph = pub::StateGraph::get_state_graph("STATION", pub::StateGraph::TYPE_STANDARD);
    personalityParams.stateId = true;

    pub::AI::SubmitState(spaceId, &personalityParams);

    if (health.has_value())
    {
        if (healthRelative)
        {
            pub::SpaceObj::SetRelativeHealth(spaceId, health.value());
        }
        else
        {
            pub::SpaceObj::SetRelativeHealth(spaceId, health.value() / solarArch->hitPoints);
        }
    }
    else
    {
        pub::SpaceObj::SetRelativeHealth(spaceId, 1.0f);
    }

    auto ptr = FLHook::GetResourceManager()->Get<CSolar>(spaceId);
    if (ptr.expired())
    {
        return {};
    }

    resourceManager.spawnedSolars.emplace_back(ptr.lock(), protectMemory);

    return ptr;
}

bool ResourceManager::SpaceObjectBuilder::ValidateSpawn() const
{
    return (Archetype::GetSolar(npcTemplate.archetypeHash.GetValue()) || Archetype::GetShip(npcTemplate.archetypeHash.GetValue())) && system.has_value() &&
           npcTemplate.loadoutHash && Loadout::Get(npcTemplate.loadoutHash.GetValue());
}

std::optional<ClientId> ResourceManager::GetLastAttackingPlayer(uint id)
{
    auto iter = npcToLastAttackingPlayerMap.find(id);
    if (iter != npcToLastAttackingPlayerMap.end())
    {
        return {};
    }

    return { iter->second };
}

void ResourceManager::Destroy(std::weak_ptr<CEqObj> object, const bool instantly)
{
    const auto ptr = object.lock();
    if (!ptr)
    {
        return;
    }

    if (ptr->objectClass == CObject::Class::CSOLAR_OBJECT)
    {
        if (const auto erased = std::erase_if(spawnedSolars, [&ptr](std::pair<std::shared_ptr<CSolar>, bool>& solar) { return ptr->id == solar.first->id; });
            !erased)
        {
            DEBUG("Tried to dispose of CSolar that wasn't owned by resource manager");
            return;
        }
    }

    if (ptr->objectClass == CObject::CSHIP_OBJECT)
    {
        if (const auto erased = std::erase_if(spawnedShips, [&ptr](std::pair<std::shared_ptr<CShip>, bool>& ship) { return ptr->id == ship.first->id; });
            !erased)
        {
            DEBUG("Tried to dispose of CShip that wasn't owned by resource manager");
            return;
        }
    }

    if (instantly)
    {
        pub::SpaceObj::Destroy(ptr->id.GetValue(), DestroyType::Vanish);
    }
    else
    {
        ptr->isDead = true;
        pub::SpaceObj::SetRelativeHealth(ptr->id.GetValue(), 0.0f);
    }
}

void ResourceManager::Despawn(std::weak_ptr<CEqObj> object) { Destroy(object, true); }

template <>
std::weak_ptr<CGuided> ResourceManager::Get<CGuided>(const uint id)
{
    const auto obj = cGuidedIdMap.find(id);
    return obj != cGuidedIdMap.end() ? obj->second : nullptr;
}

template <>
std::weak_ptr<CAsteroid> ResourceManager::Get<CAsteroid>(const uint id)
{
    const auto obj = cAsteroidIdMap.find(id);
    return obj != cAsteroidIdMap.end() ? obj->second : nullptr;
}

template <>
std::weak_ptr<CSolar> ResourceManager::Get<CSolar>(const uint id)
{
    const auto obj = cSolarIdMap.find(id);
    return obj != cSolarIdMap.end() ? obj->second : nullptr;
}

template <>
std::weak_ptr<CShip> ResourceManager::Get<CShip>(const uint id)
{
    const auto obj = cShipIdMap.find(id);
    return obj != cShipIdMap.end() ? obj->second : nullptr;
}

template <>
std::weak_ptr<CLoot> ResourceManager::Get<CLoot>(const uint id)
{
    const auto obj = cLootIdMap.find(id);
    return obj != cLootIdMap.end() ? obj->second : nullptr;
}

template <>
std::weak_ptr<CCounterMeasure> ResourceManager::Get<CCounterMeasure>(const uint id)
{
    const auto obj = cCmIdMap.find(id);
    return obj != cCmIdMap.end() ? obj->second : nullptr;
}

template <>
std::weak_ptr<CMine> ResourceManager::Get<CMine>(const uint id)
{
    const auto obj = cMineIdMap.find(id);
    return obj != cMineIdMap.end() ? obj->second : nullptr;
}

const pub::SpaceObj::ShipInfo* ResourceManager::LookupShipCreationInfo(const uint id)
{
    const auto params = shipCreationParams.find(id);
    return params != shipCreationParams.end() ? &params->second : nullptr;
}

const pub::SpaceObj::SolarInfo* ResourceManager::LookupSolarCreationInfo(const uint id)
{
    const auto params = solarCreationParams.find(id);
    return params != solarCreationParams.end() ? &params->second : nullptr;
}

const pub::SpaceObj::LootInfo* ResourceManager::LookupLootCreationInfo(const uint id)
{
    const auto params = lootCreationParams.find(id);
    return params != lootCreationParams.end() ? &params->second : nullptr;
}

void SpawnSolar(unsigned int& spaceID, const pub::SpaceObj::SolarInfo& solarInfo)
{
    // hack server.dll so it does not call create solar packet send
    static DWORD skipSolarPacket = FLHook::Offset(FLHook::BinaryType::Server, AddressList::SkipCSolarPacketSend);
    char serverHack[] = { '\xEB' };
    MemUtils::WriteProcMem(skipSolarPacket, &serverHack, 1);

    pub::SpaceObj::CreateSolar(spaceID, solarInfo);

    if (GameObject* inspect = FLHook::GetObjInspect(spaceID); inspect)
    {
        CObject* solar = inspect->cobject();

        // for every player in the same system, send solar creation packet
        struct SOLAR_STRUCT
        {
                byte dunno[0x100];
        };

        SOLAR_STRUCT packetSolar;

        static DWORD address1 = FLHook::Offset(FLHook::BinaryType::Server, AddressList::SkipCSolarPacket1);
        static DWORD address2 = FLHook::Offset(FLHook::BinaryType::Server, AddressList::SkipCSolarPacket2);

        // fill struct
        __asm
        {
			pushad
			lea ecx, packetSolar
			mov eax, address1
			call eax
			push solar
			lea ecx, packetSolar
			push ecx
			mov eax, address2
			call eax
			add esp, 8
			popad
        }

        struct PlayerData* pPD = 0;
        while (pPD = Players.traverse_active(pPD))
        {
            if (pPD->systemId == solarInfo.systemId)
            {
                GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(pPD->clientId, (FLPACKET_CREATESOLAR&)packetSolar);
            }
        }
    }

    // undo the server.dll hack
    char serverUnHack[] = { '\x74' };
    MemUtils::WriteProcMem(skipSolarPacket, &serverUnHack, 1);
}

Id ResourceManager::CreateShipSimple(SystemId system, const Vector& pos, Matrix& rot, Plugin* callingPlugin)
{

    pub::SpaceObj::ShipInfo si;
    memset(&si, 0, sizeof(si));

    si.flag = 4;
    si.system = system;
    si.pos = pos;
    si.orientation = rot;

    // TODO: add the rest of crucial parameters;

    return Id();
}

Id ResourceManager::CreateSolarSimple(SolarSpawnStruct& solarSpawnData, Plugin* plugin)
{
    solarSpawnData.spaceObjId = Id(solarSpawnData.nickname);
    if (auto object = FLHook::GetObjInspect(solarSpawnData.spaceObjId); object)
    {
        ERROR("Attempting to spawn an already existing object {{solarName}}", { "solarName", solarSpawnData.nickname });
        return Id();
    }

    pub::SpaceObj::SolarInfo si;
    memset(&si, 0, sizeof(si));
    si.flag = 4;
    si.archId = solarSpawnData.solarArchetypeId;
    si.loadoutId = solarSpawnData.loadoutArchetypeId;

    si.hitPointsLeft = -1;
    si.systemId = solarSpawnData.systemId;
    si.orientation = solarSpawnData.ori;
    si.pos = solarSpawnData.pos;
    si.costume.head = CreateID("pi_pirate2_head");
    si.costume.body = CreateID("pi_pirate8_body");
    si.costume.leftHand = 0;
    si.costume.rightHand = 0;
    si.costume.accessories = 0;
    si.voiceId = Id("atc_leg_m01");
    // TODO: something about this. Disco smuggles in destination system to a custom spawned Jump Object via otherwise unused DockWith field.
    //  Handled via clienthook. Will probably switch to a FLUF based solution?
    // si.dockWith = solarSpawnData.destSystem;
    strncpy_s(si.nickName, sizeof(si.nickName), solarSpawnData.nickname.c_str(), solarSpawnData.nickname.size());

    if (solarSpawnData.solarIds && !solarSpawnData.nameOverride.empty())
    {
        struct PlayerData* pd = nullptr;
        while (pd = Players.traverse_active(pd))
        {
            if (pd->systemId == solarSpawnData.systemId)
            {
                // TODO: Send infocard override
                // HkChangeIDSString(pd->onlineId, solarSpawnData.solarIds, solarSpawnData.nameOverride);
            }
        }
    }
    // Set the base name
    FmtStr infoname(solarSpawnData.solarIds, 0);
    infoname.begin_mad_lib(solarSpawnData.solarIds); // scanner name
    infoname.end_mad_lib();

    FmtStr infocard(solarSpawnData.solarIds, 0);
    infocard.begin_mad_lib(solarSpawnData.solarIds); // infocard
    infocard.end_mad_lib();

    pub::Reputation::Alloc(si.rep, infoname, infocard);
    pub::Reputation::SetAffiliation(si.rep, solarSpawnData.affiliation.GetValue());

    uint spaceObjId;

    SpawnSolar(spaceObjId, si);

    pub::AI::SetPersonalityParams pers = PersonalityHelper::MakePersonality();
    pub::AI::SubmitState(spaceObjId, &pers);

    solarSpawnData.spaceObjId = Id(spaceObjId);
    if (solarSpawnData.percentageHp != 1.0f)
    {
        pub::SpaceObj::SetRelativeHealth(spaceObjId, solarSpawnData.percentageHp);
    }

    if (!solarSpawnData.destObj || !solarSpawnData.destSystem)
    {
        return Id(spaceObjId);
    }

    if (plugin)
    {
        spawnedIdsPerPlugin[plugin].emplace_back(spaceObjId);
    }

    // TODO: Add Handling for gate objects
    // TODO: Create a per-plugin list of solars to dispose of when unloading the plugin

    // uint type;
    // pub::SpaceObj::GetType(spaceObjId, type);
    // if (type & (ObjectType::JumpGate | ObjectType::JumpHole))
    //{
    //     HyperJump::InitJumpHole(spaceObjId, solarSpawnData.destSystem, solarSpawnData.destObj);
    // }

    return Id(spaceObjId);
}

Id ResourceManager::CreateLootSimple(SystemId system, const Vector& pos, Id commodity, uint amount, ShipId owner, bool canAITractor, Plugin* plugin)
{

    pub::SpaceObj::LootInfo lootInfo;
    lootInfo.systemId = system;
    lootInfo.ownerId = owner.GetId().Unwrap().GetValue();
    lootInfo.equipmentArchId = commodity;
    lootInfo.itemCount = amount;
    lootInfo.pos = pos;
    lootInfo.rot = Random::RandomMatrix();
    lootInfo.isMissionLoot = false;
    lootInfo.canAITractor = canAITractor;
    lootInfo.hitPtsPercentage = 1.0f;
    lootInfo.linearVelocity = { 0, 0, 0 };
    lootInfo.angularVelocity = { 0, 0, 0 };
    lootInfo.infocardOverride = Id();

    uint spaceObjId = 0;
    if (0 == pub::SpaceObj::CreateLoot(spaceObjId, lootInfo))
    {
        return Id(spaceObjId);
    }

    return Id();
}

template <>
std::weak_ptr<CSimple> ResourceManager::Get<CSimple>(const uint id)
{
    // Solar or Asteroid
    if (id & 0x80000000)
    {
        if (auto solar = Get<CSolar>(id); !solar.expired())
        {
            return solar;
        }

        if (auto asteroid = Get<CAsteroid>(id); !asteroid.expired())
        {
            return asteroid;
        }

        return {};
    }

    if (auto ship = Get<CShip>(id); !ship.expired())
    {
        return ship;
    }

    if (auto loot = Get<CLoot>(id); !loot.expired())
    {
        return loot;
    }

    if (auto guided = Get<CGuided>(id); !guided.expired())
    {
        return guided;
    }

    if (auto mine = Get<CMine>(id); !mine.expired())
    {
        return mine;
    }

    if (auto cm = Get<CCounterMeasure>(id); !cm.expired())
    {
        return cm;
    }

    return {};
}

ResourceManager::ResourceManager()
{
    if (gameObjectDestructorDetour)
    {
        throw GameException(L"Resource manager should not be constructed multiple times.");
    }

    // Setup detours for aid in resource tracking and server optimization
    gameObjectDestructorDetour = std::make_unique<FunctionDetour<DefaultNakedType>>(reinterpret_cast<DefaultNakedType>(0x6CEE4A0));
    gameObjectDestructorDetour->Detour(GameObjectDestructorNaked);

    cSimpleInitDetour = std::make_unique<FunctionDetour<DefaultNakedType>>(reinterpret_cast<DefaultNakedType>(0x62B5B60));
    cSimpleInitDetour->Detour(CSimpleInitNaked);

    cObjDestrDetour = std::make_unique<FunctionDetour<DefaultNakedType>>(reinterpret_cast<DefaultNakedType>(0x62AF440));
    cObjDestrDetour->Detour(CObjDestrOrgNaked);

    spaceObjCreateDetour.Detour(SpaceObjCreateDetour);
    spaceObjCreateLootDetour.Detour(SpaceObjLootCreateDetour);
    spaceObjCreateSolarDetour.Detour(SpaceObjSolarCreateDetour);

    std::array<byte, 2> patchCobjDestr = { 0xEB, 0x5F };
    MemUtils::WriteProcMem(FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonCObjDestructor), patchCobjDestr.data(), patchCobjDestr.size());

    auto cobjectFindDetourFunc = reinterpret_cast<FARPROC>(&CObjectFindDetour);
    MemUtils::WriteProcMem(FLHook::Offset(FLHook::BinaryType::Server, AddressList::CObjectFind), &cobjectFindDetourFunc, 4);

    cObjAllocatorDetour = std::make_unique<FunctionDetour<CObjAllocatorType>>(
        reinterpret_cast<CObjAllocatorType>(FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonCObjectAllocator)));
    cObjAllocatorDetour->Detour(CObjAllocDetour);

    auto findStarListNaked = reinterpret_cast<FARPROC>(&FindInStarListNaked2);
    MemUtils::WriteProcMem(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetObjectInspect1), &findStarListNaked, 4);

    DWORD serverDll = FLHook::Offset(FLHook::BinaryType::Server, static_cast<AddressList>(0));
    MemUtils::PatchCallAddr(serverDll, static_cast<DWORD>(AddressList::GetObjectInspect2), FindInStarListNaked);
    MemUtils::PatchCallAddr(serverDll, static_cast<DWORD>(AddressList::GetObjectInspect3), FindInStarListNaked);

    // Now we load all our NPCs for spawning
    npcTemplates.clear();

    for (const auto& entry : std::filesystem::recursive_directory_iterator("../DATA/MISSIONS/"))
    {
        std::string path = entry.path().generic_string();
        if (!entry.is_regular_file() || entry.file_size() > UINT_MAX || path.find("npcships") == std::string::npos)
        {
            continue;
        }

        INI_Reader ini;
        if (!ini.open(path.c_str(), false))
        {
            continue;
        }

        while (ini.read_header())
        {
            if (!ini.is_header("NPCShipArch"))
            {
                continue;
            }

            NpcTemplate npc{};
            while (ini.read_value())
            {
                if (ini.is_value("nickname"))
                {
                    std::string_view nickname = ini.get_value_string();
                    npc.nickname = StringUtils::stows(ini.get_value_string());
                    npc.hash = Id(nickname);
                }
                else if (ini.is_value("loadout"))
                {
                    std::string_view loadout = ini.get_value_string();
                    npc.loadout = StringUtils::stows(ini.get_value_string());
                    npc.loadoutHash = Id(loadout);
                }
                else if (ini.is_value("pilot"))
                {
                    std::string_view pilot = ini.get_value_string();
                    npc.pilot = StringUtils::stows(pilot);
                    npc.pilotHash = Id(pilot);
                }
                else if (ini.is_value("ship_archetype"))
                {
                    std::string_view archetype = ini.get_value_string();
                    npc.archetype = StringUtils::stows(ini.get_value_string());
                    npc.archetypeHash = Id(archetype);
                }
                else if (ini.is_value("level"))
                {
                    std::string level = ini.get_value_string();
                    if (level.length() < 2)
                    {
                        continue;
                    }

                    level.erase(0, 1);
                    npc.level = StringUtils::Cast<uint>(level);
                }
            }

            if (!npc.archetypeHash || !npc.pilotHash || !npc.loadoutHash || !npc.hash || !npc.level)
            {
                continue;
            }

            npcTemplates[npc.hash] = npc;
        }
    }
}

ResourceManager::~ResourceManager()
{
    for (const auto& solar : spawnedSolars | std::views::keys)
    {
        pub::SpaceObj::Destroy(solar->id.GetValue(), DestroyType::Vanish);
        solar->Release();
    }

    for (const auto& ship : spawnedShips | std::views::keys)
    {
        pub::SpaceObj::Destroy(ship->id.GetValue(), DestroyType::Vanish);
        ship->Release();
    }
}

GameObject* ResourceManager::FindNonSolar(StarSystemMock* starSystem, const uint searchedId)
{
    const MetaListNode* node = findIObjOnListFunc(starSystem->starSystem.shipList, searchedId);
    if (node)
    {
        cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    node = findIObjOnListFunc(starSystem->starSystem.lootList, searchedId);
    if (node)
    {
        cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    node = findIObjOnListFunc(starSystem->starSystem.guidedList, searchedId);
    if (node)
    {
        cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    node = findIObjOnListFunc(starSystem->starSystem.mineList, searchedId);
    if (node)
    {
        cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    node = findIObjOnListFunc(starSystem->starSystem.counterMeasureList, searchedId);
    if (node)
    {
        cacheNonsolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    return nullptr;
}

GameObject* ResourceManager::FindSolar(StarSystemMock* starSystem, const uint searchedId)
{
    const MetaListNode* node = findIObjOnListFunc(starSystem->starSystem.solarList, searchedId);
    if (node)
    {
        cacheSolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    node = findIObjOnListFunc(starSystem->starSystem.asteroidList, searchedId);
    if (node)
    {
        cacheSolarIObjs[searchedId] = { node->value->starSystem, node->value->cobj->objectClass };
        return node->value;
    }
    return nullptr;
}

GameObject* __stdcall ResourceManager::FindInStarList(StarSystemMock* starSystem, const uint searchedId)
{
    static StarSystem* lastFoundInSystem = nullptr;
    static uint lastFoundItem = 0;
    GameObject* retVal = nullptr;

    if (searchedId == 0)
    {
        return nullptr;
    }

    if (lastFoundItem == searchedId && lastFoundInSystem != &starSystem->starSystem)
    {
        return nullptr;
    }

    if (searchedId & 0x80000000) // check if solar
    {
        const auto iter = cacheSolarIObjs.find(searchedId);
        if (iter == cacheSolarIObjs.end())
        {
            return FindSolar(starSystem, searchedId);
        }

        if (iter->second.cacheStarSystem != &starSystem->starSystem)
        {
            lastFoundItem = searchedId;
            lastFoundInSystem = iter->second.cacheStarSystem;
            return nullptr;
        }

        MetaListNode* node;
        switch (iter->second.objClass)
        {
            case CObject::Class::CSOLAR_OBJECT:
                node = findIObjOnListFunc(starSystem->starSystem.solarList, searchedId);
                if (node)
                {
                    retVal = node->value;
                }
                break;
            case CObject::Class::CASTEROID_OBJECT:
                node = findIObjOnListFunc(starSystem->starSystem.asteroidList, searchedId);
                if (node)
                {
                    retVal = node->value;
                }
                break;
            default:;
        }

        cacheSolarIObjs.erase(iter);
    }
    else
    {
        if (!playerShips.contains(searchedId)) // player can swap systems, for them search just the system's shiplist
        {
            const auto iter = cacheNonsolarIObjs.find(searchedId);
            if (iter == cacheNonsolarIObjs.end())
            {
                return FindNonSolar(starSystem, searchedId);
            }

            if (iter->second.cacheStarSystem != &starSystem->starSystem)
            {
                lastFoundItem = searchedId;
                lastFoundInSystem = iter->second.cacheStarSystem;
                return nullptr;
            }

            MetaListNode* node;
            switch (iter->second.objClass)
            {
                case CObject::Class::CSHIP_OBJECT:
                    node = findIObjOnListFunc(starSystem->starSystem.shipList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                case CObject::Class::CLOOT_OBJECT:
                    node = findIObjOnListFunc(starSystem->starSystem.lootList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                case CObject::Class::CGUIDED_OBJECT:
                    node = findIObjOnListFunc(starSystem->starSystem.guidedList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                case CObject::Class::CMINE_OBJECT:
                    node = findIObjOnListFunc(starSystem->starSystem.mineList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                case CObject::Class::CCOUNTERMEASURE_OBJECT:
                    node = findIObjOnListFunc(starSystem->starSystem.counterMeasureList, searchedId);
                    if (node)
                    {
                        retVal = node->value;
                    }
                    break;
                default:;
            }

            cacheNonsolarIObjs.erase(iter);
        }
        else
        {
            if (const MetaListNode* node = findIObjOnListFunc(starSystem->starSystem.shipList, searchedId))
            {
                retVal = node->value;
            }
        }
    }

    return retVal;
}

__declspec(naked) void ResourceManager::FindInStarListNaked()
{
    __asm
        {
        push ecx
        push[esp + 0x8]
        sub ecx, 4
        push ecx
        call FindInStarList
        pop ecx
        ret 0x4
        }
}

__declspec(naked) void ResourceManager::FindInStarListNaked2()
{
    __asm
        {
        mov eax, [esp+0x4]
        mov edx, [eax]
         mov [esp+0x4], edx
        push ecx
        push[esp + 0x8]
        sub ecx, 4
        push ecx
        call FindInStarList
        pop ecx
        ret 0x4
        }
}

void __stdcall ResourceManager::GameObjectDestructor(const uint id)
{
    if (id & 0x80000000)
    {
        cacheSolarIObjs.erase(id);
    }
    else
    {
        cacheNonsolarIObjs.erase(id);
    }
}

uint GameObjectDestructorRet = 0x6CEE4A7;
__declspec(naked) void ResourceManager::GameObjectDestructorNaked()
{
    __asm {
        push ecx
        mov ecx, [ecx+0x4]
        mov ecx, [ecx+0xB0]
        push ecx
        call GameObjectDestructor
        pop ecx
        push 0xFFFFFFFF
        push 0x6d60776
        jmp GameObjectDestructorRet
    }
}

uint CObjAllocJmp = 0x62AEE55;
__declspec(naked) CObject* __cdecl CObjAllocCallOrig(CObject::Class objClass)
{
    __asm {
        push ecx
        mov eax, [esp + 8]
        jmp CObjAllocJmp
    }
}

CObject* __cdecl ResourceManager::CObjAllocDetour(const CObject::Class objClass)
{
    CObject* retVal = CObjAllocCallOrig(objClass);
    const CObjList* cobjList = CObjListFind(objClass);

    switch (objClass)
    {
        case CObject::CASTEROID_OBJECT: cAsteroidMap[retVal] = cobjList->entry->last; break;
        case CObject::CEQUIPMENT_OBJECT: cEquipmentMap[retVal] = cobjList->entry->last; break;
        case CObject::COBJECT_MASK: cObjectMap[retVal] = cobjList->entry->last; break;
        case CObject::CSOLAR_OBJECT: cSolarMap[retVal] = cobjList->entry->last; break;
        case CObject::CSHIP_OBJECT: cShipMap[retVal] = cobjList->entry->last; break;
        case CObject::CLOOT_OBJECT: cLootMap[retVal] = cobjList->entry->last; break;
        case CObject::CBEAM_OBJECT: cBeamMap[retVal] = cobjList->entry->last; break;
        case CObject::CGUIDED_OBJECT: cGuidedMap[retVal] = cobjList->entry->last; break;
        case CObject::CCOUNTERMEASURE_OBJECT: cCmMap[retVal] = cobjList->entry->last; break;
        case CObject::CMINE_OBJECT: cMineMap[retVal] = cobjList->entry->last; break;
        default: return nullptr;
    }

    return retVal;
}

void __fastcall ResourceManager::CSimpleInit(CSimple* simple, void* edx, const CSimple::CreateParms& param)
{
    switch (simple->objectClass)
    {
        case CObject::CASTEROID_OBJECT: cAsteroidIdMap[param.id] = std::shared_ptr<CAsteroid>(reinterpret_cast<CAsteroid*>(simple), &NoOp<CAsteroid>); break;
        case CObject::CSOLAR_OBJECT: cSolarIdMap[param.id] = std::shared_ptr<CSolar>(reinterpret_cast<CSolar*>(simple), &NoOp<CSolar>); break;
        case CObject::CSHIP_OBJECT: cShipIdMap[param.id] = std::shared_ptr<CShip>(reinterpret_cast<CShip*>(simple), &NoOp<CShip>); break;
        case CObject::CLOOT_OBJECT: cLootIdMap[param.id] = std::shared_ptr<CLoot>(reinterpret_cast<CLoot*>(simple), &NoOp<CLoot>); break;
        case CObject::CGUIDED_OBJECT: cGuidedIdMap[param.id] = std::shared_ptr<CGuided>(reinterpret_cast<CGuided*>(simple), &NoOp<CGuided>); break;
        case CObject::CCOUNTERMEASURE_OBJECT:
            cCmIdMap[param.id] = std::shared_ptr<CCounterMeasure>(reinterpret_cast<CCounterMeasure*>(simple), &NoOp<CCounterMeasure>);
            break;
        case CObject::CMINE_OBJECT: cMineIdMap[param.id] = std::shared_ptr<CMine>(reinterpret_cast<CMine*>(simple), &NoOp<CMine>); break;
        default: return;
    }
}

int ResourceManager::SpaceObjCreateDetour(unsigned int& id, const pub::SpaceObj::ShipInfo& info)
{
    spaceObjCreateDetour.UnDetour();
    const auto ret = spaceObjCreateDetour.GetOriginalFunc()(id, info);
    spaceObjCreateDetour.Detour(SpaceObjCreateDetour);

    shipCreationParams[id] = info;
    return ret;
}

int ResourceManager::SpaceObjSolarCreateDetour(unsigned int& id, const pub::SpaceObj::SolarInfo& info)
{
    spaceObjCreateSolarDetour.UnDetour();
    const auto ret = spaceObjCreateSolarDetour.GetOriginalFunc()(id, info);
    spaceObjCreateSolarDetour.Detour(SpaceObjSolarCreateDetour);

    solarCreationParams[id] = info;
    return ret;
}

int ResourceManager::SpaceObjLootCreateDetour(unsigned int& id, const pub::SpaceObj::LootInfo& info)
{
    spaceObjCreateLootDetour.UnDetour();
    const auto ret = spaceObjCreateLootDetour.GetOriginalFunc()(id, info);
    spaceObjCreateLootDetour.Detour(SpaceObjLootCreateDetour);

    lootCreationParams[id] = info;
    return ret;
}

constexpr uint CSimpleRetAddr = 0x62B5B66;
__declspec(naked) void ResourceManager::CSimpleInitNaked()
{
    __asm {
        push ecx
        push [esp+0x8]
        call CSimpleInit
        pop ecx
        push ebx
        push ebp
        mov ebp, [esp+0xC]
        jmp CSimpleRetAddr
    }
}

void __fastcall ResourceManager::CObjDestr(CObject* cobj)
{
    const auto simpleCast = reinterpret_cast<CSimple*>(cobj);
    std::unordered_map<CObject*, CObjNode*>* cobjMap;
    switch (cobj->objectClass)
    {
        case CObject::CASTEROID_OBJECT:
            cAsteroidIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cAsteroidMap;
            break;
        case CObject::CEQUIPMENT_OBJECT: cobjMap = &cEquipmentMap; break;
        case CObject::COBJECT_MASK: cobjMap = &cObjectMap; break;
        case CObject::CSOLAR_OBJECT:
            solarCreationParams.erase(simpleCast->id.GetValue());
            cSolarIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cSolarMap;
            break;
        case CObject::CSHIP_OBJECT:
            shipCreationParams.erase(simpleCast->id.GetValue());
            cShipIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cShipMap;
            break;
        case CObject::CLOOT_OBJECT:
            lootCreationParams.erase(simpleCast->id.GetValue());
            cLootIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cLootMap;
            break;
        case CObject::CBEAM_OBJECT: cobjMap = &cBeamMap; break;
        case CObject::CGUIDED_OBJECT:
            cGuidedIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cGuidedMap;
            break;
        case CObject::CCOUNTERMEASURE_OBJECT:
            cCmIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cCmMap;
            break;
        case CObject::CMINE_OBJECT:
            cMineIdMap.erase(simpleCast->id.GetValue());
            cobjMap = &cMineMap;
            break;
        default: return; // will never be hit, but shuts up the InteliSense
    }

    if (const auto item = cobjMap->find(cobj); item != cobjMap->end())
    {
        CObjList* cobjList = CObjListFind(cobj->objectClass);
        static uint dummy;
        removeCObjNode(cobjList, &dummy, item->second);
        cobjMap->erase(item);
    }
}

constexpr uint CObjDestrRetAddr = 0x62AF447;
__declspec(naked) void ResourceManager::CObjDestrOrgNaked()
{
    __asm {
        push ecx
        call CObjDestr
        pop ecx
        push 0xFFFFFFFF
        push 0x06394364
        jmp CObjDestrRetAddr
    }
}

CObject* __cdecl ResourceManager::CObjectFindDetour(const uint& spaceObjId, const CObject::Class objClass)
{
    const auto manager = FLHook::GetResourceManager();
    switch (objClass)
    {
        case CObject::CASTEROID_OBJECT:
            if (const auto obj = manager->Get<CAsteroid>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CSIMPLE_MASK:
            if (const auto obj = manager->Get<CSimple>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CLOOT_OBJECT:
            if (const auto obj = manager->Get<CLoot>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CMINE_OBJECT:
            if (const auto obj = manager->Get<CMine>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CCOUNTERMEASURE_OBJECT:
            if (const auto obj = manager->Get<CCounterMeasure>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CGUIDED_OBJECT:
            if (const auto obj = manager->Get<CGuided>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CSOLAR_OBJECT:
            if (const auto obj = manager->Get<CSolar>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        case CObject::CSHIP_OBJECT:
            if (const auto obj = manager->Get<CShip>(spaceObjId); !obj.expired())
            {
                const auto ptr = obj.lock().get();
                ptr->AddRef();
                return ptr;
            }
            break;
        default: return CObject::Find(spaceObjId, objClass);
    }

    return nullptr;
}
