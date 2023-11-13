#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/FLHook.hpp"

bool IServerImplHook::OnConnectInner(ClientId client)
{
    TryHook
    {
        // If Id is too high due to disconnect buffer time then manually drop
        // the connection.
        if (client > MaxClientId)
        {
            FLHook::GetLogger().Log(LogLevel::Trace,
                                    std::format(L"INFO: Blocking connect in {} due to invalid id, id={}", StringUtils::stows(__FUNCTION__), client));
            CDPClientProxy* cdpClient = clientProxyArray[client - 1];
            if (!cdpClient)
            {
                return false;
            }
            cdpClient->Disconnect();
            return false;
        }

        // If this client is in the anti-F1 timeout then force the disconnect.
        if (ClientInfo::At(client).tmF1TimeDisconnect > TimeUtils::UnixMilliseconds())
        {
            // manual disconnect
            CDPClientProxy* cdpClient = clientProxyArray[client - 1];
            if (!cdpClient)
            {
                return false;
            }
            cdpClient->Disconnect();
            return false;
        }

        ClientInfo::At(client).connects++;
        FLHook::ClearClientInfo(client);
    }
    CatchHook({})

        return true;
}

void OnConnectInnerAfter([[maybe_unused]] ClientId client)
{
    TryHook
    {
        // TODO: implement event for OnConnect
    }
    CatchHook({})
}

void __stdcall IServerImplHook::OnConnect(ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"OnConnect(\n\tClientId client = {}\n)", client));

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
    OnConnectInnerAfter(client);

    CallPlugins(&Plugin::OnConnectAfter, client);
}
