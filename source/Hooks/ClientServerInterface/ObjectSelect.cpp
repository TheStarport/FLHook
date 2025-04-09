#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "API/Utils/Logger.hpp"

void __stdcall IServerImplHook::GfObjSelect(unsigned int unk1, unsigned int unk2)
{
    TRACE(L"{0} {1}", {L"unk1", std::to_wstring(unk1)}, {L"unk2", std::to_wstring(unk2)})

    if (const auto skip = CallPlugins(&Plugin::OnGfObjectSelect, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.GFObjSelect(unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfObjectSelectAfter, unk1, unk2);
}
