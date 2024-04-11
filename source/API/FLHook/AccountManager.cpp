#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/Utils/Reflection.hpp"

#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>
#include <stduuid/uuid.h>

struct Rumor
{
    uint IDS;
    uint unknown; // number of times read?
};

struct VNpc
{
    enum class NpcMissionStatus
    {
        NotOnAMissionForThisNpc,
        OnAMissionForThisNpc,
        CompletedMissionForThisNpc
    };

    uint baseHash;
    uint npcHash;
    uint interactionCount;
    NpcMissionStatus missionStatus = NpcMissionStatus::NotOnAMissionForThisNpc;
};

struct MPlayerDataSaveStruct
{
        uint padding0[8];
        bool padding8;                           // 32
        bool canDock;                            // 33
        uint can_dock2;                          // 36
        st6::list<uint> DockExceptions;          // 40
        bool can_tl;                             // 52
        uint padding51;                          // 56
        st6::list<uint> TL_Exceptions;           // 60
        BinarySearchTree<uint> BST_killed_ships; // 72
        BinarySearchTree<uint> BST_rm_completed; // 92
        BinarySearchTree<uint> BST_rm_Aborted;   // 112
        BinarySearchTree<uint> BST_rm_Failed;    // 132
        float totalCashEarned;                   // 156
        float totalTimePlayed;                   // 160
        st6::vector<uint> visitedSystems;        // 164
        st6::vector<uint> visitedBases;          // 180
        st6::vector<uint> visitedHoles;          // 196
        uint padding52;                          // 200
        uint padding53;                          // 204
        uint padding54;                          // 208
        uint padding55;                          // 212
        uint padding56;                          // 216
        uint padding57;                          // 220
        uint padding58;                          // 224
        st6::vector<VNpc> visitedNPCs;  // 240
        st6::vector<Rumor> receivedRumors;// 252
};

void ConvertCharacterToVanillaData(VanillaLoadData* data, const Character& character)
{
    std::wstring wCharName = StringUtils::stows(character.characterName);
    data->name = reinterpret_cast<const ushort*>(wCharName.c_str());

    // Play time
    data->datetimeHigh = static_cast<uint>((character.totalTimePlayed & 0xFFFFFFFF00000000LL) >> 32);
    data->datetimeLow = static_cast<uint>(character.totalTimePlayed & 0xFFFFFFFFLL);

    data->shipHash = character.shipHash;
    data->money = character.money;
    data->numOfKills = character.killCount;
    data->numOfSuccessMissions = character.missionSuccessCount;
    data->numOfFailedMissions = character.missionFailureCount;
    data->hullStatus = character.hullStatus;
    data->baseHullStatus = character.baseHullStatus;
    data->currentBase = character.currentBase;
    data->lastDockedBase = character.lastDockedBase;
    data->currentRoom = character.currentRoom;
    data->system = character.system;
    data->rank = character.rank;
    data->affiliation = character.affiliation;
    data->interfaceState = character.interfaceState;

    data->baseCostume = character.baseCostume;
    data->commCostume = character.commCostume;

    Vector vec = { 0.0f, 0.0f, 0.0f };
    data->pos = character.pos.value_or(vec);
    data->rot = EulerMatrix(character.rot.value_or(vec));

    for (const auto rep : character.reputation)
    {
        Reputation::Relation relation{};
        relation.hash = rep.first;
        relation.reputation = rep.second;
        data->repList.push_back(relation);
    }

    SubObjectID::EquipIdMaker eqId;
#define AddCargo(cargo, list)           \
    EquipDesc equipDesc;                \
    equipDesc.id = eqId.CreateEquipID();\
    equipDesc.archId = cargo.id;        \
    equipDesc.mounted = false;          \
    equipDesc.count = cargo.amount;     \
    equipDesc.health = cargo.health;    \
    list.push_back(equipDesc);

#define AddEquip(equip, list)           \
    EquipDesc equipDesc;                \
    equipDesc.id = eqId.CreateEquipID();\
    equipDesc.archId = equip.id;        \
    equipDesc.health = equip.health;    \
    equipDesc.mounted = equip.mounted;  \
    equipDesc.hardPoint.value = StringAlloc(equip.hardPoint.c_str(), false); \
    list.push_back(equipDesc);

    for (const auto& cargo : character.baseCargo)
    {
        AddCargo(cargo, data->baseEquipAndCargo);
    }

    for (const auto& equip : character.baseEquipment)
    {
        AddEquip(equip, data->baseEquipAndCargo);
    }
    
    eqId.Reset();

    for (const auto& cargo : character.cargo)
    {
        AddCargo(cargo, data->currentEquipAndCargo);
    }

    for (const auto& equip : character.equipment)
    {
        AddEquip(equip, data->currentEquipAndCargo);
    }

    data->equipIdEnumerator = eqId;


#undef AddEquip
#undef AddCargo

    // Collision groups
    for (const auto& cg : character.baseCollisionGroups)
    {
        data->baseCollisionGroups.push_back({ static_cast<ushort>(cg.first), 0, cg.second });
    }

    for (const auto& cg : character.collisionGroups)
    {
        data->currentCollisionGroups.push_back({ static_cast<ushort>(cg.first), 0, cg.second });
    }

    // Copy voice
    memcpy(data->voice, character.voice.c_str(), character.voice.size() + 1);
    data->voiceLen = character.voice.size() + 1;

    /*for (auto& visit : visitValues)
    {
        using sub_6D5C600Type = int(__fastcall*)(void* ptr, void* null, uint* u, uint* state);
        static auto sub_6D5C600 = sub_6D5C600Type(DWORD(server) + 0x7C600);

        const auto unkThis = (DWORD*)(DWORD(data) + 0x384);
        uint v166[5];
        uint input[2] = { visit.first, visit.second };
        sub_6D5C600(unkThis, edx, v166, input);
        *(v166 + 16) = static_cast<byte>(visit.second);
    }*/

    /*for (auto& rep : reps)
    {
        if (rep.second.empty())
        {
            continue;
        }

        struct Input
        {
                uint hash;
                float rep;
        };

        Input input = { 0, rep.first };
        TString<16> str;
        str.len = sprintf_s(str.data, "%s", rep.second.c_str());
        input.hash = Reputation::get_id(str);

        using sub_6D58B40Type = int(__fastcall*)(void* ptr, void* null, void* unk1, uint unk2, void* fac);
        static auto sub_6D58B40 = sub_6D58B40Type(DWORD(server) + 0x78B40);
        void* unkThis = PDWORD(DWORD(data) + 704);
        void* unk2 = *(PDWORD*)(DWORD(data) + 712);
        sub_6D58B40(unkThis, edx, unk2, 1, &input);
    }*/
}

void ConvertVanillaDataToCharacter(VanillaLoadData* data, Character& character)
{
    std::wstring charName = reinterpret_cast<const wchar_t*>(data->name.c_str());
    character.characterName = StringUtils::wstos(charName);
    character.shipHash = data->shipHash;
    character.money = data->money;
    character.killCount = data->numOfKills;
    character.missionSuccessCount = data->numOfSuccessMissions;
    character.missionFailureCount = data->numOfFailedMissions;
    character.hullStatus = data->hullStatus;
    character.baseHullStatus = data->baseHullStatus;
    character.currentBase = data->currentBase;
    character.lastDockedBase = data->lastDockedBase;
    character.currentRoom = data->currentRoom;
    character.system = data->system;
    character.rank = data->rank;
    character.affiliation = data->affiliation;
    character.interfaceState = data->interfaceState;

    character.commCostume = data->commCostume;
    character.baseCostume = data->baseCostume;

    character.voice = data->voice;

    SYSTEMTIME sysTime;
    FILETIME fileTime;
    GetLocalTime(&sysTime);
    SystemTimeToFileTime(&sysTime, &fileTime);
    character.totalTimePlayed = static_cast<int64>(fileTime.dwHighDateTime) << 32 | fileTime.dwLowDateTime;

    Vector vec = { 0.0f, 0.0f, 0.0f };
    character.pos = data->pos;
    character.rot = data->rot.ToEuler(true);

    for (const auto& [hash, reputation] : data->repList)
    {
        character.reputation.insert({ hash, reputation });
    }

    for (const auto& equip : data->currentEquipAndCargo)
    {
        Equipment equipment = {};
        FLCargo cargo = {};

        bool isCommodity = false;
        pub::IsCommodity(equip.archId, isCommodity);
        if (!isCommodity)
        {
            equipment.id = equip.archId;
            equipment.health = equip.health;
            equipment.mounted = equip.mounted;
            equipment.hardPoint = equip.hardPoint.value;
            character.equipment.emplace_back(equipment);
        }
        else
        {
            cargo.id = equip.archId;
            cargo.health = equip.health;
            cargo.isMissionCargo = equip.mission;
            cargo.amount = equip.count;
            character.cargo.emplace_back(cargo);
        }
    }

    for (const auto& equip : data->baseEquipAndCargo)
    {
        Equipment equipment = {};
        FLCargo cargo = {};

        bool isCommodity = false;
        pub::IsCommodity(equip.archId, isCommodity);
        if (!isCommodity)
        {
            equipment.id = equip.archId;
            equipment.health = equip.health;
            equipment.mounted = equip.mounted;
            equipment.hardPoint = equip.hardPoint.value;
            character.baseEquipment.emplace_back(equipment);
        }
        else
        {
            cargo.id = equip.archId;
            cargo.health = equip.health;
            cargo.isMissionCargo = equip.mission;
            cargo.amount = equip.count;
            character.baseCargo.emplace_back(cargo);
        }
    }

    for (const auto& col : data->currentCollisionGroups)
    {
        character.collisionGroups.insert({ col.id, col.health });
    }

    for (const auto& col : data->baseCollisionGroups)
    {
        character.baseCollisionGroups.insert({ col.id, col.health });
    }
}

using CreateCharacterLoadingData = void*(__thiscall*)(PlayerData* data, const char* buffer);
CreateCharacterLoadingData createCharacterLoadingData = reinterpret_cast<CreateCharacterLoadingData>(0x77090);

void VanillaLoadData::SetRelation(Reputation::Relation relation)
{
    static DWORD server = DWORD(GetModuleHandleA("server.dll"));

    using sub_6D58B40Type = int(__thiscall*)(void* ptr, void* unk1, uint unk2, Reputation::Relation* fac);
    static auto sub_6D58B40 = sub_6D58B40Type(DWORD(server) + 0x78B40);

    sub_6D58B40(&repList, repList.end(), 1, &relation);
}

AccountManager::LoginReturnCode __stdcall AccountManager::AccountLoginInternal(PlayerData* data, const uint clientId)
{
    const auto& account = accounts[clientId];

    for (auto& character : account.characters)
    {
        // Freelancer uses a temporary buffer to hold the character data
        // We need to emulate this behaviour
        static std::array<char, 512> characterLoadingBuffer;                          // Statically preallocate it
        std::memset(characterLoadingBuffer.data(), 0, characterLoadingBuffer.size()); // Ensure that the buffer is empty every time

        // Copy the character file name into the buffer
        memcpy_s(characterLoadingBuffer.data(), characterLoadingBuffer.size(), character.first.data(), character.first.size());

        // Pass the buffer into the original function that we populate with our data
        auto* loadData = static_cast<VanillaLoadData*>(createCharacterLoadingData(reinterpret_cast<PlayerData*>(&data->chararacterCreationPtr), characterLoadingBuffer.data()));

        // Copy the data from our DB type to the internal type
        ConvertCharacterToVanillaData(loadData, character.second);
    }

    auto& internalAccount = accounts[clientId];
    internalAccount.internalAccount->numberOfCharacters = account.characters.size();

    data->systemId = 0;
    data->shipId = 0;
    data->baseId = 0;
    data->characterId = 0;
    data->lastBaseId = 0;
    data->lastEquipId = 0;
    data->createdShipId = 0;
    data->baseRoomId = 0;

    data->account = internalAccount.internalAccount;
    data->account->dunno4[1] = clientId;
    wcscpy_s(data->accId, internalAccount.internalAccount->accId);
    data->clientId = clientId;
    data->exitedBase = 0;

    return LoginReturnCode::Success;
}

void AccountManager::LoadNewPlayerFLInfo()
{
    INI_Reader ini;
    if (!ini.open("mpnewcharacter.fl", false) || !ini.find_header("Player"))
    {
        // TODO: Log
        return;
    }

    while (ini.read_value())
    {
        const std::string key = ini.get_name_ptr();
        // TODO: add more copy possible data fields, like pos, voice, etc
        if (key == "initial_rep")
        {
            newPlayerTemplate.initialRep = ini.get_value_string();
        }
        else if (key == "rank")
        {
            newPlayerTemplate.rank = std::clamp(ini.get_value_int(0), 0, 99);
        }
        else if (key == "money")
        {
            if (!strcmp(ini.get_value_string(0), "%%MONEY%%"))
            {
                newPlayerTemplate.money = -1;
            }
            else
            {
                newPlayerTemplate.money = std::clamp(ini.get_value_int(0), 0, 999'999'999);
            }
        }
        else if (key == "costume")
        {
            newPlayerTemplate.costume = ini.get_value_string();
        }
        else if (key == "comm_costume")
        {
            newPlayerTemplate.commCostume = ini.get_value_string();
        }
        else if (key == "system")
        {
            newPlayerTemplate.system = ini.get_value_string();
            // TODO: Validate system
        }
        else if (key == "base")
        {
            newPlayerTemplate.base = ini.get_value_string();
            // TODO: look up and validate base
        }
        else if (key == "house")
        {
            newPlayerTemplate.reputationOverrides[ini.get_value_string(1)] = ini.get_value_float(0);
        }
        else if (key == "visit")
        {
            newPlayerTemplate.visitValues[ini.get_value_int(0)] = ini.get_value_int(0);
        }
        else if (key == "ship")
        {
            // TODO: Look up if valid ship
            newPlayerTemplate.ship = CreateID(ini.get_value_string());
        }
        else if (key == "%%PACKAGE%%")
        {
            newPlayerTemplate.hasPackage = true;
        }
    }

    if (!newPlayerTemplate.hasPackage)
    {
        Logger::Log(LogLevel::Warn,
                    L"Missing %%PACKAGE%% from mpnewplayer.fl. If the package is missing any data from a valid save file, "
                    L"new characters can cause server and client crashes.");
    }
}

bool AccountManager::OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* characterInfo)
{
    const auto getFlName = reinterpret_cast<GetFLNameT>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetFlName));
    const auto db = NewChar::TheDB;

    const auto package = db->FindPackage(characterInfo->package);
    const auto faction = db->FindFaction(characterInfo->nickName);
    const auto pilot = db->FindPilot(characterInfo->pilot);
    const auto base = db->FindBase(characterInfo->base);

    if (!package || !faction || !pilot || !base)
    {
        return false;
    }

    static std::array<char, 512> characterCodeBuffer;
    std::memset(characterCodeBuffer.data(), 0, characterCodeBuffer.size());

    getFlName(characterCodeBuffer.data(), characterInfo->charname);
    auto* loadData = static_cast<VanillaLoadData*>(createCharacterLoadingData(reinterpret_cast<PlayerData*>(&data->chararacterCreationPtr), characterCodeBuffer.data()));

    loadData->currentBase = newPlayerTemplate.base == "%%HOME_BASE%%" ? characterInfo->base : CreateID(newPlayerTemplate.base.c_str());
    loadData->system =
        newPlayerTemplate.system == "%%HOME_SYSTEM%%" ? Universe::get_base(characterInfo->base)->systemId : CreateID(newPlayerTemplate.system.c_str());

    loadData->name = reinterpret_cast<unsigned short*>(characterInfo->charname);

    const auto costDesc = GetCostumeDescriptions();
    costDesc->get_costume(pilot->body.c_str(), loadData->baseCostume);
    costDesc->get_costume(pilot->comm.c_str(), loadData->commCostume);
    if (pilot->voice.empty())
    {
        strcpy_s(loadData->voice, "trent_voice");
        loadData->voiceLen = 12;
    }
    else
    {
        strcpy_s(loadData->voice, pilot->voice.c_str());
        loadData->voiceLen = pilot->voice.size() + 1; // +1 for null terminator
    }

    if (newPlayerTemplate.hasPackage)
    {
        loadData->shipHash = CreateID(package->ship.c_str());
        loadData->money = package->money;

        const auto loadOut = Loadout::Get(CreateID(package->loadout.c_str()));

        for (const EquipDesc* equip = loadOut->first; equip != loadOut->end; equip++)
        {
            EquipDesc e = *equip;
            // For some reason some loadouts contain invalid entries
            // Since all hashes have the first bit set, we can filter those out
            if (!(e.archId & 0x80000000) || !e.id)
            {
                continue;
            }

            loadData->currentEquipAndCargo.push_back(e);
            loadData->baseEquipAndCargo.push_back(e);
        }
    }
    else
    {
        loadData->shipHash = newPlayerTemplate.ship;
    }
    auto& account = accounts[data->clientId];
    static std::array<char,512> characterFileNameBuffer;
    std::memset(characterFileNameBuffer.data(), 0, characterFileNameBuffer.size());
    getFlName(characterFileNameBuffer.data(), characterInfo->charname);

    Character& character = account.characters[characterFileNameBuffer.data()] = {};
    ConvertVanillaDataToCharacter(loadData, character);
    character.accountId = account.account._id;

    return SaveCharacter(character, true);
}

bool AccountManager::OnPlayerSave(PlayerData* pd)
{
    auto& client = FLHook::GetClient(ClientId(pd->clientId));
    if (client.characterName.empty() || !pd->systemId)
    {
        return true;
    }

    static DWORD contentModule = DWORD(GetModuleHandleA("content.dll")) + 0x130BBC;
    static BinarySearchTree<MPlayerDataSaveStruct*>* mdataBST = (BinarySearchTree<MPlayerDataSaveStruct*>*)contentModule;

    auto mdata = mdataBST->Find(pd->clientId);
    if (mdata == mdataBST->end())
    {
        Logger::Log(LogLevel::Err, std::format(L"Fetching mPlayer data for {}", client.characterName));
        return true;
    }

    auto& character = accounts[pd->clientId].characters.at(pd->charFile.charFilename);

    character.basesVisited.clear();
    character.jumpHolesVisited.clear();
    character.systemsVisited.clear();
    character.npcVisits.clear();

    for (auto visitedBase : mdata->value->visitedBases)
    {
        character.basesVisited.emplace_back(visitedBase);
    }
    for (auto visitedJH : mdata->value->visitedHoles)
    {
        character.jumpHolesVisited.emplace_back(visitedJH);
    }
    for (auto visitedNPC : mdata->value->visitedNPCs)
    {
        NpcVisit vnpc = { visitedNPC.baseHash, visitedNPC.npcHash, visitedNPC.interactionCount, static_cast<int>(visitedNPC.missionStatus) };
        character.npcVisits.emplace_back(vnpc);
    }
    for (auto visitedSystem : mdata->value->visitedSystems)
    {
        character.systemsVisited.emplace_back(visitedSystem);
    }

    character.shipTypesKilled.clear();


    character.equipment.clear();
    character.baseEquipment.clear();
    character.cargo.clear();
    character.baseCargo.clear();
    character.collisionGroups.clear();
    character.baseCollisionGroups.clear();

    character.characterName = StringUtils::wstos(client.characterName);
    character.shipHash = pd->shipArchetype;
    character.money = pd->money;
    character.killCount = pd->numKills;
    character.missionSuccessCount = pd->numMissionSuccesses;
    character.missionFailureCount = pd->numMissionFailures;
    character.hullStatus = pd->relativeHealth;

    if (pd->exitedBase && !pd->shipId)
    {
        character.currentBase = pd->exitedBase;
    }
    else
    {
        character.currentBase = pd->baseId;
    }

    character.lastDockedBase = pd->lastBaseId;
    character.currentRoom = pd->baseRoomId;
    character.system = pd->systemId;
    character.interfaceState = pd->interfaceState;

    character.commCostume = pd->commCostume;
    character.baseCostume = pd->baseCostume;

    character.voice = pd->voice;

    SYSTEMTIME sysTime;
    FILETIME fileTime;
    GetLocalTime(&sysTime);
    SystemTimeToFileTime(&sysTime, &fileTime);
    character.totalTimePlayed = static_cast<int64>(fileTime.dwHighDateTime) << 32 | fileTime.dwLowDateTime;

    if(pd->shipId)
    {
        //TODO: Release the cship?
        auto cship = ShipId(pd->shipId).GetCShip(false).Handle();
        character.pos = cship->position;
        character.rot = cship->orientation.ToEuler(true);
    }
    uint affiliation;
    uint rank;
    unsigned char relationCount;
    FmtStr firstName, secondName;
    const unsigned short* name;
    std::vector<Reputation::Relation> relations;
    relations.resize(Reputation::group_count());
    Reputation::Vibe::Get(pd->reputation, affiliation, rank, relationCount, relations.data(), firstName, secondName, name);
    for (const auto& [hash, reputation] : relations)
    {
        character.reputation.insert({ hash, reputation });
    }

    character.rank = rank;
    character.affiliation = affiliation;

    for (const auto& equip : pd->equipAndCargo.equip)
    {
        Equipment equipment = {};
        FLCargo cargo = {};

        bool isCommodity = false;
        pub::IsCommodity(equip.archId, isCommodity);
        if (!isCommodity)
        {
            equipment.id = equip.archId;
            equipment.health = equip.health;
            equipment.mounted = equip.mounted;
            equipment.hardPoint = equip.hardPoint.value;
            character.equipment.emplace_back(equipment);
        }
        else
        {
            cargo.id = equip.archId;
            cargo.health = equip.health;
            cargo.isMissionCargo = equip.mission;
            cargo.amount = equip.count;
            character.cargo.emplace_back(cargo);
        }
    }

    for (const auto& col : pd->collisionGroupDesc)
    {
        character.collisionGroups.insert({ col.id, col.health });
    }

    if(character.currentBase)
    {
        character.baseCargo = character.cargo;
        character.baseEquipment = character.equipment;
        character.baseHullStatus = character.hullStatus;
    }
    else
    {
        std::string charName = StringUtils::wstos(client.characterName.data());
        const auto getFlName = reinterpret_cast<GetFLNameT>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetFlName));

        char fileName[50];
        getFlName(fileName, client.characterName.data());

        CharacterBaseDataInfo* currPlayer = pd->accountCharacterDataBegin->root;
        while(currPlayer != pd->accountCharacterDataEnd)
        {
            int i = _stricmp(fileName,currPlayer->filename);
            if(i == 0)
            {
                break;
            }
            else if(i < 0)
            {
                currPlayer = currPlayer->head;
            }
            else
            {
                currPlayer = currPlayer->right;
            }
        }
        if (currPlayer == pd->accountCharacterDataEnd)
        {
            //TODO: character failed to fetch, handle/fix
            Logger::Log(LogLevel::Err, std::format(L"Fetching Base Status failed for {}", client.characterName));
            return true;
        }

        character.baseHullStatus = currPlayer->baseHealth;
        for (const auto& col : currPlayer->baseColgrps)
        {
            character.baseCollisionGroups.insert({ col.id, col.health });
        }
        for (const auto& equip : currPlayer->baseEquipList)
        {
            Equipment equipment = {};
            FLCargo cargo = {};

            bool isCommodity = false;
            pub::IsCommodity(equip.archId, isCommodity);
            if (!isCommodity)
            {
                equipment.id = equip.archId;
                equipment.health = equip.health;
                equipment.mounted = equip.mounted;
                equipment.hardPoint = equip.hardPoint.value;
                character.baseEquipment.emplace_back(equipment);
            }
            else
            {
                cargo.id = equip.archId;
                cargo.health = equip.health;
                cargo.isMissionCargo = equip.mission;
                cargo.amount = equip.count;
                character.baseCargo.emplace_back(cargo);
            }
        }
    }

    return SaveCharacter(character, false);
}

std::wstring newAccountString;
void AccountManager::PlayerDbInitDetour(PlayerDB* db, void* edx, uint unk, bool unk2)
{
    dbInitDetour->UnDetour();
    db->init(unk, unk2);

    // Create our 256 CAccounts
    std::random_device rd;
    auto seed = std::array<int, std::mt19937::state_size>{};
    std::ranges::generate(seed, std::ref(rd));
    std::seed_seq seq(std::begin(seed), std::end(seed));
    std::mt19937 generator(seq);
    uuids::uuid_random_generator gen{ generator };

    for (int i = 0; i < accounts.size(); ++i)
    {
        const auto uuid = gen();

        const std::string uuidStr = to_string(uuid);
        newAccountString = StringUtils::stows(uuidStr);

        st6::wstring st6Str(reinterpret_cast<const unsigned short*>(newAccountString.data()), newAccountString.size());

        assert(db->CreateAccount(st6Str));
        db->numAccounts++;
    }
}

// EAX return
// 3 = login success
// 2 = login username / password incorrect
// 1 = already in use
// 0 = banned

PlayerDbLoadUserDataAssembly::PlayerDbLoadUserDataAssembly()
{
    mov(eax, dword[esp + 0x1320 + 0x8]);
    mov(edx, dword[esp + 0x1320 - 0x1310]);
    push(eax);
    imul(eax, eax, 0x418);
    lea(ecx, dword[esp + 0x1324 - 0x1208]);
    mov(ecx, dword[edx]);
    lea(ecx, dword[eax + ecx - 0x418]);
    push(ecx); // Player Data
    call(AccountManager::AccountLoginInternal);
    pop(edi);
    pop(esi);
    pop(ebp);
    pop(ebx);
    add(esp, 0x1310);
    ret(8);
}

uint __fastcall CopyToBuffer(WORD* buffer)
{
    int i = 0;

    for (const auto& c : newAccountString)
    {
        constexpr DWORD magicThing = 0x63E22E6E;
        const DWORD charTest = c;

        const auto xorTest = charTest ^ magicThing;
        const auto wCharTest = static_cast<wchar_t>(xorTest);

        buffer[i++] = wCharTest;
    }

    return i;
}

_declspec(naked) void InitFromFolderIoBypass()
{
    __asm {
        mov [esp+0x10], ecx // CAccount*
        push eax
        push ecx
        lea ecx, [esp+0x918+0x8]
        call CopyToBuffer
        pop ecx
        mov ebp, eax // Buffer length
        pop eax
        sub esp, 0x14 // Fix the stack
        push 0x6D569B4
        ret
    }
}

void __fastcall AccountManager::CreateAccountInitFromFolderBypass(CAccount* account, void* edx, char* dir)
{
    account->InitFromFolder(dir);

    for (auto& acc : accounts)
    {
        if (!acc.internalAccount)
        {
            account->accId[36] = L'\0';
            account->accId[37] = L'\0';
            account->accId[38] = L'\0';
            account->accId[39] = L'\0';
            account->dunno2[0] = 36;
            acc.internalAccount = account;
            break;
        }
    }
}

AccountManager::AccountManager()
{
    instance = this;

    const auto serverOffset = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));

    LoadNewPlayerFLInfo();

    createCharacterLoadingData = reinterpret_cast<CreateCharacterLoadingData>(reinterpret_cast<DWORD>(createCharacterLoadingData) + serverOffset);

    // Replace Server.dll PlayerDB init
    MemUtils::PatchAssembly(serverOffset + 0x713FF, reinterpret_cast<void*>(serverOffset + 0x714D2)); // Bypass reading folders for accounts
    // Create 256 CAccounts on startup, replace Login function entirely.

    onPlayerSaveDetour = std::make_unique<FunctionDetour<OnPlayerSaveType>>(reinterpret_cast<OnPlayerSaveType>(serverOffset + 0x6C430));
    onPlayerSaveDetour->Detour(OnPlayerSave);

    dbInitDetour = std::make_unique<FunctionDetour<DbInitType>>(reinterpret_cast<DbInitType>(serverOffset + 0x710C0)); // Detour the init function
    dbInitDetour->Detour(PlayerDbInitDetour);

    onCreateNewCharacterDetour = std::make_unique<FunctionDetour<OnCreateNewCharacterType>>(reinterpret_cast<OnCreateNewCharacterType>(serverOffset + 0x6B790));
    onCreateNewCharacterDetour->Detour(OnCreateNewCharacter);

    // MemUtils::PatchAssembly(mod + 0x76940, reinterpret_cast<PVOID>(mod + 0x76BBA)); // Patch out CAccount::InitFromFolder
    //  PlayerDB::load_user_data hacks
    MemUtils::NopAddress(serverOffset + 0x734DE, 0x6D534F1 - 0x6D534DE); // Don't call Access to look for a file

    std::array<byte, 2> removeStringCheck = { 0xB0, 0x01 }; // mov al, 1
    MemUtils::WriteProcMem(serverOffset + 0x734FEE, removeStringCheck.data(), removeStringCheck.size());
    MemUtils::NopAddress(serverOffset + 0x7352B, 0x6D53589 - 0x6D5352B);
    std::array<byte, 3> stackFix = { 0x83, 0xEC, 0x14 };
    MemUtils::WriteProcMem(serverOffset + 0x7357A, stackFix.data(), stackFix.size());

    MemUtils::PatchAssembly(serverOffset + 0x735A9, loadUserDataAssembly.getCode());

    // Patch out folder creation in create account
    MemUtils::NopAddress(serverOffset + 0x72499, 0x6D5252C - 0x6D52499);
    MemUtils::NopAddress(serverOffset + 0x725A6, 0x6D525FE - 0x6D525A6);

    MemUtils::PatchCallAddr(serverOffset, 0x72697, CreateAccountInitFromFolderBypass);

    // Patch out IO in InitFromFolder
    MemUtils::PatchAssembly(serverOffset + 0x76955, InitFromFolderIoBypass);
}

void AccountManager::ClearClientInfo(ClientId client)
{
    auto& account = accounts.at(client.GetValue());
    account.characters.clear();
}

bool AccountManager::DeleteCharacter(ClientId client, const std::wstring& characterCode)
{
    auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    auto& account = accounts.at(client.GetValue());
    std::string charCodeString = StringUtils::wstos(characterCode);
    std::string charName = account.characters.at(charCodeString).characterName;
    std::wstring wideCharName = StringUtils::stows(charName);
    try
    {
        // TODO: Handle soft deletes
        auto accountsCollection = db->database("FLHook")["accounts"];

        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        mongocxx::options::find_one_and_delete deleteOptions;
        deleteOptions.projection(make_document(kvp("accountId", 1)));
        
        const auto findDoc = make_document(kvp("characterName", charName));
        const auto ret = accountsCollection.find_one_and_delete(findDoc.view(), deleteOptions);
        if(!ret.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Character deletion failed!");
        }
        auto oid = ret->view()["_id"].get_oid();
        const auto findAcc = make_document(kvp("_id", ret->view()["accountId"].get_string()));
        const auto deleteCharacter = make_document(kvp("$pull", make_document(kvp("characters", oid))));
        accountsCollection.update_one(findAcc.view(), deleteCharacter.view());

        session.commit_transaction();
        account.characters.erase(charCodeString);
        account.internalAccount->numberOfCharacters = account.characters.size();
        Logger::Log(LogLevel::Info, std::format(L"Successfully hard deleted character: {}", wideCharName));
        return true;
    }
    catch (bsoncxx::exception& ex)
    {
        Logger::Log(LogLevel::Err, std::format(L"Error hard deleted character ({}): {}", wideCharName, StringUtils::stows(ex.what())));
        session.abort_transaction();
        return false;
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Log(LogLevel::Err, std::format(L"Error hard deleted character ({}): {}", wideCharName, StringUtils::stows(ex.what())));
        session.abort_transaction();
        return false;
    }
}

void AccountManager::Login(const std::wstring& wideAccountId, const ClientId& client)
{
    auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accountsCollection = db->database("FLHook")["accounts"];

        std::string accId = StringUtils::wstos(wideAccountId);

        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        const auto findDoc = make_document(kvp("_id", accId));
        const auto accountBson = accountsCollection.find_one(findDoc.view());

        // If account does not exist
        if (!accountBson.has_value())
        {
            // Create a new account with the provided ID
            Account account{ accId };
            auto bytes = rfl::bson::write(account);
            bsoncxx::document::view doc{ reinterpret_cast<uint8_t*>(bytes.data()), bytes.size() };

            accountsCollection.insert_one(doc);
            accounts[client.GetValue()].account = account;
            session.commit_transaction();
            return;
        }

        auto& accountRaw = accountBson.value();
        auto accountResult = rfl::bson::read<Account>(accountRaw.view().data(), accountRaw.view().length());
        if (accountResult.error().has_value())
        {
            // TODO: Log
            session.abort_transaction();
            return;
        }

        const auto account = accountResult.value();

        auto& internalAcc = accounts[client.GetValue()];
        internalAcc.account = account;

        // Convert vector to bson array
        auto idArr = bsoncxx::builder::basic::array{};
        for (auto& [bytes] : account.characters)
        {
            idArr.append(bsoncxx::oid(reinterpret_cast<const char*>(bytes), bsoncxx::oid::size()));
        }

        // Get all documents that are in the provided array
        auto filter = make_document(kvp("_id", make_document(kvp("$in", idArr))));

        for (auto cursor = accountsCollection.find(filter.view()); auto doc : cursor)
        {
            auto characterResult = rfl::bson::read<Character>(doc.data(), doc.length());
            if (characterResult.error().has_value())
            {
                Logger::Log(LogLevel::Err, std::format(L"Error when loading a character: {}", StringUtils::stows(characterResult.error().value().what())));
                session.abort_transaction();
                return;
            }
            auto character = characterResult.value();
            if (character.characterName.empty())
            {
                Logger::Log(LogLevel::Err, std::format(L"Error when reading character name for account: {}", wideAccountId));
                continue;
            }
            char charNameBuf[50];
            const static auto getFlName = reinterpret_cast<GetFLNameT>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetFlName));
            getFlName(charNameBuf, StringUtils::stows(character.characterName).c_str());
            std::string charFileName = charNameBuf;
            internalAcc.characters[charFileName] = character;
        }
        session.commit_transaction();
        return;
    }
    catch (bsoncxx::exception& ex)
    {
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        session.abort_transaction();
    }
}

bool AccountManager::SaveCharacter(Character& newCharacter, const bool isNewCharacter)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accounts = db->database("FLHook")["accounts"];
        auto findCharDoc = make_document(kvp("characterName", newCharacter.characterName));

        if (isNewCharacter)
        {
            if (const auto checkCharNameDoc = accounts.find_one(findCharDoc.view()); checkCharNameDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Character already exists while trying to create new character");
            }
        }

        // Upsert Character
        auto bsonBytes = rfl::bson::write(newCharacter);
        bsoncxx::document::view savedCharDoc{ reinterpret_cast<uint8_t*>(bsonBytes.data()), bsonBytes.size() };

        // Update account character list if new character
        if (isNewCharacter)
        {
            auto insertedDoc = accounts.insert_one(savedCharDoc);

            if (!insertedDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Unable to upsert a character.");
            }
            bson_oid_t charOid;
            memcpy_s(charOid.bytes, sizeof(charOid.bytes), insertedDoc->inserted_id().get_oid().value.bytes(), bsoncxx::v_noabi::oid::k_oid_length);

            newCharacter._id = charOid;
            const auto findAccDoc = make_document(kvp("_id", newCharacter.accountId));
            const auto charUpdateDoc = make_document(kvp("$push", make_document(kvp("characters", insertedDoc->inserted_id()))));
            accounts.update_one(findAccDoc.view(), charUpdateDoc.view());
        }
        else
        {
            auto updateDoc = make_document(kvp("$set", savedCharDoc));
            accounts.update_one(findCharDoc.view(), updateDoc.view());
        }
    }
    catch (bsoncxx::exception& ex)
    {
        if(isNewCharacter)
        {
            Logger::Log(LogLevel::Err, std::format(L"Error while creating character {}", StringUtils::stows(ex.what())));
        }
        else
        {
            Logger::Log(LogLevel::Err, std::format(L"Error while updating character {}", StringUtils::stows(ex.what())));
        }
        session.abort_transaction();
        return false;
    }
    catch (mongocxx::exception& ex)
    {
        if(isNewCharacter)
        {
            Logger::Log(LogLevel::Err, std::format(L"Error while creating character {}", StringUtils::stows(ex.what())));
        }
        else
        {
            Logger::Log(LogLevel::Err, std::format(L"Error while updating character {}", StringUtils::stows(ex.what())));
        }
        session.abort_transaction();
        return false;
    }

    session.commit_transaction();
    return true;
}
