#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Core/IEngineHook.hpp"

#include <magic_enum.hpp>

using namespace magic_enum::bitwise_operators;

/*
 * Called when a torp/missile/mine/wasp hits a ship
 * return false -> pass on to server.dll
 * return true -> suppress
 */
bool __stdcall IEngineHook::GuidedHit(char* ecx, char* p1, DamageList* dmgList)
{
    bool retValue = false;
    TryHook
    {
        char* p;
        memcpy(&p, ecx + 0x10, 4);
        uint client;
        memcpy(&client, p + 0xB4, 4);
        uint spaceId;
        memcpy(&spaceId, p + 0xB0, 4);
        uint inflictorShip;
        memcpy(&inflictorShip, p1 + 4, 4);

        auto [rval, skip] = CallPlugins<bool>(&Plugin::OnGuidedHit, ShipId(inflictorShip), ClientId(client), ObjectId(spaceId), dmgList);
        if (skip)
        {
            return rval;
        }

        FLHook::dmgToClient = ClientId(client);
        FLHook::dmgToSpaceId = ObjectId(spaceId);
        if (client)
        {
            // a player was hit
            if (const auto inflictorId = ShipId(inflictorShip).GetPlayer(); !inflictorId.has_value())
            {
                retValue = false; // hit by npc
            }
            else
            {
                if (!AllowPlayerDamage(inflictorId.value(), ClientId(client)))
                {
                    retValue = true;
                }

                if (FLHookConfig::i()->general.changeCruiseDisruptorBehaviour &&
                    ((dmgList->get_cause() == DamageCause::CruiseDisrupter || dmgList->get_cause() == DamageCause::UnkDisrupter) &&
                     ClientId(client).GetData().cruiseActivated))
                {
                    dmgList->set_cause(DamageCause::DummyDisrupter); // change to something else, so client won't recognize it as a disruptor
                }
            }
        }

        CallPlugins(&Plugin::OnGuidedHitAfter, ShipId(inflictorShip), ClientId(client), ObjectId(spaceId), dmgList);
    }
    CatchHook({});

    return retValue;
}

__declspec(naked) void IEngineHook::NakedGuidedHit()
{
    __asm {
		mov eax, [esp+4]
		mov edx, [esp+8]
		push ecx
		push edx
		push eax
		push ecx
		call GuidedHit
		pop ecx
		test al, al
		jnz go_ahead
		mov edx, [esp] ; suppress
		add esp, 0Ch
		jmp edx
		go_ahead:
		jmp [oldGuidedHit]
    }
}

/*
 * Called when ship was damaged
 * however you can't figure out here, which ship is being damaged, that's why we use the g_DmgTo variable.
 */

void __stdcall IEngineHook::AddDamageEntry(DamageList* dmgList, unsigned short subObjId, float hitPts, DamageEntry::SubObjFate fate)
{
    if (CallPlugins(&Plugin::OnAddDamageEntry, dmgList, subObjId, hitPts, fate))
    {
        return;
    }

    // check if we got damaged by a cd with changed behaviour
    if (dmgList->get_cause() == DamageCause::DummyDisrupter)
    {
        // check if player should be protected (f.e. in a docking cut scene)
        bool unk1 = false;
        bool unk2 = false;
        float unk;
        pub::SpaceObj::GetInvincible(FLHook::dmgToSpaceId.GetValue(), unk1, unk2, unk);
        // if so, suppress the damage
        if (unk1 && unk2)
        {
            return;
        }
    }

    if (FLHook::nonGunHitsBase && dmgList->get_cause() == DamageCause::MissileTorpedo)
    {
        const float damage = FLHook::lastHitPts - hitPts;
        hitPts = FLHook::lastHitPts - damage * FLHookConfig::i()->general.torpMissileBaseDamageMultiplier;
        if (hitPts < 0)
        {
            hitPts = 0;
        }
    }

    if (!dmgList->is_inflictor_a_player()) // npcs always do damage
    {
        dmgList->add_damage_entry(subObjId, hitPts, fate);
    }
    else if (FLHook::dmgToClient)
    {
        // lets see if player should do damage to other player
        const auto dmgFrom = ShipId(dmgList->get_inflictor_id()).GetPlayer();
        if (dmgFrom.has_value() && AllowPlayerDamage(dmgFrom.value(), FLHook::dmgToClient))
        {
            dmgList->add_damage_entry(subObjId, hitPts, fate);
        }
    }
    else
    {
        dmgList->add_damage_entry(subObjId, hitPts, fate);
    }

    TryHook
    {
        // check for base kill (when hull health = 0)
        if (hitPts == 0 && subObjId == 1)
        {
            uint type;
            pub::SpaceObj::GetType(FLHook::dmgToSpaceId.GetValue(), type);
            const auto clientKiller = ShipId(dmgList->get_inflictor_id()).GetPlayer();
            if (clientKiller.has_value() && type & static_cast<uint>(ObjectType::DockingRing | ObjectType::Station | ObjectType::WeaponPlatform))
            {
                BaseDestroyed(FLHook::dmgToSpaceId, clientKiller.value());
            }
        }

        if (FLHook::dmgToClient && subObjId == 1) // only save hits on the hull (subObjId=1)
        {
            FLHook::dmgToClient.GetData().dmgLast = *dmgList;
        }
    }
    CatchHook({});

    CallPlugins(&Plugin::OnAddDamageEntryAfter, dmgList, subObjId, hitPts, fate);

    FLHook::dmgToClient = ClientId();
    FLHook::dmgToSpaceId = ObjectId();
}

__declspec(naked) void IEngineHook::NakedAddDamageEntry()
{
    __asm {
		push [esp+0Ch]
		push [esp+0Ch]
		push [esp+0Ch]
		push ecx
		call AddDamageEntry
		mov eax, [esp]
		add esp, 10h
		jmp eax
    }
}

void __stdcall IEngineHook::DamageHit(char* ecx)
{
    TryHook
    {
        char* p;
        memcpy(&p, ecx + 0x10, 4);
        uint client;
        memcpy(&client, p + 0xB4, 4);
        uint spaceId;
        memcpy(&spaceId, p + 0xB0, 4);

        FLHook::dmgToClient = ClientId(client);
        FLHook::dmgToSpaceId = ObjectId(spaceId);

        CallPlugins(&Plugin::OnDamageHit, ClientId(client), ObjectId(spaceId));
    }
    CatchHook({});
}

__declspec(naked) void IEngineHook::NakedDamageHit()
{
    __asm {
		push ecx
		push ecx
		call DamageHit
		pop ecx
		jmp [oldDamageHit]
    }
}

__declspec(naked) void IEngineHook::NakedDamageHit2()
{
    __asm {
		push ecx
		push ecx
		call DamageHit
		pop ecx
		jmp [oldDamageHit2]
    }
}

bool IEngineHook::AllowPlayerDamage(ClientId client, ClientId clientTarget)
{
    auto [rval, skip] = CallPlugins<bool>(&Plugin::OnAllowPlayerDamage, client, clientTarget);
    if (skip)
    {
        return rval;
    }

    const auto* config = FLHookConfig::c();

    if (clientTarget)
    {
        auto time = TimeUtils::UnixTime<std::chrono::milliseconds>();

        // anti-dockkill check
        if (auto& targetData = clientTarget.GetData(); targetData.spawnProtected)
        {
            if (time - targetData.spawnTime <= config->general.antiDockKill)
            {
                return false; // target is protected
            }

            targetData.spawnProtected = false;
        }

        if (auto& clientData = client.GetData(); clientData.spawnProtected)
        {
            if (time - clientData.spawnTime <= config->general.antiDockKill)
            {
                return false; // target may not shoot
            }

            clientData.spawnProtected = false;
        }

        // no-pvp check
        uint systemId;
        pub::Player::GetSystem(client.GetValue(), systemId);
        if (std::ranges::find(config->general.noPVPSystemsHashed, systemId) != config->general.noPVPSystemsHashed.end())
        {
            return false;
        }
    }

    return true;
}

void __stdcall IEngineHook::NonGunWeaponHitsBaseBefore(const char* ecx, [[maybe_unused]] const char* p1, const DamageList* dmg)
{
    CSimple* simple;
    memcpy(&simple, ecx + 0x10, 4);
    FLHook::lastHitPts = simple->get_hit_pts();
    FLHook::nonGunHitsBase = true;
}

void IEngineHook::NonGunWeaponHitsBaseAfter() { FLHook::nonGunHitsBase = false; }

__declspec(naked) void IEngineHook::NakedNonGunWeaponHitsBase()
{
    __asm {
		mov eax, [esp+4]
		mov edx, [esp+8]
		push ecx
		push edx
		push eax
		push ecx
		call NonGunWeaponHitsBaseBefore
		pop ecx

		mov eax, [esp]
		mov [nonGunWeaponHitsBaseRetAddress], eax
		lea eax, return_here
		mov [esp], eax
		jmp [oldNonGunWeaponHitsBase]
		return_here:
		pushad
		call NonGunWeaponHitsBaseAfter
		popad
		jmp [nonGunWeaponHitsBaseRetAddress]
    }
}
