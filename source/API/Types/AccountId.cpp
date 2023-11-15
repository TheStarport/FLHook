#include "PCH.hpp"

#include "API/Types/AccountId.hpp"

#include "Core/TempBan.hpp"

#define ValidAccountCheck                            \
    if (!this->operator bool())                      \
    {                                                \
        return { cpp::fail(Error::InvalidAccount) }; \
    }

AccountId::AccountId(const ClientId client) { value = Players.FindAccountFromClientID(client.GetValue()); }

AccountId::AccountId(const std::wstring_view characterName)
{
    st6::wstring wStr{ reinterpret_cast<const unsigned short*>(characterName.data()), characterName.size() };
    value = Players.FindAccountFromCharacterName(wStr);

    // It wasn't a character name, attempt an id search
    if (!value)
    {
        auto str = StringUtils::wstos(std::wstring(characterName));
        st6::string str6 = { str.data(), str.size() };
        value = Players.FindAccountFromCharacterID(str6);
    }
}

AccountId::operator bool() const { return value != nullptr; }

CAccount* AccountId::GetValue() const { return value; }

Action<std::wstring, Error> AccountId::GetDirectoryName() const
{
    ValidAccountCheck;

    const auto getFlName = reinterpret_cast<GetFLNameT>(FLHook::Offset(FLHook::BinaryType::Server, AddressList::GetFlName));

    static std::array<char, 64> buffer;
    std::fill_n(buffer, buffer.size(), '\0');
    getFlName(buffer.data(), value->accId);
    return { StringUtils::stows(buffer.data()) };
}

bool AccountId::IsAdmin() const
{
    // TODO once db setup
    return false;
}
Action<void, Error> AccountId::UnBan() const
{
    ValidAccountCheck;

    st6::wstring fr(reinterpret_cast<ushort*>(value->accId));
    Players.BanAccount(fr, false);

    return { {} };
}

Action<void, Error> AccountId::Ban(uint tempBanDays) const
{
    ValidAccountCheck;

    if (tempBanDays)
    {
        FLHook::GetTempBanManager().AddTempBan(*this, tempBanDays);
    }

    st6::wstring fr(reinterpret_cast<ushort*>(value->accId));
    Players.BanAccount(fr, true);
    return { {} };
}

Action<void, Error> AccountId::LockAccountAccess(const bool kick) const
{
    ValidAccountCheck;

    static const std::array<byte, 1> jmp = { 0xEB };
    static const std::array<byte, 1> jbe = { 0x76 };

    st6::wstring fr((ushort*)value->accId);

    if (!kick)
    {
        MemUtils::WriteProcMem((void*)0x06D52A6A, jmp.data(), 1);
    }

    Players.LockAccountAccess(fr); // also kicks player on this account

    if (!kick)
    {
        MemUtils::WriteProcMem((void*)0x06D52A6A, jbe.data(), 1);
    }

    return { {} };
}

Action<void, Error> AccountId::UnlockAccountAccess() const
{
    ValidAccountCheck;

    st6::wstring id(reinterpret_cast<ushort*>(value->accId));
    Players.UnlockAccountAccess(id);
    return { {} };
}
Action<void, Error> AccountId::Logout() const
{
    ValidAccountCheck;

    value->ForceLogout();

    return { {} };
}

Action<void, Error> AccountId::CreateCharacter(const std::wstring_view name) const
{
    ValidAccountCheck;

    LockAccountAccess(true);
    UnlockAccountAccess();

    INI_Reader ini;
    if (!ini.open(R"(..\DATA\CHARACTERS\newcharacter.ini)", false))
    {
        return { cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid) };
    }

    // Emulate char create by logging in.
    SLoginInfo loginData;
    wcsncpy_s(loginData.account, value->accId, 36);
    Players.login(loginData, Players.GetMaxPlayerCount() + 1);

    SCreateCharacterInfo newCharacter;
    wcsncpy_s(newCharacter.charname, name.data(), name.size());
    newCharacter.charname[23] = 0;

    newCharacter.nickName = 0;
    newCharacter.base = 0;
    newCharacter.package = 0;
    newCharacter.pilot = 0;

    if (!ini.find_header("Faction"))
    {
        return { cpp::fail(Error::MalformedData) };
    }

    while (ini.read_value())
    {
        if (ini.is_value("nickname"))
        {
            newCharacter.nickName = CreateID(ini.get_value_string());
        }
        else if (ini.is_value("base"))
        {
            newCharacter.base = CreateID(ini.get_value_string());
        }
        else if (ini.is_value("package"))
        {
            newCharacter.package = CreateID(ini.get_value_string());
        }
        else if (ini.is_value("pilot"))
        {
            newCharacter.pilot = CreateID(ini.get_value_string());
        }
    }

    ini.close();

    if (newCharacter.nickName == 0)
    {
        newCharacter.nickName = CreateID("new_player");
    }
    if (newCharacter.base == 0)
    {
        newCharacter.base = CreateID("Li01_01_Base");
    }
    if (newCharacter.package == 0)
    {
        newCharacter.package = CreateID("ge_fighter");
    }
    if (newCharacter.pilot == 0)
    {
        newCharacter.pilot = CreateID("trent");
    }

    // Fill struct with valid data (though it isn't used it is needed)
    newCharacter.dunno[4] = 65536;
    newCharacter.dunno[5] = 65538;
    newCharacter.dunno[6] = 0;
    newCharacter.dunno[7] = 1058642330;
    newCharacter.dunno[8] = 3206125978;
    newCharacter.dunno[9] = 65537;
    newCharacter.dunno[10] = 0;
    newCharacter.dunno[11] = 3206125978;
    newCharacter.dunno[12] = 65539;
    newCharacter.dunno[13] = 65540;
    newCharacter.dunno[14] = 65536;
    newCharacter.dunno[15] = 65538;

    Server.CreateNewCharacter(newCharacter, Players.GetMaxPlayerCount() + 1);
    ClientId(Players.GetMaxPlayerCount() + 1).SaveChar();
    Players.logout(Players.GetMaxPlayerCount() + 1);

    return { {} };
}

Action<void, Error> AccountId::DeleteCharacter(std::wstring_view name) const
{
    ValidAccountCheck;

    LockAccountAccess(true);

    if (st6::wstring str(reinterpret_cast<const ushort*>(name.data()), name.size()); !Players.DeleteCharacterFromName(str))
    {
        UnlockAccountAccess();
        return { cpp::fail(Error::UnknownError) };
    }

    UnlockAccountAccess();

    return { {} };
}

Action<void, Error> AccountId::AddRoles(const std::vector<std::wstring_view>& roles)
{
    // TODO once db setp
    return { {} };
}

Action<void, Error> AccountId::RemoveRoles(const std::vector<std::wstring_view>& roles, bool clear)
{
    // TODO once db setp
    return { {} };
}

Action<void, Error> AccountId::SetRoles(const std::vector<std::wstring_view>& roles)
{
    // TODO once db setp
    return { {} };
}
