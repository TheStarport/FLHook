#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

void DisConnectInner(ClientId client, EFLConnection)
{
    if (client <= MaxClientId && client > 0 && !ClientInfo::At(client).disconnected)
    {
        ClientInfo::At(client).disconnected = true;
        ClientInfo::At(client).moneyFix.clear();
        ClientInfo::At(client).tradePartner = 0;

        // TODO: implement event for disconnect
    }
}

void __stdcall IServerImplHook::DisConnect(ClientId client, EFLConnection conn)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"DisConnect(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnDisconnect, client, conn);

    DisConnectInner(client, conn);

    if (!skip)
    {
        CallServerPreamble { Server.DisConnect(client, conn); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnDisconnectAfter, client, conn);
}
