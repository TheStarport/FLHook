#include "Global.hpp"

EXPORT uint g_DmgTo = 0;
EXPORT uint g_DmgToSpaceId = 0;
DamageList g_LastDmgList;

bool g_NonGunHitsBase = false;
float g_LastHitPts;

/**************************************************************************************************************
Called when a torp/missile/mine/wasp hits a ship
return 0 -> pass on to server.dll
return 1 -> suppress
**************************************************************************************************************/

FARPROC g_OldGuidedHit;

int __stdcall GuidedHit(char* ecx, char* p1, DamageList* dmgList)
{
	auto [rval, skip] = CallPluginsBefore<int>(HookedCall::IEngine__GuidedHit, ecx, p1, dmgList);
	if (skip)
		return rval;

	TRY_HOOK
		{
			char* p;
			memcpy(&p, ecx + 0x10, 4);
			uint client;
			memcpy(&client, p + 0xB4, 4);
			uint spaceId;
			memcpy(&spaceId, p + 0xB0, 4);

			g_DmgTo = client;
			g_DmgToSpaceId = spaceId;
			if (client)
			{
				// a player was hit
				uint inflictorShip;
				memcpy(&inflictorShip, p1 + 4, 4);
				const auto clientInflictor = Hk::Client::GetClientIdByShip(inflictorShip);
				if (clientInflictor.has_error())
					return 0; // hit by npc

				if (!AllowPlayerDamage(clientInflictor.value(), client))
					return 1;

				if (FLHookConfig::i()->general.changeCruiseDisruptorBehaviour && ((dmgList->get_cause() == DamageCause::CruiseDisrupter || dmgList->get_cause()
						== DamageCause::UnkDisrupter) &&
					!ClientInfo[client].bCruiseActivated))
				{
					dmgList->set_cause(DamageCause::DummyDisrupter); // change to sth else, so client won't recognize it as a disruptor
				}
			}
		}
	CATCH_HOOK({})

	return 0;
}

__declspec(naked) void Naked__GuidedHit()
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

/**************************************************************************************************************
Called when ship was damaged
however you can't figure out here, which ship is being damaged, that's why i use
the g_DmgTo variable...
**************************************************************************************************************/

void __stdcall AddDamageEntry(
	DamageList* dmgList, unsigned short subObjId, float hitPts, enum DamageEntry::SubObjFate fate)
{
	if (CallPluginsBefore(HookedCall::IEngine__AddDamageEntry, dmgList, subObjId, hitPts, fate))
		return;

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
			return;
	}

	if (g_NonGunHitsBase && (dmgList->get_cause() == DamageCause::CruiseDisrupter))
	{
		const float damage = g_LastHitPts - hitPts;
		hitPts = g_LastHitPts - damage * FLHookConfig::i()->general.torpMissileBaseDamageMultiplier;
		if (hitPts < 0)
			hitPts = 0;
	}

	if (!dmgList->is_inflictor_a_player()) // npcs always do damage
		dmgList->add_damage_entry(subObjId, hitPts, fate);
	else if (g_DmgTo)
	{
		// lets see if player should do damage to other player
		const auto dmgFrom = Hk::Client::GetClientIdByShip(dmgList->get_inflictor_id());
		if (dmgFrom.has_value() && AllowPlayerDamage(dmgFrom.value(), g_DmgTo))
			dmgList->add_damage_entry(subObjId, hitPts, fate);
	}
	else
		dmgList->add_damage_entry(subObjId, hitPts, fate);

	TRY_HOOK
		{
			g_LastDmgList = *dmgList; // save

			// check for base kill (when hull health = 0)
			if (hitPts == 0 && subObjId == 1)
			{
				uint type;
				pub::SpaceObj::GetType(g_DmgToSpaceId, type);
				const auto clientKiller = Hk::Client::GetClientIdByShip(dmgList->get_inflictor_id());
				if (clientKiller.has_value() && type & (OBJ_DOCKING_RING | OBJ_STATION | OBJ_WEAPONS_PLATFORM))
					BaseDestroyed(g_DmgToSpaceId, clientKiller.value());
			}

			if (g_DmgTo && subObjId == 1) // only save hits on the hull (subObjId=1)
			{
				ClientInfo[g_DmgTo].dmgLast = *dmgList;
			}
		}
	CATCH_HOOK({})

	CallPluginsAfter(HookedCall::IEngine__AddDamageEntry, dmgList, subObjId, hitPts, fate);

	g_DmgTo = 0;
	g_DmgToSpaceId = 0;
}

__declspec(naked) void Naked__AddDamageEntry()
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

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

FARPROC g_OldDamageHit, g_OldDamageHit2;

void __stdcall DamageHit(char* ecx)
{
	CallPluginsBefore(HookedCall::IEngine__DamageHit, ecx);

	TRY_HOOK
		{
			char* p;
			memcpy(&p, ecx + 0x10, 4);
			uint client;
			memcpy(&client, p + 0xB4, 4);
			uint spaceId;
			memcpy(&spaceId, p + 0xB0, 4);

			g_DmgTo = client;
			g_DmgToSpaceId = spaceId;
		}
	CATCH_HOOK({})
}

__declspec(naked) void Naked__DamageHit()
{
	__asm {
		push ecx
		push ecx
		call DamageHit
		pop ecx
		jmp [g_OldDamageHit]
		}
}

__declspec(naked) void Naked__DamageHit2()
{
	__asm {
		push ecx
		push ecx
		call DamageHit
		pop ecx
		jmp [g_OldDamageHit2]
		}
}

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

bool AllowPlayerDamage(ClientId client, ClientId clientTarget)
{
	auto [rval, skip] = CallPluginsBefore<bool>(HookedCall::IEngine__AllowPlayerDamage, client, clientTarget);
	if (skip)
		return rval;

	const auto* config = FLHookConfig::c();

	if (clientTarget)
	{
		// anti-dockkill check
		if (ClientInfo[clientTarget].bSpawnProtected)
		{
			if ((Hk::Time::GetUnixMiliseconds() - ClientInfo[clientTarget].tmSpawnTime) <= config->general.antiDockKill)
				return false; // target is protected
			ClientInfo[clientTarget].bSpawnProtected = false;
		}
		if (ClientInfo[client].bSpawnProtected)
		{
			if ((Hk::Time::GetUnixMiliseconds() - ClientInfo[client].tmSpawnTime) <= config->general.antiDockKill)
				return false; // target may not shoot
			ClientInfo[client].bSpawnProtected = false;
		}

		// no-pvp check
		uint systemId;
		pub::Player::GetSystem(client, systemId);
		if (std::ranges::find(config->general.noPVPSystemsHashed, systemId) != config->general.noPVPSystemsHashed.end())
			return false;
	}

	return true;
}

/**************************************************************************************************************
**************************************************************************************************************/

FARPROC g_OldNonGunWeaponHitsBase;

void __stdcall NonGunWeaponHitsBaseBefore(const char* ECX, [[maybe_unused]] const char* p1, const DamageList* dmg)
{
	CSimple* simple;
	memcpy(&simple, ECX + 0x10, 4);
	g_LastHitPts = simple->get_hit_pts();

	g_NonGunHitsBase = true;
}

void NonGunWeaponHitsBaseAfter()
{
	g_NonGunHitsBase = false;
}

ulong g_NonGunWeaponHitsBaseRetAddress;

__declspec(naked) void Naked__NonGunWeaponHitsBase()
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

///////////////////////////
