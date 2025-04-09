#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::MineAsteroid(SystemId systemId, const Vector& pos, ArchId crateId, ArchId lootId, uint count, ClientId client)
{
    TRACE(L"{0} {1} {2} {3} {4}",
          { L"systemId", std::to_wstring(systemId.GetValue()) },
          { L"crateId", std::to_wstring(crateId) },
          { L"lootId", std::to_wstring(lootId) },
          { L"count", std::to_wstring(count) },
          { L"clientId", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnMineAsteroid, client, systemId, pos, crateId, lootId, count); !skip)
    {
        CallServerPreamble { Server.MineAsteroid(systemId.GetValue(), pos, crateId, lootId, count, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnMineAsteroidAfter, client, systemId, pos, crateId, lootId, count);
}
