#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::MineAsteroid(SystemId systemId, const Vector& pos, Id crateId, Id lootId, uint count, ClientId client)
{
    TRACE("{{systemId}} {{crateId}} {{lootId}} {{count}} {{clientId}}",
          { "systemId", systemId },
          { "crateId", crateId },
          { "lootId", lootId },
          { "count", count },
          { "clientId", client });

    if (const auto skip = CallPlugins(&Plugin::OnMineAsteroid, client, systemId, pos, crateId, lootId, count); !skip)
    {
        CallServerPreamble { Server.MineAsteroid(systemId.GetValue(), pos, crateId.GetValue(), lootId.GetValue(), count, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnMineAsteroidAfter, client, systemId, pos, crateId, lootId, count);
}
