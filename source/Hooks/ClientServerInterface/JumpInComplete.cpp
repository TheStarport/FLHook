#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::JumpInComplete(SystemId systemId, Id shipId)
{
    TRACE(L"JumpInComplete({0}{1})", { L"systemId", std::to_wstring(systemId.GetValue()) }, { L"shipId", std::to_wstring(shipId.GetValue()) })

    auto ship = shipId.AsShip();
    if (const auto skip = CallPlugins(&Plugin::OnJumpInComplete, systemId, ship); !skip)
    {
        CallServerPreamble { Server.JumpInComplete(systemId.GetValue(), shipId.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnJumpInCompleteAfter, systemId, ship);
}
