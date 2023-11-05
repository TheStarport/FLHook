#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall GFObjSelect(unsigned int _genArg1, unsigned int _genArg2)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"GFObjSelect(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n)", _genArg1, _genArg2));

        if (const auto skip = CallPlugins(&Plugin::OnGfObjectSelect, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.GFObjSelect(_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnGfObjectSelectAfter, _genArg1, _genArg2);
    }
} // namespace IServerImplHook
