#include "PCH.hpp"

#include "API/API.hpp"
#include "API/Types/ClientId.hpp"

#include "Core/FLHook.hpp"

bool ClientId::IsValidClientId() const
{
    if (value == 0 || value >= 255)
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

Action<std::wstring, Error> ClientId::GetCharacterName()
{
    ClientCheck;
    return { reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(value)) };
}

Action<BaseId, Error> ClientId::GetCurrentBase()
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

Action<SystemId, Error> ClientId::GetSystemId()
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

Action<CAccount*, Error> ClientId::GetAccount()
{
    ClientCheck;
    CAccount* acc = Players.FindAccountFromClientID(value);

    return { acc };
}

// TODO: This may fail in base when it shouldn't
Action<const Archetype::Ship*, Error> ClientId::GetShipArch()
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return cpp::fail{ Error::InvalidShip };
    }

    const auto cShip = dynamic_cast<CShip*>(CObject::Find(ship, CObject::CSHIP_OBJECT));

    if (!cShip)
    {
        return cpp::fail{ Error::InvalidShip };
    }

    return { cShip->shiparch() };
}

Action<ShipId, Error> ClientId::GetShipId()
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return cpp::fail{ Error::InvalidShip };
    }

    return { ShipId(ship) };
}

Action<std::list<EquipDesc>, Error> ClientId::GetEquipment()
{
    ClientCheck;
    CharSelectCheck;

    // TODO: Lmao
}

Action<CPlayerGroup*, Error> ClientId::GetGroup()
{
    ClientCheck;
    auto id = Players.GetGroupID(value);

    if (!id)
    {
        return cpp::fail{ Error::InvalidGroupId };
    }

    return { CPlayerGroup::FromGroupID(id) };
}

Action<std::optional<std::wstring>, Error> ClientId::GetAffiliation()
{
    ClientCheck;
    CharSelectCheck;

    const char* repGroup;

    // TODO: Lol said the scorpion.
}

Action<CShip*, Error> ClientId::GetShip()
{
    ClientCheck;
    CharSelectCheck;

    uint ship;
    pub::Player::GetShip(value, ship);

    if (!ship)
    {
        return cpp::fail{ Error::InvalidShip };
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

Action<std::wstring_view, Error> ClientId::GetActiveCharacterName()
{
    ClientCheck;
    CharSelectCheck;

    const auto player = Players.GetActiveCharacterName(value);
    const auto wcharPlayer = reinterpret_cast<const wchar_t*>(player);
    auto length = wcslen(wcharPlayer);

    return { std::wstring_view(wcharPlayer, wcslen(wcharPlayer)) };
}

bool ClientId::InSpace()
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

bool ClientId::IsDocked()
{
    if (InCharacterSelect())
    {
        return false;
    }

    return !InSpace();
}

bool ClientId::InCharacterSelect()
{
    // TODO: This pub function is undocumented and unknown, will require testing.
    uint character;
    pub::Player::GetCharacter(value, character);
    return character == 0;
}

bool ClientId::IsAlive()
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

// Server wide message upon kicking. Delay is defaulted to zero.
Action<void, Error> ClientId::Kick(std::optional<std::wstring_view> reason, std::optional<uint> delay)
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
    const mstime kickTime = TimeUtils::UnixSeconds() + delay.value();
    if (!ClientInfo::At(value).kickTime || ClientInfo::At(value).kickTime > kickTime)
    {
        ClientInfo::At(value).kickTime = kickTime;
    }
}

// This messages the player directly instead of doing a universe message. Defaults to a delay of 10 seconds.
Action<void, Error> ClientId::MessageAndKick(std::wstring_view reason, uint delay)
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

    const mstime kickTime = TimeUtils::UnixSeconds() + delay;
    if (!ClientInfo::At(value).kickTime || ClientInfo::At(value).kickTime > kickTime)
    {
        ClientInfo::At(value).kickTime = kickTime;
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
}

Action<void, Error> ClientId::SetPvpKills(uint killAmount)
{
    ClientCheck;
    CharSelectCheck;
    pub::Player::SetNumKills(value, killAmount);

    return { {} };
}

Action<void, Error> ClientId::AddCash(uint amount) { return AdjustCash(static_cast<int>(amount)); }
Action<void, Error> ClientId::RemoveCash(uint amount) { return AdjustCash(-static_cast<int>(amount)); }

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
        else
        {
            baseId = std::get<uint>(base);
        }
    }
    if (std::ranges::find(BannedBases, baseId) != BannedBases.end())
    {
        return { cpp::fail(Error::InvalidBaseName) };
    }

    uint sysId;
    pub::Player::GetSystem(value, sysId);
    const Universe::IBase* base = Universe::get_base(baseId);

    if (!base)
    {
        return { cpp::fail(Error::InvalidBase) };
    }

    pub::Player::ForceLand(value, baseId); // beam

    if (base->systemId != sysId)
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

Action<void, Error> ClientId::Message(std::wstring message, MessageFormat format, MessageColor color)
{
    ClientCheck;
    CharSelectCheck;
    auto formattedMessage = Hk::Chat::FormatMsg(color, format, message);
    Hk::Chat::FMsg(value, formattedMessage);

    return { {} };
}

Action<void, Error> ClientId::SetRep(std::variant<ushort, std::wstring_view> repGroup, float rep)
{


}
