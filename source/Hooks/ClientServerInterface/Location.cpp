#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "API/Utils/Logger.hpp"

void __stdcall IServerImplHook::SetVisitedState(ClientId client, uint objHash, int state)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"SetVisitedState(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));

    if (const auto skip = CallPlugins(&Plugin::OnSetVisitedState, client, objHash, state); !skip)
    {
        CallServerPreamble { Server.SetVisitedState(client.GetValue(), (uchar*)objHash, state); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetVisitedStateAfter, client, objHash, state);
}

void __stdcall IServerImplHook::RequestBestPath(ClientId client, RequestBestPathStruct* bestPath, int unused)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"RequestBestPath(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, (uint)bestPath, unused));

    if (const auto skip = CallPlugins(&Plugin::OnRequestBestPath, client, bestPath, unused); !skip)
    {
        CallServerPreamble { Server.RequestBestPath(client.GetValue(), (uchar*)bestPath, unused); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestBestPathAfter, client, bestPath, unused);
}

void __stdcall IServerImplHook::LocationInfoRequest(unsigned int unk1, unsigned int unk2, bool unk3)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"LocationInfoRequest(\n\tunsigned int unk1 = {}\n\tunsigned int unk2 = {}\n\tbool unk3 = {}\n)", unk1, unk2, unk3));

    if (const auto skip = CallPlugins(&Plugin::OnRequestLocationInfo, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.LocationInfoRequest(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestLocationInfoAfter, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::LocationExit(uint locationId, ClientId client)
{
    Logger::Log(LogLevel::Trace, std::format(L"LocationExit(\n\tuint locationId = {}\n\tClientId client = {}\n)", locationId, client));

    if (const auto skip = CallPlugins(&Plugin::OnLocationExit, client, locationId); !skip)
    {
        CallServerPreamble { Server.LocationExit(locationId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLocationExitAfter, client, locationId);
}

void __stdcall IServerImplHook::LocationEnter(uint locationId, ClientId client)
{
    Logger::Log(LogLevel::Trace, std::format(L"LocationEnter(\n\tuint locationId = {}\n\tClientId client = {}\n)", locationId, client));

    if (const auto skip = CallPlugins(&Plugin::OnLocationEnter, client, locationId); !skip)
    {
        CallServerPreamble { Server.LocationEnter(locationId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnLocationEnterAfter, client, locationId);
}
