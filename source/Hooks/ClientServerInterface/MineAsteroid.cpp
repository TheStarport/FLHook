#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall MineAsteroid(uint systemId, const Vector& pos, uint crateId, uint lootId, uint count, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"MineAsteroid(\n\tuint systemId = {}\n\tuint crateId = {}\n\tuint lootId = {}\n\tuint count = "
                                     L"{}\n\tClientId client = {}\n)",
                                     systemId,
                                     crateId,
                                     lootId,
                                     count,
                                     client));

        if (const auto skip = CallPlugins(&Plugin::OnMineAsteroid, client, systemId, pos, crateId, lootId, count); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.MineAsteroid(systemId, pos, crateId, lootId, count, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnMineAsteroidAfter, client, systemId, pos, crateId, lootId, count);
    }

} // namespace IServerImplHook
