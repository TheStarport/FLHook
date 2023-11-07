#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

#include "API/API.hpp"

void PlayerLaunchInner(uint shipId, ClientId client)
{
    TryHook
    {
        ClientInfo::At(client).ship = shipId;
        ClientInfo::At(client).killsInARow = 0;
        ClientInfo::At(client).cruiseActivated = false;
        ClientInfo::At(client).thrusterActivated = false;
        ClientInfo::At(client).engineKilled = false;
        ClientInfo::At(client).tradelane = false;

        // adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
        std::wstring charName = StringUtils::ToLower(Hk::Client::GetCharacterNameByID(client).Handle());
        for (const auto& i : ClientInfo::At(client).moneyFix)
        {
            if (i.character == charName)
            {
                Hk::Player::AddCash(charName, i.amount);
                ClientInfo::At(client).moneyFix.remove(i);
                break;
            }
        }
    }
    CatchHook({})
}

void PlayerLaunchInnerAfter([[maybe_unused]] uint shipId, ClientId client)
{
    TryHook
    {
        if (!ClientInfo::At(client).lastExitedBaseId)
        {
            ClientInfo::At(client).lastExitedBaseId = 1;
        }
    }
    CatchHook({})
}

void __stdcall IServerImplHook::PlayerLaunch(uint shipId, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"PlayerLaunch(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));

    const auto skip = CallPlugins(&Plugin::OnPlayerLaunch, client, shipId);

    CheckForDisconnect;

    PlayerLaunchInner(shipId, client);

    if (!skip)
    {
        CallServerPreamble { Server.PlayerLaunch(shipId, client); }
        CallServerPostamble(true, );
    }
    PlayerLaunchInnerAfter(shipId, client);

    CallPlugins(&Plugin::OnPlayerLaunchAfter, client, shipId);
}
