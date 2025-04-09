#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::AbortMission(ClientId client, uint unk1)
{
    TRACE(L"{0} {1}", { L"client", std::to_wstring(client.GetValue()) }, { L"unk1", std::to_wstring(unk1) });

    if (const auto skip = CallPlugins(&Plugin::OnAbortMission, client, unk1); !skip)
    {
        CallServerPreamble { Server.AbortMission(client.GetValue(), unk1); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnAbortMissionAfter, client, unk1);
}

void __stdcall IServerImplHook::MissionResponse(unsigned int unk1, unsigned long unk2, bool unk3, ClientId client)
{
    TRACE(L"{0} {1} {2} {3}",
          { L"unk1", std::to_wstring(unk1) },
          { L"unk2", std::to_wstring(unk2) },
          { L"unk3", std::to_wstring(unk3) },
          { L"client", std::to_wstring(client.GetValue()) }, );

    if (const auto skip = CallPlugins(&Plugin::OnMissionResponse, client, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.MissionResponse(unk1, unk2, unk3, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnMissionResponseAfter, client, unk1, unk2, unk3);
}
