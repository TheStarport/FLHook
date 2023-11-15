#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Logger.hpp"

void ActivateEquipInner(ClientId client, const XActivateEquip& aq)
{
    TryHook
    {
        int _;
        auto& data = client.GetData();

        for (const auto cargoList = client.EnumCargo(_).Raw(); auto& cargo : cargoList.value())
        {
            if (cargo.id == aq.id)
            {
                auto eq = EquipmentId(cargo.archId);
                const EquipmentType eqType = eq.GetType().Unwrap();

                if (eqType == EquipmentType::Engine)
                {
                    data.engineKilled = !aq.activate;
                    if (!aq.activate)
                    {
                        data.cruiseActivated = false; // enginekill enabled
                    }
                }
            }
        }
    }
    CatchHook({})
}
void __stdcall IServerImplHook::ActivateEquip(ClientId client, const XActivateEquip& aq)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"ActivateEquip(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnActivateEquip, client, aq);

    CheckForDisconnect;

    ActivateEquipInner(client, aq);

    if (!skip)
    {
        CallServerPreamble { Server.ActivateEquip(client.GetValue(), aq); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnActivateEquipAfter, client, aq);
}

void __stdcall IServerImplHook::ReqEquipment(const EquipDescList& edl, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"ReqEquipment(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestEquipment, client, edl); !skip)
    {
        CallServerPreamble { Server.ReqEquipment(edl, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestEquipmentAfter, client, edl);
}

void __stdcall IServerImplHook::FireWeapon(ClientId client, const XFireWeaponInfo& fwi)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"FireWeapon(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnFireWeapon, client, fwi);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.FireWeapon(client.GetValue(), fwi); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnFireWeaponAfter, client, fwi);
}

void __stdcall IServerImplHook::SetWeaponGroup(ClientId client, uint unk1, int unk2)
{
    FLHook::GetLogger().Log(LogLevel::Trace,
                            std::format(L"SetWeaponGroup(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnSetWeaponGroup, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.SetWeaponGroup(client.GetValue(), (uchar*)unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetWeaponGroupAfter, client, unk1, unk2);
}

// We think this is hook involving usage of nanobots and shield batteries but not sure.
void __stdcall IServerImplHook::SpRequestUseItem(const SSPUseItem& ui, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SPRequestUseItem(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnSpRequestUseItem, client, ui); !skip)
    {
        CallServerPreamble { Server.SPRequestUseItem(ui, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpRequestUseItemAfter, client, ui);
}

void ActivateThrustersInner(ClientId client, const XActivateThrusters& at)
{
    TryHook { client.GetData().thrusterActivated = at.activate; }
    CatchHook({})
}

void __stdcall IServerImplHook::ActivateThrusters(ClientId client, const XActivateThrusters& at)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"ActivateThrusters(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnActivateThrusters, client, at);

    CheckForDisconnect;

    ActivateThrustersInner(client, at);

    if (!skip)
    {
        CallServerPreamble { Server.ActivateThrusters(client.GetValue(), at); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnActivateThrustersAfter, client, at);
}

void ActivateCruiseInner(ClientId client, const XActivateCruise& ac)
{
    TryHook { client.GetData().cruiseActivated = ac.activate; }
    CatchHook({})
}

void __stdcall IServerImplHook::ActivateCruise(ClientId client, const XActivateCruise& ac)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"ActivateCruise(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnActivateCruise, client, ac);

    CheckForDisconnect;

    ActivateCruiseInner(client, ac);

    if (!skip)
    {
        CallServerPreamble { Server.ActivateCruise(client.GetValue(), ac); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnActivateCruiseAfter, client, ac);
}
