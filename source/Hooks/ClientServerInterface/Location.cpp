#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SetVisitedState(ClientId client, uint objHash, int state)
{
    TRACE(L"{0}{1}{2}", { L"clientId", std::to_wstring(client.GetValue()) }, { L"objHash", std::to_wstring(objHash) }, { L"state", std::to_wstring(state) })

    if (const auto skip = CallPlugins(&Plugin::OnSetVisitedState, client, objHash, state); !skip)
    {
        CallServerPreamble { Server.SetVisitedState(client.GetValue(), (uchar*)objHash, state); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetVisitedStateAfter, client, objHash, state);
}

void __stdcall IServerImplHook::RequestBestPath(ClientId client, RequestBestPathStruct* bestPath, int unused)
{
    TRACE(L"{0}{1}{2}",
          { L"clientId", std::to_wstring(client.GetValue()) },
          { L"bestPathPointer", std::to_wstring((uint)bestPath) },
          { L"unused", std::to_wstring(unused) })

    if (const auto skip = CallPlugins(&Plugin::OnRequestBestPath, client, bestPath, unused); !skip)
    {
        CallServerPreamble { Server.RequestBestPath(client.GetValue(), (uchar*)bestPath, unused); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestBestPathAfter, client, bestPath, unused);
}

void __stdcall IServerImplHook::LocationInfoRequest(unsigned int unk1, unsigned int unk2, bool unk3)
{
    TRACE(L"{0}{1}{2}", { L"unk1", std::to_wstring(unk1) }, { L"unk2", std::to_wstring(unk2) }, { L"unk3", std::to_wstring(unk3) })

    if (const auto skip = CallPlugins(&Plugin::OnRequestLocationInfo, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.LocationInfoRequest(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestLocationInfoAfter, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::LocationExit(uint locationId, ClientId client)
{

    TRACE(L"{0}{1}", { L"locationId", std::to_wstring(locationId) }, { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnLocationExit, client, locationId); !skip)
    {
        CallServerPreamble { Server.LocationExit(locationId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLocationExitAfter, client, locationId);
}

void __stdcall IServerImplHook::LocationEnter(uint locationId, ClientId client)
{
    TRACE(L"{0}{1}", { L"locationId", std::to_wstring(locationId) }, { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnLocationEnter, client, locationId); !skip)
    {
        CallServerPreamble { Server.LocationEnter(locationId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLocationEnterAfter, client, locationId);
}
