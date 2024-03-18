#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"

#include "API/FLHook/Database.hpp"

#include <stduuid/uuid.h>

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
    data->reputationId = character.reputationId;
    data->interfaceState = character.interfaceState;

    data->baseCostume = character.baseCostume;
    data->commCostume = character.commCostume;

    Vector vec = { 0.0f, 0.0f, 0.0f };
    data->pos = character.pos.value_or(vec);
    data->rot = EulerMatrix(character.rot.value_or(vec));

    for (const auto rep : character.reputation)
    {
        Reputation::Relation relation;
        relation.hash = rep.first;
        relation.reptuation = rep.second;
        data->repList.push_back(relation);
    }

    ushort id = 1;
#define AddCargo(cargo, list)        \
    EquipDesc equipDesc;             \
    equipDesc.id = id++;             \
    equipDesc.archId = cargo.id;     \
    equipDesc.mounted = false;       \
    equipDesc.count = cargo.amount;  \
    equipDesc.health = cargo.health; \
    list.push_back(equipDesc);

#define AddEquip(equip, list)          \
    EquipDesc equipDesc;               \
    equipDesc.id = id++;               \
    equipDesc.archId = equip.id;       \
    equipDesc.health = equip.health;   \
    equipDesc.mounted = equip.mounted; \
    equipDesc.hardPoint.value = StringAlloc(equip.hardPoint.c_str(), false);

    for (const auto& cargo : character.cargo)
    {
        AddCargo(cargo, data->currentEquipAndCargo);
    }

    for (const auto& equip : character.equipment)
    {
        AddEquip(equip, data->currentEquipAndCargo);
    }

    id = 1;
    for (const auto& cargo : character.baseCargo)
    {
        AddCargo(cargo, data->baseEquipAndCargo);
    }

    for (const auto& equip : character.baseEquipment)
    {
        AddEquip(equip, data->baseEquipAndCargo);
    }

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

Character ConvertVanillaDataToCharacter(VanillaLoadData* data)
{
    Character character;

    std::wstring wCharName = reinterpret_cast<const wchar_t*>(data->name.c_str());
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
    character.reputationId = data->reputationId;
    character.interfaceState = data->interfaceState;

    character.commCostume = data->commCostume;
    character.baseCostume = data->baseCostume;

    SYSTEMTIME sysTime;
    FILETIME fileTime;
    GetLocalTime(&sysTime);
    SystemTimeToFileTime(&sysTime, &fileTime);
    character.totalTimePlayed = static_cast<int64>(fileTime.dwHighDateTime) << 32 | fileTime.dwLowDateTime;

    Vector vec = { 0.0f, 0.0f, 0.0f };
    character.pos = data->pos;
    character.rot = data->rot.ToEuler(true);

    for (const auto& rep : data->repList)
    {
        character.reputation.insert({ rep.hash, rep.reptuation });
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

    return character;
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
    const auto& account = accounts[clientId - 1];

    for (auto& character : account.characters)
    {
        // Freelancer uses a temporary buffer to hold the character data
        // We need to emulate this behaviour
        static std::array<char, 512> characterLoadingBuffer;                          // Statically preallocate it
        std::memset(characterLoadingBuffer.data(), 0, characterLoadingBuffer.size()); // Ensure that the buffer is empty every time

        // Copy the character name into the buffer
        memcpy_s(characterLoadingBuffer.data(), characterLoadingBuffer.size(), character.characterName.c_str(), character.characterName.size());

        // Pass the buffer into the original function that we populate with our data
        auto* loadData = static_cast<VanillaLoadData*>(createCharacterLoadingData(reinterpret_cast<PlayerData*>(&data->x050), characterLoadingBuffer.data()));

        // Copy the data from our DB type to the internal type
        ConvertCharacterToVanillaData(loadData, character);
    }

    auto& internalAccount = accounts[clientId - 1];

    data->numberOfCharacters = account.characters.size();

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
    data->onlineId = clientId;
    data->exitedBase = 0;

    return LoginReturnCode::Success;
}

void AccountManager::LoadNewPlayerFLInfo()
{
    INI_Reader ini;
    if (!ini.open("mpnewplayer.fl", false) || !ini.find_header("Player"))
    {
        // TODO: Log
        return;
    }

    while (ini.read_value())
    {
        std::string key = ini.get_name();
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
        else if (key == "%%PACKAGE%%")
        {
            newPlayerTemplate.hasPackage = true;
        }
    }

    if (!newPlayerTemplate.hasPackage)
    {
        // TODO: Log the absolute disaster this can cause
    }
}

bool AccountManager::OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* characterInfo)
{
    const auto db = NewChar::TheDB;

    const auto package = db->FindPackage(characterInfo->package);
    const auto faction = db->FindFaction(characterInfo->nickName);
    const auto pilot = db->FindPilot(characterInfo->pilot);
    const auto base = db->FindBase(characterInfo->base);

    if (!package || !faction || !pilot || !base)
    {
        return false;
    }

    static std::array<char, 512> characterNameBuffer;
    std::memset(characterNameBuffer.data(), 0, characterNameBuffer.size());
    memcpy_s(characterNameBuffer.data(), characterNameBuffer.size(), characterInfo->charname, sizeof(characterInfo->charname));
    auto* loadData = static_cast<VanillaLoadData*>(createCharacterLoadingData(reinterpret_cast<PlayerData*>(&data->x050), characterNameBuffer.data()));

    loadData->currentBase = newPlayerTemplate.base == "%%HOME_BASE%%" ? characterInfo->base : CreateID(newPlayerTemplate.base.c_str());
    // TODO: System needs to be reverse engineered. loadData->system = newPlayerTemplate.system == "%%HOME_SYSTEM%%%" ? character
    loadData->name = reinterpret_cast<unsigned short*>(characterInfo->charname);

    const auto costDesc = GetCostumeDescriptions();
    costDesc->get_costume(pilot->body.c_str(), loadData->baseCostume);
    costDesc->get_costume(pilot->comm.c_str(), loadData->commCostume);

    if (newPlayerTemplate.hasPackage)
    {
        loadData->shipHash = CreateID(package->ship.c_str());
        loadData->money = package->money;

        const auto loadOut = Loadout::Get(CreateID(package->loadout.c_str()));

        // TODO: Verify this map traverse works.
        for (const EquipDesc* equip = loadOut->first; equip != loadOut->end; equip++)
        {
            loadData->currentEquipAndCargo.push_back(*equip);
            loadData->baseEquipAndCargo.push_back(*equip);
        }
    }

    auto& mongo = FLHook::GetDatabase();
    auto character = ConvertVanillaDataToCharacter(loadData);
    mongo.CreateCharacter(StringUtils::wstos(data->accId), character);

    return true;
}

bool AccountManager::OnPlayerSave(PlayerData* data) { return true; }

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

    // static auto lastAccountStr = (wchar_t**)(DWORD(GetModuleHandleA("server.dll")) + 0x84EFC);

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

    const auto mod = reinterpret_cast<DWORD>(GetModuleHandleA("server.dll"));

    LoadNewPlayerFLInfo();

    createCharacterLoadingData = reinterpret_cast<CreateCharacterLoadingData>(reinterpret_cast<DWORD>(createCharacterLoadingData) + mod);

    // Replace Server.dll PlayerDB init
    MemUtils::PatchAssembly(mod + 0x713FF, reinterpret_cast<void*>(mod + 0x714D2)); // Bypass reading folders for accounts
    // Create 256 CAccounts on startup, replace Login function entirely.

    onPlayerSaveDetour = std::make_unique<FunctionDetour<OnPlayerSaveType>>(reinterpret_cast<OnPlayerSaveType>(mod + 0x6C430));
    onPlayerSaveDetour->Detour(OnPlayerSave);

    dbInitDetour = std::make_unique<FunctionDetour<DbInitType>>(reinterpret_cast<DbInitType>(mod + 0x710C0)); // Detour the init function
    dbInitDetour->Detour(PlayerDbInitDetour);

    onCreateNewCharacterDetour = std::make_unique<FunctionDetour<OnCreateNewCharacterType>>(reinterpret_cast<OnCreateNewCharacterType>(mod + 0x6B790));
    onCreateNewCharacterDetour->Detour(OnCreateNewCharacter);

    // MemUtils::PatchAssembly(mod + 0x76940, reinterpret_cast<PVOID>(mod + 0x76BBA)); // Patch out CAccount::InitFromFolder
    //  PlayerDB::load_user_data hacks
    MemUtils::NopAddress(mod + 0x734DE, 0x6D534F1 - 0x6D534DE); // Don't call Access to look for a file

    std::array<byte, 2> removeStringCheck = { 0xB0, 0x01 }; // mov al, 1
    MemUtils::WriteProcMem(mod + 0x734FEE, removeStringCheck.data(), removeStringCheck.size());
    MemUtils::NopAddress(mod + 0x7352B, 0x6D53589 - 0x6D5352B);
    std::array<byte, 3> stackFix = { 0x83, 0xEC, 0x14 };
    MemUtils::WriteProcMem(mod + 0x7357A, stackFix.data(), stackFix.size());

    MemUtils::PatchAssembly(mod + 0x735A9, loadUserDataAssembly.getCode());

    // Patch out folder creation in create account
    MemUtils::NopAddress(mod + 0x72499, 0x6D5252C - 0x6D52499);
    MemUtils::NopAddress(mod + 0x725A6, 0x6D525FE - 0x6D525A6);

    MemUtils::PatchCallAddr(mod, 0x72697, CreateAccountInitFromFolderBypass);

    // Patch out IO in InitFromFolder
    MemUtils::PatchAssembly(mod + 0x76955, InitFromFolderIoBypass);
}

void AccountManager::DeleteCharacter(const std::wstring& characterName)
{
    //
    //
}

void AccountManager::Login(const std::wstring& info, const ClientId& client)
{
    auto& db = FLHook::GetDatabase();
    // TODO: Make accounts a dynamic config field
    auto accountsCollection = db.GetCollection("accounts");
    if (!accountsCollection.has_value())
    {
        return;
    }

    std::string accId = StringUtils::wstos(info);

    const auto accountBson = accountsCollection->GetItemByIdRaw(accId);
    if (!accountBson.has_value())
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_array;
        using bsoncxx::builder::basic::make_document;
        const auto doc = make_document(kvp("_id", accId));

        accountsCollection->InsertIntoCollection(doc.view());
        return;
    }

    auto& accountRaw = accountBson.value();
    auto accountResult = rfl::bson::read<Account>(accountRaw.view().data(), accountRaw.view().length());
    if (accountResult.error().has_value())
    {
        return;
    }

    const auto account = accountResult.value();
    auto& internalAcc = accounts[client.GetValue() - 1];
    internalAcc.account = account;
}
