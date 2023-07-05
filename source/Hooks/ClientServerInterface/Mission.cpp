#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void __stdcall AbortMission(ClientId client, uint _genArg1)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"AbortMission(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission, client, _genArg1); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.AbortMission(client, _genArg1); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__AbortMission, client, _genArg1);
    }

    void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"MissionResponse(\n\tunsigned int _genArg1 = {}\n\tunsigned long _genArg2 = {}\n\tbool _genArg3 = "
                                     "{}\n\tClientId client = {}\n)",
                                     _genArg1,
                                     _genArg2,
                                     _genArg3,
                                     client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.MissionResponse(_genArg1, _genArg2, _genArg3, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, client);
    }

}