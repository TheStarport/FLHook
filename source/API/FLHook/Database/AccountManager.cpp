#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "FLCore/Common/Globals.hpp"

#include <uuid.h>

void AccountManager::ConvertCharacterToVanillaData(CharacterData* data, const Character& character, uint clientId)
{
    const std::wstring wCharName = StringUtils::stows(character.characterName);
    data->name = reinterpret_cast<const ushort*>(wCharName.c_str());

    // TODO: figure out last time online

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
    data->interfaceState = character.interfaceState;

    data->baseCostume = character.baseCostume;
    data->commCostume = character.commCostume;

    data->pos = character.pos;
    data->rot = EulerMatrix(character.rot);

    for (const auto& [factionHash, rep] : character.reputation)
    {
        Reputation::Relation relation{};
        relation.hash = MakeId(factionHash.c_str());
        relation.reputation = rep;
        data->repList.push_back(relation);
    }

#define AddCargo(cargo, list)                               \
    EquipDesc equipDesc;                                    \
    equipDesc.id = data->equipIdEnumerator.CreateEquipID(); \
    equipDesc.archId = Id(cargo.archId);                    \
    equipDesc.mounted = false;                              \
    equipDesc.count = cargo.amount;                         \
    equipDesc.health = cargo.health;                        \
    list.push_back(equipDesc);

#define AddEquip(equip, list)                                                \
    EquipDesc equipDesc;                                                     \
    equipDesc.id = data->equipIdEnumerator.CreateEquipID();                  \
    equipDesc.archId = Id(equip.archId);                                     \
    equipDesc.health = equip.health;                                         \
    equipDesc.mounted = true;                                                \
    equipDesc.count = 1;                                                     \
    equipDesc.hardPoint.value = StringAlloc(equip.hardPoint.c_str(), false); \
    list.push_back(equipDesc);

    data->equipIdEnumerator.Reset();

    data->currentEquipAndCargo.clear();
    data->baseEquipAndCargo.clear();

    for (const auto& cargo : character.baseCargo)
    {
        AddCargo(cargo, data->baseEquipAndCargo);
    }

    for (const auto& equip : character.baseEquipment)
    {
        AddEquip(equip, data->baseEquipAndCargo);
    }

    data->equipIdEnumerator.Reset();

    for (const auto& cargo : character.cargo)
    {
        AddCargo(cargo, data->currentEquipAndCargo);
    }

    for (const auto& equip : character.equipment)
    {
        AddEquip(equip, data->currentEquipAndCargo);
    }

#undef AddEquip
#undef AddCargo

    data->currentCollisionGroups.clear();
    data->baseCollisionGroups.clear();

    // Collision groups
    for (const auto& [sId, health] : character.baseCollisionGroups)
    {
        data->baseCollisionGroups.push_back({ StringUtils::Cast<ushort>(sId), health });
    }

    for (const auto& [sId, health] : character.collisionGroups)
    {
        data->currentCollisionGroups.push_back({ StringUtils::Cast<ushort>(sId), health });
    }

    // Copy voice
    memcpy(data->voice, character.voice.c_str(), character.voice.size() + 1);
    data->voiceLen = character.voice.size() + 1;

    data->visits.clear();
    for (const auto& [objectId, visitFlag] : character.visits)
    {
        data->visits[static_cast<uint>(objectId)] = static_cast<char>(visitFlag);
    }
}

using MDataSetterFinish = uint(__thiscall*)(MPlayerDataSaveStruct* mdata);
static MDataSetterFinish MDataSetterFinishFunc;

using FLMapInsert = int(__thiscall*)(void* BST, const uint& value, const uint& key);
FLMapInsert FLMapInsertShipFunc;
FLMapInsert FLMapInsertRMFunc;

void AccountManager::InitContentDLLDetours()
{
    static DWORD contentHandle = DWORD(GetModuleHandleA("content.dll"));

    MDataSetterFinishFunc = MDataSetterFinish(contentHandle + 0xA9440);

    FLMapInsertShipFunc = FLMapInsert(contentHandle + 0x603E0);
    FLMapInsertRMFunc = FLMapInsert(contentHandle + 0xAA140);

    loadPlayerMDataDetour = std::make_unique<FunctionDetour<LoadMDataType>>(reinterpret_cast<LoadMDataType>(contentHandle + 0xA83D0));
    loadPlayerMDataDetour->Detour(LoadPlayerMData);
}
Character* AccountManager::GetCurrentCharacterData(const ClientId client)
{
    const std::string characterCode = client.GetData().playerData->charFile.charFilename;
    auto& characterMap = accounts[client.GetValue()].characters;
    const auto characterIter = characterMap.find(characterCode);
    if (characterIter == characterMap.end())
    {
        return nullptr;
    }
    return &characterIter->second;
}

Character* AccountManager::GetCurrentCharacterData(ClientId client, std::wstring_view characterName)
{

    if (!client && characterName.empty())
    {
        return nullptr;
    }

    char charNameBuffer[50];

    getFlName(charNameBuffer, characterName.data());
    ;

    auto& characterMap = accounts[client.GetValue()].characters;
    const auto charData = characterMap.find(charNameBuffer);
    if (charData == characterMap.end())
    {
        return nullptr;
    }

    return &charData->second;
}

void __fastcall AccountManager::LoadPlayerMData(MPlayerDataSaveStruct* mdata, void* edx, struct INI_Reader* ini)
{
    std::wstring selectedChar = (const wchar_t*)Players.GetActiveCharacterName(mdata->clientId2);

    char selectedCharBuffer[50];

    getFlName(selectedCharBuffer, selectedChar.c_str());

    Character character = accounts[mdata->clientId2].characters.at(std::string(selectedCharBuffer));

    mdata->visitedNPCs.clear();
    mdata->receivedRumors.clear();

    mdata->canDock = character.canDock;
    mdata->canTL = character.canTradeLane;
    if (character.tlExceptions.has_value())
    {
        for (auto [startRing, nextRing] : character.tlExceptions.value())
        {
            mdata->tlExceptions.push_back({ static_cast<uint>(startRing), static_cast<uint>(nextRing) });
        }
    }

    if (character.dockExceptions.has_value())
    {
        for (auto dockException : character.dockExceptions.value())
        {
            mdata->dockExceptions.push_back(dockException);
        }
    }

    mdata->totalTimePlayed = character.totalTimePlayed;

    for (const auto system : character.systemsVisited)
    {
        mdata->visitedSystems.push_back(system);
    }

    for (const auto base : character.basesVisited)
    {
        mdata->visitedBases.push_back(base);
    }

    for (const auto jumpHole : character.jumpHolesVisited)
    {
        mdata->visitedHoles.push_back(jumpHole);
    }

    for (const auto& vnpc : character.npcVisits)
    {
        mdata->visitedNPCs.push_back(
            { static_cast<uint>(vnpc.baseId), static_cast<uint>(vnpc.id), vnpc.interactionCount, static_cast<VNpc::NpcMissionStatus>(vnpc.missionStatus) });
    }

    for (auto [rumorIDS, rumorPriority] : character.rumorsReceived)
    {
        mdata->receivedRumors.push_back({ static_cast<uint>(rumorIDS), static_cast<uint>(rumorPriority) });
    }

    for (auto [shipArch, killCount] : character.shipTypesKilled)
    {
        int var = killCount;
        FLMapInsertShipFunc(&mdata->killedShips, var, StringUtils::Cast<uint>(shipArch));
    }
    for (auto abortedMission : character.randomMissionsAborted)
    {
        int var = abortedMission.second;
        FLMapInsertRMFunc(&mdata->rmAborted, var, StringUtils::Cast<uint>(abortedMission.first));
    }
    for (auto completedMission : character.randomMissionsCompleted)
    {
        int var = completedMission.second;
        FLMapInsertRMFunc(&mdata->rmCompleted, var, StringUtils::Cast<uint>(completedMission.first));
    }
    for (auto failedMission : character.randomMissionsFailed)
    {
        int var = failedMission.second;
        FLMapInsertRMFunc(&mdata->rmFailed, var, StringUtils::Cast<uint>(failedMission.first));
    }

    MDataSetterFinishFunc(mdata);
}

void ConvertVanillaDataToCharacter(CharacterData* data, Character& character)
{
    std::wstring charName = reinterpret_cast<const wchar_t*>(data->name.c_str());
    character.characterName = StringUtils::wstos(charName);
    character.wideCharacterName = charName;
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
    character.interfaceState = data->interfaceState;

    character.commCostume = data->commCostume;
    character.baseCostume = data->baseCostume;

    character.voice = data->voice;

    Vector vec = { 0.0f, 0.0f, 0.0f };
    character.pos = data->pos;
    character.rot = data->rot.ToEuler(true);

    for (const auto& [hash, reputation] : data->repList)
    {
        TString<16> nickname;
        if (Reputation::get_nickname(nickname, hash) == -1)
        {
            continue;
        }
        std::string nicknameStr = { nickname.data, static_cast<uint>(nickname.len) };
        character.reputation.insert({ nicknameStr, reputation });
    }

    character.equipment.clear();
    character.baseEquipment.clear();

    for (const auto& equip : data->currentEquipAndCargo)
    {
        Equipment equipment = {};
        FLCargo cargo = {};

        bool isCommodity = false;
        pub::IsCommodity(equip.archId.GetValue(), isCommodity);
        if (!isCommodity)
        {
            equipment.archId = equip.archId.GetValue();
            equipment.health = equip.health;
            equipment.hardPoint = equip.hardPoint.value;
            character.equipment.emplace_back(equipment);
        }
        else
        {
            cargo.archId = equip.archId.GetValue();
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
        pub::IsCommodity(equip.archId.GetValue(), isCommodity);
        if (!isCommodity)
        {
            equipment.archId = equip.archId.GetValue();
            equipment.health = equip.health;
            equipment.hardPoint = equip.hardPoint.value;
            character.baseEquipment.emplace_back(equipment);
        }
        else
        {
            cargo.archId = equip.archId.GetValue();
            cargo.health = equip.health;
            cargo.isMissionCargo = equip.mission;
            cargo.amount = equip.count;
            character.baseCargo.emplace_back(cargo);
        }
    }

    character.collisionGroups.clear();
    character.baseCollisionGroups.clear();

    for (const auto& col : data->currentCollisionGroups)
    {
        character.collisionGroups.insert({ std::to_string(col.id), col.health });
    }

    for (const auto& col : data->baseCollisionGroups)
    {
        character.baseCollisionGroups.insert({ std::to_string(col.id), col.health });
    }
}

using CreateCharacterLoadingData = void*(__thiscall*)(st6::map<CHARACTER_ID, CharacterData>* data, const char* buffer);
CreateCharacterLoadingData createCharacterLoadingData = reinterpret_cast<CreateCharacterLoadingData>(0x77090);

AccountManager::LoginReturnCode __stdcall AccountManager::AccountLoginInternal(PlayerData* data, const uint clientId)
{
    auto& account = accounts[clientId];
    if (!account.loginSuccess)
    {
        return LoginReturnCode::InvalidUsernamePassword;
    }

    ClientId(clientId).GetData().account = &account.account;

    if (account.account.scheduledUnbanDate)
    {
        if (account.account.scheduledUnbanDate <= TimeUtils::UnixTime<std::chrono::seconds>())
        {
            AccountId::GetAccountFromClient(ClientId(clientId)).value().UnBan();
        }
        else
        {
            return LoginReturnCode::Banned;
        }
    }

    if (loggedInAccounts.contains(account.account._id))
    {
        return LoginReturnCode::AlreadyLoggedIn;
    }

    for (auto& character : account.characters)
    {
        // Freelancer uses a temporary buffer to hold the character data
        // We need to emulate this behaviour
        static std::array<char, 512> characterLoadingBuffer;                          // Statically preallocate it
        std::memset(characterLoadingBuffer.data(), 0, characterLoadingBuffer.size()); // Ensure that the buffer is empty every time

        // Copy the character file name into the buffer
        memcpy_s(characterLoadingBuffer.data(), characterLoadingBuffer.size(), character.first.data(), character.first.size());

        // Pass the buffer into the original function that we populate with our data
        auto* loadData = static_cast<CharacterData*>(createCharacterLoadingData(&data->characterMap, characterLoadingBuffer.data()));

        // Copy the data from our DB type to the internal type
        ConvertCharacterToVanillaData(loadData, character.second, clientId);
    }

    account.internalAccount->numberOfCharacters = account.characters.size();
    account.internalAccount->clientId = clientId;

    data->systemId = SystemId();
    data->shipId = 0;
    data->baseId = BaseId();
    data->characterId = 0;
    data->lastBaseId = BaseId();
    data->lastEquipId = 0;
    data->createdShipId = 0;
    data->baseRoomId = 0;

    data->account = account.internalAccount;
    wcscpy_s(data->accId, account.internalAccount->accId);
    data->clientId = clientId;
    data->exitedBase = BaseId();

    loggedInAccounts.insert(account.account._id);

    return LoginReturnCode::Success;
}

void AccountManager::LoadNewPlayerFLInfo()
{
    DEBUG("Loading new player information");
    INI_Reader ini;
    if (!ini.open("mpnewcharacter.fl", false) || !ini.find_header("Player"))
    {
        ERROR("Unable to load [Player] from mpnewcharacter.fl");
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
                newPlayerTemplate.money = std::nullopt;
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
            if (newPlayerTemplate.system != "%%HOME_SYSTEM%%" && !Universe::get_system(CreateID(newPlayerTemplate.system.c_str())))
            {
                ERROR("System referenced inside of newplayer.fl is not valid!");
            }
        }
        else if (key == "base")
        {
            newPlayerTemplate.base = ini.get_value_string();
            if (newPlayerTemplate.base != "%%HOME_BASE%%" && !Universe::get_base(CreateID(newPlayerTemplate.base.c_str())))
            {
                ERROR("Base referenced inside of newplayer.fl is not valid!");
            }
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
            newPlayerTemplate.ship = CreateID(ini.get_value_string());
            if (!Archetype::GetShip(newPlayerTemplate.ship))
            {
                ERROR("Base referenced inside of newplayerfl is not valid!");
            }
        }
        else if (key == "%%PACKAGE%%")
        {
            newPlayerTemplate.hasPackage = true;
        }
    }

    if (!newPlayerTemplate.hasPackage)
    {
        ERROR("Missing %%PACKAGE%% from mpnewplayer.fl. If the package is missing any data from a valid save file, "
              "new characters can cause server and client crashes.");
    }

    ini.close();
    if (!ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
    {
        return;
    }

    while (ini.read_header())
    {
        if (!ini.is_header("Faction"))
        {
            continue;
        }

        uint visitList = 0;
        uint faction = 0;
        while (ini.read_value())
        {
            if (ini.is_value("visit_list"))
            {
                visitList = CreateID(ini.get_value_string());
            }
            else if (ini.is_value("nickname"))
            {
                faction = CreateID(ini.get_value_string());
            }
        }

        if (visitList && faction)
        {
            factionVisitListMapping[faction] = visitList;
        }
    }

    ini.close();

    if (!ini.open("..\\DATA\\CHARACTERS\\visitlists.ini", false))
    {
        return;
    }

    while (ini.read_header())
    {
        if (!ini.is_header("VisitList"))
        {
            continue;
        }

        uint nickname = 0;
        NewPlayerTemplate::VisitList list;
        while (ini.read_value())
        {
            if (ini.is_value("nickname"))
            {
                nickname = CreateID(ini.get_value_string());
            }
            else if (ini.is_value("faction"))
            {
                list.factions.emplace(MakeId(ini.get_value_string()));
            }
            else if (ini.is_value("visit"))
            {
                list.visits.emplace(CreateID(ini.get_value_string(0)), static_cast<char>(ini.get_value_int(0)));
            }
        }

        if (nickname)
        {
            visitLists[nickname] = list;
        }
    }
}

void CreateNewCharacterCallback(const SCreateCharacterInfo& createCharacterInfo, ClientId client, bool creationSuccess)
{
    Server.CharacterInfoReq(client.GetValue(), !creationSuccess);
    // TODO: Implement a queue for firing After type plugin calls after async actions
    // Probably just call Schedule() with callback doing only the plugin callout
    // CallPlugins(&Plugin::OnCharacterCreationAfter, client, createCharacterInfo);
}

bool __fastcall AccountManager::OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* characterInfo)
{
    if (data->characterMap.size() >= characterLimit)
    {
        return false;
    }

    auto* const db = NewChar::TheDB;

    const auto base = db->FindBase(characterInfo->base);
    const auto faction = db->FindFaction(characterInfo->nickName);
    const auto pilot = db->FindPilot(characterInfo->pilot);
    const auto package = db->FindPackage(characterInfo->package);

    if (!package || !faction || !pilot || !base)
    {
        return false;
    }

    static std::array<char, 512> characterCodeBuffer;
    std::memset(characterCodeBuffer.data(), 0, characterCodeBuffer.size());

    getFlName(characterCodeBuffer.data(), characterInfo->charname);
    auto* loadData = static_cast<CharacterData*>(createCharacterLoadingData(&data->characterMap, characterCodeBuffer.data()));

    loadData->currentBase = newPlayerTemplate.base == "%%HOME_BASE%%" ? characterInfo->base : CreateID(newPlayerTemplate.base.c_str());
    loadData->system = newPlayerTemplate.system == "%%HOME_SYSTEM%%" ? Universe::get_base(characterInfo->base)->systemId.GetValue()
                                                                     : CreateID(newPlayerTemplate.system.c_str());

    loadData->name = reinterpret_cast<unsigned short*>(characterInfo->charname);

    const auto* const costDesc = GetCostumeDescriptions();
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

    if (!newPlayerTemplate.initialRep.empty())
    {
        uint factionHash = MakeId(newPlayerTemplate.initialRep.c_str());
        if (newPlayerTemplate.initialRep == "%%FACTION%%")
        {
            factionHash = MakeId(faction->repGroup.c_str());
        }

        // ReSharper disable once CppUseElementsView
        for (auto& [hash, group] : GameData::repGroups)
        {
            float attitude = 0.f;
            Reputation::get_feelings_towards(factionHash, hash, attitude);

            Reputation::Relation relation{ hash, attitude };
            loadData->repList.push_back(relation);
        }
    }

    for (auto& [id, rep] : newPlayerTemplate.reputationOverrides)
    {
        Reputation::Relation relation{ MakeId(id.c_str()), rep };
        loadData->repList.push_back(relation);
    }

    for (auto& [obj, flag] : newPlayerTemplate.visitValues)
    {
        loadData->visits[obj] = static_cast<char>(flag);
    }

    if (auto factionVisitKey = factionVisitListMapping.find(faction->nickname); factionVisitKey != factionVisitListMapping.end())
    {
        const auto& visitList = visitLists.find(factionVisitKey->second);
        if (visitList != visitLists.end())
        {
            for (auto& visitFaction : visitList->second.factions)
            {
                // 65 is the flag for a known faction
                loadData->visits[visitFaction] = 65;
            }

            for (auto& [visit, flag] : visitList->second.visits)
            {
                loadData->visits[visit] = flag;
            }
        }
    }

    if (newPlayerTemplate.rank > 0)
    {
        loadData->rank = newPlayerTemplate.rank;
    }

    loadData->money = newPlayerTemplate.money.value_or(0);

    if (newPlayerTemplate.hasPackage)
    {
        loadData->shipHash = CreateID(package->ship.c_str());
        loadData->money = package->money;

        const auto loadOut = Loadout::Get(CreateID(package->loadout.c_str()));

        for (const EquipDesc* equip = loadOut->first; equip != loadOut->last; equip++)
        {
            EquipDesc e = *equip;
            // For some reason some loadouts contain invalid entries
            // Since all hashes have the first two bit set, we can filter those out
            if ((e.archId.GetValue() & 0x80000000) == 0 || !e.id)
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

    loadData->equipIdEnumerator.currSID += loadData->currentEquipAndCargo.size();
    loadData->interfaceState = 3;

    auto& account = accounts[data->clientId];
    std::unique_lock lock{ account.mutex };

    static std::array<char, 512> characterFileNameBuffer;
    std::memset(characterFileNameBuffer.data(), 0, characterFileNameBuffer.size());
    getFlName(characterFileNameBuffer.data(), characterInfo->charname);

    Character& character = account.characters[characterFileNameBuffer.data()] = {};
    ConvertVanillaDataToCharacter(loadData, character);
    character.accountId = account.account._id;

    // Unlock early because SaveCharacterInternal will also try and lock
    lock.unlock();

    const bool creationSuccessful = SaveCharacterInternal(ClientId(data->clientId), &account, &character, true);
    CreateNewCharacterCallback(*characterInfo, ClientId(data->clientId), creationSuccessful);
    return creationSuccessful;
}

void AccountManager::OnCreateNewCharacterCopy(PlayerData* data, SCreateCharacterInfo characterInfo) { OnCreateNewCharacter(data, nullptr, &characterInfo); }

void UpdateCharacterCache(PlayerData* pd, CharacterData* cd)
{
    cd->money = pd->money;
    cd->numOfKills = pd->numKills;
    cd->numOfSuccessMissions = pd->numMissionSuccesses;
    cd->numOfFailedMissions = pd->numMissionFailures;
    cd->hullStatus = pd->relativeHealth;

    cd->currentEquipAndCargo = pd->equipAndCargo.equip;
    cd->currentCollisionGroups = pd->collisionGroupDesc;

    cd->equipIdEnumerator.currSID = pd->lastEquipId;

    cd->currentBase = pd->baseId.GetValue();
    cd->lastDockedBase = pd->lastBaseId.GetValue();

    cd->system = pd->systemId.GetValue();
    cd->pos = pd->position;
    cd->rot = pd->orientation;

    cd->voiceLen = pd->voiceLen;
    strcpy(cd->voice, pd->voice);

    cd->interfaceState = pd->interfaceState;
    cd->visits = pd->visitEntries;
    cd->prefilledWeaponGroupIni = pd->weaponGroups;
    cd->shipHash = pd->shipArchetype;
    cd->currentRoom = pd->baseRoomId;
    cd->logInfo = pd->neuralNetLog;
    cd->startingRing = pd->unknownLocId;
    cd->baseCostume = pd->baseCostume;
    cd->commCostume = pd->commCostume;

    uchar relationCount;
    Reputation::Relation relation[256];
    uint rank;
    FmtStr fmtStr1;
    FmtStr fmtStr2;
    const unsigned short* name;
    Reputation::Vibe::Get(pd->reputation, cd->affiliation, rank, relationCount, relation, fmtStr1, fmtStr2, name);
    cd->rank = rank;

    cd->repList.clear();
    for (int i = 0; i < relationCount; i++)
    {
        cd->repList.push_back(relation[i]);
    }
}

bool AccountManager::OnPlayerSave(PlayerData* pd)
{
    auto& client = FLHook::GetClient(ClientId(pd->clientId));
    if (!client.characterId || !pd->systemId)
    {
        return true;
    }

    auto characterData = pd->characterMap.find(pd->charFile.charFilename);
    UpdateCharacterCache(pd, &characterData->second);

    auto playerMapCache = pd->characterMap.find(client.playerData->charFile);
    if (playerMapCache == pd->characterMap.end())
    {
        ERROR("Fetching Base Status failed for {{characterName}}", { "characterName", client.characterId });
        return true;
    }

    static DWORD contentModule = DWORD(GetModuleHandleA("content.dll")) + 0x130BBC;
    static auto* mdataBST = reinterpret_cast<st6::map<uint, MPlayerDataSaveStruct*>*>(contentModule);

    auto mdataIter = mdataBST->find(pd->clientId);
    if (mdataIter == mdataBST->end())
    {
        ERROR("Fetching mPlayer data failed for {{characterName}}", { "characterName", client.characterId });
        return true;
    }

    auto* mdata = mdataIter->second;

    auto& account = accounts[pd->clientId];
    std::unique_lock lock{ account.mutex };
    auto& character = account.characters.at(pd->charFile.charFilename);

    character.basesVisited.clear();
    character.jumpHolesVisited.clear();
    character.systemsVisited.clear();
    character.npcVisits.clear();

    for (auto visitedBase : mdata->visitedBases)
    {
        character.basesVisited.emplace_back(visitedBase);
    }
    for (auto visitedJH : mdata->visitedHoles)
    {
        character.jumpHolesVisited.emplace_back(visitedJH);
    }
    for (auto& visitedNPC : mdata->visitedNPCs)
    {
        character.npcVisits.push_back({ static_cast<int>(visitedNPC.baseHash),
                                        static_cast<int>(visitedNPC.npcHash),
                                        visitedNPC.interactionCount,
                                        static_cast<int>(visitedNPC.missionStatus) });
    }
    for (auto visitedSystem : mdata->visitedSystems)
    {
        character.systemsVisited.emplace_back(visitedSystem);
    }

    if (!mdata->tlExceptions.empty())
    {
        if (character.tlExceptions.has_value())
        {
            character.tlExceptions.value().clear();
        }
        else
        {
            character.tlExceptions = {};
        }
        for (auto tlException : mdata->tlExceptions)
        {
            character.tlExceptions.value().push_back({ static_cast<int>(tlException.startRing), static_cast<int>(tlException.nextRing) });
        }
    }
    if (!mdata->dockExceptions.empty())
    {
        if (character.dockExceptions.has_value())
        {
            character.dockExceptions.value().clear();
        }
        else
        {
            character.dockExceptions = {};
        }
        for (auto dockException : mdata->dockExceptions)
        {
            character.dockExceptions.value().emplace_back(dockException);
        }
    }

    character.shipTypesKilled.clear();
    character.randomMissionsCompleted.clear();
    character.randomMissionsAborted.clear();
    character.randomMissionsFailed.clear();
    for (auto iterBST : mdata->killedShips)
    {
        character.shipTypesKilled[std::to_string(iterBST.first)] = iterBST.second;
    }
    for (auto iterBST : mdata->rmCompleted)
    {
        character.randomMissionsCompleted[std::to_string(iterBST.first)] = iterBST.second;
    }
    for (auto iterBST : mdata->rmAborted)
    {
        character.randomMissionsAborted[std::to_string(iterBST.first)] = iterBST.second;
    }
    for (auto iterBST : mdata->rmFailed)
    {
        character.randomMissionsFailed[std::to_string(iterBST.first)] = iterBST.second;
    }

    character.totalTimePlayed = mdata->totalTimePlayed;

    character.visits.clear();
    // for (auto visit = pd->visitEntries.begin(); visit != pd->visitEntries.end(); ++visit)
    for (auto visit : pd->visitEntries)
    {
        character.visits.emplace_back(std::array{ static_cast<int>(visit.first), static_cast<int>(visit.second) });
    }
    character.equipment.clear();
    character.baseEquipment.clear();
    character.cargo.clear();
    character.baseCargo.clear();
    character.collisionGroups.clear();
    character.baseCollisionGroups.clear();

    character.characterName = StringUtils::wstos(client.characterId.GetValue());
    character.shipHash = pd->shipArchetype;
    character.money = pd->money;
    character.killCount = pd->numKills;
    character.missionSuccessCount = pd->numMissionSuccesses;
    character.missionFailureCount = pd->numMissionFailures;
    character.hullStatus = pd->relativeHealth;

    if (pd->exitedBase && !pd->shipId)
    {
        character.currentBase = pd->exitedBase.GetValue();
    }
    else
    {
        character.currentBase = pd->baseId.GetValue();
    }

    character.lastDockedBase = pd->lastBaseId.GetValue();
    character.currentRoom = pd->baseRoomId;
    character.system = pd->systemId.GetValue();
    character.interfaceState = pd->interfaceState;

    character.commCostume = pd->commCostume;
    character.baseCostume = pd->baseCostume;

    character.voice = pd->voice;

    if (pd->shipId)
    {
        if (auto ship = ShipId(pd->shipId).GetValue().lock(); ship)
        {
            character.pos = ship->position;
            character.rot = ship->orientation.ToEuler(true);
        }
    }

    uint affiliation;
    uint rank;
    unsigned char relationCount;
    FmtStr firstName, secondName;
    const unsigned short* name;
    std::vector<Reputation::Relation> relations;
    relations.resize(Reputation::group_count());
    Reputation::Vibe::Get(pd->reputation, affiliation, rank, relationCount, relations.data(), firstName, secondName, name);
    if (relationCount)
    {
        TString<16> nickname;
        for (const auto& [hash, reputation] : relations)
        {
            if (!hash)
            {
                continue;
            }

            if (Reputation::get_nickname(nickname, hash) == -1)
            {
                continue;
            }

            std::string nicknameStr = { nickname.data, static_cast<uint>(nickname.len) };
            character.reputation.insert({ nicknameStr, reputation });
        }

        character.rank = rank;
        character.repGroup = std::nullopt;

        if (affiliation && Reputation::get_nickname(nickname, affiliation) != -1)
        {
            character.repGroup = std::string{ nickname.data, static_cast<uint>(nickname.len) };
        }
    }

    for (const auto& equip : pd->equipAndCargo.equip)
    {
        bool isCommodity = false;
        pub::IsCommodity(equip.archId.GetValue(), isCommodity);
        if (!isCommodity)
        {
            Equipment equipment = {};
            equipment.archId = equip.archId.GetValue();
            equipment.hardPoint = equip.hardPoint.value;
            equipment.health = equip.health;
            character.equipment.emplace_back(equipment);
        }
        else
        {
            FLCargo cargo = {};
            cargo.archId = equip.archId.GetValue();
            cargo.health = equip.health;
            cargo.isMissionCargo = equip.mission;
            cargo.amount = equip.count;
            character.cargo.emplace_back(cargo);
        }
    }

    for (const auto& equip : playerMapCache->second.baseEquipAndCargo)
    {
        bool isCommodity = false;
        pub::IsCommodity(equip.archId.GetValue(), isCommodity);
        if (!isCommodity)
        {
            Equipment equipment = {};
            equipment.archId = equip.archId.GetValue();
            equipment.hardPoint = equip.hardPoint.value;
            equipment.health = equip.health;
            character.baseEquipment.emplace_back(equipment);
        }
        else
        {
            FLCargo cargo = {};

            cargo.archId = equip.archId.GetValue();
            cargo.health = equip.health;
            cargo.isMissionCargo = equip.mission;
            cargo.amount = equip.count;
            character.baseCargo.emplace_back(cargo);
        }
    }

    for (const auto& col : pd->collisionGroupDesc)
    {
        character.collisionGroups.insert({ std::to_string(col.id), col.health });
    }
    for (const auto& col : playerMapCache->second.baseCollisionGroups)
    {
        character.baseCollisionGroups.insert({ std::to_string(col.id), col.health });
    }

    if (character.currentBase)
    {
        character.baseCargo = character.cargo;
        character.baseEquipment = character.equipment;
        character.baseHullStatus = character.hullStatus;
    }
    else if (character.hullStatus == 0.0f)
    {
        character.currentBase = character.lastDockedBase;
        // TODO: customizable cargo loss on death
        character.cargo = character.baseCargo;
        character.equipment = character.baseEquipment;
        character.hullStatus = character.baseHullStatus;
        character.collisionGroups = character.baseCollisionGroups;
    }

    // Update the character kept in the account cache.
    ConvertCharacterToVanillaData(&characterData->second, character, pd->clientId);

    lock.unlock();
    FLHook::GetTaskScheduler()->ScheduleTask(SaveCharacterInternal, client.id, &account, &character, false);
    return true;
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
		mov[esp + 0x10], ecx // CAccount*
		push eax
		push ecx
		lea ecx, [esp + 0x918 + 0x8]
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
        if (!acc.internalAccount && account->accId)
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

CAccount* __fastcall AccountManager::PlayerDbGetCAccountByCharacterName(PlayerDB* playerDb, void* edx, st6::wstring& charName)
{
    char charNameBuffer[50];

    getFlName(charNameBuffer, (const wchar_t*)charName.data());
    PlayerData* db = nullptr;
    while (db = Players.traverse_active(db))
    {
        if (strcmp(charNameBuffer, db->charFile.charFilename) == 0)
        {
            return db->account;
        }
    }

    return nullptr;
}

AccountManager::AccountManager()
{
    instance = this;

    const auto serverOffset = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));
    getFlName = reinterpret_cast<GetFLNameT>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetFlName));

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

    MemUtils::NopAddress(serverOffset + 0x69A07, 7);

    std::array<byte, 2> removeStringCheck = { 0xB0, 0x01 }; // mov al, 1
    MemUtils::WriteProcMem(serverOffset + 0x734FE, removeStringCheck.data(), removeStringCheck.size());
    MemUtils::NopAddress(serverOffset + 0x7352B, 0x6D53589 - 0x6D5352B);
    std::array<byte, 3> stackFix = { 0x83, 0xEC, 0x14 };
    MemUtils::WriteProcMem(serverOffset + 0x7357A, stackFix.data(), stackFix.size());

    MemUtils::PatchAssembly(serverOffset + 0x735A9, loadUserDataAssembly.getCode());

    // Patch out folder creation in create account
    MemUtils::NopAddress(serverOffset + 0x72499, 0x6D5252C - 0x6D52499);
    MemUtils::NopAddress(serverOffset + 0x725A6, 0x6D525FE - 0x6D525A6);

    // Double size of visit packet
    auto remoteClient = DWORD(GetModuleHandleA("remoteclient.dll"));
    std::array<byte, 2> visitPatch = { 0x90, 0x01 };
    MemUtils::WriteProcMem(remoteClient + 0x96E9, visitPatch.data(), visitPatch.size());
    MemUtils::WriteProcMem(remoteClient + 0x9708, visitPatch.data(), visitPatch.size());

    MemUtils::PatchCallAddr(serverOffset, 0x72697, CreateAccountInitFromFolderBypass);

    playerDbGetAccountByCharNameDetour =
        std::make_unique<FunctionDetour<PlayerDbGetAccountByCharNameType>>(reinterpret_cast<PlayerDbGetAccountByCharNameType>(serverOffset + 0x72B60));
    playerDbGetAccountByCharNameDetour->Detour(PlayerDbGetCAccountByCharacterName);

    // Patch out IO in InitFromFolder
    MemUtils::PatchAssembly(serverOffset + 0x76955, InitFromFolderIoBypass);
}

void AccountManager::ClearClientInfo(ClientId client)
{
    auto& account = accounts.at(client.GetValue());
    for (auto& next : FLHook::Clients())
    {
        if (next.id != client && account.account._id == next.account->_id)
        {
            // If there's another player logged into this account, don't erase anything.
            // We ironically need to keep the loggedInAccounts data here to detect and *prevent* duplicate account login scenarios.
            return;
        }
    }
    loggedInAccounts.erase(account.account._id);
    account.characters.clear();
}
