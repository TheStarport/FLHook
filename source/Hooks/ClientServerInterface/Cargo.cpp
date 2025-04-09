#include "API/FLHook/TaskScheduler.hpp"
#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SpScanCargo(const uint& unk1, const uint& unk2, uint unk3)
{
    TRACE(L"{0},{1},{2}", { L"unk1", std::to_wstring(unk1) }, { L"unk2", std::to_wstring(unk2) }, { L"unk3", std::to_wstring(unk3) });

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
    TRACE(L"{0},{1},{2},{3},{4},{5}",
          { L"goodId", std::to_wstring(goodId) },
          { L"hardpoint", StringUtils::stows(std::string(hardpoint)) },
          { L"count", std::to_wstring(count) },
          { L"status", std::to_wstring(status) },
          { L"mounted", std::to_wstring(mounted) },
          { L"client", std::to_wstring(client.GetValue()) })

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
    TRACE(L"{0},{1},{2}",
          { L"slotId", std::to_wstring(slotId) },
          { L"count", std::to_wstring(count) },
          { L"clientId", std::to_wstring(client.GetValue()) })

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

    TRACE(L"{0},{1},{2},{3},{4},{5}",
          { L"goodId", std::to_wstring(slotId) },
          { L"hardpoint", StringUtils::stows(std::string(hardpoint)) },
          { L"count", std::to_wstring(count) },
          { L"status", std::to_wstring(status) },
          { L"mounted", std::to_wstring(mounted) },
          { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnRequestModifyItem, client, slotId, std::wstring_view(hp), count, status, mounted); !skip)
    {
        CallServerPreamble { Server.ReqModifyItem(slotId, hardpoint, count, status, mounted, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestModifyItemAfter, client, slotId, std::wstring_view(hp), count, status, mounted);
}

void __stdcall IServerImplHook::JettisonCargo(ClientId client, const XJettisonCargo& jc)
{
    TRACE(L"{0}", { L"clientId", std::to_wstring(client.GetValue()) })

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
        3000);

    CallPlugins(&Plugin::OnCargoJettisonAfter, client, jc);
}

void __stdcall IServerImplHook::TractorObjects(ClientId client, const XTractorObjects& to)
{
    TRACE(L"{0}", { L"clientId", std::to_wstring(client.GetValue()) })


    if (const auto skip = CallPlugins(&Plugin::OnTractorObjects, client, to); !skip)
    {
        CallServerPreamble { Server.TractorObjects(client.GetValue(), to); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTractorObjectsAfter, client, to);
}
