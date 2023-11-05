#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall SetManeuver(ClientId client, const XSetManeuver& sm)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SetManeuver(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnSetManeuver, client, sm); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetManeuver(client, sm); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSetManeuverAfter, client, sm);
    }
} // namespace IServerImplHook
