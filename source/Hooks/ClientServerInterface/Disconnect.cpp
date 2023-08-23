#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void DisConnect__Inner(ClientId client, EFLConnection)
    {
        if (client <= MaxClientId && client > 0 && !ClientInfo[client].disconnected)
        {
            ClientInfo[client].disconnected = true;
            ClientInfo[client].moneyFix.clear();
            ClientInfo[client].tradePartner = 0;

            // TODO: implement event for disconnect
        }
    }
    void __stdcall DisConnect(ClientId client, EFLConnection conn)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"DisConnect(\n\tClientId client = {}\n)", client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect, client, conn);

        DisConnect__Inner(client, conn);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.DisConnect(client, conn); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__DisConnect, client, conn);
    }
}