#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    bool OnConnect__Inner(ClientId client)
    {
        TRY_HOOK
        {
            // If Id is too high due to disconnect buffer time then manually drop
            // the connection.
            if (client > MaxClientId)
            {
                Logger::i()->Log(LogLevel::Trace,
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
            if (ClientInfo[client].tmF1TimeDisconnect > TimeUtils::UnixMilliseconds())
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

            ClientInfo[client].connects++;
            ClearClientInfo(client);
        }
        CATCH_HOOK({})

        return true;
    }

    void OnConnect__InnerAfter([[maybe_unused]] ClientId client)
    {
        TRY_HOOK
        {
            // TODO: implement event for OnConnect
        }
        CATCH_HOOK({})
    }

    void __stdcall OnConnect(ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"OnConnect(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnConnect, client);

        if (const bool innerCheck = OnConnect__Inner(client); !innerCheck)
        {
            return;
        }
        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.OnConnect(client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        OnConnect__InnerAfter(client);

        CallPlugins(&Plugin::OnConnectAfter, client);
    }
} // namespace IServerImplHook
