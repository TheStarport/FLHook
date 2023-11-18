#include "PCH.hpp"

#include "API/Types/ClientId.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "Core/FLHook.hpp"

#define ClientCheck                                   \
    if (!IsValidClientId())                           \
    {                                                 \
        return { cpp::fail(Error::InvalidClientId) }; \
    }

#define CharSelectCheck                                    \
    if (InCharacterSelect())                               \
    {                                                      \
        return { cpp::fail(Error::CharacterNotSelected) }; \
    }

Action<void, Error> ClientId::AdjustCash(const int amount) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::AdjustCash(value, amount);
    return { {} };
}

bool ClientId::IsValidClientId() const
{
    if (value == 0 || value > MaxClientId)
    {
        return false;
    }

    return FLHook::Clients()[value].isValid;
}

uint ClientId::GetClientIdFromCharacterName(std::wstring_view name)
{
    st6::wstring st6Name{ (unsigned short*)name.data(), name.size() };
    const auto account = Players.FindAccountFromCharacterName(st6Name);
    if (!account)
    {
        return 0;
    }

    uint client = 0;
    PlayerData* playerDb = nullptr;
    while ((playerDb = Players.traverse_active(playerDb)))
    {
        if (playerDb->account == account)
        {
            client = playerDb->onlineId;
            break;
        }
    }

    if (!client || ClientId(client).InCharacterSelect())
    {
        return 0;
    }

    if (const auto newCharacter = ClientId(client).GetCharacterName().Unwrap(); StringUtils::ToLower(newCharacter) != StringUtils::ToLower(name))
    {
        return 0;
    }

    return client;
}

ClientId::operator bool() const
{
    if (value > 0 || value < MaxClientId)
    {
        return false;
    }

    for (auto& client : FLHook::Clients())
    {
        if (client.id.GetValue() == value)
        {
            return true;
        }
    }

    return false;
}

Action<BaseId, Error> ClientId::GetCurrentBase() const
{
    ClientCheck;
    CharSelectCheck;

    uint base;
    pub::Player::GetBase(value, base);
    if (base)
    {
        return { BaseId(base) };
    }

    return { cpp::fail(Error::PlayerNotDocked) };
}

Action<SystemId, Error> ClientId::GetSystemId() const
{
    ClientCheck;
    CharSelectCheck;

    uint sys;
    pub::Player::GetSystem(value, sys);

    if (!sys)
    {
        return { cpp::fail(Error::InvalidSystem) };
    }
    return { SystemId(sys) };
}

Action<AccountId, Error> ClientId::GetAccount() const
{
    ClientCheck;

    return { AccountId(*this) };
}

Action<const Archetype::Ship*, Error> ClientId::GetShipArch() const
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShipID(value, ship);

    if (!ship)
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    return { Archetype::GetShip(ship) };
}

Action<ShipId, Error> ClientId::GetShipId() const
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    return cpp::result<ShipId, Error>(ShipId(ship));
}

Action<uint, Error> ClientId::GetLatency() const
{
    // TODO: Implement latency func when ConData is slated.
    return { 0u };
}

Action<CPlayerGroup*, Error> ClientId::GetGroup() const
{
    ClientCheck;
    const auto id = Players.GetGroupID(value);

    if (!id)
    {
        return { cpp::fail(Error::PlayerNotInGroup) };
    }

    return { CPlayerGroup::FromGroupID(id) };
}

Action<RepId, Error> ClientId::GetReputation() const
{
    ClientCheck;
    CharSelectCheck;

    int rep;
    pub::Player::GetRep(value, rep);

    if (!Reputation::Vibe::Verify(rep))
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { RepId(rep) };
}

Action<CShip*, Error> ClientId::GetShip() const
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return { cpp::fail(Error::PlayerNotInSpace) };
    }

    return { dynamic_cast<CShip*>(CObject::Find(ship, CObject::CSHIP_OBJECT)) };
}

Action<uint, Error> ClientId::GetRank() const
{
    ClientCheck;
    CharSelectCheck;
    int rank = 0;
    pub::Player::GetRank(value, rank);
    return { rank };
}

Action<uint, Error> ClientId::GetWealth() const
{
    ClientCheck;
    CharSelectCheck;

    // Cause Freelancer decided wealth should be accessed as a float.(Despite it being stored as an integer)
    float wealth;
    pub::Player::GetAssetValue(value, wealth);
    return { static_cast<uint>(wealth) };
}

Action<int, Error> ClientId::GetPvpKills() const
{
    ClientCheck;
    CharSelectCheck;

    int kills;
    pub::Player::GetNumKills(value, kills);
    return { kills };
}

Action<uint, Error> ClientId::GetCash() const
{
    ClientCheck;
    CharSelectCheck;

    int cash;
    pub::Player::InspectCash(value, cash);
    return { cash };
}

Action<std::wstring_view, Error> ClientId::GetCharacterName() const
{
    ClientCheck;
    CharSelectCheck;

    const auto player = Players.GetActiveCharacterName(value);
    const auto wcharPlayer = reinterpret_cast<const wchar_t*>(player);

    return { std::wstring_view(wcharPlayer, wcsnlen_s(wcharPlayer, 36)) };
}

bool ClientId::InSpace() const
{
    if (InCharacterSelect())
    {
        return false;
    }

    uint baseId = 0;
    pub::Player::GetBase(value, baseId);

    if (!baseId)
    {
        return true;
    }
    return false;
}

bool ClientId::IsDocked() const
{
    if (InCharacterSelect())
    {
        return false;
    }

    return !InSpace();
}

bool ClientId::InCharacterSelect() const { return Players.GetActiveCharacterName(value) == nullptr; }

bool ClientId::IsAlive() const
{
    const bool charMenu = InCharacterSelect();
    const bool docked = IsDocked();
    uint ship;
    pub::Player::GetShip(value, ship);

    // If they are not in char menu, docked, or have a valid ship id they are probably dead in space.
    if (!(charMenu || ship || docked))
    {
        return false;
    }

    return true;
}

Action<std::list<CargoInfo>, Error> ClientId::EnumCargo(int& remainingHoldSize) const
{
    ClientCheck;
    CharSelectCheck;

    std::list<CargoInfo> cargo;

    char* classPtr;
    memcpy(&classPtr, &Players, 4);
    classPtr += 0x418 * (value - 1);

    pub::SpaceObj::EquipItem* eqList;
    memcpy(&eqList, classPtr + 0x27C, 4);
    const pub::SpaceObj::EquipItem* eq = eqList->next;
    while (eq != eqList)
    {
        CargoInfo ci = { eq->id, static_cast<int>(eq->count), eq->goodId, eq->status, eq->mission, eq->mounted, eq->hardpoint };
        cargo.push_back(ci);

        eq = eq->next;
    }

    float remainingHold;
    pub::Player::GetRemainingHoldSize(value, remainingHold);
    remainingHoldSize = static_cast<int>(remainingHold);
    return { cargo };
}

ClientData& ClientId::GetData() const { return FLHook::Clients()[value]; }

Action<std::wstring, Error> ClientId::GetPlayerIp() const
{
    const CDPClientProxy* cdpClient = FLHook::clientProxyArray[value - 1];
    // clang-format off
    if (!cdpClient)
    {
    
        // I hate labels, but it allows our inline asm to escape properly
        invalid:
        return { cpp::fail(Error::InvalidClientId) };
        // clang-format on
    }

    // get ip
    char* p1;
    char* address;
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    static const wchar_t hostname[] = L"hostname";
    memcpy(&p1, reinterpret_cast<void*>(reinterpret_cast<DWORD>(FLHook::cdpServer) + 4), 4);

    wchar_t wIp[1024] = L"";
    long sizeOfIp = sizeof wIp;
    long dataType = 1;
    __asm {
		push 0 ; flags
		lea edx, address
		push edx ; address
		mov edx, [cdpClient]
		mov edx, [edx+8]
		push edx ; dpnid
		mov eax, [p1]
		push eax
		mov ecx, [eax]
		call dword ptr[ecx + 0x28] ; GetClientAddress
		cmp eax, 0
		jnz invalid

		lea eax, dataType
		push eax
		lea eax, sizeOfIp
		push eax
		lea eax, wIp
		push eax
		lea eax, hostname
		push eax
		mov ecx, [address]
		push ecx
		mov ecx, [ecx]
		call dword ptr[ecx+0x40] ; GetComponentByName

		mov ecx, [address]
		push ecx
		mov ecx, [ecx]
		call dword ptr[ecx+0x08] ; Release
    }

    return { std::wstring(wIp) };
}

EngineState ClientId::GetEngineState() const
{
    const auto& data = GetData();

    if (!data.ship)
    {
        return EngineState::NotInSpace;
    }

    if (data.inTradelane)
    {
        return EngineState::Tradelane;
    }

    if (data.cruiseActivated)
    {
        return EngineState::Cruise;
    }

    if (data.thrusterActivated)
    {
        return EngineState::Thruster;
    }

    if (!data.engineKilled)
    {
        return EngineState::Engine;
    }

    return EngineState::Killed;
}

// Server wide message upon kicking. Delay is defaulted to zero.
Action<void, Error> ClientId::Kick(const std::optional<std::wstring_view>& reason, const std::optional<uint> delay) const
{
    ClientCheck;

    if (reason.has_value())
    {
        const std::wstring msg = StringUtils::ReplaceStr(
            FLHookConfig::i()->chatConfig.msgStyle.kickMsg, std::wstring_view(L"%reason"), std::wstring_view(StringUtils::XmlText(reason.value())));
        FLHook::MessageUniverse(msg);
    }

    if (!delay.has_value())
    {
        CAccount* acc = Players.FindAccountFromClientID(value);
        acc->ForceLogout();
        return { {} };
    }

    const mstime kickTime = TimeUtils::UnixTime<std::chrono::seconds>() + delay.value();
    if (auto& client = FLHook::Clients()[value]; !client.kickTime || client.kickTime > kickTime)
    {
        client.kickTime = kickTime;
    }
    return { {} };
}

Action<void, Error> ClientId::SaveChar() const
{
    // We do not validate the client id here because new/delete characters make use of this method with an invalid, transient, id

    const DWORD jmp = FLHook::Offset(FLHook::BinaryType::Server, AddressList::SaveCharacter);

    std::array<byte, 2> nop = { 0x90, 0x90 };
    std::array<byte, 2> testAl = { 0x74, 0x44 };
    MemUtils::WriteProcMem(jmp, nop.data(), nop.size()); // nop the SinglePlayer() check
    pub::Save(value, 1);
    MemUtils::WriteProcMem(jmp, testAl.data(), testAl.size()); // restore

    const auto& data = FLHook::Clients()[value];

    // Save account data
    auto dir = GetAccount().Unwrap().GetDirectoryName().Unwrap();
    const std::wstring userFile = std::format(L"{}{}\\accData.json", FLHook::GetAccountPath(), dir);

    const auto content = data.accountData.dump(4);
    std::ofstream of(userFile);
    of.write(content.c_str(), content.size());

    return { {} };
}

Action<void, Error> ClientId::SetPvpKills(const uint killAmount) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::SetNumKills(value, static_cast<int>(killAmount));

    return { {} };
}

Action<void, Error> ClientId::SetCash(uint amount) const
{
    ClientCheck;
    CharSelectCheck;

    // TODO: Implement me!
    return { {} };
}

Action<void, Error> ClientId::AddCash(const uint amount) const { return AdjustCash(static_cast<int>(amount)); }
Action<void, Error> ClientId::RemoveCash(const uint amount) const { return AdjustCash(-static_cast<int>(amount)); }

// TODO: This should more accessible throughout the plugin and configurable
const std::array BannedBases = {
    CreateID("br_m_beryllium_miner"),  CreateID("[br_m_hydrocarbon_miner]"),
    CreateID("[br_m_niobium_miner]"),  CreateID("[co_khc_copper_miner]"),
    CreateID("[co_khc_cobalt_miner]"), CreateID("[co_kt_hydrocarbon_miner]"),
    CreateID("[co_shi_h-fuel_miner]"), CreateID("[co_shi_water_miner]"),
    CreateID("[co_ti_water_miner]"),   CreateID("[gd_gm_h-fuel_miner]"),
    CreateID("[gd_im_oxygen_miner]"),  CreateID("[gd_im_copper_miner]"),
    CreateID("[gd_im_silver_miner]"),  CreateID("[gd_im_water_miner]"),
    CreateID("[rh_m_diamond_miner]"),  CreateID("intro3_base"),
    CreateID("intro2_base"),           CreateID("intro1_base"),
    CreateID("st03b_01_base"),         CreateID("st02_01_base"),
    CreateID("st01_02_base"),          CreateID("iw02_03_base"),
    CreateID("rh02_07_base"),          CreateID("li04_06_base"),
    CreateID("li01_15_base"),
};

Action<void, Error> ClientId::Beam(BaseId base) const
{
    ClientCheck;
    CharSelectCheck;
    if (IsDocked())
    {
        return { cpp::fail(Error::PlayerNotInSpace) };
    }

    if (std::ranges::find(BannedBases, base.GetValue()) != BannedBases.end())
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    uint sysId;
    pub::Player::GetSystem(value, sysId);
    const Universe::IBase* basePtr = Universe::get_base(base.GetValue());

    if (!basePtr)
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    pub::Player::ForceLand(value, base.GetValue()); // beam

    if (basePtr->systemId != sysId)
    {
        Server.BaseEnter(base.GetValue(), value);
        Server.BaseExit(base.GetValue(), value);

        const std::wstring newFile = std::format(L"{}.fl", GetCharacterName().Unwrap());
        CHARACTER_ID charId;
        strcpy_s(charId.charFilename, StringUtils::wstos(newFile.substr(0, 14)).c_str());
        Server.CharacterSelect(charId, value);
    }

    return { {} };
}

Action<void, Error> ClientId::MarkObject(uint objId, int markStatus) const
{
    ClientCheck;
    CharSelectCheck;

    if (pub::Player::MarkObj(value, objId, markStatus) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    return { {} };
}

Action<void, Error> ClientId::Message(const std::wstring_view message, const MessageFormat format, const MessageColor color) const
{
    ClientCheck;
    CharSelectCheck;

    const auto formattedMessage = StringUtils::FormatMsg(color, format, std::wstring(message));
    InternalApi::SendMessage(*this, formattedMessage);

    return { {} };
}

Action<void, Error> ClientId::MessageLocal(std::wstring_view message, float range, MessageFormat format, MessageColor color) const
{
    ClientCheck;
    CharSelectCheck;

    auto ship = GetShipId().Raw();
    if (ship.has_error())
    {
        return { cpp::fail(ship.error()) };
    }

    const auto system = ship->GetSystem().Unwrap();
    const auto position = ship->GetPositionAndOrientation().Unwrap();
    for (auto& client : FLHook::Clients())
    {
        if (client.id == *this || !client.ship || system != client.ship.GetSystem().Unwrap())
        {
            continue;
        }

        auto otherPos = client.ship.GetPositionAndOrientation().Unwrap();
        const auto distance = glm::abs(glm::distance<3, float, glm::packed_highp>(position.first, otherPos.first));
        if (distance <= range)
        {
            Message(message, format, color);
        }
    }

    return { {} };
}

Action<void, Error> ClientId::MessageFrom(ClientId destinationClient, std::wstring message) const
{
    ClientCheck;
    CharSelectCheck;

    if (!destinationClient)
    {
        return { cpp::fail(Error::InvalidClientId) };
    }

    // TODO: validate color
    destinationClient.Message(std::move(message));
    return { {} };
}

Action<void, Error> ClientId::SetEquip(const st6::list<EquipDesc>& equip) const
{
    ClientCheck;
    CharSelectCheck;

    auto& data = GetData();

    // Update FLHook's lists to make anticheat pleased.
    if (&equip != &data.playerData->shadowEquipDescList.equip)
    {
        data.playerData->shadowEquipDescList.equip = equip;
    }

    if (&equip != &data.playerData->equipDescList.equip)
    {
        data.playerData->equipDescList.equip = equip;
    }

    // Calculate packet size. First two bytes reserved for items count.
    uint itemBufSize = 2;
    for (const auto& item : equip)
    {
        itemBufSize += sizeof(SetEquipmentItem) + strlen(item.hardPoint.value) + 1;
    }

    FlPacket* packet = FlPacket::Create(itemBufSize, FlPacket::FLPACKET_SERVER_SETEQUIPMENT);
    const auto setEquipmentPacket = reinterpret_cast<FlPacketSetEquipment*>(packet->content);

    // Add items to packet as array of variable size.
    uint index = 0;
    for (const auto& item : equip)
    {
        SetEquipmentItem setEquipItem;
        setEquipItem.count = item.count;
        setEquipItem.health = item.health;
        setEquipItem.archId = item.archId;
        setEquipItem.id = item.id;
        setEquipItem.mounted = item.mounted;
        setEquipItem.mission = item.mission;

        if (const uint len = strlen(item.hardPoint.value); len && item.hardPoint.value != "BAY")
        {
            setEquipItem.hardPointLen = static_cast<ushort>(len + 1); // add 1 for the null - char* is a null-terminated string in C++
        }
        else
        {
            setEquipItem.hardPointLen = 0;
        }
        setEquipmentPacket->count++;

        const byte* buf = (byte*)&setEquipItem;
        for (int i = 0; i < sizeof(SetEquipmentItem); i++)
        {
            setEquipmentPacket->items[index++] = buf[i];
        }

        const byte* hardPoint = (byte*)item.hardPoint.value;
        for (int i = 0; i < setEquipItem.hardPointLen; i++)
        {
            setEquipmentPacket->items[index++] = hardPoint[i];
        }
    }

    if (packet->SendTo(*this))
    {
        return { {} };
    }

    return { cpp::fail(Error::UnknownError) };
}

Action<void, Error> ClientId::AddEquip(uint goodId, const std::wstring& hardpoint) const
{
    ClientCheck;
    CharSelectCheck;

    const auto& data = GetData();

    if (!data.playerData->enteredBase)
    {
        data.playerData->enteredBase = data.playerData->baseId;
        Server.ReqAddItem(goodId, StringUtils::wstos(hardpoint).c_str(), 1, 1.0f, true, value);
        data.playerData->enteredBase = 0;
    }
    else
    {
        Server.ReqAddItem(goodId, StringUtils::wstos(hardpoint).c_str(), 1, 1.0f, true, value);
    }

    // Add to check-list which is being compared to the users equip-list when
    // saving char to fix "Ship or Equipment not sold on base" kick
    EquipDesc ed;
    ed.id = data.playerData->lastEquipId;
    ed.count = 1;
    ed.archId = goodId;
    data.playerData->shadowEquipDescList.add_equipment_item(ed, false);

    return { {} };
}
