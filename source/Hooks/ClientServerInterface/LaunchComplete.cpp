#include "PCH.hpp"

#include "Global.hpp"
#include "API/API.hpp"

namespace IServerImplHook
{
    void LaunchComplete__Inner(uint, uint shipId)
    {
        TRY_HOOK
        {

            if (ClientId client = Hk::Client::GetClientIdByShip(shipId).Unwrap())
            {
                ClientInfo[client].tmSpawnTime = TimeUtils::UnixMilliseconds(); // save for anti-dockkill
                // is there spawnprotection?
                if (FLHookConfig::i()->general.antiDockKill > 0)
                {
                    ClientInfo[client].spawnProtected = true;
                }
                else
                {
                    ClientInfo[client].spawnProtected = false;
                }
            }
        }
        CATCH_HOOK({});
    }
    void __stdcall LaunchComplete(uint baseId, uint shipId)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"LaunchComplete(\n\tuint baseId = {}\n\tuint shipId = {}\n)", baseId, shipId));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LaunchComplete, baseId, shipId);

        LaunchComplete__Inner(baseId, shipId);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.LaunchComplete(baseId, shipId); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__LaunchComplete, baseId, shipId);
    }

}