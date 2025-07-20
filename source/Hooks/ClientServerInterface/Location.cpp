#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SetVisitedState(ClientId client, uint objHash, int state)
{
    TRACE("{{clientId}} {{objHash}} {{state}}", { "clientId", client }, { "objHash", std::to_wstring(objHash) }, { "state", std::to_wstring(state) });

    if (const auto skip = CallPlugins(&Plugin::OnSetVisitedState, client, objHash, state); !skip)
    {
        CallServerPreamble { Server.SetVisitedState(client.GetValue(), (uchar*)objHash, state); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetVisitedStateAfter, client, objHash, state);
}

void __stdcall IServerImplHook::RequestBestPath(ClientId client, RequestBestPathStruct* bestPath, int unused)
{
    if (const auto skip = CallPlugins(&Plugin::OnRequestBestPath, client, bestPath, unused); !skip)
    {
        CallServerPreamble { Server.RequestBestPath(client.GetValue(), (uchar*)bestPath, unused); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestBestPathAfter, client, bestPath, unused);
}

void __stdcall IServerImplHook::LocationInfoRequest(unsigned int unk1, unsigned int unk2, bool unk3)
{
    if (const auto skip = CallPlugins(&Plugin::OnRequestLocationInfo, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.LocationInfoRequest(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestLocationInfoAfter, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::LocationExit(uint locationId, ClientId client)
{
    if (const auto skip = CallPlugins(&Plugin::OnLocationExit, client, locationId); !skip)
    {
        CallServerPreamble { Server.LocationExit(locationId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLocationExitAfter, client, locationId);
}

void __stdcall IServerImplHook::LocationEnter(uint locationId, ClientId client)
{
    if (const auto skip = CallPlugins(&Plugin::OnLocationEnter, client, locationId); !skip)
    {
        CallServerPreamble { Server.LocationEnter(locationId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLocationEnterAfter, client, locationId);
}
