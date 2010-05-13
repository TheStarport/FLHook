#include "hook.h"

EXPORT uint iDmgTo = 0;
EXPORT uint iDmgToSpaceID = 0;
DamageList	LastDmgList;

bool g_gNonGunHitsBase = false;
float g_LastHitPts;

/**************************************************************************************************************
Called when a torp/missile/mine/wasp hits a ship
return 0 -> pass on to server.dll
return 1 -> suppress
**************************************************************************************************************/

FARPROC fpOldMissileTorpHit;

int __stdcall HkCB_MissileTorpHit(char *ECX, char *p1, DamageList *dmg)
{

	CALL_PLUGINS(PLUGIN_HkCB_MissileTorpHit,(ECX,p1,dmg));
	if(bPluginReturn)
		return reinterpret_cast<int>(vPluginRet);

	try {
		// get client id
		char *szP;
		memcpy(&szP, ECX + 0x10, 4);
		uint iClientID;
		memcpy(&iClientID, szP + 0xB4, 4);
		uint iSpaceID;
		memcpy(&iSpaceID, szP + 0xB0, 4);

		iDmgTo = iClientID;
		iDmgToSpaceID = iSpaceID;
		if(iClientID) 
		{ // a player was hit
			uint iInflictorShip;
			memcpy(&iInflictorShip, p1 + 4, 4);
			uint iClientIDInflictor = HkGetClientIDByShip(iInflictorShip);
			if(!iClientIDInflictor)
				return 0; // hit by npc

			if(!AllowPlayerDamage(iClientIDInflictor, iClientID))
				return 1;

			if(set_bChangeCruiseDisruptorBehaviour)
			{
				if(((dmg->get_cause() == 6) || (dmg->get_cause() == 0x15)) && !ClientInfo[iClientID].bCruiseActivated)
					dmg->set_cause((enum DamageCause)0xC0); // change to sth else, so client won't recognize it as a disruptor
			}
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
	return 0;
}

__declspec(naked) void _HookMissileTorpHit()
{
	__asm 
	{
		mov eax, [esp+4]
		mov edx, [esp+8]
		push ecx
		push edx
		push eax
		push ecx
		call HkCB_MissileTorpHit
		pop ecx
		cmp eax, 1
		jnz go_ahead
		mov edx, [esp] ; suppress
		add esp, 0Ch
		jmp edx

go_ahead:
		jmp [fpOldMissileTorpHit]
	}
}

/**************************************************************************************************************
Called when ship was damaged
however you can't figure out here, which ship is being damaged, that's why i use the iDmgTo variable...
**************************************************************************************************************/

void __stdcall HkCb_AddDmgEntry_AFTER(DamageList *dmgList, unsigned short p1, float p2, enum DamageEntry::SubObjFate p3)
{
	CALL_PLUGINS(PLUGIN_HkCb_AddDmgEntry,(dmgList,p1,p2,p3));
}

void __stdcall HkCb_AddDmgEntry(DamageList *dmgList, unsigned short p1, float p2, enum DamageEntry::SubObjFate p3)
{

	CALL_PLUGINS(PLUGIN_HkCb_AddDmgEntry,(dmgList,p1,p2,p3));
	if(bPluginReturn)
		return;

	if(g_gNonGunHitsBase && (dmgList->get_cause() == 5))
	{
		float fDamage = g_LastHitPts - p2;
		p2 = g_LastHitPts - fDamage * set_fTorpMissileBaseDamageMultiplier;
		if(p2 < 0)
			p2 = 0;
	}

	dmgList->add_damage_entry(p1, p2, p3);
	try {
		LastDmgList = *dmgList; // save

//		float fHealth,fMaxHealth;32 256
//		pub::SpaceObj::GetHealth(ClientInfo[iDmgTo].iShip,fHealth,fMaxHealth);

		// check for base kill (when hull health = 0)
		if(p2 == 0 && p1 == 1)
		{
			uint iType;
			pub::SpaceObj::GetType(iDmgToSpaceID,iType);
			uint iClientIDKiller = HkGetClientIDByShip(dmgList->get_inflictor_id());
			if(iClientIDKiller && iType & (OBJ_DOCKING_RING | OBJ_STATION))
				BaseDestroyed(iDmgToSpaceID, iClientIDKiller);
		}

		if(iDmgTo && p1 == 1) // only save hits on the hull (p1=1)
		{
			ClientInfo[iDmgTo].dmgLast = *dmgList;
			iDmgTo = 0;
		}


	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }

	HkCb_AddDmgEntry_AFTER(dmgList, p1, p2, p3);
}

__declspec(naked) void _HkCb_AddDmgEntry()
{
	__asm
	{
		push [esp+0Ch]
		push [esp+0Ch]
		push [esp+0Ch]
		push ecx
		call HkCb_AddDmgEntry
		mov eax, [esp]
		add esp, 10h
		jmp eax
	}
}

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

FARPROC fpOldGeneralDmg;

void __stdcall HkCb_GeneralDmg(char *szECX)
{

	CALL_PLUGINS(PLUGIN_HkCb_GeneralDmg,(szECX));
	if(bPluginReturn)
		return;

	try {
		char *szP;
		memcpy(&szP, szECX + 0x10, 4);
		uint iClientID;
		memcpy(&iClientID, szP + 0xB4, 4);
		uint iSpaceID;
		memcpy(&iSpaceID, szP + 0xB0, 4);

		iDmgTo = iClientID;
		iDmgToSpaceID = iSpaceID;
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

__declspec(naked) void _HkCb_GeneralDmg()
{
	__asm
	{
		push ecx
		push ecx
		call HkCb_GeneralDmg
		pop ecx
		jmp [fpOldGeneralDmg]
	}
}

/**************************************************************************************************************
Called when ship was damaged
**************************************************************************************************************/

bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget)
{
	CALL_PLUGINS(PLUGIN_AllowPlayerDamage,(iClientID,iClientIDTarget));
	if(bPluginReturn)
		return reinterpret_cast<bool>(vPluginRet);

	if(iClientIDTarget)
	{
		// anti-dockkill check
		if(ClientInfo[iClientIDTarget].bSpawnProtected)
		{
			if((timeInMS() - ClientInfo[iClientIDTarget].tmSpawnTime) <= set_iAntiDockKill) 
				return false; // target is protected
			else
				ClientInfo[iClientIDTarget].bSpawnProtected = false;
		}
		if(ClientInfo[iClientID].bSpawnProtected)
		{
			if((timeInMS() - ClientInfo[iClientID].tmSpawnTime) <= set_iAntiDockKill) 
				return false; // target may not shoot
			else
				ClientInfo[iClientID].bSpawnProtected = false;
		}


		// no-pvp check
		uint iSystemID;
		pub::Player::GetSystem(iClientID, iSystemID);
		foreach(set_lstNoPVPSystems, uint, i)
		{
			if(iSystemID == (*i))
				return false; // no pvp
		}		
	}

	return true;
}

/**************************************************************************************************************
**************************************************************************************************************/

FARPROC fpOldNonGunWeaponHitsBase;

float fHealthBefore;
uint iHitObject;
uint iClientIDInflictor;

void __stdcall HkCb_NonGunWeaponHitsBaseBefore(char *ECX, char *p1, DamageList *dmg)
{

	CSimple *simple;
	memcpy(&simple, ECX + 0x10, 4);
	g_LastHitPts = simple->get_hit_pts();


	g_gNonGunHitsBase = true;
}

void HkCb_NonGunWeaponHitsBaseAfter()
{
	g_gNonGunHitsBase = false;

}

ulong lRetAddress;

__declspec(naked) void _HkCb_NonGunWeaponHitsBase()
{
	__asm
	{
		mov eax, [esp+4]
		mov edx, [esp+8]
		push ecx
		push edx
		push eax
		push ecx
		call HkCb_NonGunWeaponHitsBaseBefore
		pop ecx

		mov eax, [esp]
		mov [lRetAddress], eax
		lea eax, return_here
		mov [esp], eax
		jmp [fpOldNonGunWeaponHitsBase]
return_here:
		pushad
		call HkCb_NonGunWeaponHitsBaseAfter
		popad
		jmp [lRetAddress]
	}
}