#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::JumpInComplete(SystemId systemId, ShipId shipId)
{
    Logger::Log(LogLevel::Trace, std::format(L"JumpInComplete(\n\tuint systemId = {}\n\tuint shipId = {}\n)", systemId, shipId));

    if (const auto skip = CallPlugins(&Plugin::OnJumpInComplete, systemId, shipId); !skip)
    {
        CallServerPreamble { Server.JumpInComplete(systemId.GetValue(), shipId.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnJumpInCompleteAfter, systemId, shipId);
}
