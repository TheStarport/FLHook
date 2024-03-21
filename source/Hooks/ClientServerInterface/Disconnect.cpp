#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Types/ClientId.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

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
    const auto msg = std::format(L"DisConnect(\n\tClientId client = {}\n)", client);

    Logger::Log(LogLevel::Trace, msg);

    const auto skip = CallPlugins(&Plugin::OnDisconnect, client, conn);

    DisConnectInner(client, conn);

    if (!skip)
    {
        static PerfTimer timer(FUNCTION_W, 100);
        timer.Start();
        TryHook
        {
            Server.DisConnect(client.GetValue(), conn);
            FLHook::instance->clientList->PlayerDisconnect(client.GetValue());
        }
        CatchHook({});
    };

    CallPlugins(&Plugin::OnDisconnectAfter, client, conn);
}
