#include "PCH.hpp"

#include "Global.hpp"

namespace IServerImplHook
{
    void TerminateTrade__InnerAfter(ClientId client, int accepted)
    {
        TRY_HOOK
        {
            if (accepted)
            {
                // save both chars to prevent cheating in case of server crash
                Hk::Player::SaveChar(client);
                if (ClientInfo[client].tradePartner)
                {
                    Hk::Player::SaveChar(ClientInfo[client].tradePartner);
                }
            }

            if (ClientInfo[client].tradePartner)
            {
                ClientInfo[ClientInfo[client].tradePartner].tradePartner = 0;
            }
            ClientInfo[client].tradePartner = 0;
        }
        CATCH_HOOK({})
    }
    void __stdcall TerminateTrade(ClientId client, int accepted)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"TerminateTrade(\n\tClientId client = {}\n\tint accepted = {}\n)", client, accepted));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade, client, accepted);

        CHECK_FOR_DISCONNECT;

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.TerminateTrade(client, accepted); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        TerminateTrade__InnerAfter(client, accepted);

        CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade, client, accepted);
    }
}