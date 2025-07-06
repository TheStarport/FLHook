#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void IServerImplHook::ActivateEquipInner(ClientId client, const XActivateEquip& aq)
{
    TryHook
    {
        auto& data = client.GetData();

        for (const auto cargoList = client.GetEquipCargo().Raw(); const auto& cargo : cargoList.value()->equip)
        {
            if (cargo.id != aq.id)
            {
                continue;
            }
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
    CatchHook({})
}
void __stdcall IServerImplHook::ActivateEquip(ClientId client, const XActivateEquip& aq)
{

    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

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
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestEquipment, client, edl); !skip)
    {
        CallServerPreamble { Server.ReqEquipment(edl, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestEquipmentAfter, client, edl);
}

void __stdcall IServerImplHook::FireWeapon(ClientId client, const XFireWeaponInfo& fwi)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

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
    TRACE(L"{0} {1} {2})",
          { L"client", std::to_wstring(client.GetValue()) },
          { L"unk1", std::to_wstring(unk1) },
          { L"unk2", std::to_wstring(unk2) });

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
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnSpRequestUseItem, client, ui); !skip)
    {
        CallServerPreamble { Server.SPRequestUseItem(ui, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpRequestUseItemAfter, client, ui);
}

void IServerImplHook::ActivateThrustersInner(ClientId client, const XActivateThrusters& at)
{
    TryHook { client.GetData().thrusterActivated = at.activate; }
    CatchHook({})
}

void __stdcall IServerImplHook::ActivateThrusters(ClientId client, const XActivateThrusters& at)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

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

void IServerImplHook::ActivateCruiseInner(ClientId client, const XActivateCruise& ac)
{
    TryHook { client.GetData().cruiseActivated = ac.activate; }
    CatchHook({})
}

void __stdcall IServerImplHook::ActivateCruise(ClientId client, const XActivateCruise& ac)
{

    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

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
