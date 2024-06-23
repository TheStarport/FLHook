#include "PCH.hpp"

#include "API/FLHook/PersonalityHelper.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/FLHook/ClientList.hpp"

#include "API/InternalApi.hpp"
#include "API/Utils/Random.hpp"

template<typename T>
void NoOp(T*) {}

void ResourceManager::SendSolarPacket(uint spaceId, pub::SpaceObj::SolarInfo& si)
{
    if (IObjInspectImpl * inspect; FLHook::GetObjInspect(spaceId, inspect))
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
        if (spawnedSolar->id != solar->get_id())
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
        if (spawnedShip->id != ship->get_id())
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
    const auto found = npcTemplates.find(InternalApi::CreateID(npcNickname.c_str()));

    if (found == npcTemplates.end())
    {
        Logger::Warn(std::format(L"WithNpc called with invalid npc nickname: {}", npcNickname));
        return *this;
    }

    isNpc = true;
    npcTemplate = found->second;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithArchetype(const std::wstring& archetype)
{
    npcTemplate.archetype = archetype;
    npcTemplate.archetypeHash = InternalApi::CreateID(archetype.c_str());
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithArchetype(const uint archetype)
{
    npcTemplate.archetypeHash = archetype;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithLoadout(const std::wstring& loadout)
{
    npcTemplate.loadout = loadout;
    npcTemplate.loadoutHash = InternalApi::CreateID(loadout.c_str());
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithLoadout(uint loadout)
{
    npcTemplate.loadoutHash = loadout;
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
        Logger::Warn(std::format(L"WithPersonality called with an invalid personality: {}", personalityNickname));
    }

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithPersonality(pub::AI::Personality& personality)
{
    personalityOverride = &personality;
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

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithSystem(uint systemHash)
{
    if (Universe::get_system(systemHash))
    {
        system = systemHash;
    }
    else
    {
        Logger::Warn(L"WithSystem called with invalid hash");
    }

    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithSystem(const std::wstring& systemNick)
{
    if (auto id = Universe::get_system_id(StringUtils::wstos(systemNick).c_str()))
    {
        system = id;
    }
    else
    {
        Logger::Warn(L"WithSystem called with invalid nickname");
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
    voiceOverride = InternalApi::CreateID(voice.c_str());
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
    reputation = rep;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithReputation(uint affiliation)
{
    this->affiliation = affiliation;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithDockTo(uint base)
{
    dockTo = base;
    return *this;
}

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithFuse(const Fuse& fuse)
{
    if (fuse.lifetime < 0.0f)
    {
        Logger::Warn(L"WithFuse called with negative lifetime.");
    }

    if (fuse.radius < 0.0f)
    {
        Logger::Warn(L"WithFuse called with negative radius.");
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

ResourceManager::SpaceObjectBuilder& ResourceManager::SpaceObjectBuilder::WithRandomReputation()
{
    auto* groups = reinterpret_cast<FlMap<uint, Reputation::RepGroup>*>(0x64018EC);

    const uint randomGroupIndex = Random::Uniform(0u, groups->size() - 1);
    uint currentIndex = 0;
    for (auto listItem = groups->begin(); listItem != groups->end(); ++listItem, ++currentIndex)
    {
        if (currentIndex == randomGroupIndex)
        {
            reputation = StringUtils::stows(std::string_view(listItem.value()->name));
            break;
        }
    }

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
        Logger::Warn(L"Attempting to spawn NPC/Solar with invalid data within the builder.");
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
        if (EqObj* inspect = nullptr; FLHook::GetObjInspect(obj->id, reinterpret_cast<IObjInspectImpl*&>(inspect)))
        {
            inspect->light_fuse(
                0,
                ID_String{ fuse->fuse.index() == 0 ? InternalApi::CreateID(std::get<std::wstring>(fuse->fuse)) : std::get<uint>(fuse->fuse) },
                fuse->equipmentId,
                fuse->radius,
                fuse->lifetime);
        }
    }

    return obj;
}

std::weak_ptr<CShip> ResourceManager::SpaceObjectBuilder::SpawnNpc()
{
    const auto shipArch = Archetype::GetShip(npcTemplate.archetypeHash);

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
    si.loadout = npcTemplate.loadoutHash;

    if (costumeOverride.has_value())
    {
        auto costume = costumeOverride.value();
        si.look1 = costume.head;
        si.look2 = costume.body;
        si.comm = costume.accessory[0];
    }
    else
    {
        si.look1 = CreateID("li_newscaster_head_gen_hat");
        si.look2 = CreateID("pl_female1_journeyman_body");
        si.comm = CreateID("comm_br_darcy_female");
    }

    si.pilotVoice = voiceOverride.value_or(CreateID("pilot_f_leg_f01a"));

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

        if (secondName != 0)
        {
            pilotName.append_string(secondName);
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

    personalityParams.stateGraph = pub::StateGraph::get_state_graph("FIGHTER", pub::StateGraph::TYPE_STANDARD);
    personalityParams.stateId = true;

    pub::Reputation::Alloc(si.rep, scannerName, pilotName);
    if (affiliation.has_value())
    {
        pub::Reputation::SetAffiliation(si.rep, affiliation.value());
    }
    else if (reputation.has_value())
    {
        uint r;
        pub::Reputation::GetReputationGroup(r, StringUtils::wstos(reputation.value()).c_str());
        if (r)
        {
            pub::Reputation::SetAffiliation(si.rep, r);
        }
    }

    uint spaceObj = 0;
    pub::SpaceObj::Create(spaceObj, si);

    if (!spaceObj)
    {
        return {};
    }

    // Add the personality to the space obj
    pub::AI::SubmitState(spaceObj, &personalityParams);

    auto ptr = static_cast<CShip*>(CObject::Find(spaceObj, CObject::Class::CSHIP_OBJECT)); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
    if (!ptr)
    {
        return {};
    }

    std::shared_ptr<CShip> shared{ptr, &NoOp<CShip>};
    FLHook::GetResourceManager().spawnedShips.emplace_back(shared, protectMemory);

    return shared;
}

std::weak_ptr<CSolar> ResourceManager::SpaceObjectBuilder::SpawnSolar()
{
    const auto solarArch = Archetype::GetSolar(npcTemplate.archetypeHash);

    pub::SpaceObj::SolarInfo si{};
    si.flag = 4;

    // Prepare the settings for the space object
    si.archId = npcTemplate.archetypeHash;
    si.loadoutId = npcTemplate.loadoutHash;
    si.hitPointsLeft = 1000;
    si.systemId = system.value();
    si.orientation = rotation.value_or(Matrix::Identity());

    si.costume =
        costumeOverride.value_or(Costume(CreateID("benchmark_male_head"), CreateID("benchmark_male_body"))); // NOLINT(clang-diagnostic-c++20-extensions)
    si.voiceId = voiceOverride.value_or(CreateID("atc_leg_m01"));

    si.rep = MakeId(StringUtils::wstos(reputation.value_or(L"")).c_str());

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

    si.dockWith = dockTo.value_or(0);

    // Define the string used for the scanner name.
    // Because this is empty, the solar_name will be used instead.
    FmtStr scannerName(0, nullptr);
    scannerName.begin_mad_lib(0);
    scannerName.end_mad_lib();

    // Define the string used for the solar name.
    FmtStr solarName(0, nullptr);
    solarName.begin_mad_lib(16163); // ids of "%s0 %s1"
    if (name.has_value())
    {
        auto [firstName, secondName] = name.value();
        solarName.append_string(firstName);
        if (secondName != 0)
        {
            solarName.append_string(secondName);
        }
    }
    solarName.end_mad_lib();

    // Set Reputation
    pub::Reputation::Alloc(si.rep, scannerName, solarName);

    if (reputation.has_value())
    {
        uint r;
        pub::Reputation::GetReputationGroup(r, StringUtils::wstos(reputation.value()).c_str());
        pub::Reputation::SetAffiliation(si.rep, r);
    }

    // prevent the game from sending the solar creation packet (we need to do it ourselves)
    auto address = FLHook::Offset(FLHook::BinaryType::Server,  AddressList::CreateSolar);;
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

    // Find increases the ref count by 1
    auto ptr = static_cast<CSolar*>(CObject::Find(spaceId, CObject::Class::CSOLAR_OBJECT)); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)

    if (!ptr)
    {
        return {};
    }

    std::shared_ptr<CSolar> shared{ptr, &NoOp<CSolar>};
    FLHook::GetResourceManager().spawnedSolars.emplace_back(shared, protectMemory);

    return shared;
}

bool ResourceManager::SpaceObjectBuilder::ValidateSpawn() const
{
    return (Archetype::GetSolar(npcTemplate.archetypeHash) || Archetype::GetShip(npcTemplate.archetypeHash)) && system.has_value() && npcTemplate.loadoutHash &&
           Loadout::Get(npcTemplate.loadoutHash);
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
        if (const auto erased = std::erase_if(spawnedSolars, [&ptr](std::shared_ptr<CSolar> solar) { return ptr->id == solar->id; });
            !erased)
        {
            Logger::Debug(L"Tried to dispose of CSolar that wasn't owned by resource manager");
            return;
        }
    }

    if (ptr->objectClass == CObject::CSHIP_OBJECT)
    {
        if (const auto erased = std::erase_if(spawnedShips, [&ptr](std::shared_ptr<CShip> ship) { return ptr->id == ship->id; });
            !erased)
        {
            Logger::Debug(L"Tried to dispose of CShip that wasn't owned by resource manager");
            return;
        }
    }

    if (instantly)
    {
        pub::SpaceObj::Destroy(ptr->id, DestroyType::Vanish);
    }
    else
    {
        ptr->isDead = true;
        pub::SpaceObj::SetRelativeHealth(ptr->id, 0.0f);
    }

    ptr->Release();
}

void ResourceManager::Despawn(std::weak_ptr<CEqObj> object)
{
    Destroy(object, true);
}

ResourceManager::ResourceManager()
{
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
                    npc.hash = CreateID(nickname.data());
                }
                else if (ini.is_value("loadout"))
                {
                    std::string_view loadout = ini.get_value_string();
                    npc.loadout = StringUtils::stows(ini.get_value_string());
                    npc.loadoutHash = CreateID(loadout.data());
                }
                else if (ini.is_value("pilot"))
                {
                    std::string_view pilot = ini.get_value_string();
                    npc.pilot = StringUtils::stows(pilot);
                    npc.pilotHash = CreateID(pilot.data());
                }
                else if (ini.is_value("ship_archetype"))
                {
                    std::string_view archetype = ini.get_value_string();
                    npc.archetype = StringUtils::stows(ini.get_value_string());
                    npc.archetypeHash = CreateID(archetype.data());
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
        pub::SpaceObj::Destroy(solar->id, DestroyType::Vanish);
        solar->Release();
    }

    for (const auto& ship : spawnedShips | std::views::keys)
    {
        pub::SpaceObj::Destroy(ship->id, DestroyType::Vanish);
        ship->Release();
    }
}
