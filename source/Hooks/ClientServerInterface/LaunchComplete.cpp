#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void IServerImplHook::LaunchCompleteInner(BaseId, ShipId shipId)
{
    TryHook
    {

        if (const auto client = ShipId(shipId).GetPlayer().value_or(ClientId()))
        {
            auto& data = client.GetData();
            data.spawnTime = TimeUtils::UnixTime<std::chrono::milliseconds>(); // save for anti-dockkill
            // is there spawnprotection?
            if (FLHookConfig::i()->general.antiDockKill > 0)
            {
                data.spawnProtected = true;
            }
            else
            {
                data.spawnProtected = false;
            }
        }
    }
    CatchHook({});
}

void __stdcall IServerImplHook::LaunchComplete(BaseId baseId, ShipId shipId)
{
    Logger::Log(LogLevel::Trace, std::format(L"LaunchComplete(\n\tuint baseId = {}\n\tuint shipId = {}\n)", baseId, shipId));

    const auto skip = CallPlugins(&Plugin::OnLaunchComplete, baseId, shipId);

    LaunchCompleteInner(baseId, shipId);

    if (!skip)
    {
        CallServerPreamble { Server.LaunchComplete(baseId.GetValue(), shipId.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLaunchCompleteAfter, baseId, shipId);
}
