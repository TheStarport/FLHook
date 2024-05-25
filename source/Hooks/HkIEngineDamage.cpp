#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Core/IEngineHook.hpp"

#include <magic_enum.hpp>

using namespace magic_enum::bitwise_operators;

void __fastcall IEngineHook::ShipExplosionHit(Ship* ship, void* edx, ExplosionDamageEvent* explosion, DamageList* dmgList)
{
    CallPlugins(&Plugin::OnShipExplosionHit, ship, explosion, dmgList);

    using IShipHullDmgType = void(__thiscall*)(Ship*, ExplosionDamageEvent*, DamageList*);
    static_cast<IShipHullDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageHull)))(ship, explosion, dmgList);
}

void __fastcall IEngineHook::ShipHullDamage(Ship* ship, void* edx, float damage, DamageList* dmgList)
{
    CallPlugins(&Plugin::OnShipHullDmg, ship, damage, dmgList);

    using IShipHullDmgType = void(__thiscall*)(Ship*, float, DamageList*);
    static_cast<IShipHullDmgType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::DamageHull)))(ship, damage, dmgList);
}

void __fastcall IEngineHook::SolarHullDamage(Solar* solar, void* edx, float damage, DamageList* dmgList)
{
    CallPlugins(&Plugin::OnSolarHullDmg, solar, damage, dmgList);

    using ISolarHullDmgType = void(__thiscall*)(Solar*, float, DamageList*);
    static_cast<ISolarHullDmgType>(iSolarVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::DamageHull)))(solar, damage, dmgList);
}

bool IEngineHook::AllowPlayerDamage(ClientId client, ClientId clientTarget)
{
    auto [rval, skip] = CallPlugins<bool>(&Plugin::OnAllowPlayerDamage, client, clientTarget);
    if (skip)
    {
        return rval;
    }

    const auto& config = FLHook::GetConfig();
    if (config.general.damageMode == DamageMode::None)
    {
        return false;
    }

    if (clientTarget)
    {
        if (!magic_enum::enum_flags_test(config.general.damageMode, DamageMode::PvP))
        {
            return false;
        }

        const auto time = TimeUtils::UnixTime<std::chrono::milliseconds>();

        // anti-dockkill check
        if (auto& targetData = clientTarget.GetData(); targetData.spawnProtected)
        {
            if (time - targetData.spawnTime <= config.general.antiDockKill)
            {
                return false; // target is protected
            }

            targetData.spawnProtected = false;
        }

        if (auto& clientData = client.GetData(); clientData.spawnProtected)
        {
            if (time - clientData.spawnTime <= config.general.antiDockKill)
            {
                return false; // target may not shoot
            }

            clientData.spawnProtected = false;
        }

        // no-pvp check
        uint systemId;
        pub::Player::GetSystem(client.GetValue(), systemId);
        if (std::ranges::find(config.general.noPVPSystemsHashed, systemId) != config.general.noPVPSystemsHashed.end())
        {
            return false;
        }
    }

    return true;
}
