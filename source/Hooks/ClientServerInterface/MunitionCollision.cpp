#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

#include "Defs/FLHookConfig.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/PluginManager.hpp"
#include "Exceptions/StopProcessingException.hpp"

void __stdcall IServerImplHook::SpMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client)
{
    TRACE("IServerImplHook::SpMunitionCollision client={{client}}", { "client", client });

    const auto skip = CallPlugins(&Plugin::OnSpMunitionCollision, client, mci);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.SPMunitionCollision(mci, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpMunitionCollisionAfter, client, mci);
}
