#include "PCH.hpp"

#include "API/Types/ClientId.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "Core/FLHook.hpp"
#include "Defs/FLPacket.hpp"

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

ClientId ClientId::GetClientIdFromCharacterName(const std::wstring_view name)
{

    for (auto& client : FLHook::Clients())
    {
        if (client.characterName == name)
        {
            return client.id;
        }
    }

    return ClientId(0);
}

ClientId::operator bool() const
{
    if (value == 0 || value > MaxClientId)
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

    if (const uint base = Players[value].baseId; base)
    {
        return { BaseId(base) };
    }

    return { cpp::fail(Error::PlayerNotDocked) };
}

Action<SystemId, Error> ClientId::GetSystemId() const
{
    ClientCheck;
    CharSelectCheck;

    const uint sys = Players[value].systemId;

    if (!sys)
    {
        return { cpp::fail(Error::InvalidSystem) };
    }
    return { SystemId(sys) };
}

Action<AccountId, Error> ClientId::GetAccount() const
{
    ClientCheck;

    if (auto acc = AccountId::GetAccountFromClient(*this); acc.has_value())
    {
        return { acc.value() };
    }
    return { cpp::fail(Error::InvalidClientId) };
}

Action<const Archetype::Ship*, Error> ClientId::GetShipArch() const
{
    ClientCheck;
    CharSelectCheck;

    const uint ship = Players[value].shipArchetype;

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

    const uint ship = Players[value].shipId;

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

    const int rep = Players[value].reputation;

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

    const uint ship = Players[value].shipId;

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

    return { Players[value].rank };
}

Action<uint, Error> ClientId::GetWealth() const
{
    ClientCheck;
    CharSelectCheck;

    return { static_cast<uint>(Players[value].worth) };
}

Action<int, Error> ClientId::GetPvpKills() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].numKills };
}

Action<uint, Error> ClientId::GetCash() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].money };
}

Action<float, Error> ClientId::GetRelativeHealth() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].relativeHealth };
}

Action<void, Error> ClientId::SetRelativeHealth(const float setHealth) const
{
    ClientCheck;
    CharSelectCheck;

    if (IsDocked())
    {
        return { cpp::fail(Error::PlayerNotDocked) };
    }

    Players[value].relativeHealth = std::clamp(setHealth, 0.f, 1.f);
    return { {} };
}

Action<std::wstring_view, Error> ClientId::GetCharacterName() const
{
    if (value == 0)
    {
        return { L"CONSOLE"sv };
    }

    CharSelectCheck;

    return { GetData().characterName };
}

bool ClientId::InSpace() const { return Players[value].shipId; }

bool ClientId::IsDocked() const { return Players[value].systemId && Players[value].baseId; }

bool ClientId::InCharacterSelect() const { return !Players[value].systemId; }

bool ClientId::IsAlive() const { return Players[value].systemId && Players[value].shipId; }

Action<st6::list<EquipDesc>* const, Error> ClientId::GetEquipCargo() const
{
    ClientCheck;
    CharSelectCheck;

    return { &Players[value].equipAndCargo.equip };
}

Action<float, Error> ClientId::GetRemainingCargo() const
{
    ClientCheck;
    CharSelectCheck;

    float remainingHold;
    pub::Player::GetRemainingHoldSize(value, remainingHold);
    return { remainingHold };
}

Action<st6::list<CollisionGroupDesc>* const, Error> ClientId::GetCollisionGroups() const
{
    ClientCheck;
    CharSelectCheck;

    return { &Players[value].collisionGroupDesc };
}

ClientData& ClientId::GetData() const { return FLHook::Clients()[value]; }

Action<std::wstring, Error> ClientId::GetPlayerIp() const
{
    const CDPClientProxy* cdpClient = FLHook::clientProxyArray[value - 1];
    // clang-format off
    if (!cdpClient)
    {
        //TODO: Rewrite this whole thing
        // I hate labels, but it allows our inline asm to escape properly
        //invalid:
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
    // TODO: Reimplement with Xbyak
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
        invalid:
    }

    return { wIp };
}

EngineState ClientId::GetEngineState() const
{
    const auto& data = GetData();

    if (!data.shipId)
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
Action<void, Error> ClientId::Kick(const std::optional<std::wstring_view>& reason, std::optional<uint> delay) const
{
    ClientCheck;

    if (reason.has_value())
    {
        const std::wstring msg = std::vformat(FLHook::GetConfig().chatConfig.msgStyle.kickMsg, std::make_wformat_args(reason.value()));
        (void)Message(msg, MessageFormat::Big, MessageColor::Red);

        if (!delay.has_value())
        {
            // Give them time to read the reason
            delay = { 5u };
        }
    }

    if (!delay.has_value())
    {
        CAccount* acc = Players.FindAccountFromClientID(value);
        acc->ForceLogout();
        return { {} };
    }

    const int64 kickTime = TimeUtils::UnixTime<std::chrono::seconds>() + delay.value();
    if (auto& client = FLHook::Clients()[value]; !client.kickTime || client.kickTime > kickTime)
    {
        client.kickTime = kickTime;
    }
    return { {} };
}

Action<void, Error> ClientId::SaveChar() const
{
    // We do not validate the client id here because new/delete characters make use of this method with an invalid, transient, id
    pub::Save(value, 1);

    return { {} };
}

Action<void, Error> ClientId::SetPvpKills(const uint killAmount) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::SetNumKills(value, static_cast<int>(killAmount));

    return { {} };
}

Action<void, Error> ClientId::SetCash(const uint amount) const
{
    ClientCheck;
    CharSelectCheck;

    const int currCash = Players[value].money;
    pub::Player::AdjustCash(value, static_cast<int>(amount) - currCash);
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

Action<void, Error> ClientId::Beam(const BaseId base) const
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

    const uint sysId = Players[value].systemId;
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

Action<void, Error> ClientId::MarkObject(const uint objId, const int markStatus) const
{
    ClientCheck;
    CharSelectCheck;

    if (pub::Player::MarkObj(value, objId, markStatus) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    return { {} };
}

Action<void, Error> ClientId::Message(const std::wstring_view message, const MessageFormat format, const MessageColor color) const
{
    if (value == 0)
    {
        Logger::Info(message);
        return { {} };
    }

    ClientCheck;
    CharSelectCheck;

    for (auto messages = StringUtils::GetParams(message, L'\n'); auto msg : messages)
    {
        const auto formattedMessage = StringUtils::FormatMsg(color, format, std::wstring(msg));
        InternalApi::SendMessage(*this, formattedMessage);
    }

    return { {} };
}

Action<void, Error> ClientId::MessageLocal(const std::wstring_view message, const float range, const MessageFormat format, const MessageColor color) const
{
    ClientCheck;
    CharSelectCheck;

    auto ship = GetShipId().Raw();
    if (ship.has_error())
    {
        return { cpp::fail(ship.error()) };
    }

    const auto system = ship->GetSystem().Unwrap();
    const auto [position, _] = ship->GetPositionAndOrientation().Unwrap();
    for (auto& client : FLHook::Clients())
    {
        if (client.id == *this || !client.shipId || system != client.shipId.GetSystem().Unwrap())
        {
            continue;
        }

        auto [otherPos, _] = client.shipId.GetPositionAndOrientation().Unwrap();
        if (const auto distance = glm::abs(glm::distance<3, float, glm::packed_highp>(position, otherPos)); distance <= range)
        {
            (void)Message(message, format, color);
        }
    }

    return { {} };
}

Action<void, Error> ClientId::MessageFrom(const ClientId destinationClient, const std::wstring_view message) const
{
    ClientCheck;
    CharSelectCheck;

    if (!destinationClient)
    {
        return { cpp::fail(Error::InvalidClientId) };
    }

    // TODO: validate color
    (void)destinationClient.Message(message);
    return { {} };
}

Action<void, Error> ClientId::MessageCustomXml(const std::wstring_view rawXml) const
{
    static std::array<char, 0xFFFF> buffer;
    std::fill_n(buffer.begin(), buffer.size(), '\0');

    uint ret;
    if (const auto err = InternalApi::FMsgEncodeXml(StringUtils::XmlText(rawXml), buffer.data(), buffer.size(), ret).Raw(); err.has_error())
    {
        return { cpp::fail(err.error()) };
    }

    InternalApi::FMsgSendChat(*this, buffer.data(), ret);
    return { {} };
}

Action<void, Error> ClientId::SetEquip(const st6::list<EquipDesc>& equip) const
{
    ClientCheck;
    CharSelectCheck;

    // Update FLHook's lists to make anticheat pleased.
    if (const auto& data = GetData(); &equip != &data.playerData->equipAndCargo.equip)
    {
        data.playerData->equipAndCargo.equip = equip;
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
        SetEquipmentItem setEquipItem{};
        setEquipItem.count = item.count;
        setEquipItem.health = item.health;
        setEquipItem.archId = item.archId;
        setEquipItem.id = item.id;
        setEquipItem.mounted = item.mounted;
        setEquipItem.mission = item.mission;

        static const std::string bayHp = "BAY";
        if (const uint len = strlen(item.hardPoint.value); len && item.hardPoint.value != bayHp)
        {
            setEquipItem.hardPointLen = static_cast<ushort>(len + 1); // add 1 for the null - char* is a null-terminated string in C++
        }
        else
        {
            setEquipItem.hardPointLen = 0;
        }
        setEquipmentPacket->count++;

        const byte* buf = reinterpret_cast<byte*>(&setEquipItem);
        for (int i = 0; i < sizeof(SetEquipmentItem); i++)
        {
            setEquipmentPacket->items[index++] = buf[i];
        }

        const byte* hardPoint = reinterpret_cast<byte*>(item.hardPoint.value);
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

Action<void, Error> ClientId::AddEquip(const uint goodId, const std::wstring& hardpoint) const
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

    return { {} };
}

Action<void, Error> ClientId::AddCargo(const uint goodId, const uint count, const bool isMission) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::AddCargo(value, goodId, count, 1.0, isMission);
    return { {} };
}
Action<void, Error> ClientId::RemoveCargo(rfl::Variant<GoodId, EquipmentId, ushort> goodId, uint count) const
{
    switch (goodId.index())
    {
        case 0:
            {
                goodId = EquipmentId(Arch2Good(rfl::get<GoodId>(goodId).GetHash().Handle()));
            }
        case 1:
            {
                const EquipmentId& eqId = rfl::get<EquipmentId>(goodId);
                bool foundItem = false;
                for (auto& equip : Players[value].equipAndCargo.equip)
                {
                    if (equip.archId == eqId.GetValue()->archId)
                    {
                        goodId = equip.id;
                        foundItem = true;
                        break;
                    }
                }
                if (!foundItem)
                {
                    return { cpp::fail(Error::UnknownError) };
                }
            }
        case 2:
            {
                const int result = pub::Player::RemoveCargo(value, rfl::get<ushort>(goodId), count);
                if (result == -2)
                {
                    return { cpp::fail(Error::UnknownError) };
                }
            }
        default: return { cpp::fail(Error::UnknownError) };
    }
    return { {} };
}

Action<void, Error> ClientId::Undock(Vector pos, std::optional<Matrix> orientation) const
{
    ClientCheck;
    CharSelectCheck;

    const auto rotation = Quaternion(orientation.value_or(Matrix::Identity()));

    FLPACKET_LAUNCH launchPacket;
    launchPacket.ship = Players[value].shipId;
    launchPacket.base = 0;
    launchPacket.state = 0xFFFFFFFF;
    launchPacket.rotate[0] = rotation.w;
    launchPacket.rotate[1] = rotation.x;
    launchPacket.rotate[2] = rotation.y;
    launchPacket.rotate[3] = rotation.z;
    launchPacket.pos[0] = pos.x;
    launchPacket.pos[1] = pos.y;
    launchPacket.pos[2] = pos.z;

    FLHook::hookClientImpl->Send_FLPACKET_SERVER_LAUNCH(value, launchPacket);

    return { {} };
}

Action<void, Error> ClientId::PlaySound(const uint hash) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Audio::PlaySoundEffect(value, hash);
    return { {} };
}

Action<void, Error> ClientId::InvitePlayer(ClientId otherClient) const
{
    ClientCheck;
    CharSelectCheck;

    if (!otherClient)
    {
        return { cpp::fail(Error::InvalidClientId) };
    }

    const std::wstring XML = L"<TEXT>/i " + StringUtils::XmlText(otherClient.GetCharacterName().Handle()) + L"</TEXT>";

    // Allocates a stack-sized std::array once per run-time and clear every invocation.
    static std::array<char, USHRT_MAX> buf{};
    std::ranges::fill(buf, 0);

    uint retVal;
    InternalApi::FMsgEncodeXml(XML, buf.data(), sizeof buf, retVal).Handle();

    // Mimics Freelancer's ingame invite system by using their chatID and chat commands from pressing the invite target button.
    CHAT_ID chatId{};
    chatId.id = value;
    CHAT_ID chatIdTo{};
    chatIdTo.id = static_cast<int>(SpecialChatIds::System);
    Server.SubmitChat(chatId, retVal, buf.data(), chatIdTo, -1);

    return {{}};
}
