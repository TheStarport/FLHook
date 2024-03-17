#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"

#include "API/FLHook/Database.hpp"

#include <stduuid/uuid.h>

VanillaLoadData ConvertCharacterToVanillaData(const Character& character)
{
  VanillaLoadData vanData;

  std::wstring wCharName = StringUtils::stows(character.characterName);
  vanData.name	= reinterpret_cast<const ushort*>(wCharName.c_str());

  vanData.shipHash = character.shipHash;
  vanData.money = character.money;
  vanData.numOfKills = character.killCount;
  vanData.numOfSuccessMissions = character.missionSuccessCount;
  vanData.numOfFailedMissions = character.missionFailureCount;
  vanData.hullStatus = character.hullStatus;
  vanData.baseHullStatus = character.baseHullStatus;
  vanData.currentBase = character.currentBase;
  vanData.lastDockedBase = character.lastDockedBase;
  vanData.currentRoom = character.currentRoom;
  vanData.system = character.system;
  vanData.rank = character.rank;
  vanData.reputationId = character.reputationId;
  vanData.interfaceState = character.interfaceState;


  Costume cos;
  vanData.commCostume = character.commCostume.value_or(cos);
  vanData.commCostume = character.commCostume.value_or(cos);

  Vector vec = {0.0f,0.0f,0.0f};
  vanData.pos = character.pos.value_or(vec);
  vanData.rot = EulerMatrix(character.rot.value_or(vec));


  for(const auto rep : character.reputation)
  {
	Reputation::Relation relation;
	relation.hash = rep.first;
	relation.reptuation = rep.second;
	vanData.repList.push_back(relation);
  }

  st6::list<EquipDesc> equipAndCargo;

  for(const auto& cargo : character.cargo)
  {
	EquipDesc equipDesc;

	equipDesc.id = cargo.id;
	equipDesc.mounted = false;
	equipDesc.count = cargo.amount;
	equipDesc.health = cargo.health;

	equipAndCargo.push_back(equipDesc);
  }
  for (const auto& equip : character.equipment)
  {
	EquipDesc equipDesc;

	equipDesc.id = equip.id;
	equipDesc.health = equip.health;
	equipDesc.mounted = equip.mounted;
	equipDesc.hardPoint.value = StringAlloc(equip.hardPoint.c_str(), false);

	equipAndCargo.push_back(equipDesc);
  }

  vanData.currentEquipAndCargo = equipAndCargo;

  st6::list<EquipDesc> baseEquipAndCargo;

  for(const auto& cargo : character.baseCargo)
  {
	EquipDesc equipDesc;

	equipDesc.id = cargo.id;
	equipDesc.mounted = false;
	equipDesc.count = cargo.amount;
	equipDesc.health = cargo.health;

	baseEquipAndCargo.push_back(equipDesc);
  }
  for (const auto& equip : character.baseEquipment)
  {
	EquipDesc equipDesc;

	equipDesc.id = equip.id;
	equipDesc.health = equip.health;
	equipDesc.mounted = equip.mounted;
	equipDesc.hardPoint.value = StringAlloc(equip.hardPoint.c_str(), false);


	baseEquipAndCargo.push_back(equipDesc);
  }
  vanData.currentEquipAndCargo = baseEquipAndCargo;

  for(const auto& col : character.collisionGroups)
  {
	CollisionGroupDesc colDesc = {};
	colDesc.id = col.first;
	colDesc.health = col.second;
	vanData.currentCollisionGroups.push_back(colDesc);
  }

  for(const auto& col : character.baseCollisionGroups)
  {
	CollisionGroupDesc colDesc = {};
	colDesc.id = col.first;
	colDesc.health = col.second;
	vanData.baseCollisionGroups.push_back(colDesc);
  }

  return vanData;
}

Character ConvertVanillaDataToCharacter(VanillaLoadData& vanData)
{
  Character character;

  std::wstring wCharName = reinterpret_cast<const wchar_t*>(vanData.name.c_str());
  character.shipHash = vanData.shipHash;
  character.money = vanData.money;
  character.killCount = vanData.numOfKills;
  character.missionSuccessCount = vanData.numOfSuccessMissions;
  character.missionFailureCount = vanData.numOfFailedMissions;
  character.hullStatus = vanData.hullStatus;
  character.baseHullStatus = vanData.baseHullStatus;
  character.currentBase	= vanData.currentBase;
  character.lastDockedBase = vanData.lastDockedBase;
  character.currentRoom = vanData.currentRoom;
  character.system = vanData.system;
  character.rank = vanData.rank;
  character.reputationId = vanData.reputationId;
  character.interfaceState = vanData.interfaceState;

  Costume cos;
  character.commCostume.value_or(cos) = vanData.commCostume;
  character.baseCostume.value_or(cos) = vanData.baseCostume;

  Vector vec = {0.0f,0.0f,0.0f};
  character.pos.value_or(vec) = vanData.pos;
  EulerMatrix(character.rot.value_or(vec)) = vanData.rot;

  for(const auto& rep: vanData.repList)
  {
	character.reputation.insert({rep.hash,rep.reptuation});
  }

  for (const auto& equip : vanData.currentEquipAndCargo)
  {
	Equipment equipment = {};
	FLCargo cargo = {};

	bool isCommodity = false;
	pub::IsCommodity(equip.archId, isCommodity);
	if(!isCommodity)
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

  for (const auto& equip : vanData.baseEquipAndCargo)
  {
	Equipment equipment = {};
	FLCargo cargo = {};

	bool isCommodity = false;
	pub::IsCommodity(equip.archId, isCommodity);
	if(!isCommodity)
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

  for(const auto& col : vanData.currentCollisionGroups)
  {
	character.collisionGroups.insert({col.id,col.health});
  }
    for(const auto& col : vanData.baseCollisionGroups)
  {
	character.baseCollisionGroups.insert({col.id,col.health});
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

void AccountManager::LoadCharacter(VanillaLoadData* data, std::wstring_view characterName)
{
    // DWORD server = DWORD(GetModuleHandleA("server.dll"));
    // loadDetour.UnDetour();
    // auto result = loadDetour.GetOriginalFunc()(unk, edx, filename, data);
    // loadDetour.Detour(LoadDetour);
    //  return result;

    data->baseCostume = {
        2223155968, 3144214861, 2479975689, 2264565644, {0, 0, 0, 0, 0, 0, 0, 0},
            0
    };
    data->commCostume = {
        2223155968, 3144214861, 2479975689, 2264565644, {0, 0, 0, 0, 0, 0, 0, 0},
            0
    };
    data->money = 42069;
    data->rank = 69;
    data->baseHullStatus = 0.9f;
    data->hullStatus = 0.8f;
    data->currentBase = 0;
    data->pos = { 30000.0f, 0, -25000.0f };
    data->rot = EulerMatrix({ 0.0f, 0.0f, 0.0f });
    data->datetimeHigh = 0;
    data->datetimeLow = 0;
    data->descripStrId = 0;
    data->name = reinterpret_cast<const unsigned short*>(characterName.data());
    data->description = reinterpret_cast<const unsigned short*>(L"08/16/23 19:29:11");
    data->interfaceState = 1;
    data->lastDockedBase = CreateID("Li01_01_Base");
    data->currentRoom = 0;
    data->numOfKills = 5;
    data->numOfFailedMissions = 6;
    data->numOfSuccessMissions = 7;
    data->shipHash = CreateID("co_elite2");
    Archetype::GetShip(data->shipHash)->get_undamaged_collision_group_list(data->baseCollisionGroups);
    Archetype::GetShip(data->shipHash)->get_undamaged_collision_group_list(data->currentCollisionGroups);
    data->system = CreateID("li03");
    const auto voice = "trent_voice";
    memcpy(data->voice, voice, strlen(voice) + 1);
    data->voiceLen = 11;
    /*{
        EquipDesc powerplant;
        powerplant.mounted = true;
        powerplant.health = 1;
        powerplant.archId = CreateID("li_elite_power01");
        powerplant.count = 1;
        powerplant.id = 1;
        powerplant.make_internal();

        EquipDesc engine;
        engine.mounted = true;
        engine.health = 1;
        engine.archId = CreateID("ge_le_engine_01");
        engine.count = 1;
        engine.id = 2;
        engine.make_internal();

        EquipDesc scanner;
        scanner.mounted = true;
        scanner.health = 1;
        scanner.archId = CreateID("ge_s_scanner_01");
        scanner.count = 1;
        scanner.id = 34;
        scanner.make_internal();

        EquipDesc tractorBeam;
        tractorBeam.mounted = true;
        tractorBeam.health = 1;
        tractorBeam.archId = CreateID("ge_s_tractor_01");
        tractorBeam.count = 1;
        tractorBeam.id = 35;
        tractorBeam.make_internal();

        EquipDesc shield;
        shield.set_equipped(true);
        const char* shieldHPName = "HpShield01";
        CacheString str((PCHAR)shieldHPName);
        shield.set_hardpoint(str);
        shield.set_status(1.0f);
        shield.set_arch_id(CreateID("shield03_mark04_hf"));

        data->baseEquipAndCargo.push_back(powerplant);
        data->baseEquipAndCargo.push_back(engine);
        data->baseEquipAndCargo.push_back(scanner);
        data->baseEquipAndCargo.push_back(tractorBeam);
        data->baseEquipAndCargo.push_back(shield);

        data->currentEquipAndCargo.push_back(powerplant);
        data->currentEquipAndCargo.push_back(engine);
        data->currentEquipAndCargo.push_back(scanner);
        data->currentEquipAndCargo.push_back(tractorBeam);
        data->currentEquipAndCargo.push_back(shield);
    }*/

    {
        EquipDesc powerplant;

        const uint archId = CreateID("li_elite_power01");
        CacheString hardpointName;
        hardpointName.value = (char*)"";
        const float health = 1.0f;

        powerplant.set_equipped(true);
        powerplant.set_count(1);
        powerplant.set_arch_id(archId);
        if (strlen(hardpointName.value) == 0)
        {
            powerplant.make_internal();
        }
        else
        {
            powerplant.set_hardpoint(hardpointName);
        }
        if (health != 0.0f)
        {
            powerplant.set_status(health);
        }

        data->baseEquipAndCargo.push_back(powerplant);
        data->currentEquipAndCargo.push_back(powerplant);
    }

    {
        const bool isMission = false;
        const int count = 5;
        const uint archId = CreateID("commodity_gold");
        EquipDesc cargo;
        CacheString hardpointName;
        hardpointName.value = (char*)"";
        const float health = 0.5f;

        cargo.set_equipped(false);
        cargo.set_arch_id(archId);
        if (count)
        {
            cargo.set_count(count);
        }
        if (strlen(hardpointName.value) == 0)
        {
            cargo.make_internal();
        }
        else
        {
            cargo.set_hardpoint(hardpointName);
        }
        if (health != 0.0f)
        {
            cargo.set_status(health);
        }
        if (isMission)
        {
            cargo.mission = isMission;
        }

        data->baseEquipAndCargo.push_back(cargo);
        data->currentEquipAndCargo.push_back(cargo);
    }

    {
        const Archetype::Ship* ship = Archetype::GetShip(data->shipHash);

        auto cg = ship->collisionGroup;
        while (cg)
        {
            if (std::string(cg->name.value) == "wing_port_lod1")
            {
                for (auto& colGrp : data->currentCollisionGroups)
                {
                    if (colGrp.id == cg->id)
                    {
                        // set health
                        colGrp.health = 0.5f;
                        break;
                    }
                }
                break;
            }
            cg = cg->next;
        }
    }

    EquipDesc engine;
    engine.mounted = true;
    engine.health = 1;
    engine.archId = CreateID("ge_le_engine_01");
    engine.count = 1;
    engine.id = 2;
    engine.make_internal();

    EquipDesc scanner;
    scanner.mounted = true;
    scanner.health = 1;
    scanner.archId = CreateID("ge_s_scanner_01");
    scanner.count = 1;
    scanner.id = 34;
    scanner.make_internal();

    EquipDesc tractorBeam;
    tractorBeam.mounted = true;
    tractorBeam.health = 1;
    tractorBeam.archId = CreateID("ge_s_tractor_01");
    tractorBeam.count = 1;
    tractorBeam.id = 35;
    tractorBeam.make_internal();

    data->currentEquipAndCargo.push_back(engine);
    data->currentEquipAndCargo.push_back(tractorBeam);
    data->currentEquipAndCargo.push_back(scanner);
    data->baseEquipAndCargo.push_back(scanner);
    data->baseEquipAndCargo.push_back(tractorBeam);
    data->baseEquipAndCargo.push_back(engine);

    data->tempCargoIdEnumerator = 37;

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

AccountManager::LoginReturnCode __stdcall AccountManager::AccountLoginInternal(PlayerData* data, const uint clientId)
{
    // TODO: Log account login

    // foreach character
    std::string name = "e";
    auto i = 0u;
    for (; i < 2; i++)
    {
        static std::array<char, 512> characterLoadingBuffer;
        std::memset(characterLoadingBuffer.data(), 0, characterLoadingBuffer.size());
        memcpy_s(characterLoadingBuffer.data(), characterLoadingBuffer.size(), name.c_str(), name.size());
        auto* loadData = static_cast<VanillaLoadData*>(createCharacterLoadingData(reinterpret_cast<PlayerData*>(&data->x050), characterLoadingBuffer.data()));
        LoadCharacter(loadData, StringUtils::stows(name));
        name += "e";
    }

    data->numberOfCharacters = i;

    data->systemId = 0;
    data->shipId = 0;
    data->baseId = 0;
    data->characterId = 0;
    data->lastBaseId = 0;
    data->lastEquipId = 0;
    data->createdShipId = 0;
    data->baseRoomId = 0;

    data->account = accounts[clientId - 1].account;
    data->account->dunno4[1] = clientId;
    wcscpy_s(data->accId, instance->currentAccountString.c_str());
    data->onlineId = clientId;
    data->exitedBase = 0;

    data->account->numberOfCharacters = i;

    instance->currentAccountString = L"";

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
            // TODO: look up and validate system
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

bool AccountManager::OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* character)
{
    const auto db = NewChar::TheDB;

    const auto package = db->FindPackage(character->package);
    const auto faction = db->FindFaction(character->nickName);
    const auto pilot = db->FindPilot(character->pilot);
    const auto base = db->FindBase(character->base);

    if (!package || !faction || !pilot || !base)
    {
        return false;
    }

    static std::array<char, 512> characterNameBuffer;
    std::memset(characterNameBuffer.data(), 0, characterNameBuffer.size());
    memcpy_s(characterNameBuffer.data(), characterNameBuffer.size(), character->charname, sizeof(character->charname));
    auto* loadData = static_cast<VanillaLoadData*>(createCharacterLoadingData(reinterpret_cast<PlayerData*>(&data->x050), characterNameBuffer.data()));

    loadData->currentBase = newPlayerTemplate.base == "%%HOME_BASE%%" ? character->base : CreateID(newPlayerTemplate.base.c_str());
    // TODO: System needs to be reverse engineered. loadData->system = newPlayerTemplate.system == "%%HOME_SYSTEM%%%" ? character
    loadData->name = reinterpret_cast<unsigned short*>(character->charname);

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

   // auto& mongo = FLHook::GetDatabase();
   // mongo.CreateCharacter(StringUtils::wstos(data->accId), loadData);

    LoadCharacter(loadData, character->charname);
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

    //static auto lastAccountStr = (wchar_t**)(DWORD(GetModuleHandleA("server.dll")) + 0x84EFC);

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
        if (!acc.account)
        {
            account->accId[36] = L'\0';
            account->accId[37] = L'\0';
            account->accId[38] = L'\0';
            account->accId[39] = L'\0';
            account->dunno2[0] = 36;
            acc.account = account;
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

void AccountManager::OnLogin(const ClientId& client) { currentAccountString = accounts[client.GetValue() - 1].account->accId; }
