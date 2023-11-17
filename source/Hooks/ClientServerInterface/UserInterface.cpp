#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::Hail(unsigned int unk1, unsigned int unk2, unsigned int unk3)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"Hail(\n\tunsigned int unk1 = {}\n\tunsigned int unk2 = {}\n\tunsigned int unk3 = {}\n)", unk1, unk2, unk3));

    if (const auto skip = CallPlugins(&Plugin::OnHail, unk1, unk2, unk3); !skip)
    {
        CallServerPreamble { Server.Hail(unk1, unk2, unk3); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnHail, unk1, unk2, unk3);
}

void __stdcall IServerImplHook::RequestEvent(int eventType, ShipId shipId, ObjectId dockTarget, uint unk1, ulong unk2, ClientId client)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"RequestEvent(\n\tint eventType = {}\n\tuint shipId = {}\n\tuint dockTarget = {}\n\tuint unk1 = {}\n\tulong unk2 = "
                                        "{}\n\tClientId client = {}\n)",
                                        eventType,
                                        shipId,
                                        dockTarget,
                                        unk1,
                                        unk2,
                                        client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestEvent, client, eventType, shipId, dockTarget, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestEvent(eventType, shipId.GetValue(), dockTarget.GetValue(), unk1, unk2, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestEventAfter, client, eventType, shipId, dockTarget, unk1, unk2);
}

void __stdcall IServerImplHook::RequestCancel(int eventType, ShipId shipId, ObjectId dockTarget, ulong unk2, ClientId client)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"RequestCancel(\n\tint eventType = {}\n\tuint shipId = {}\n\tuint unk1 = {}\n\tulong unk2 = {}\n\tClientId client = {}\n)",
                    eventType,
                    shipId,
                    dockTarget,
                    unk2,
                    client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestCancel, client, eventType, shipId, dockTarget, unk2); !skip)
    {
        CallServerPreamble { Server.RequestCancel(eventType, shipId.GetValue(), dockTarget.GetValue(), unk2, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCancelAfter, client, eventType, shipId, dockTarget, unk2);
}

void __stdcall IServerImplHook::InterfaceItemUsed(uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"InterfaceItemUsed(\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnInterfaceItemUsed, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.InterfaceItemUsed(unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnInterfaceItemUsedAfter, unk1, unk2);
}

void __stdcall IServerImplHook::PopupDialog(ClientId client, ::PopupDialog buttonClicked)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"PopupDialog(\n\tClientId client = {}\n\tuint buttonClicked = {}\n)", client, static_cast<uint>(buttonClicked)));

    if (const auto skip = CallPlugins(&Plugin::OnPopupDialogueConfirm, client, buttonClicked); !skip)
    {
        CallServerPreamble { Server.PopUpDialog(client.GetValue(), static_cast<uint>(buttonClicked)); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnPopupDialogueConfirmAfter, client, buttonClicked);
}

void __stdcall IServerImplHook::SetInterfaceState(ClientId client, uint unk1, int unk2)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"SetInterfaceState(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnSetInterfaceState, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.SetInterfaceState(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetInterfaceStateAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::RequestGroupPositions(ClientId client, uint unk1, int unk2)
{
    Logger::Log(LogLevel::Trace,
                            std::format(L"RequestGroupPositions(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestGroupPositions, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestGroupPositions(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestGroupPositionsAfter, client, unk1, unk2);
}

void __stdcall IServerImplHook::SetTarget(ClientId client, const XSetTarget& st)
{
    Logger::Log(LogLevel::Trace, std::format(L"SetTarget(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnSetTarget, client, st); !skip)
    {
        CallServerPreamble { Server.SetTarget(client.GetValue(), st); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetTargetAfter, client, st);
}
