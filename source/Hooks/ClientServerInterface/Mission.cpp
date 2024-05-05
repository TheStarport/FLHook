#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "API/Utils/Logger.hpp"

void __stdcall IServerImplHook::AbortMission(ClientId client, uint unk1)
{
    Logger::Trace(std::format(L"AbortMission(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    if (const auto skip = CallPlugins(&Plugin::OnAbortMission, client, unk1); !skip)
    {
        CallServerPreamble { Server.AbortMission(client.GetValue(), unk1); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnAbortMissionAfter, client, unk1);
}

void __stdcall IServerImplHook::MissionResponse(unsigned int unk1, unsigned long unk2, bool unk3, ClientId client)
{
    Logger::Trace(std::format(L"MissionResponse(\n\tunsigned int unk1 = {}\n\tunsigned long unk2 = {}\n\tbool unk3 = "
                                        "{}\n\tClientId client = {}\n)",
                                        unk1,
                                        unk2,
                                        unk3,
                                        client));

    if (const auto skip = CallPlugins(&Plugin::OnMissionResponse, client, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.MissionResponse(unk1, unk2, unk3, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnMissionResponseAfter, client, unk1, unk2, unk3);
}
