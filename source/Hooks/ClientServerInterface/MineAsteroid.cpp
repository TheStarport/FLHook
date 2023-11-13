#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::MineAsteroid(uint systemId, const Vector& pos, uint crateId, uint lootId, uint count, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace,
                            std::format(L"MineAsteroid(\n\tuint systemId = {}\n\tuint crateId = {}\n\tuint lootId = {}\n\tuint count = "
                                        L"{}\n\tClientId client = {}\n)",
                                        systemId,
                                        crateId,
                                        lootId,
                                        count,
                                        client));

    if (const auto skip = CallPlugins(&Plugin::OnMineAsteroid, client, systemId, pos, crateId, lootId, count); !skip)
    {
        CallServerPreamble { Server.MineAsteroid(systemId, pos, crateId, lootId, count, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnMineAsteroidAfter, client, systemId, pos, crateId, lootId, count);
}
