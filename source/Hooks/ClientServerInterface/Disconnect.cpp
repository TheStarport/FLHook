#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Logger.hpp"

void DisConnectInner(ClientId client, EFLConnection)
{
    auto& data = client.GetData();
    if (!data.disconnected)
    {
        data.disconnected = true;
        data.moneyFix.clear();
        data.tradePartner = ClientId();

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
        CallServerPreamble { Server.DisConnect(client.GetValue(), conn); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnDisconnectAfter, client, conn);
}
