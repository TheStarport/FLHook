#include "PCH.hpp"

#include "API/API.hpp"

bool Api::IsValidClientId(ClientId id)
{
    if (id == 0 || id >= MaxClientId)
    {
        return false;
    }

    PlayerData* playerDb = nullptr;
    while ((playerDb = Players.traverse_active(playerDb)))
    {
        if (playerDb->onlineId == id)
        {
            return true;
        }
    }

    return false;
}

Action<uint, Error> Api::GetClientIdFromAccount(const CAccount* acc)
{
    PlayerData* playerDb = nullptr;
    while ((playerDb = Players.traverse_active(playerDb)))
    {
        if (playerDb->account == acc)
        {
            return { playerDb->onlineId };
        }
    }

    return { cpp::fail(Error::PlayerNotLoggedIn) };
}

Action<CAccount*, Error> Api::GetAccountByCharName(std::wstring_view character)
{
    st6::wstring fr((ushort*)character.data(), character.size());
    CAccount* acc = Players.FindAccountFromCharacterName(fr);

    if (!acc)
    {
        return { cpp::fail(Error::CharacterDoesNotExist) };
    }

    return { acc };
}

Action<uint, Error> Api::GetClientIdFromCharName(std::wstring_view character)
{
    const auto acc = GetAccountByCharName(character).Raw();
    if (acc.has_error())
    {
        return { cpp::fail(acc.error()) };
    }

    const auto client = GetClientIdFromAccount(acc.value()).Raw();
    if (client.has_error())
    {
        return { cpp::fail(client.error()) };
    }

    const auto newCharacter = GetCharacterNameByID(client.value()).Raw();
    if (newCharacter.has_error())
    {
        return { cpp::fail(newCharacter.error()) };
    }

    if (StringUtils::ToLower(newCharacter.value()) == StringUtils::ToLower(character))
    {
        return { cpp::fail(Error::CharacterDoesNotExist) };
    }

    return { client };
}

Action<std::wstring, Error> Api::GetAccountID(CAccount* acc)
{
    if (acc && acc->accId)
    {
        return { acc->accId };
    }

    return { cpp::fail(Error::CannotGetAccount) };
}

Action<std::wstring, Error> Api::GetCharacterNameByID(ClientId client)
{
    if (!IsValidClientId(client) || IsInCharSelectMenu(client))
    {
        return { cpp::fail(Error::InvalidClientId) };
    }

    return { reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client)) };
}

Action<ClientId, Error> Api::GetClientIdByShip(const ShipId ship)
{
    if (const auto foundClient = std::ranges::find_if(ClientInfo::clients, [ship](const ClientInfo& ci) { return ci.ship == ship; });
        foundClient != ClientInfo::clients.end())
    {
        return { std::ranges::distance(ClientInfo::clients.begin(), foundClient) };
    }

    return { cpp::fail(Error::InvalidShip) };
}

bool Api::IsInCharSelectMenu(const uint& player)
{
    ClientId client = ExtractClientID(player);
    if (client == UINT_MAX)
    {
        return false;
    }

    uint base = 0;
    uint system = 0;
    pub::Player::GetBase(client, base);
    pub::Player::GetSystem(client, system);
    if (!base && !system)
    {
        return true;
    }
    return false;
}

void Api::PrintUserCmdText(ClientId client, std::wstring_view text)
{
    if (const auto newLineChar = text.find(L'\n'); newLineChar == std::wstring::npos)
    {
        const std::wstring xml =
            std::format(L"<TRA data=\"{}\" mask=\"-1\"/><TEXT>{}</TEXT>", FLHookConfig::i()->chatConfig.msgStyle.userCmdStyle, StringUtils::XmlText(text));
        Hk::Chat::FMsg(client, xml);
    }
    else
    {
        // Split text into two strings, one from the beginning to the character before newLineChar, and one after newLineChar till the end.
        // It will then recursively call itself for each new line char until the original text is all displayed.
        PrintUserCmdText(client, text.substr(0, newLineChar));
        PrintUserCmdText(client, text.substr(newLineChar + 1, std::wstring::npos));
    }
}

// Print message to all ships within the specific number of meters of the player.
void Api::PrintLocalUserCmdText(ClientId client, const std::wstring_view msg, float distance)
{
    uint ship;
    pub::Player::GetShip(client, ship);

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(ship, pos, rot);

    uint system;
    pub::Player::GetSystem(client, system);

    // For all players in system...
    PlayerData* playerDb = nullptr;
    while ((playerDb = Players.traverse_active(playerDb)))
    {
        // Get the this player's current system and location in the system.
        ClientId client2 = playerDb->onlineId;
        uint system2 = 0;
        pub::Player::GetSystem(client2, system2);
        if (system != system2)
        {
            continue;
        }

        uint ship2;
        pub::Player::GetShip(client2, ship2);

        Vector pos2;
        Matrix rot2;
        pub::SpaceObj::GetLocation(ship2, pos2, rot2);

        // Is player within the specified range of the sending char.
        if (Hk::Math::Distance3D(pos, pos2) > distance)
        {
            continue;
        }

        PrintUserCmdText(client2, msg);
    }
}
