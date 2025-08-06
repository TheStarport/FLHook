#include "API/FLHook/TaskScheduler.hpp"
#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/PluginManager.hpp"

void __stdcall IServerImplHook::SpScanCargo(const uint& unk1, const uint& unk2, uint unk3)
{
    if (const auto skip = CallPlugins(&Plugin::OnSpScanCargo, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.SPScanCargo(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpScanCargoAfter, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::ReqAddItem(uint goodId, const char* hardpoint, int count, float status, bool mounted, ClientId client)
{
    const std::wstring hp = StringUtils::stows(hardpoint);
    TRACE("IServerImplHook::ReqAddItem goodId={{goodId}},hardpoint={{hardpoint}},count={{count}},status={{status}},mounted={{mounted}},client={{client}}",
          { "goodId", goodId },
          { "hardpoint", hardpoint },
          { "count", count },
          { "status", status },
          { "mounted", mounted },
          { "client", client });

    auto good = GoodId(goodId);

    if (const auto skip = CallPlugins(&Plugin::OnRequestAddItem, client, good, std::wstring_view(hp), count, status, mounted); !skip)
    {
        CallServerPreamble { Server.ReqAddItem(goodId, hardpoint, count, status, mounted, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestAddItemAfter, client, good, std::wstring_view(hp), count, status, mounted);
}

void __stdcall IServerImplHook::ReqRemoveItem(ushort slotId, int count, ClientId client)
{
    TRACE("IServerImplHook::ReqRemoveItem slotId={{slotId}},count={{count}},clientId={{clientId}}",
          { "slotId", slotId },
          { "count", count },
          { "clientId", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestRemoveItem, client, slotId, count); !skip)
    {
        CallServerPreamble { Server.ReqRemoveItem(slotId, count, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestRemoveItemAfter, client, slotId, count);
}

void __stdcall IServerImplHook::ReqModifyItem(ushort slotId, const char* hardpoint, int count, float status, bool mounted, ClientId client)
{
    std::wstring hp = StringUtils::stows(hardpoint);

    TRACE(
        "IServerImplHook::ReqModifyItem goodId={{goodId}}, hardpoint={{hardpoint}}, count={{count}}, status={{status}}, mounted={{mounted}}, client={{client}}",
        { "goodId", slotId },
        { "hardpoint", hardpoint },
        { "count", count },
        { "status", status },
        { "mounted", mounted },
        { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestModifyItem, client, slotId, std::wstring_view(hp), count, status, mounted); !skip)
    {
        CallServerPreamble { Server.ReqModifyItem(slotId, hardpoint, count, status, mounted, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestModifyItemAfter, client, slotId, std::wstring_view(hp), count, status, mounted);
}

void __stdcall IServerImplHook::JettisonCargo(ClientId client, const XJettisonCargo& jc)
{
    TRACE("IServerImplHook::JettisonCargo client={{clientId}}", { "clientId", client });

    if (const auto skip = CallPlugins(&Plugin::OnCargoJettison, client, jc); !skip)
    {
        CallServerPreamble { Server.JettisonCargo(client.GetValue(), jc); }
        CallServerPostamble(true, );
    }

    // Queue save to prevent item duplication
    Timer::AddOneShot(
        [client]
        {
            if (client.IsValidClientId() && !client.InCharacterSelect())
            {
                pub::Save(client.GetValue(), 1);
            }
        },
        3s);

    CallPlugins(&Plugin::OnCargoJettisonAfter, client, jc);
}

void __stdcall IServerImplHook::TractorObjects(ClientId client, const XTractorObjects& to)
{
    TRACE("IServerImplHook::TractorObjects clientId={{clientId}}", { "clientId", client });

    if (const auto skip = CallPlugins(&Plugin::OnTractorObjects, client, to); !skip)
    {
        CallServerPreamble { Server.TractorObjects(client.GetValue(), to); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTractorObjectsAfter, client, to);
}
