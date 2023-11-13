#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void LaunchCompleteInner(uint, uint shipId)
{
    TryHook
    {

        if (ClientId client = Hk::Client::GetClientIdByShip(shipId).Unwrap())
        {
            ClientInfo::At(client).tmSpawnTime = TimeUtils::UnixMilliseconds(); // save for anti-dockkill
            // is there spawnprotection?
            if (FLHookConfig::i()->general.antiDockKill > 0)
            {
                ClientInfo::At(client).spawnProtected = true;
            }
            else
            {
                ClientInfo::At(client).spawnProtected = false;
            }
        }
    }
    CatchHook({});
}

void __stdcall IServerImplHook::LaunchComplete(uint baseId, uint shipId)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"LaunchComplete(\n\tuint baseId = {}\n\tuint shipId = {}\n)", baseId, shipId));

    const auto skip = CallPlugins(&Plugin::OnLaunchComplete, baseId, shipId);

    LaunchCompleteInner(baseId, shipId);

    if (!skip)
    {
        CallServerPreamble { Server.LaunchComplete(baseId, shipId); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLaunchCompleteAfter, baseId, shipId);
}
