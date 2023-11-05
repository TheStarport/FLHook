#include "PCH.hpp"

#include "API/FLServer/Client.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void JumpInComplete__InnerAfter(uint systemId, uint shipId)
    {
        TRY_HOOK
        {
            const auto client = Hk::Client::GetClientIdByShip(shipId).Raw();
            if (client.has_error())
            {
                return;
            }

            // TODO: Implement event for jump in
        }
        CATCH_HOOK({})
    }

    void __stdcall JumpInComplete(uint systemId, uint shipId)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"JumpInComplete(\n\tuint systemId = {}\n\tuint shipId = {}\n)", systemId, shipId));

        if (const auto skip = CallPlugins(&Plugin::OnJumpInComplete, systemId, shipId); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.JumpInComplete(systemId, shipId); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        JumpInComplete__InnerAfter(systemId, shipId);

        CallPlugins(&Plugin::OnJumpInCompleteAfter, systemId, shipId);
    }
} // namespace IServerImplHook
