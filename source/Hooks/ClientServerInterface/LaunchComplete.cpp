#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/PluginManager.hpp"
#include "Exceptions/StopProcessingException.hpp"

void IServerImplHook::LaunchCompleteInner(BaseId, const ShipId& shipId)
{
    TryHook
    {

        if (const auto client = shipId.GetPlayer().Unwrap())
        {
            auto& data = client.GetData();
            data.spawnTime = TimeUtils::UnixTime<std::chrono::milliseconds>(); // save for anti-dockkill
            // is there spawnprotection?
            if (FLHook::GetConfig()->general.antiDockKill > 0)
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

void __stdcall IServerImplHook::LaunchComplete(BaseId baseId, Id shipId)
{
    TRACE("IServerImplHook::LaunchComplete baseId={{baseId}} shipId={{shipId}}", { "baseId", baseId }, { "shipId", shipId });

    auto ship = shipId.AsShip();

    const auto skip = CallPlugins(&Plugin::OnLaunchComplete, baseId, ship);

    LaunchCompleteInner(baseId, ship);

    if (!skip)
    {
        CallServerPreamble { Server.LaunchComplete(baseId.GetValue(), shipId.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLaunchCompleteAfter, baseId, ship);
}
