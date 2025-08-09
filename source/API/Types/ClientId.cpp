#include "PCH.hpp"

#include "API/Types/ClientId.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "API/Utils/PathHelper.hpp"
#include "Core/FLHook.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Defs/FLPacket.hpp"
#include "FLCore/FLCoreDALib.h"

#define ClientCheck                                   \
    if (!IsValidClientId())                           \
    {                                                 \
        return { cpp::fail(Error::InvalidClientId) }; \
    }

#define CharSelectCheck                                       \
    if (InCharacterSelect())                                  \
    {                                                         \
        return { cpp::fail(Error::PlayerInCharacterSelect) }; \
    }

Action<void> ClientId::AdjustCash(const int amount) const
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

ClientId ClientId::GetClientIdFromCharacterName(const CharacterId name)
{

    for (auto& client : FLHook::Clients())
    {
        if (client.characterId == name)
        {
            return client.id;
        }
    }

    return ClientId(0);
}
ClientId::ClientId(const std::wstring_view str) : value(GetClientIdFromCharacterName(CharacterId{ str })) {}

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

Action<BaseId> ClientId::GetCurrentBase() const
{
    ClientCheck;
    CharSelectCheck;

    if (const BaseId base = Players[value].baseId; base)
    {
        return { base };
    }

    return { cpp::fail(Error::PlayerNotDocked) };
}

Action<BaseId> ClientId::GetLastDockedBase() const
{
    ClientCheck;
    CharSelectCheck;

    if (const BaseId base = Players[value].lastBaseId; base)
    {
        return { base };
    }

    return { cpp::fail(Error::InvalidBase) };
}

Action<SystemId> ClientId::GetSystemId() const
{
    ClientCheck;
    CharSelectCheck;

    const SystemId sys = Players[value].systemId;

    if (!sys)
    {
        return { cpp::fail(Error::InvalidSystem) };
    }
    return { sys };
}

Action<AccountId> ClientId::GetAccount() const
{
    ClientCheck;

    if (auto acc = AccountId::GetAccountFromClient(*this); acc.has_value())
    {
        return { acc.value() };
    }
    return { cpp::fail(Error::InvalidClientId) };
}

Action<EquipmentId> ClientId::GetShipArch() const
{
    ClientCheck;
    CharSelectCheck;

    auto id = EquipmentId{ Players[value].shipArchetype };
    if (!id)
    {
        return { cpp::fail(Error::InvalidObject) };
    }

    return { id };
}

Action<ShipId> ClientId::GetShip() const
{
    ClientCheck;
    CharSelectCheck;

    const ShipId ship = ShipId(Players[value].shipId);

    if (!ship)
    {
        return { cpp::fail(Error::PlayerNotInSpace) };
    }

    return { ship };
}

Action<uint> ClientId::GetLatency() const
{
    // TODO: Implement latency func when ConData is slated.
    return { 0u };
}

Action<GroupId> ClientId::GetGroup() const
{
    ClientCheck;

    const auto id = Players.GetGroupID(value);

    if (!id)
    {
        return { cpp::fail(Error::PlayerNotInGroup) };
    }

    return { GroupId(id) };
}

Action<RepId> ClientId::GetReputation() const
{
    ClientCheck;
    CharSelectCheck;

    const int rep = Players[value].reputation;

    if (!Reputation::Vibe::Verify(rep))
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { RepId(rep) };
}

Action<uint> ClientId::GetRank() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].rank };
}

Action<uint> ClientId::GetWealth() const
{
    ClientCheck;
    CharSelectCheck;

    return { static_cast<uint>(Players[value].worth) };
}

Action<int> ClientId::GetPvpKills() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].numKills };
}

Action<float> ClientId::GetRelativeHealth() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].relativeHealth };
}

Action<void> ClientId::SetRelativeHealth(const float setHealth) const
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

Action<CharacterId> ClientId::GetCharacterId() const
{
    if (value == 0)
    {
        return { CharacterId{ L"CONSOLE"sv } };
    }

    CharSelectCheck;

    return { GetData().characterId };
}

bool ClientId::InSpace() const
{
    if (Players[value].shipId)
    {
        return true;
    }
    return false;
}

bool ClientId::IsDocked() const { return Players[value].systemId && Players[value].baseId; }

bool ClientId::InCharacterSelect() const { return !Players[value].systemId; }

bool ClientId::IsAlive() const { return Players[value].systemId && Players[value].shipId; }

Action<EquipDescList* const> ClientId::GetEquipCargo() const
{
    ClientCheck;
    CharSelectCheck;

    return { &Players[value].equipAndCargo };
}

Action<float> ClientId::GetRemainingCargo() const
{
    ClientCheck;
    CharSelectCheck;

    float remainingHold;
    pub::Player::GetRemainingHoldSize(value, remainingHold);
    return { remainingHold };
}

Action<st6::list<CollisionGroupDesc>* const> ClientId::GetCollisionGroups() const
{
    ClientCheck;
    CharSelectCheck;

    return { &Players[value].collisionGroupDesc };
}

Action<CPlayerTradeOffer*> ClientId::GetTradeOffer() const
{
    ClientCheck;
    CharSelectCheck;

    if (Players[value].tradeOffer)
    {
        return { Players[value].tradeOffer };
    }

    return { cpp::fail(Error::PlayerNoTradeActive) };
}

ClientData& ClientId::GetData() const { return FLHook::Clients()[value]; }

Action<std::wstring> ClientId::GetPlayerIp() const
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
Action<void> ClientId::Kick(const std::optional<std::wstring_view>& reason, std::optional<uint> delay) const
{
    ClientCheck;

    if (reason.has_value())
    {
        const std::wstring msg = std::vformat(FLHook::GetConfig()->chatConfig.msgStyle.kickMsg, std::make_wformat_args(reason.value()));
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
        if (acc)
        {
            acc->ForceLogout();
        }
        return { {} };
    }

    const int64 kickTime = TimeUtils::UnixTime<std::chrono::seconds>() + delay.value();
    if (auto& client = FLHook::Clients()[value]; !client.kickTime || client.kickTime > kickTime)
    {
        client.kickTime = kickTime;
    }
    return { {} };
}

Action<void> ClientId::SaveChar() const
{
    // We do not validate the client id here because new/delete characters make use of this method with an invalid, transient, id
    pub::Save(value, 1);

    return { {} };
}

Action<void> ClientId::SetPvpKills(const uint killAmount) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::SetNumKills(value, static_cast<int>(killAmount));

    return { {} };
}

Action<void> ClientId::Beam(const BaseId base) const
{
    ClientCheck;
    CharSelectCheck;
    if (IsDocked())
    {
        return { cpp::fail(Error::PlayerNotInSpace) };
    }

    // TODO: Is there some trick we can pull to have the bannedBases config hashed during loading, so this code becomes a simple set contains check?
    for (auto bannedBase : FLHook::GetConfig()->general.bannedBases)
    {
        if (CreateID(bannedBase.c_str()) == base.GetValue())
        {
            return { cpp::fail(Error::InvalidBase) };
        }
    }

    const SystemId sysId = Players[value].systemId;
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

        const std::wstring newFile = std::format(L"{}.fl", GetCharacterId().Unwrap());
        CHARACTER_ID charId;
        strcpy_s(charId.charFilename, StringUtils::wstos(newFile.substr(0, 14)).c_str());
        Server.CharacterSelect(charId, value);
    }

    return { {} };
}

Action<void> ClientId::MarkObject(const uint objId, const int markStatus) const
{
    ClientCheck;
    CharSelectCheck;

    if (pub::Player::MarkObj(value, objId, markStatus) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidObject) };
    }

    return { {} };
}

Action<void> ClientId::Message(const std::wstring_view message, const MessageFormat format, const MessageColor color) const
{
    if (value == 0)
    {
        Logger::MessageConsole(message);
        return { {} };
    }

    ClientCheck;
    CharSelectCheck;

    for (auto messages = StringUtils::GetParams(message, L'\n'); auto msg : messages)
    {
        const auto formattedMessage = StringUtils::FormatMsg(color, format, std::wstring(msg));
        InternalApi::SendMessage(*this, formattedMessage, ClientId());
    }

    return { {} };
}
Action<void> ClientId::MessageErr(const std::wstring_view message) const { return Message(message, MessageFormat::Normal, MessageColor::Crimson); }

Action<std::vector<ClientId>> ClientId::GetLocalClients(const float range) const
{
    ClientCheck;
    CharSelectCheck;

    auto ship = GetShip().Raw();
    if (ship.has_error())
    {
        return { cpp::fail(ship.error()) };
    }

    const auto system = ship->GetSystem().Unwrap();
    const auto [position, _] = ship->GetPositionAndOrientation().Unwrap();
    std::vector<ClientId> locals;
    for (auto& client : system.GetPlayersInSystem().Unwrap())
    {
        auto [otherPos, _] = client.GetShip().Raw()->GetPositionAndOrientation().Unwrap();
        if (const auto distance = glm::abs(glm::distance<3, float, glm::packed_highp>(position, otherPos)); distance <= range)
        {
            locals.emplace_back(client);
        }
    }

    return { locals };
}

Action<void> ClientId::MessageLocal(const std::wstring_view message, const float range, const MessageFormat format, const MessageColor color) const
{
    ClientCheck;
    CharSelectCheck;

    auto recipients = GetLocalClients(10'000.0f).Handle();
    for (auto client : recipients)
    {
        (void)client.Message(message, format, color);
    }

    return { {} };
}

Action<void> ClientId::MessageFrom(const ClientId destinationClient, const std::wstring_view message, std::wstring_view colour) const
{
    ClientCheck;
    CharSelectCheck;

    if (!destinationClient)
    {
        return { cpp::fail(Error::InvalidClientId) };
    }

    (void)MessageCustomXml(std::format(L"<TRA data=\"0xFFFFFF00\" mask=\"-1\"/><TEXT>{}: </TEXT><TRA data=\"0x{}00\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                       StringUtils::XmlText(destinationClient.GetCharacterId().Handle().GetValue()),
                                       colour,
                                       StringUtils::XmlText(message)));

    return { {} };
}

Action<void> ClientId::MessageCustomXml(const std::wstring_view rawXml) const
{
    static std::array<char, 0xFFFF> buffer;
    std::fill_n(buffer.begin(), buffer.size(), '\0');

    uint ret;
    if (const auto err = InternalApi::FMsgEncodeXml(rawXml, buffer.data(), buffer.size(), ret).Raw(); err.has_error())
    {
        return { cpp::fail(err.error()) };
    }

    InternalApi::FMsgSendChat(*this, buffer.data(), ret);
    return { {} };
}

Action<void> ClientId::SetEquip(const st6::list<EquipDesc>& equip) const
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

    return { cpp::fail(Error::PacketError) };
}

Action<void> ClientId::AddEquip(const Id goodId, const std::wstring& hardpoint) const
{
    ClientCheck;
    CharSelectCheck;

    const auto& data = GetData();

    if (!data.playerData->enteredBase)
    {
        data.playerData->enteredBase = data.playerData->baseId;
        Server.ReqAddItem(goodId.GetValue(), StringUtils::wstos(hardpoint).c_str(), 1, 1.0f, true, value);
        data.playerData->enteredBase = BaseId();
    }
    else
    {
        Server.ReqAddItem(goodId.GetValue(), StringUtils::wstos(hardpoint).c_str(), 1, 1.0f, true, value);
    }

    // Add to check-list which is being compared to the users equip-list when
    // saving char to fix "Ship or Equipment not sold on base" kick
    EquipDesc ed;
    ed.id = data.playerData->lastEquipId;
    ed.count = 1;
    ed.archId = goodId;

    return { {} };
}

Action<void> ClientId::PlayMusic(const pub::Audio::Tryptich& info) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Audio::SetMusic(value, info);
    return { {} };
}

Action<void> ClientId::PlaySound(const Id hash) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Audio::PlaySoundEffect(value, hash.GetValue());
    return { {} };
}

Action<void> ClientId::DisplayMissionObjective(const uint ids) const
{
    ClientCheck;
    CharSelectCheck;

    FmtStr fmt(0, nullptr);
    fmt.begin_mad_lib(ids);
    fmt.end_mad_lib();

    pub::Player::DisplayMissionMessage(value, fmt, MissionMessageType::Type2, true);
    return { {} };
}

Action<DPN_CONNECTION_INFO> ClientId::GetConnectionData() const
{
    ClientCheck;
    CharSelectCheck;

    const auto cdpClient = FLHook::GetClientProxyArray()[value - 1];
    DPN_CONNECTION_INFO connectionInfo;
    if (!cdpClient || !cdpClient->GetConnectionStats(&connectionInfo))
    {
        return { cpp::fail(Error::PacketError) };
    }

    return { { connectionInfo } };
}

Action<void> ClientId::InvitePlayer(ClientId otherClient) const
{
    ClientCheck;
    CharSelectCheck;

    if (!otherClient)
    {
        return { cpp::fail(Error::InvalidClientId) };
    }

    const std::wstring XML = L"<TEXT>/i " + StringUtils::XmlText(otherClient.GetCharacterId().Handle().GetValue()) + L"</TEXT>";

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

    return { {} };
}

Action<Vector> ClientId::GetPosition() const
{
    auto ship = this->GetShip();
    if (ship.Raw().has_error())
    {
        return { GetData().playerData->position };
    }
    return { ship.Unwrap().GetValue().lock()->position };
}

Action<void> ClientId::SendBestPath(SystemId targetSystem, Vector targetPosition) const
{
    static RequestPath<2> bpi;
    bpi.pathEntries[0].systemId = GetSystemId().Handle();
    bpi.pathEntries[0].pos = GetPosition().Handle();
    bpi.pathEntries[1].systemId = targetSystem;
    bpi.pathEntries[1].pos = targetPosition;
    bpi.waypointCount = 2;
    bpi.noPathFound = false;
    bpi.repId = GetReputation().Handle();

    // The original asm code sets 52 as size for the struct. So better hard code it instead of sizeof in case someone messes up the struct in a commit.
    Server.RequestBestPath(GetValue(), (uchar*)&bpi, 12 + (20 * 2));

    return { {} };
}
bool ClientId::HasFluf() const { return GetData().usingFlufClientHook; }

Action<void> ClientId::ToastMessage(std::wstring_view title, std::wstring_view message, ToastType type, int timeOut, bool withSeperator,
                                    bool sendMessageIfNoFluf) const
{
    ClientCheck;
    CharSelectCheck;

    if (HasFluf())
    {
        const ToastPayload payload{ .title = StringUtils::wstos(title),
                                    .content = StringUtils::wstos(message),
                                    .toastType = type,
                                    .timeUntilDismiss = timeOut,
                                    .addSeparator = withSeperator };

        return SendFlufPayload<ToastPayload>("toast", payload);
    }

    if (sendMessageIfNoFluf)
    {
        if (type == ToastType::Error)
        {
            return MessageErr(message);
        }

        return Message(message);
    }

    return { {} };
}

Action<void> ClientId::SetCash(const uint amount) const
{
    ClientCheck;
    CharSelectCheck;

    const int currCash = Players[value].money;
    pub::Player::AdjustCash(value, static_cast<int>(amount) - currCash);
    return { {} };
}

Action<void> ClientId::AddCash(const uint amount) const { return AdjustCash(static_cast<int>(amount)); }

Action<void> ClientId::RemoveCash(const uint amount) const { return AdjustCash(-static_cast<int>(amount)); }

Action<void> ClientId::AddCargo(const Id goodId, const uint count, const bool isMission) const
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::AddCargo(value, goodId.GetValue(), count, 1.0, isMission);
    return { {} };
}
Action<void> ClientId::RemoveCargo(rfl::Variant<GoodId, EquipmentId, ushort> goodId, uint count) const
{
    switch (goodId.index())
    {
        case 0:
            {
                goodId = EquipmentId(Arch2Good(rfl::get<GoodId>(goodId).GetHash().Handle().GetValue()));
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
                        if (equip.count < count)
                        {
                            count = equip.count;
                        }
                        break;
                    }
                }

                if (!foundItem)
                {
                    return { cpp::fail(Error::InvalidEquipment) };
                }
            }
        case 2:
            {
                if (const int result = pub::Player::RemoveCargo(value, rfl::get<ushort>(goodId), count); result == -2)
                {
                    return { cpp::fail(Error::InvalidGood) };
                }
            }
        default: return { cpp::fail(Error::UnknownError) };
    }
}

Action<uint> ClientId::GetCash() const
{
    ClientCheck;
    CharSelectCheck;

    return { Players[value].money };
}
