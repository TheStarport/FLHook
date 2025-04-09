#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::Hail(unsigned int unk1, unsigned int unk2, unsigned int unk3)
{
    TRACE(L"{0} {1} {2}", { L"unk1", std::to_wstring(unk1) }, { L"unk2", std::to_wstring(unk2) }, { L"unk3", std::to_wstring(unk3) });

    if (const auto skip = CallPlugins(&Plugin::OnHail, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.Hail(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnHail, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::RequestEvent(int eventType, Id shipId, Id dockTarget, uint unk1, ulong unk2, ClientId client)
{
    auto ship = shipId.AsShip();
    auto dockTargetObj = dockTarget.AsObject();

    TRACE(L"{0} {1} {2} {3} {4} {5}",
          { L"eventType", std::to_wstring(eventType) },
          { L"shipId", std::to_wstring(shipId.GetValue()) },
          { L"dockTarget", std::to_wstring(dockTarget.GetValue()) },
          { L"unk1", std::to_wstring(unk1) },
          { L"unk2", std::to_wstring(unk2) },
          { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestEvent, client, eventType, ship, dockTargetObj, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestEvent(eventType, shipId.GetValue(), dockTarget.GetValue(), unk1, unk2, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestEventAfter, client, eventType, ship, dockTargetObj, unk1, unk2);
}

void __stdcall IServerImplHook::RequestCancel(int eventType, Id shipId, Id dockTarget, ulong unk2, ClientId client)
{
    auto ship = shipId.AsShip();
    auto dockTargetObj = dockTarget.AsObject();

    TRACE(L"{0} {1} {2} {3} {4}",
          { L"eventType", std::to_wstring(eventType) },
          { L"shipId", std::to_wstring(shipId.GetValue()) },
          { L"dockTarget", std::to_wstring(dockTarget.GetValue()) },
          { L"unk2", std::to_wstring(unk2) },
          { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestCancel, client, eventType, ship, dockTargetObj, unk2); !skip)
    {
        CallServerPreamble { Server.RequestCancel(eventType, shipId.GetValue(), dockTarget.GetValue(), unk2, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCancelAfter, client, eventType, ship, dockTargetObj, unk2);
}

void __stdcall IServerImplHook::InterfaceItemUsed(uint unk1, uint unk2)
{
    TRACE(L"{0} {1}", { L"unk1", std::to_wstring(unk1) }, { L"unk2", std::to_wstring(unk2) });

    if (const auto skip = CallPlugins(&Plugin::OnInterfaceItemUsed, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.InterfaceItemUsed(unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnInterfaceItemUsedAfter, unk1, unk2);
}

void __stdcall IServerImplHook::PopupDialog(ClientId client, ::PopupDialog buttonClicked)
{
    TRACE(L"{0} {1}", { L"client", std::to_wstring(client.GetValue()) }, { L"buttonClicked", std::to_wstring(static_cast<uint>(buttonClicked)) });

    if (const auto skip = CallPlugins(&Plugin::OnPopupDialogueConfirm, client, buttonClicked); !skip)
    {
        CallServerPreamble { Server.PopUpDialog(client.GetValue(), static_cast<uint>(buttonClicked)); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnPopupDialogueConfirmAfter, client, buttonClicked);
}

void __stdcall IServerImplHook::SetInterfaceState(ClientId client, uint unk1, int unk2)
{

    TRACE(L"{0} {1} {2}", { L"client", std::to_wstring(client.GetValue()) }, { L"unk1", std::to_wstring(unk1) }, { L"unk2", std::to_wstring(unk2) });

    if (const auto skip = CallPlugins(&Plugin::OnSetInterfaceState, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.SetInterfaceState(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetInterfaceStateAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::RequestGroupPositions(ClientId client, uint unk1, int unk2)
{
    TRACE(L"{0} {1} {2}", { L"client", std::to_wstring(client.GetValue()) }, { L"unk1", std::to_wstring(unk1) }, { L"unk2", std::to_wstring(unk2) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestGroupPositions, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestGroupPositions(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestGroupPositionsAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::SetTarget(ClientId client, const XSetTarget& st)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnSetTarget, client, st); !skip)
    {
        CallServerPreamble { Server.SetTarget(client.GetValue(), st); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetTargetAfter, client, st);
}
