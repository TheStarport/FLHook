#include "PCH.hpp"

#include "Core/IEngineHook.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/ResourceManager.hpp"

IEngineHook::CallAndRet::CallAndRet(void* toCall, void* ret)
{
    push(ecx); // Reserve ecx
    push(ecx);
    call(toCall);
    pop(ecx); // Restore ecx
    jmp(ret);
}

void IEngineHook::Init()
{
    INI_Reader ini;
    std::string factionPropLocation = R"(..\DATA\MISSIONS\faction_prop.ini)";

    if (!ini.open(factionPropLocation.c_str(), false))
    {
        return;
    }

    while (ini.read_header())
    {
        if (!ini.is_header("FactionProps"))
        {
            continue;
        }

        uint currentAff;
        SendCommData::FactionData facData{};

        while (ini.read_value())
        {
            if (ini.is_value("affiliation"))
            {
                currentAff = MakeId(ini.get_value_string());
            }
            else if (ini.is_value("msg_id_prefix"))
            {
                facData.msgId = CreateID(ini.get_value_string());
            }
            else if (ini.is_value("formation_desig"))
            {
                const int start = ini.get_value_int(0);
                const int end = ini.get_value_int(1);

                for (int i = start; i <= end; i++)
                {
                    char buf[50];
                    const int number = i - SendCommData::Callsign::AlphaDesignation + 1;
                    sprintf_s(buf, "gcs_refer_formationdesig_%02d", number);
                    facData.formationHashes.emplace_back(CreateID(buf));
                }
            }
        }

        sendCommData.factions[currentAff] = facData;
    }

    ini.close();

    for (int i = 1; i < 20; i++)
    {
        uint number1 = CreateID(std::format("gcs_misc_number_{}", i).c_str());
        uint number2 = CreateID(std::format("gcs_misc_number_{}-", i).c_str());
        sendCommData.numberHashes[i] = { number1, number2 };
    }
}

int IEngineHook::FreeReputationVibe(const int& p1)
{
    using FreeRepFunc = void (*)(const int& p);
    static auto freeRep = reinterpret_cast<FreeRepFunc>(reinterpret_cast<DWORD>(FLHook::serverDll) + 0x65C20);
    freeRep(p1);
    return Reputation::Vibe::Free(p1);
}

void IEngineHook::UpdateTime(const double interval) { Timing::UpdateGlobalTime(interval); }

static void* dummy;

void __stdcall IEngineHook::ElapseTime(const float interval)
{
    // TODO: Figure out if this is even needed
    dummy = &Server;
    Server.ElapseTime(interval);

    // low server load missile jitter bug fix
    const uint curLoad = GetTickCount() - lastTicks;
    if (curLoad < 5)
    {
        const uint fakeLoad = 5 - curLoad;
        Sleep(fakeLoad);
    }
    lastTicks = GetTickCount();
}

int IEngineHook::DockCall(const uint& shipId, const uint& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
{
    //	dockPortIndex == -1, response -> 2 --> Dock Denied!
    //	dockPortIndex == -1, response -> 3 --> Dock in Use
    //	dockPortIndex != -1, response -> 4 --> Dock ok, proceed (dockPortIndex starts from 0 for docking point 1)
    //	dockPortIndex == -1, response -> 5 --> now DOCK!

    auto [override, skip] = CallPlugins<std::optional<DOCK_HOST_RESPONSE>>(&Plugin::OnDockCall, ShipId(shipId), ObjectId(spaceId), dockPortIndex, response);

    if (skip)
    {
        return 0;
    }

    if (override.has_value())
    {
        response = override.value();
    }

    int retVal = 0;
    TryHook
    {
        // Print out a message when a player ship docks.
        if (FLHook::GetConfig()->chatConfig.dockingMessages && response == DOCK_HOST_RESPONSE::Dock)
        {
            if (const auto client = ShipId(shipId).GetPlayer().Unwrap())
            {
                std::wstring msg = L"Traffic control alert: %player has docked.";
                msg = StringUtils::ReplaceStr(msg, std::wstring_view(L"%player"), client.GetCharacterId().Unwrap().GetValue());
                (void)client.MessageLocal(msg, 15000.0f);
            }
        }
        // Actually dock
        retVal = pub::SpaceObj::Dock(shipId, spaceId, dockPortIndex, response);
    }
    CatchHook({})

        CallPlugins(&Plugin::OnDockCallAfter, ShipId(shipId), ObjectId(spaceId), dockPortIndex, response);

    return retVal;
}

bool __stdcall IEngineHook::LaunchPosition(const uint spaceId, CEqObj& obj, Vector& position, Matrix& orientation, const int dock)
{
    const LaunchData data = { &obj, position, orientation, dock };
    if (auto [retVal, skip] = CallPlugins<std::optional<LaunchData>>(&Plugin::OnLaunchPosition, ObjectId(spaceId), data); skip && retVal.has_value())
    {
        const auto& val = retVal.value();
        position = val.pos;
        orientation = val.orientation;
        return true;
    }

    return obj.launch_pos(position, orientation, dock);
}

void __fastcall IEngineHook::CShipInit(CShip* ship, void* edx, CShip::CreateParms* creationParams)
{
    using CShipInitType = void(__thiscall*)(CShip*, CShip::CreateParms*);
    static_cast<CShipInitType>(cShipVTable.GetOriginal(static_cast<ushort>(CShipVTable::InitCShip)))(ship, creationParams);

    CallPlugins(&Plugin::OnCShipInitAfter, ship);
}

void __fastcall IEngineHook::CSolarInit(CSolar* solar, void* edx, CSolar::CreateParms* creationParams)
{
    using CSolarInitType = void(__thiscall*)(CSolar*, CSolar::CreateParms*);
    static_cast<CSolarInitType>(cSolarVTable.GetOriginal(static_cast<ushort>(CSolarVTable::InitCSolar)))(solar, creationParams);

    CallPlugins(&Plugin::OnCSolarInitAfter, solar);
}

void __fastcall IEngineHook::CLootInit(CLoot* loot, void* edx, CLoot::CreateParms* creationParams)
{
    using CLootInitType = void(__thiscall*)(CLoot*, CLoot::CreateParms*);
    static_cast<CLootInitType>(cLootVTable.GetOriginal(static_cast<ushort>(CLootVTable::InitCLoot)))(loot, creationParams);

    CallPlugins(&Plugin::OnCLootInitAfter, loot);
}

void __fastcall IEngineHook::CGuidedInit(CGuided* guided, void* edx, CGuided::CreateParms* creationParams)
{
    using CGuidedInitType = void(__thiscall*)(CGuided*, CGuided::CreateParms*);
    static_cast<CGuidedInitType>(cGuidedVTable.GetOriginal(static_cast<ushort>(CGuidedVTable::InitCEquipObject)))(guided, creationParams);

    CallPlugins(&Plugin::OnCGuidedInitAfter, guided);
}

int __fastcall IEngineHook::GetAmmoCapacityDetourHash(CShip* cship, void* edx, Id ammoArch)
{
    if (auto [retVal, skip] = CallPlugins<int>(&Plugin::OnGetAmmoCapacity, cship, ammoArch); skip)
    {
        return retVal;
    }

    using CShipGetAmmoCapacityType = int(__thiscall*)(CShip*, Id);
    int origCallVal =
        reinterpret_cast<CShipGetAmmoCapacityType>(FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonCShipGetAmmoCapacity))(cship, ammoArch);

    return origCallVal;
}

int __fastcall IEngineHook::GetAmmoCapacityDetourEq(CShip* cship, void* edx, Archetype::Equipment* ammoType)
{
    return GetAmmoCapacityDetourHash(cship, edx, ammoType->archId);
}

TractorFailureCode __fastcall IEngineHook::CETractorVerifyTarget(CETractor* tractor, void* edx, CLoot* target)
{
    using CETractorVerifyTargetType = TractorFailureCode(__thiscall*)(CETractor*);
    TractorFailureCode origCallVal =
        reinterpret_cast<CETractorVerifyTargetType>(FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonCETractorVerifyTarget))(tractor);

    if (auto [retVal, skip] = CallPlugins<TractorFailureCode>(&Plugin::OnTractorVerifyTarget, tractor, origCallVal); skip)
    {
        return retVal;
    }

    return origCallVal;
}

float __fastcall IEngineHook::GetCargoRemaining(CShip* cship)
{
    if (auto [retVal, skip] = CallPlugins<float>(&Plugin::OnGetCargoRemaining, cship); skip)
    {
        return retVal;
    }

    using CShipGetCargoRemainingType = float(__thiscall*)(CShip*);
    int origCallVal = reinterpret_cast<CShipGetCargoRemainingType>(FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonCShipGetAmmoCapacity))(cship);

    return origCallVal;
}

int __fastcall IEngineHook::GetSpaceForCargoType(CShip* cship, void* edx, Archetype::Equipment* archEquip)
{
    if (auto [retVal, skip] = CallPlugins<float>(&Plugin::OnGetSpaceForCargoType, cship, archEquip); skip)
    {
        return retVal;
    }

    using CShipGetSpaceForCargoType = int(__thiscall*)(CShip*, Archetype::Equipment*);
    int origCallVal =
        reinterpret_cast<CShipGetSpaceForCargoType>(FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonCShipGetSpaceForCargoType))(cship, archEquip);

    return origCallVal;
}

FireResult __fastcall IEngineHook::CELauncherFireAfter(CELauncher* launcher, void* edx, const Vector& pos)
{
    using CELauncherFireType = FireResult(__thiscall*)(CELauncher*, const Vector&);
    FireResult retVal = static_cast<CELauncherFireType>(ceLauncherVTable.GetOriginal(static_cast<ushort>(CELauncherVTable::Fire)))(launcher, pos);

    CallPlugins(&Plugin::OnCELauncherFireAfter, launcher, pos, retVal);

    return retVal;
}

IEngineHook::LaunchPositionAssembly::LaunchPositionAssembly()
{
    push(ecx);
    push(dword[esp + 16]);
    push(dword[esp + 16]);
    push(dword[esp + 16]);
    push(ecx);
    push(dword[ecx + 176]);
    call(LaunchPosition);
    pop(ecx);
    ret(0xC);
}

auto __stdcall IEngineHook::LoadReputationFromCharacterFile(const RepDataList* savedReps, const LoadRepData* repToSave) -> bool
{
    // check of the rep id is valid
    if (repToSave->repId == 0xFFFFFFFF)
    {
        return false; // rep id not valid!
    }

    const LoadRepData* repIt = savedReps->begin;

    while (repIt != savedReps->end)
    {
        if (repIt->repId == repToSave->repId)
        {
            return false; // we already saved this rep!
        }

        repIt++;
    }

    // everything seems fine, add
    return true;
}

IEngineHook::LoadReputationFromCharacterFileAssembly::LoadReputationFromCharacterFileAssembly()
{
    push(ecx);
    push(dword[esp + 16]);
    push(ecx);
    call(LoadReputationFromCharacterFile);
    pop(ecx);
    test(al, al);
    jz(".abort");
    jmp(oldLoadReputationFromCharacterFile);

    inLocalLabel();
    L(".abort");
    ret(0xC);
}

int IEngineHook::SendCommDetour(const uint sender, uint receiver, const uint voiceId, const Costume* costume, const uint infocardId, uint* lines,
                                const int lineCount, const uint infocardId2, const float radioSilenceTimerAfter, const bool global)
{
    static uint freelancerHash = CreateID("gcs_refer_faction_player_short");
    static BYTE origMemory[] = { 0x8B, 0x44, 0x24, 0x18, 0x83 };
    static BYTE currMemory[5];
    memcpy(currMemory, pub::SpaceObj::SendComm, sizeof(currMemory));

    auto& playerMap = FLHook::GetResourceManager()->playerShips;
    if (auto client = playerMap.find(receiver); client != playerMap.end())
    {
        auto cd = sendCommData.callsigns.find(client->second.GetValue());
        if (cd != sendCommData.callsigns.end())
        {
            for (int i = 0; i < lineCount; ++i)
            {
                if (lines[i] != SendCommData::Callsign::FreelancerCommHash)
                {
                    continue;
                }

                if (i + 4 > lineCount)
                {
                    break;
                }

                lines[i] = cd->second.factionLine;
                lines[i + 1] = cd->second.formationLine;
                lines[i + 2] = cd->second.number1;
                lines[i + 4] = cd->second.number2;

                break;
            }
        }
    }

    memcpy(pub::SpaceObj::SendComm, origMemory, sizeof(origMemory));
    const int retVal = pub::SpaceObj::SendComm(sender, receiver, voiceId, costume, infocardId, lines, lineCount, infocardId2, radioSilenceTimerAfter, global);
    memcpy(pub::SpaceObj::SendComm, currMemory, sizeof(currMemory));

    return retVal;
}
