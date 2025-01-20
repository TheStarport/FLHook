#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/FLHook.hpp"

bool IServerImplHook::OnConnectInner(ClientId client)
{
    TryHook
    {
        // If Id is too high due to disconnect buffer time then manually drop the connection.
        if (client.GetValue() > MaxClientId)
        {
            TRACE(L"Blocking connect due to invalid Id {0}", { L"client", std::to_wstring(client.GetValue()) });

            CDPClientProxy* cdpClient = FLHook::clientProxyArray[client.GetValue() - 1];
            if (!cdpClient)
            {
                return false;
            }
            cdpClient->Disconnect();
            return false;
        }

        auto& data = client.GetData();
        // If this client is in the anti-F1 timeout then force the disconnect.
        if (data.timeDisconnect > TimeUtils::UnixTime<std::chrono::milliseconds>())
        {
            // manual disconnect
            CDPClientProxy* cdpClient = FLHook::clientProxyArray[client.GetValue() - 1];
            if (!cdpClient)
            {
                return false;
            }
            cdpClient->Disconnect();
            return false;
        }

        FLHook::instance->clientList->PlayerConnect(client.GetValue());
        data.connects++;
        FLHook::ClearClientInfo(client);
    }
    CatchHook({});

    return true;
}

void __stdcall IServerImplHook::OnConnect(ClientId client)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    const auto skip = CallPlugins(&Plugin::OnConnect, client);

    if (const bool innerCheck = OnConnectInner(client); !innerCheck)
    {
        return;
    }
    if (!skip)
    {
        CallServerPreamble { Server.OnConnect(client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnConnectAfter, client);
}
