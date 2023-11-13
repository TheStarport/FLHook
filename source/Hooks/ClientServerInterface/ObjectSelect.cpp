#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::GfObjSelect(unsigned int unk1, unsigned int unk2)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"GFObjSelect(\n\tunsigned int unk1 = {}\n\tunsigned int unk2 = {}\n)", unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnGfObjectSelect, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.GFObjSelect(unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfObjectSelectAfter, unk1, unk2);
}
