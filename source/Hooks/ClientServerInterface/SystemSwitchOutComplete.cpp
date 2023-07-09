#include "PCH.hpp"

#include "Global.hpp"
#include "API/FLServer/Client.hpp"

namespace IServerImplHook
{

    void SystemSwitchOutComplete__InnerAfter(uint, ClientId client)
    {
        TRY_HOOK
        {
            const auto system = Hk::Client::GetPlayerSystem(client);
            // TODO: Implement event for switch out
        }
        CATCH_HOOK({})
    }
    void __stdcall SystemSwitchOutComplete(uint shipId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SystemSwitchOutComplete(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete, shipId, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SystemSwitchOutComplete(shipId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        SystemSwitchOutComplete__InnerAfter(shipId, client);

        CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete, shipId, client);
    }

}
