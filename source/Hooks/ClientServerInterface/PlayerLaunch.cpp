#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{

    void PlayerLaunch__Inner(uint shipId, ClientId client)
    {
        TRY_HOOK
        {
            ClientInfo[client].ship = shipId;
            ClientInfo[client].killsInARow = 0;
            ClientInfo[client].cruiseActivated = false;
            ClientInfo[client].thrusterActivated = false;
            ClientInfo[client].engineKilled = false;
            ClientInfo[client].tradelane = false;

            // adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
            std::wstring charName = StringUtils::ToLower(Hk::Client::GetCharacterNameByID(client).Handle());
            for (const auto& i : ClientInfo[client].moneyFix)
            {
                if (i.character == charName)
                {
                    Hk::Player::AddCash(charName, i.amount);
                    ClientInfo[client].moneyFix.remove(i);
                    break;
                }
            }
        }
        CATCH_HOOK({})
    }
    void PlayerLaunch__InnerAfter([[maybe_unused]] uint shipId, ClientId client)
    {
        TRY_HOOK
        {
            if (!ClientInfo[client].lastExitedBaseId)
            {
                ClientInfo[client].lastExitedBaseId = 1;
            }
        }
        CATCH_HOOK({})
    }

    void __stdcall PlayerLaunch(uint shipId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"PlayerLaunch(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch, shipId, client);

        CHECK_FOR_DISCONNECT;

        PlayerLaunch__Inner(shipId, client);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.PlayerLaunch(shipId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        PlayerLaunch__InnerAfter(shipId, client);

        CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch, shipId, client);
    }

} 