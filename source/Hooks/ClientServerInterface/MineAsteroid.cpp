#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::MineAsteroid(SystemId systemId, const Vector& pos, ArchId crateId, ArchId lootId, uint count, ClientId client)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"MineAsteroid(\n\tuint systemId = {}\n\tuint crateId = {}\n\tuint lootId = {}\n\tuint count = "
                                        L"{}\n\tClientId client = {}\n)",
                                        systemId,
                                        crateId,
                                        lootId,
                                        count,
                                        client));

    if (const auto skip = CallPlugins(&Plugin::OnMineAsteroid, client, systemId, pos, crateId, lootId, count); !skip)
    {
        CallServerPreamble { Server.MineAsteroid(systemId.GetValue(), pos, crateId, lootId, count, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnMineAsteroidAfter, client, systemId, pos, crateId, lootId, count);
}
