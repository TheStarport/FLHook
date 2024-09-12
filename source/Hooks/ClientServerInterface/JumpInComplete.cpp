#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::JumpInComplete(SystemId systemId, Id shipId)
{
    Logger::Trace(std::format(L"JumpInComplete(\n\tuint systemId = {}\n\tuint shipId = {}\n)", systemId, shipId));

    auto ship = shipId.AsShip();
    if (const auto skip = CallPlugins(&Plugin::OnJumpInComplete, systemId, ship); !skip)
    {
        CallServerPreamble { Server.JumpInComplete(systemId.GetValue(), shipId.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnJumpInCompleteAfter, systemId, ship);
}
