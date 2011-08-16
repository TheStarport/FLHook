#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromAccount(CAccount *acc)
{
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		CAccount *cur;
		memcpy(&cur, (char*)pPD + 0x400, 4);
		if(cur == acc)
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			return iClientID;
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromPD(struct PlayerData *pPD)
{
	char *p1 = (char*)pPD;
	char *p2 = (char*)&Players;
	memcpy(&p2, p2, 4);
	return (uint)(((p1 - p2) / 0x418) + 1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CAccount* HkGetAccountByCharname(const wstring &wscCharname)
{

	flstr *flStr = CreateWString(wscCharname.c_str());
	CAccount *acc = Players.FindAccountFromCharacterName(*flStr);
	FreeWString(flStr);

	return acc;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIdFromCharname(const wstring &wscCharname)
{
	CAccount *acc = HkGetAccountByCharname(wscCharname);
	if(!acc)
		return -1;

	uint iClientID = HkGetClientIdFromAccount(acc);
	if(iClientID == -1)
		return -1;

	wchar_t *wszActiveCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
	if(!wszActiveCharname)
		return -1;
	
	wstring wscActiveCharname = wszActiveCharname;
	wscActiveCharname = ToLower(wscActiveCharname);
	if(wscActiveCharname.compare(ToLower(wscCharname)) != 0)
		return -1;

	return iClientID;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetAccountID(CAccount *acc)
{
	if (acc && acc->wszAccID)
		return acc->wszAccID;
	return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsEncoded(const string &scFilename)
{
	bool bRet = false;
	FILE *f = fopen(scFilename.c_str(), "r");
	if(!f)
		return false;

	char szMagic[] = "FLS1";
	char szFile[sizeof(szMagic)] = "";
	fread(szFile, 1, sizeof(szMagic), f);
	if(!strncmp(szMagic, szFile, sizeof(szMagic) - 1))
		bRet = true;
	fclose(f);

	return bRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsInCharSelectMenu(const wstring &wscCharname)
{
	CAccount *acc = HkGetAccountByCharname(wscCharname);
	if(!acc)
		return false;

	uint iClientID = HkGetClientIdFromAccount(acc);
	if(iClientID == -1)
		return false;

	uint iBase = 0;
	uint iSystem = 0;
	pub::Player::GetBase(iClientID, iBase);
	pub::Player::GetSystem(iClientID, iSystem);
	if(!iBase && !iSystem)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsInCharSelectMenu(uint iClientID)
{

	uint iBase = 0;
	uint iSystem = 0;
	pub::Player::GetBase(iClientID, iBase);
	pub::Player::GetSystem(iClientID, iSystem);
	if(!iBase && !iSystem)
		return true;
	else
		return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkIsValidClientID(uint iClientID)
{

	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iID = HkGetClientIdFromPD(pPD);
		if(iID == iClientID)
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResolveId(const wstring &wscCharname, uint &iClientID)
{
	wstring wscCharnameLower = ToLower(wscCharname);
	if(wscCharnameLower.find(L"id ") == 0)
	{
		uint iID = 0;
		swscanf(wscCharnameLower.c_str(), L"id %u", &iID);
		if(!HkIsValidClientID(iID))
			return HKE_INVALID_CLIENT_ID;
		iClientID = iID;
		return HKE_OK;
	}

	return HKE_INVALID_ID_STRING;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkResolveShortCut(const wstring &wscShortcut, uint &_iClientID)
{
	wstring wscShortcutLower = ToLower(wscShortcut);
	if(wscShortcutLower.find(L"sc ") != 0)
		return HKE_INVALID_SHORTCUT_STRING;

	wscShortcutLower = wscShortcutLower.substr(3);

	uint iClientIDFound = -1;
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);

		const wchar_t *wszCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
		if(!wszCharname)
			continue;

		wstring wscCharname = wszCharname;
		if(ToLower(wscCharname).find(wscShortcutLower) != -1)
		{
			if(iClientIDFound == -1)
				iClientIDFound = iClientID;
			else
				return HKE_AMBIGUOUS_SHORTCUT;
		}
	}

	if(iClientIDFound == -1)
		return HKE_NO_MATCHING_PLAYER;

	_iClientID = iClientIDFound;
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIDByShip(uint iShip)
{
	for(uint i = 0; (i <= Players.GetMaxPlayerCount()); i++)
	{
		if(ClientInfo[i].iShip == iShip || ClientInfo[i].iShipOld == iShip)
			return i;
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetAccountDirName(CAccount *acc, wstring &wscDir)
{
	_GetFLName GetFLName = (_GetFLName)((char*)hModServer + 0x66370);

	char szDir[1024] = "";
	GetFLName(szDir, acc->wszAccID);
	wscDir = stows(szDir);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetAccountDirName(const wstring &wscCharname, wstring &wscDir)
{
	HK_GET_CLIENTID(iClientID, wscCharname);
	CAccount *acc;
	if(iClientID != -1)
		acc = Players.FindAccountFromClientID(iClientID);		
	else {
		if(!(acc = HkGetAccountByCharname(wscCharname)))
			return HKE_CHAR_DOES_NOT_EXIST;
	}

	return HkGetAccountDirName(acc, wscDir);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkGetCharFileName(const wstring &wscCharname, wstring &wscFilename)
{
	static _GetFLName GetFLName = 0;
	if (!GetFLName)
		GetFLName = (_GetFLName)((char*)hModServer + 0x66370);

	char szBuf[1024] = "";

	HK_GET_CLIENTID(iClientID, wscCharname);
	if(iClientID != -1)
	{
		GetFLName(szBuf, Players.GetActiveCharacterName(iClientID));
	}
	else
	{
		GetFLName(szBuf, wscCharname.c_str());
	}

	wscFilename = stows(szBuf);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetBaseNickByID(uint iBaseID)
{
	char szBasename[1024] = "";
	pub::GetBaseNickname(szBasename, sizeof(szBasename), iBaseID);
	return stows(szBasename);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetSystemNickByID(uint iSystemID)
{
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return stows(szSystemname);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring HkGetPlayerSystem(uint iClientID)
{
	uint iSystemID;
	pub::Player::GetSystem(iClientID, iSystemID);
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return stows(szSystemname);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkLockAccountAccess(CAccount *acc, bool bKick)
{
	char szJMP[] = { '\xEB' };
	char szJBE[] = { '\x76' };

	flstr *flStr = CreateWString(HkGetAccountID(acc).c_str());

	if(!bKick)
		WriteProcMem((void*)0x06D52A6A, &szJMP, 1);

	Players.LockAccountAccess(*flStr); // also kicks player on this account
	if(!bKick)
		WriteProcMem((void*)0x06D52A6A, &szJBE, 1);

	FreeWString(flStr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkUnlockAccountAccess(CAccount *acc)
{
	flstr *flStr = CreateWString(HkGetAccountID(acc).c_str());
	Players.UnlockAccountAccess(*flStr);
	FreeWString(flStr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkGetItemsForSale(uint iBaseID, list<uint> &lstItems)
{
	lstItems.clear();
	char szNOP[] = { '\x90', '\x90'};
	char szJNZ[] = { '\x75', '\x1D'};
	WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), &szNOP, 2); // patch, else we only get commodities
	uint iArray[1024];
	int iSize = sizeof(iArray)/sizeof(uint);
	pub::Market::GetCommoditiesForSale(iBaseID, (uint*const)&iArray, &iSize);
	WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), &szJNZ, 2);

	for(int i = 0; (i < iSize); i++)
		lstItems.push_back(iArray[i]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

IObjInspectImpl* HkGetInspect(uint iClientID)
{
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	uint iDunno;
	IObjInspectImpl *inspect;
	if(!GetShipInspect(iShip, inspect, iDunno))
		return 0;
	else
		return inspect;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

ENGINE_STATE HkGetEngineState(uint iClientID)
{
	if(ClientInfo[iClientID].bTradelane)
		return ES_TRADELANE;
	else if(ClientInfo[iClientID].bCruiseActivated)
		return ES_CRUISE;
	else if(ClientInfo[iClientID].bThrusterActivated)
		return ES_THRUSTER;
	else if(!ClientInfo[iClientID].bEngineKilled)
		return ES_ENGINE;
	else
		return ES_KILLED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EQ_TYPE HkGetEqType(Archetype::Equipment *eq)
{
	uint iVFTableMine = (uint)hModCommon + ADDR_COMMON_VFTABLE_MINE;
	uint iVFTableCM = (uint)hModCommon + ADDR_COMMON_VFTABLE_CM;
	uint iVFTableGun = (uint)hModCommon + ADDR_COMMON_VFTABLE_GUN;
	uint iVFTableShieldGen = (uint)hModCommon + ADDR_COMMON_VFTABLE_SHIELDGEN;
	uint iVFTableThruster = (uint)hModCommon + ADDR_COMMON_VFTABLE_THRUSTER;
	uint iVFTableShieldBat = (uint)hModCommon + ADDR_COMMON_VFTABLE_SHIELDBAT;
	uint iVFTableNanoBot = (uint)hModCommon + ADDR_COMMON_VFTABLE_NANOBOT;
	uint iVFTableMunition = (uint)hModCommon + ADDR_COMMON_VFTABLE_MUNITION;
	uint iVFTableEngine = (uint)hModCommon + ADDR_COMMON_VFTABLE_ENGINE;
	uint iVFTableScanner = (uint)hModCommon + ADDR_COMMON_VFTABLE_SCANNER;
	uint iVFTableTractor = (uint)hModCommon + ADDR_COMMON_VFTABLE_TRACTOR;
	uint iVFTableLight = (uint)hModCommon + ADDR_COMMON_VFTABLE_LIGHT;
	
	uint iVFTable = *((uint*)eq);
	if(iVFTable == iVFTableGun) {
		Archetype::Gun *gun = (Archetype::Gun *)eq;
		Archetype::Equipment *eqAmmo = Archetype::GetEquipment(gun->iProjectileArchID);
		int iMissile;
		memcpy(&iMissile, (char*)eqAmmo + 0x90, 4);
		uint iGunType = gun->get_hp_type_by_index(0);
		if(iGunType == 36)
			return ET_TORPEDO;
		else if(iGunType == 35)
			return ET_CD;
		else if(iMissile)
			return ET_MISSILE;
		else
			return ET_GUN;
	} else if(iVFTable == iVFTableCM)
		return ET_CM;
	else if(iVFTable == iVFTableShieldGen)
		return ET_SHIELDGEN;
	else if(iVFTable == iVFTableThruster)
		return ET_THRUSTER;
	else if(iVFTable == iVFTableShieldBat)
		return ET_SHIELDBAT;
	else if(iVFTable == iVFTableNanoBot)
		return ET_NANOBOT;
	else if(iVFTable == iVFTableMunition)
		return ET_MUNITION;
	else if(iVFTable == iVFTableMine)
		return ET_MINE;
	else if(iVFTable == iVFTableEngine)
		return ET_ENGINE;
	else if(iVFTable == iVFTableLight)
		return ET_LIGHT;
	else if(iVFTable == iVFTableScanner)
		return ET_SCANNER;
	else if(iVFTable == iVFTableTractor)
		return ET_TRACTOR;
	else
		return ET_OTHER;
}