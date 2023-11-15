#include "PCH.hpp"

#include "Core/IEngineHook.hpp"

/*
 * Called when a torp/missile/mine/wasp hits a ship
 * return 0 -> pass on to server.dll
 * return 1 -> suppress
 */
int __stdcall IEngineHook::GuidedHit(char* ecx, char* p1, DamageList* dmgList)
{
    uint retValue = 0;
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

        auto [rval, skip] = CallPlugins<bool>(&Plugin::OnGuidedHit, inflictorShip, client, spaceId, dmgList);
        if (skip)
        {
            return rval;
        }

        g_DmgTo = client;
        g_DmgToSpaceId = spaceId;
        if (client)
        {
            // a player was hit
            const auto clientInflictor = Hk::Client::GetClientIdByShip(inflictorShip).Raw();
            if (clientInflictor.has_error())
            {
                retValue = 0; // hit by npc
            }
            else
            {
                if (!AllowPlayerDamage(clientInflictor.value(), client))
                {
                    retValue = 1;
                }

                if (FLHookConfig::i()->general.changeCruiseDisruptorBehaviour &&
                    ((dmgList->get_cause() == DamageCause::CruiseDisrupter || dmgList->get_cause() == DamageCause::UnkDisrupter) &&
                     !ClientInfo::At(client).cruiseActivated))
                {
                    dmgList->set_cause(DamageCause::DummyDisrupter); // change to sth else, so client won't recognize it as a disruptor
                }
            }
        }

        CallPlugins(&Plugin::OnGuidedHitAfter, inflictorShip, client, spaceId, dmgList);
    }
    CatchHook({}) return retValue;
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
		cmp eax, 1
		jnz go_ahead
		mov edx, [esp] ; suppress
		add esp, 0Ch
		jmp edx
		go_ahead:
		jmp [g_OldGuidedHit]
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
        pub::SpaceObj::GetInvincible(ClientInfo[g_DmgTo].ship, unk1, unk2, unk);
        // if so, suppress the damage
        if (unk1 && unk2)
        {
            return;
        }
    }

    if (g_NonGunHitsBase && dmgList->get_cause() == DamageCause::MissileTorpedo)
    {
        const float damage = g_LastHitPts - hitPts;
        hitPts = g_LastHitPts - damage * FLHookConfig::i()->general.torpMissileBaseDamageMultiplier;
        if (hitPts < 0)
        {
            hitPts = 0;
        }
    }

    if (!dmgList->is_inflictor_a_player()) // npcs always do damage
    {
        dmgList->add_damage_entry(subObjId, hitPts, fate);
    }
    else if (g_DmgTo)
    {
        // lets see if player should do damage to other player
        const auto dmgFrom = Hk::Client::GetClientIdByShip(dmgList->get_inflictor_id()).Raw();
        if (dmgFrom.has_value() && AllowPlayerDamage(dmgFrom.value(), g_DmgTo))
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
        g_LastDmgList = *dmgList; // save

        // check for base kill (when hull health = 0)
        if (hitPts == 0 && subObjId == 1)
        {
            uint type;
            pub::SpaceObj::GetType(g_DmgToSpaceId, type);
            const auto clientKiller = Hk::Client::GetClientIdByShip(dmgList->get_inflictor_id()).Raw();
            if (clientKiller.has_value() && type & static_cast<uint>(ObjectType::DockingRing | ObjectType::Station | ObjectType::WeaponPlatform))
            {
                BaseDestroyed(g_DmgToSpaceId, clientKiller.value());
            }
        }

        if (g_DmgTo && subObjId == 1) // only save hits on the hull (subObjId=1)
        {
            ClientInfo[g_DmgTo].dmgLast = *dmgList;
        }
    }
    CatchHook({})

        CallPlugins(&Plugin::OnAddDamageEntryAfter, dmgList, subObjId, hitPts, fate);

    g_DmgTo = 0;
    g_DmgToSpaceId = 0;
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

        g_DmgTo = client;
        g_DmgToSpaceId = spaceId;

        CallPlugins(&Plugin::OnDamageHit, client, spaceId);
    }
    CatchHook({})
}

__declspec(naked) void IEngineHook::NakedDamageHit()
{
    __asm {
		push ecx
		push ecx
		call DamageHit
		pop ecx
		jmp [g_OldDamageHit]
    }
}

__declspec(naked) void IEngineHook::NakedDamageHit2()
{
    __asm {
		push ecx
		push ecx
		call DamageHit
		pop ecx
		jmp [g_OldDamageHit2]
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
        // anti-dockkill check
        if (ClientInfo[clientTarget].spawnProtected)
        {
            if (TimeUtils::UnixMilliseconds() - ClientInfo[clientTarget].tmSpawnTime <= config->general.antiDockKill)
            {
                return false; // target is protected
            }
            ClientInfo[clientTarget].spawnProtected = false;
        }
        if (ClientInfo::At(client).spawnProtected)
        {
            if (TimeUtils::UnixMilliseconds() - ClientInfo::At(client).tmSpawnTime <= config->general.antiDockKill)
            {
                return false; // target may not shoot
            }
            ClientInfo::At(client).spawnProtected = false;
        }

        // no-pvp check
        uint systemId;
        pub::Player::GetSystem(client, systemId);
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
    g_LastHitPts = simple->get_hit_pts();

    g_NonGunHitsBase = true;
}

void IEngineHook::NonGunWeaponHitsBaseAfter() { g_NonGunHitsBase = false; }

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
		mov [g_NonGunWeaponHitsBaseRetAddress], eax
		lea eax, return_here
		mov [esp], eax
		jmp [g_OldNonGunWeaponHitsBase]
		return_here:
		pushad
		call NonGunWeaponHitsBaseAfter
		popad
		jmp [g_NonGunWeaponHitsBaseRetAddress]
    }
}
