#include "PCH.hpp"

#include "API/API.hpp"
#include "API/Types/ClientId.hpp"

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
    // TODO: Validate this can be done with a view
    auto& ref = name;
    const auto account = Players.FindAccountFromCharacterName(reinterpret_cast<st6::wstring&>(ref));
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

    PlayerData* playerDb = nullptr;
    while ((playerDb = Players.traverse_active(playerDb)))
    {
        if (playerDb->onlineId == value)
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

Action<CAccount*, Error> ClientId::GetAccount() const
{
    ClientCheck;
    CAccount* acc = Players.FindAccountFromClientID(value);

    return { acc };
}

Action<const Archetype::Ship*, Error> ClientId::GetShipArch()
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShipID(value, ship);

    if (!ship)
    {
        return { cpp::fail{ Error::InvalidShip } };
    }

    return { Archetype::GetShip(ship) };
}

Action<ShipId, Error> ClientId::GetShipId()
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return { cpp::fail{ Error::InvalidShip } };
    }

    return { ShipId(ship) };
}

Action<CPlayerGroup*, Error> ClientId::GetGroup()
{
    ClientCheck;
    const auto id = Players.GetGroupID(value);

    if (!id)
    {
        return { cpp::fail{ Error::PlayerNotInGroup } };
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

Action<CShip*, Error> ClientId::GetShip()
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return { cpp::fail{ Error::InvalidShip } };
    }

    return { dynamic_cast<CShip*>(CObject::Find(ship, CObject::CSHIP_OBJECT)) };
}

Action<uint, Error> ClientId::GetRank()
{
    ClientCheck;
    CharSelectCheck;
    int rank = 0;
    pub::Player::GetRank(value, rank);
    return { rank };
}

Action<uint, Error> ClientId::GetWealth()
{
    ClientCheck;
    CharSelectCheck;

    // Cause Freelancer decided wealth should be accessed as a float.(Despite it being stored as an integer)
    float wealth;
    pub::Player::GetAssetValue(value, wealth);
    return { static_cast<uint>(wealth) };
}

Action<int, Error> ClientId::GetPvpKills()
{
    ClientCheck;
    CharSelectCheck;

    int kills;
    pub::Player::GetNumKills(value, kills);
    return { kills };
}

Action<uint, Error> ClientId::GetCash()
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
    bool charMenu = InCharacterSelect();
    bool docked = IsDocked();
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

// Server wide message upon kicking. Delay is defaulted to zero.
Action<void, Error> ClientId::Kick(const std::optional<std::wstring_view>& reason, const std::optional<uint> delay)
{
    ClientCheck;

    if (reason.has_value())
    {
        const std::wstring msg = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.kickMsg, L"%reason", StringUtils::XmlText(reason.value()));
        Hk::Chat::MsgU(msg);
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

// This messages the player directly instead of doing a universe message. Defaults to a delay of 10 seconds.
Action<void, Error> ClientId::MessageAndKick(const std::wstring_view reason, const uint delay)
{
    ClientCheck;

    if (reason.length())
    {
        const std::wstring msg = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.kickMsg, L"%reason", StringUtils::XmlText(reason));
        Hk::Chat::Msg(value, msg);
    }

    if (!delay)
    {
        CAccount* acc = Players.FindAccountFromClientID(value);
        acc->ForceLogout();
        return { {} };
    }

    const mstime kickTime = TimeUtils::UnixTime<std::chrono::seconds>() + delay;
    if (auto& client = FLHook::Clients()[value]; !client.kickTime || client.kickTime > kickTime)
    {
        client.kickTime = kickTime;
    }

    return { {} };
}

Action<void, Error> ClientId::SaveChar()
{
    ClientCheck;
    CharSelectCheck;

    DWORD jmp = FLHook::Offset(FLHook::BinaryType::Server, AddressList::SaveCharacter);

    std::array<byte, 2> nop = { 0x90, 0x90 };
    std::array<byte, 2> testAl = { 0x74, 0x44 };
    MemUtils::WriteProcMem(jmp, nop.data(), nop.size()); // nop the SinglePlayer() check
    pub::Save(value, 1);
    MemUtils::WriteProcMem(jmp, testAl.data(), testAl.size()); // restore

    return { {} };
}

Action<void, Error> ClientId::SetPvpKills(const uint killAmount)
{
    ClientCheck;
    CharSelectCheck;

    pub::Player::SetNumKills(value, static_cast<int>(killAmount));

    return { {} };
}

Action<void, Error> ClientId::AddCash(const uint amount) { return AdjustCash(static_cast<int>(amount)); }
Action<void, Error> ClientId::RemoveCash(const uint amount) { return AdjustCash(-static_cast<int>(amount)); }

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

Action<void, Error> ClientId::Beam(std::variant<BaseId, std::wstring_view> base)
{
    ClientCheck;
    CharSelectCheck;
    if (IsDocked())
    {
        return { cpp::fail(Error::PlayerNotInSpace) };
    }

    uint baseId;

    if (base.index() == 1)
    {
        const std::string baseName = StringUtils::wstos(std::wstring(std::get<std::wstring_view>(base)));

        // get base id
        if (pub::GetBaseID(baseId, baseName.c_str()) == -4)
        {
            return { cpp::fail(Error::InvalidBaseName) };
        }
    }
    else
    {
        baseId = std::get<uint>(base);
    }

    if (std::ranges::find(BannedBases, baseId) != BannedBases.end())
    {
        return { cpp::fail(Error::InvalidBaseName) };
    }

    uint sysId;
    pub::Player::GetSystem(value, sysId);
    const Universe::IBase* basePtr = Universe::get_base(baseId);

    if (!basePtr)
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    pub::Player::ForceLand(value, baseId); // beam

    if (basePtr->systemId != sysId)
    {
        Server.BaseEnter(baseId, value);
        Server.BaseExit(baseId, value);
        auto fileName = Hk::Client::GetCharFileName(value).Raw();
        if (fileName.has_error())
        {
            return { cpp::fail(fileName.error()) };
        }
        const std::wstring newFile = std::format(L"{}.fl", fileName.value());
        CHARACTER_ID charId;
        strcpy_s(charId.charFilename, StringUtils::wstos(newFile.substr(0, 14)).c_str());
        Server.CharacterSelect(charId, value);
    }

    return { {} };
}

Action<void, Error> ClientId::Message(const std::wstring_view message, const MessageFormat format, const MessageColor color) const
{
    ClientCheck;
    CharSelectCheck;

    const auto formattedMessage = Hk::Chat::FormatMsg(color, format, message);
    Hk::Chat::FMsg(value, formattedMessage);

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

void ClientId::Save()
{
    // Save character file
    pub::Save(value, 1);

    auto& data = FLHook::Clients()[value];

    // Save account data
    const CAccount* acc = Players.FindAccountFromClientID(value);
    const std::wstring dir = Hk::Client::GetAccountDirName(acc);
    const std::wstring userFile = std::format(L"{}{}\\accData.json", FLHook::GetAccountPath(), dir);

    const auto content = data.accountData.dump(4);
    std::ofstream of(userFile);
    of.write(content.c_str(), content.size());
}
