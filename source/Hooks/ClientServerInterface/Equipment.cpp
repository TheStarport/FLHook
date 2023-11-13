#include "PCH.hpp"

#include "API/FLServer/Client.hpp"
#include "API/FLServer/Player.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

void ActivateEquipInner(ClientId client, const XActivateEquip& aq)
{
    TryHook
    {
        int _;

        for (const auto cargoList = Hk::Player::EnumCargo(client, _).Raw(); auto& cargo : cargoList.value())
        {
            if (cargo.id == aq.id)
            {
                Archetype::Equipment* eq = Archetype::GetEquipment(cargo.archId);
                const EquipmentType eqType = Hk::Client::GetEqType(eq);

                if (eqType == ET_ENGINE)
                {
                    ClientInfo::At(client).engineKilled = !aq.activate;
                    if (!aq.activate)
                    {
                        ClientInfo::At(client).cruiseActivated = false; // enginekill enabled
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
        CallServerPreamble { Server.ActivateEquip(client, aq); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnActivateEquipAfter, client, aq);
}

void __stdcall IServerImplHook::ReqEquipment(const EquipDescList& edl, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"ReqEquipment(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestEquipment, client, edl); !skip)
    {
        CallServerPreamble { Server.ReqEquipment(edl, client); }
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
        CallServerPreamble { Server.FireWeapon(client, fwi); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnFireWeaponAfter, client, fwi);
}

void __stdcall IServerImplHook::SetWeaponGroup(ClientId client, uint unk1, int unk2)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SetWeaponGroup(\n\tClientId client = {}\n\tuint unk1 = 0x{:08X}\n\tint unk2 = {}\n)", client, unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnSetWeaponGroup, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.SetWeaponGroup(client, (uchar*)unk1, unk2); }
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
        CallServerPreamble { Server.SPRequestUseItem(ui, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpRequestUseItemAfter, client, ui);
}

void ActivateThrustersInner(ClientId client, const XActivateThrusters& at)
{
    TryHook { ClientInfo::At(client).thrusterActivated = at.activate; }
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
        CallServerPreamble { Server.ActivateThrusters(client, at); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnActivateThrustersAfter, client, at);
}

void ActivateCruiseInner(ClientId client, const XActivateCruise& ac)
{
    TryHook { ClientInfo::At(client).cruiseActivated = ac.activate; }
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
        CallServerPreamble { Server.ActivateCruise(client, ac); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnActivateCruiseAfter, client, ac);
}
