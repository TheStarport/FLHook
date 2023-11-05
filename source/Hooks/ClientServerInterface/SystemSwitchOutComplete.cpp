#include "PCH.hpp"

#include "API/FLServer/Client.hpp"
#include "Global.hpp"

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

        if (const auto skip = CallPlugins(&Plugin::OnSystemSwitchOutComplete, client, shipId); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SystemSwitchOutComplete(shipId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        SystemSwitchOutComplete__InnerAfter(shipId, client);

        CallPlugins(&Plugin::OnSystemSwitchOutCompleteAfter, client, shipId);
    }

} // namespace IServerImplHook
