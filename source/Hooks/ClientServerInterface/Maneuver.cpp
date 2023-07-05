#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void __stdcall SetManeuver(ClientId client, const XSetManeuver& sm)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SetManeuver(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver, client, sm); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetManeuver(client, sm); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SetManeuver, client, sm);
    }
} // namespace IServerImplHook