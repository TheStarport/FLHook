#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/PluginManager.hpp"
#include "Exceptions/StopProcessingException.hpp"

void __stdcall IServerImplHook::JumpInComplete(SystemId systemId, Id shipId)
{
    TRACE("IServerImplHook::JumpInComplete systemId={{systemId}} shipId={{shipId}}", { "systemId", systemId }, { "shipId", shipId });

    auto ship = shipId.AsShip();
    if (const auto skip = CallPlugins(&Plugin::OnJumpInComplete, systemId, ship); !skip)
    {
        CallServerPreamble { Server.JumpInComplete(systemId.GetValue(), shipId.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnJumpInCompleteAfter, systemId, ship);
}
