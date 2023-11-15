#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Logger.hpp"

void JumpInCompleteInnerAfter(uint systemId, uint shipId)
{
    TryHook
    {

        // TODO: Implement event for jump in
    }
    CatchHook({})
}

void __stdcall IServerImplHook::JumpInComplete(uint systemId, uint shipId)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"JumpInComplete(\n\tuint systemId = {}\n\tuint shipId = {}\n)", systemId, shipId));

    if (const auto skip = CallPlugins(&Plugin::OnJumpInComplete, systemId, shipId); !skip)
    {
        CallServerPreamble { Server.JumpInComplete(systemId, shipId); }
        CallServerPostamble(true, );
    }
    JumpInCompleteInnerAfter(systemId, shipId);

    CallPlugins(&Plugin::OnJumpInCompleteAfter, systemId, shipId);
}
