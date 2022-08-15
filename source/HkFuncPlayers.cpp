#include "Global.hpp"
#include <fstream>

HkError HkAddToGroup(uint clientId, uint iGroupID)
{
	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	uint iCurrentGroupID = Players.GetGroupID(clientId);
	if (iCurrentGroupID == iGroupID)
		return InvalidGroupId;

	CPlayerGroup* group = CPlayerGroup::FromGroupID(iGroupID);
	if (!group)
		return InvalidGroupId;
	group->AddMember(clientId);
	return HKE_OK;
}

HkError HkGetGroupID(uint clientId, uint& iGroupID)
{
	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	iGroupID = Players.GetGroupID(clientId);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetCash(const std::variant<uint, std::wstring>& player, int& iCash)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId != -1)
	{
		if (HkIsInCharSelectMenu(clientId))
			return NoCharSelected;

		pub::Player::InspectCash(clientId, iCash);
		return HKE_OK;
	}

	if (!player.index())
		return InvalidClientId;

	// player not logged in
	std::wstring wscDir;
	if (!HKHKSUCCESS(HkGetAccountDirName(std::get<std::wstring>(player), wscDir)))
		return CharDoesNotExist;
	std::wstring wscFile;
	HkGetCharFileName(player, wscFile);

	std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";

	FILE* fTest;
	fopen_s(&fTest, scCharFile.c_str(), "r");
	if (!fTest)
		return CharDoesNotExist;
	else
		fclose(fTest);

	if (HkIsEncoded(scCharFile))
	{
		std::string scCharFileNew = scCharFile + ".ini";
		if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return CouldNotDecodeCharFile;

		iCash = IniGetI(scCharFileNew, "Player", "money", -1);
		DeleteFile(scCharFileNew.c_str());
	}
	else
	{
		iCash = IniGetI(scCharFile, "Player", "money", -1);
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkAddCash(const std::variant<uint, std::wstring>& player, int iAmount)
{
	const uint clientId = HkExtractClientId(player);

	if ((clientId != -1) && HkIsInCharSelectMenu(clientId))
		return NoCharSelected;
	else if (clientId != -1)
	{ // player logged in
		pub::Player::AdjustCash(clientId, iAmount);
		return HKE_OK;
	}
	else if (player.index())
	{
		const auto character = std::get<std::wstring>(player);
		std::wstring wscDir;
		if (!HKHKSUCCESS(HkGetAccountDirName(character, wscDir)))
			return CharDoesNotExist;
		std::wstring wscFile;
		HkGetCharFileName(character, wscFile);

		std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
		int iRet;
		if (HkIsEncoded(scCharFile))
		{
			std::string scCharFileNew = scCharFile + ".ini";

			if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
				return CouldNotDecodeCharFile;

			iRet = IniGetI(scCharFileNew, "Player", "money", -1);
			// Add a space to the value so the ini file line looks like "<key> =
			// <value>" otherwise IFSO can't decode the file correctly
			IniWrite(scCharFileNew, "Player", "money", " " + std::to_string(iRet + iAmount));

			if (!FLHookConfig::i()->general.disableCharfileEncryption)
				if (!flc_encode(scCharFileNew.c_str(), scCharFile.c_str()))
					return CouldNotEncodeCharFile;

			DeleteFile(scCharFileNew.c_str());
		}
		else
		{
			iRet = IniGetI(scCharFile, "Player", "money", -1);
			// Add a space to the value so the ini file line looks like "<key> =
			// <value>" otherwise IFSO can't decode the file correctly
			IniWrite(scCharFile, "Player", "money", " " + std::to_string(iRet + iAmount));
		}

		if (HkIsInCharSelectMenu(character) || (clientId != -1))
		{ // money fix in case player logs in with this account
			bool bFound = false;
			std::wstring characterLower = ToLower(character);
			for (auto& money : ClientInfo[clientId].lstMoneyFix)
			{
				if (money.character == characterLower)
				{
					money.iAmount += iAmount;
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				MONEY_FIX mf;
				mf.character = characterLower;
				mf.iAmount = iAmount;
				ClientInfo[clientId].lstMoneyFix.push_back(mf);
			}
		}

		return HKE_OK;
	}

	return InvalidClientId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkKick(CAccount* acc)
{
	acc->ForceLogout();
	return HKE_OK;
}

HkError HkKick(const std::variant<uint, std::wstring>& player)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1)
		return PlayerNotLoggedIn;

	CAccount* acc = Players.FindAccountFromClientID(clientId);
	acc->ForceLogout();
	return HKE_OK;
}

HkError HkKickReason(const std::variant<uint, std::wstring>& player, const std::wstring& wscReason)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1)
		return PlayerNotLoggedIn;

	if (wscReason.length())
		HkMsgAndKick(clientId, wscReason, FLHookConfig::i()->msgStyle.kickMsgPeriod);
	else
		HkKick(Players.FindAccountFromClientID(clientId));

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkBan(const std::variant<uint, std::wstring>& player, bool bBan)
{
	CAccount* acc = HkExtractAccount(player);
	if (!acc)
		return CharDoesNotExist;

	std::wstring wscID = HkGetAccountID(acc);
	st6::wstring flStr((ushort*)wscID.c_str());
	Players.BanAccount(flStr, bBan);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkBeam(const std::variant<uint, std::wstring>& player, const std::wstring& wscBasename)
{
	const uint clientId = HkExtractClientId(player);

	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	std::string scBasename = wstos(wscBasename);
	// check if ship in space
	uint iShip = 0;
	pub::Player::GetShip(clientId, iShip);
	if (!iShip)
		return PlayerNotInSpace;

	// get base id
	uint iBaseID;

	if (pub::GetBaseID(iBaseID, scBasename.c_str()) == -4)
	{
		return InvalidBaseName;
	}

	uint iSysID;
	pub::Player::GetSystem(clientId, iSysID);
	Universe::IBase* base = Universe::get_base(iBaseID);

	pub::Player::ForceLand(clientId, iBaseID); // beam

	// if not in the same system, emulate F1 charload
	if (base->iSystemID != iSysID)
	{
		Server.BaseEnter(iBaseID, clientId);
		Server.BaseExit(iBaseID, clientId);
		std::wstring wscCharFileName;
		HkGetCharFileName(clientId, wscCharFileName);
		wscCharFileName += L".fl";
		CHARACTER_ID cID;
		strcpy_s(cID.szCharFilename, wstos(wscCharFileName.substr(0, 14)).c_str());
		Server.CharacterSelect(cID, clientId);
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkSaveChar(const std::variant<uint, std::wstring>& player)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1)
		return PlayerNotLoggedIn;

	void* pJmp = (char*)hModServer + 0x7EFA8;
	char szNop[2] = { '\x90', '\x90' };
	char szTestAlAl[2] = { '\x74', '\x44' };
	WriteProcMem(pJmp, szNop, sizeof(szNop)); // nop the SinglePlayer() check
	pub::Save(clientId, 1);
	WriteProcMem(pJmp, szTestAlAl, sizeof(szTestAlAl)); // restore

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct EQ_ITEM
{
	EQ_ITEM* next;
	uint i2;
	ushort s1;
	ushort sID;
	uint iGoodID;
	CacheString hardpoint;
	bool bMounted;
	char sz[3];
	float fStatus;
	uint iCount;
	bool bMission;
};

HkError HkEnumCargo(const std::variant<uint, std::wstring>& player, std::list<CARGO_INFO>& lstCargo, int& iRemainingHoldSize)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1 || HkIsInCharSelectMenu(clientId))
		return PlayerNotLoggedIn;

	lstCargo.clear();

	char* szClassPtr;
	memcpy(&szClassPtr, &Players, 4);
	szClassPtr += 0x418 * (clientId - 1);

	EQ_ITEM* eqLst;
	memcpy(&eqLst, szClassPtr + 0x27C, 4);
	EQ_ITEM* eq;
	eq = eqLst->next;
	while (eq != eqLst)
	{
		CARGO_INFO ci = {
			eq->sID, (int)eq->iCount, eq->iGoodID, eq->fStatus, eq->bMission, eq->bMounted, eq->hardpoint
		};
		lstCargo.push_back(ci);

		eq = eq->next;
	}

	float fRemHold;
	pub::Player::GetRemainingHoldSize(clientId, fRemHold);
	iRemainingHoldSize = (int)fRemHold;
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkRemoveCargo(const std::variant<uint, std::wstring>& player, uint iID, int iCount)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1 || HkIsInCharSelectMenu(clientId))
		return PlayerNotLoggedIn;

	std::list<CARGO_INFO> lstCargo;
	int iHold;
	HkEnumCargo(player, lstCargo, iHold);
	for (auto& cargo : lstCargo)
	{
		if ((cargo.iID == iID) && (cargo.iCount < iCount))
			iCount = cargo.iCount; // trying to remove more than actually there,
								   // thus fix
	}

	pub::Player::RemoveCargo(clientId, iID, iCount);

	// anti-cheat related
	/*	char *szClassPtr;
			memcpy(&szClassPtr, &Players, 4);
			szClassPtr += 0x418 * (clientId - 1);
			EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;
			const EquipDesc *ed = edlList->find_equipment_item(iID);
			if(ed)
			{
					ed->get_id();
					edlList->remove_equipment_item(
			} */

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkAddCargo(const std::variant<uint, std::wstring>& player, uint iGoodID, int iCount, bool bMission)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1 || HkIsInCharSelectMenu(clientId))
		return PlayerNotLoggedIn;

	/*	// anti-cheat related
			char *szClassPtr;
			memcpy(&szClassPtr, &Players, 4);
			szClassPtr += 0x418 * (clientId - 1);
			EquipDescList *edlList = (EquipDescList*)szClassPtr + 0x328;
			bool bCargoFound = true;
			if(!edlList->find_matching_cargo(iGoodID, 0, 1))
					bCargoFound = false;*/

	// add
	const GoodInfo* gi;
	if (!(gi = GoodList::find_by_id(iGoodID)))
		return InvalidGood;

	bool bMultiCount;
	memcpy(&bMultiCount, (char*)gi + 0x70, 1);

	uint iBase = 0;
	pub::Player::GetBase(clientId, iBase);
	uint iLocation = 0;
	pub::Player::GetLocation(clientId, iLocation);

	// trick cheat detection
	if (iBase)
	{
		if (iLocation)
			Server.LocationExit(iLocation, clientId);
		Server.BaseExit(iBase, clientId);
		if (!HkIsValidClientID(clientId)) // got cheat kicked
			return PlayerNotLoggedIn;
	}

	if (bMultiCount)
	{ // it's a good that can have multiple units(commodities,
	  // missile ammo, etc)
		int iRet;

		// we need to do this, else server or client may crash
		std::list<CARGO_INFO> lstCargo;
		HkEnumCargo(player, lstCargo, iRet);
		for (auto& cargo : lstCargo)
		{
			if ((cargo.iArchID == iGoodID) && (cargo.bMission != bMission))
			{
				HkRemoveCargo(player, cargo.iID, cargo.iCount);
				iCount += cargo.iCount;
			}
		}

		pub::Player::AddCargo(clientId, iGoodID, iCount, 1, bMission);
	}
	else
	{
		for (int i = 0; (i < iCount); i++)
			pub::Player::AddCargo(clientId, iGoodID, 1, 1, bMission);
	}

	if (iBase)
	{ // player docked on base
		///////////////////////////////////////////////////
		// fix, else we get anti-cheat msg when undocking
		// this DOES NOT disable anti-cheat-detection, we're
		// just making some adjustments so that we dont get kicked

		Server.BaseEnter(iBase, clientId);
		if (iLocation)
			Server.LocationEnter(iLocation, clientId);

		/*		// fix "Ship or Equipment not sold on base" kick
						if(!bCargoFound)
						{
								// get last equipid
								char *szLastEquipID = szClassPtr + 0x3C8;
								ushort sEquipID;
								memcpy(&sEquipID, szLastEquipID, 2);

								// add to check-list which is being compared to
		   the users equip-list when saving char EquipDesc ed; memset(&ed, 0,
		   sizeof(ed)); ed.id = sEquipID; ed.count = iCount; ed.archid =
		   iGoodID; edlList->add_equipment_item(ed, true);
						}

						// fix "Ship Related" kick, update crc
						ulong lCRC;
						__asm
						{
								mov ecx, [szClassPtr]
								call [CRCAntiCheat]
								mov [lCRC], eax
						}
						memcpy(szClassPtr + 0x320, &lCRC, 4);*/
	}

	return HKE_OK;
}

HkError HkAddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& wscGood, int iCount, bool bMission)
{
	uint iGoodID = ToInt(wscGood.c_str());
	if (!iGoodID)
		pub::GetGoodID(iGoodID, wstos(wscGood).c_str());
	if (!iGoodID)
		return InvalidGood;

	return HkAddCargo(player, iGoodID, iCount, bMission);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkRename(const std::variant<uint, std::wstring>& player, const std::wstring& wscNewCharname, bool bOnlyDelete)
{
	const uint clientId = HkExtractClientId(player);

	if ((clientId == -1) && player.index() && !HkGetAccountByCharname(std::get<std::wstring>(player)))
		return CharDoesNotExist;

	if (!bOnlyDelete && HkGetAccountByCharname(wscNewCharname))
		return AlreadyExists;

	if (!bOnlyDelete && (wscNewCharname.length() > 23))
		return CharacterNameTooLong;

	if (!bOnlyDelete && !wscNewCharname.length())
		return CharacterNameTooShort;

	INI_Reader ini;
	if (!bOnlyDelete && !(ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false)))
		return MpNewCharacterFileNotFoundOrInvalid;

	CAccount* acc;
	std::wstring wscOldCharname;
	if (clientId != -1)
	{
		acc = Players.FindAccountFromClientID(clientId);
		wscOldCharname = (wchar_t*)Players.GetActiveCharacterName(clientId);
	}
	else
	{
		wscOldCharname = std::get<std::wstring>(player);
		acc = HkGetAccountByCharname(std::get<std::wstring>(player));
	}

	std::wstring wscAccountDirname;
	HkGetAccountDirName(acc, wscAccountDirname);
	std::wstring wscNewFilename;
	HkGetCharFileName(wscNewCharname, wscNewFilename);
	std::wstring wscOldFilename;
	HkGetCharFileName(wscOldCharname, wscOldFilename);

	std::string scNewCharfilePath = scAcctPath + wstos(wscAccountDirname) + "\\" + wstos(wscNewFilename) + ".fl";
	std::string scOldCharfilePath = scAcctPath + wstos(wscAccountDirname) + "\\" + wstos(wscOldFilename) + ".fl";

	if (bOnlyDelete)
	{
		// delete character
		st6::wstring str((ushort*)wscOldCharname.c_str());
		HkLockAccountAccess(acc, true); // also kicks player on this account
		Players.DeleteCharacterFromName(str);
		HkUnlockAccountAccess(acc);
		return HKE_OK;
	}

	HkLockAccountAccess(acc, true); // kick player if online
	HkUnlockAccountAccess(acc);

	// Copy existing char file into tmp
	std::string scTmpPath = scOldCharfilePath + ".tmp";
	DeleteFile(scTmpPath.c_str());
	CopyFile(scOldCharfilePath.c_str(), scTmpPath.c_str(), FALSE);

	// Delete existing char otherwise a rename of the char in slot 5 fails.
	st6::wstring str((ushort*)wscOldCharname.c_str());
	Players.DeleteCharacterFromName(str);

	// Emulate char create
	SLoginInfo logindata;
	wcsncpy_s(logindata.wszAccount, HkGetAccountID(acc).c_str(), 36);
	Players.login(logindata, MaxClientId + 1);

	SCreateCharacterInfo newcharinfo;
	wcsncpy_s(newcharinfo.wszCharname, wscNewCharname.c_str(), 23);
	newcharinfo.wszCharname[23] = 0;

	newcharinfo.iNickName = 0;
	newcharinfo.iBase = 0;
	newcharinfo.iPackage = 0;
	newcharinfo.iPilot = 0;

	while (ini.read_header())
	{
		if (ini.is_header("Faction"))
		{
			while (ini.read_value())
			{
				if (ini.is_value("nickname"))
					newcharinfo.iNickName = CreateID(ini.get_value_string());
				else if (ini.is_value("base"))
					newcharinfo.iBase = CreateID(ini.get_value_string());
				else if (ini.is_value("Package"))
					newcharinfo.iPackage = CreateID(ini.get_value_string());
				else if (ini.is_value("Pilot"))
					newcharinfo.iPilot = CreateID(ini.get_value_string());
			}
			break;
		}
	}
	ini.close();

	if (newcharinfo.iNickName == 0)
		newcharinfo.iNickName = CreateID("new_player");
	if (newcharinfo.iBase == 0)
		newcharinfo.iBase = CreateID("Li01_01_Base");
	if (newcharinfo.iPackage == 0)
		newcharinfo.iPackage = CreateID("ge_fighter");
	if (newcharinfo.iPilot == 0)
		newcharinfo.iPilot = CreateID("trent");

	// Fill struct with valid data (though it isnt used it is needed)
	newcharinfo.iDunno[4] = 65536;
	newcharinfo.iDunno[5] = 65538;
	newcharinfo.iDunno[6] = 0;
	newcharinfo.iDunno[7] = 1058642330;
	newcharinfo.iDunno[8] = 3206125978;
	newcharinfo.iDunno[9] = 65537;
	newcharinfo.iDunno[10] = 0;
	newcharinfo.iDunno[11] = 3206125978;
	newcharinfo.iDunno[12] = 65539;
	newcharinfo.iDunno[13] = 65540;
	newcharinfo.iDunno[14] = 65536;
	newcharinfo.iDunno[15] = 65538;
	Server.CreateNewCharacter(newcharinfo, MaxClientId + 1);
	HkSaveChar(wscNewCharname);
	Players.logout(MaxClientId + 1);

	// Decode the backup of the old char and overwrite the new char file
	if (!flc_decode(scTmpPath.c_str(), scNewCharfilePath.c_str()))
	{                                          // file wasn't encoded, thus
											   // simply rename it
		DeleteFile(scNewCharfilePath.c_str()); // just to get sure...
		CopyFile(scTmpPath.c_str(), scNewCharfilePath.c_str(), FALSE);
	}
	DeleteFile(scTmpPath.c_str());

	// Update the char name in the new char file.
	// Add a space to the value so the ini file line looks like "<key> =
	// <value>" otherwise Ioncross Server Operator can't decode the file
	// correctly
	std::string scValue = " ";
	for (uint i = 0; (i < wscNewCharname.length()); i++)
	{
		char cHiByte = wscNewCharname[i] >> 8;
		char cLoByte = wscNewCharname[i] & 0xFF;
		char szBuf[8];
		sprintf_s(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		scValue += szBuf;
	}
	IniWrite(scNewCharfilePath, "Player", "Name", scValue);

	// Re-encode the char file if needed.
	if (!FLHookConfig::i()->general.disableCharfileEncryption)
		if (!flc_encode(scNewCharfilePath.c_str(), scNewCharfilePath.c_str()))
			return CouldNotEncodeCharFile;

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkMsgAndKick(uint clientId, const std::wstring& wscReason, uint iIntervall)
{
	if (!ClientInfo[clientId].tmKickTime)
	{
		std::wstring wscMsg = ReplaceStr(FLHookConfig::i()->msgStyle.kickMsg, L"%reason", XMLText(wscReason));
		HkFMsg(clientId, wscMsg);
		ClientInfo[clientId].tmKickTime = timeInMS() + iIntervall;
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkKill(const std::variant<uint, std::wstring>& player)
{
	const uint clientId = HkExtractClientId(player);

	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	uint iShip;
	pub::Player::GetShip(clientId, iShip);
	if (!iShip)
		return PlayerNotInSpace;

	pub::SpaceObj::SetRelativeHealth(iShip, 0.0f);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetReservedSlot(const std::variant<uint, std::wstring>& player, bool& bResult)
{
	const uint clientId = HkExtractClientId(player);

	CAccount* acc;
	if (clientId != -1)
		acc = Players.FindAccountFromClientID(clientId);
	else
		acc = player.index() ? HkGetAccountByCharname(std::get<std::wstring>(player)) : nullptr;

	if (!acc)
		return CharDoesNotExist;

	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	bResult = IniGetB(scUserFile, "Settings", "ReservedSlot", false);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkSetReservedSlot(const std::variant<uint, std::wstring>& player, bool bReservedSlot)
{
	CAccount* acc = HkExtractAccount(player);
	if (!acc)
		return CharDoesNotExist;

	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	if (bReservedSlot)
		IniWrite(scUserFile, "Settings", "ReservedSlot", "yes");
	else
		IniWrite(scUserFile, "Settings", "ReservedSlot", "no");
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct AUTOBUY_CARTITEM
{
	uint iArchID;
	uint iCount;
	std::wstring wscDescription;
};

int HkPlayerAutoBuyGetCount(std::list<CARGO_INFO>& lstCargo, uint iItemArchID)
{
	for (auto& cargo : lstCargo)
	{
		if (cargo.iArchID == iItemArchID)
			return cargo.iCount;
	}

	return 0;
}

#define ADD_EQUIP_TO_CART(desc)                                                        \
	{                                                                                  \
		aci.iArchID = ((Archetype::Launcher*)eq)->iProjectileArchID;                   \
		aci.iCount = MAX_PLAYER_AMMO - HkPlayerAutoBuyGetCount(lstCargo, aci.iArchID); \
		aci.wscDescription = desc;                                                     \
		lstCart.push_back(aci);                                                        \
	}

void HkPlayerAutoBuy(uint clientId, uint iBaseID)
{
	// player cargo
	int iRemHoldSize;
	std::list<CARGO_INFO> lstCargo;
	HkEnumCargo(clientId, lstCargo, iRemHoldSize);

	// shopping cart
	std::list<AUTOBUY_CARTITEM> lstCart;

	if (ClientInfo[clientId].bAutoBuyReload)
	{ // shield bats & nanobots
		Archetype::Ship* ship = Archetype::GetShip(Players[clientId].iShipArchetype);

		uint iNanobotsID;
		pub::GetGoodID(iNanobotsID, "ge_s_repair_01");
		uint iRemNanobots = ship->iMaxNanobots;
		uint iShieldBatsID;
		pub::GetGoodID(iShieldBatsID, "ge_s_battery_01");
		uint iRemShieldBats = ship->iMaxShieldBats;
		bool bNanobotsFound = false;
		bool bShieldBattsFound = false;
		for (auto& cargo : lstCargo)
		{
			AUTOBUY_CARTITEM aci;
			if (cargo.iArchID == iNanobotsID)
			{
				aci.iArchID = iNanobotsID;
				aci.iCount = ship->iMaxNanobots - cargo.iCount;
				aci.wscDescription = L"Nanobots";
				lstCart.push_back(aci);
				bNanobotsFound = true;
			}
			else if (cargo.iArchID == iShieldBatsID)
			{
				aci.iArchID = iShieldBatsID;
				aci.iCount = ship->iMaxShieldBats - cargo.iCount;
				aci.wscDescription = L"Shield Batteries";
				lstCart.push_back(aci);
				bShieldBattsFound = true;
			}
		}

		if (!bNanobotsFound)
		{ // no nanos found -> add all
			AUTOBUY_CARTITEM aci;
			aci.iArchID = iNanobotsID;
			aci.iCount = ship->iMaxNanobots;
			aci.wscDescription = L"Nanobots";
			lstCart.push_back(aci);
		}

		if (!bShieldBattsFound)
		{ // no batts found -> add all
			AUTOBUY_CARTITEM aci;
			aci.iArchID = iShieldBatsID;
			aci.iCount = ship->iMaxShieldBats;
			aci.wscDescription = L"Shield Batteries";
			lstCart.push_back(aci);
		}
	}

	if (ClientInfo[clientId].bAutoBuyCD || ClientInfo[clientId].bAutoBuyCM || ClientInfo[clientId].bAutoBuyMines ||
		ClientInfo[clientId].bAutoBuyMissiles || ClientInfo[clientId].bAutoBuyTorps)
	{
		// add mounted equip to a new list and eliminate double equipment(such
		// as 2x lancer etc)
		std::list<CARGO_INFO> lstMounted;
		for (auto& cargo : lstCargo)
		{
			if (!cargo.bMounted)
				continue;

			bool bFound = false;
			for (auto& mounted : lstMounted)
			{
				if (mounted.iArchID == cargo.iArchID)
				{
					bFound = true;
					break;
				}
			}

			if (!bFound)
				lstMounted.push_back(cargo);
		}

		uint iVFTableMines = (uint)hModCommon + ADDR_COMMON_VFTABLE_MINE;
		uint iVFTableCM = (uint)hModCommon + ADDR_COMMON_VFTABLE_CM;
		uint iVFTableGun = (uint)hModCommon + ADDR_COMMON_VFTABLE_GUN;

		// check mounted equip
		for (auto& mounted : lstMounted)
		{
			uint i = mounted.iArchID;
			AUTOBUY_CARTITEM aci;
			Archetype::Equipment* eq = Archetype::GetEquipment(mounted.iArchID);
			EquipmentType eq_type = HkGetEqType(eq);
			if (eq_type == ET_MINE)
			{
				if (ClientInfo[clientId].bAutoBuyMines)
					ADD_EQUIP_TO_CART(L"Mines")
			}
			else if (eq_type == ET_CM)
			{
				if (ClientInfo[clientId].bAutoBuyCM)
					ADD_EQUIP_TO_CART(L"Countermeasures")
			}
			else if (eq_type == ET_TORPEDO)
			{
				if (ClientInfo[clientId].bAutoBuyTorps)
					ADD_EQUIP_TO_CART(L"Torpedos")
			}
			else if (eq_type == ET_CD)
			{
				if (ClientInfo[clientId].bAutoBuyCD)
					ADD_EQUIP_TO_CART(L"Cruise Disruptors")
			}
			else if (eq_type == ET_MISSILE)
			{
				if (ClientInfo[clientId].bAutoBuyMissiles)
					ADD_EQUIP_TO_CART(L"Missiles")
			}
		}
	}

	// search base in base-info list
	BASE_INFO* bi = 0;
	for (auto& base : lstBases)
	{
		if (base.iBaseID == iBaseID)
		{
			bi = &base;
			break;
		}
	}

	if (!bi)
		return; // base not found

	int iCash;
	HkGetCash(clientId, iCash);

	for (auto& buy : lstCart)
	{
		if (!buy.iCount || !Arch2Good(buy.iArchID))
			continue;

		// check if good is available and if player has the neccessary rep
		bool bGoodAvailable = false;
		for (auto& available : bi->lstMarketMisc)
		{
			if (available.iArchID == buy.iArchID)
			{
				// get base rep
				int iSolarRep;
				pub::SpaceObj::GetSolarRep(bi->iObjectID, iSolarRep);
				uint iBaseRep;
				pub::Reputation::GetAffiliation(iSolarRep, iBaseRep);
				if (iBaseRep == -1)
					continue; // rep can't be determined yet(space object not
							  // created yet?)

				// get player rep
				int iRepID;
				pub::Player::GetRep(clientId, iRepID);

				// check if rep is sufficient
				float fPlayerRep;
				pub::Reputation::GetGroupFeelingsTowards(iRepID, iBaseRep, fPlayerRep);
				if (fPlayerRep < available.fRep)
					break; // bad rep, not allowed to buy
				bGoodAvailable = true;
				break;
			}
		}

		if (!bGoodAvailable)
			continue; // base does not sell this item or bad rep

		float fPrice;
		if (pub::Market::GetPrice(iBaseID, buy.iArchID, fPrice) == -1)
			continue; // good not available

		Archetype::Equipment* eq = Archetype::GetEquipment(buy.iArchID);
		if (iRemHoldSize < (eq->fVolume * buy.iCount))
		{
			uint iNewCount = (uint)(iRemHoldSize / eq->fVolume);
			if (!iNewCount)
			{
				//				PrintUserCmdText(clientId,
				// L"Auto-Buy(%s): FAILED! Insufficient cargo space",
				// (*it4).wscDescription.c_str());
				continue;
			}
			else
				buy.iCount = iNewCount;
		}

		int iCost = ((int)fPrice * buy.iCount);
		if (iCash < iCost)
			PrintUserCmdText(clientId, L"Auto-Buy(%s): FAILED! Insufficient Credits", buy.wscDescription.c_str());
		else
		{
			HkAddCash(clientId, -iCost);
			iCash -= iCost;
			iRemHoldSize -= ((int)eq->fVolume * buy.iCount);
			//			HkAddCargo(clientId,
			//(*it4).iArchID, (*it4).iCount, false);

			// add the item, dont use hkaddcargo for performance/bug reasons
			// assume we only mount multicount goods (missiles, ammo, bots)
			pub::Player::AddCargo(clientId, buy.iArchID, buy.iCount, 1, false);

			PrintUserCmdText(
				clientId, L"Auto-Buy(%s): Bought %u unit(s), cost: %s$", buy.wscDescription.c_str(), buy.iCount,
				ToMoneyStr(iCost).c_str());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkResetRep(const std::variant<uint, std::wstring>& player)
{
	const uint clientId = HkExtractClientId(player);

	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	INI_Reader ini;
	if (!ini.open("mpnewcharacter.fl", false))
		return MpNewCharacterFileNotFoundOrInvalid;

	ini.read_header();
	if (!ini.is_header("Player"))
	{
		ini.close();
		return MpNewCharacterFileNotFoundOrInvalid;
	}

	int iPlayerRep;
	pub::Player::GetRep(clientId, iPlayerRep);
	while (ini.read_value())
	{
		if (ini.is_value("house"))
		{
			float fRep = ini.get_value_float(0);
			const char* szRepGroupName = ini.get_value_string(1);

			uint iRepGroupID;
			pub::Reputation::GetReputationGroup(iRepGroupID, szRepGroupName);
			pub::Reputation::SetReputation(iPlayerRep, iRepGroupID, fRep);
		}
	}

	ini.close();
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkSetRep(const std::variant<uint, std::wstring>& player, const std::wstring& wscRepGroup, float fValue)
{
	const uint clientId = HkExtractClientId(player);
	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	uint iRepGroupID;
	pub::Reputation::GetReputationGroup(iRepGroupID, wstos(wscRepGroup).c_str());
	if (iRepGroupID == -1)
		return InvalidRepGroup;

	int iPlayerRep;
	pub::Player::GetRep(clientId, iPlayerRep);
	pub::Reputation::SetReputation(iPlayerRep, iRepGroupID, fValue);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetRep(const std::variant<uint, std::wstring>& player, const std::wstring& wscRepGroup, float& fValue)
{
	const uint clientId = HkExtractClientId(player);
	if (clientId == -1)
		return PlayerNotLoggedIn;

	uint iRepGroupID;
	pub::Reputation::GetReputationGroup(iRepGroupID, wstos(wscRepGroup).c_str());
	if (iRepGroupID == -1)
		return InvalidRepGroup;

	int iPlayerRep;
	pub::Player::GetRep(clientId, iPlayerRep);
	pub::Reputation::GetGroupFeelingsTowards(iPlayerRep, iRepGroupID, fValue);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetGroupMembers(const std::variant<uint, std::wstring>& player, std::list<GROUP_MEMBER>& lstMembers)
{
	lstMembers.clear();
	const uint clientId = HkExtractClientId(player);

	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	// hey, at least it works! beware of the VC optimiser.
	st6::vector<uint> vMembers;
	pub::Player::GetGroupMembers(clientId, vMembers);

	for (uint i : vMembers)
	{
		GROUP_MEMBER gm;
		gm.clientId = i;
		gm.character = (wchar_t*)Players.GetActiveCharacterName(i);
		lstMembers.push_back(gm);
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkReadCharFile(const std::variant<uint, std::wstring>& player, std::list<std::wstring>& lstOutput)
{
	lstOutput.clear();
	const uint clientId = HkExtractClientId(player);

	std::wstring wscDir;
	CAccount* acc;
	if (clientId != -1)
	{
		acc = Players.FindAccountFromClientID(clientId);
		const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(clientId);
		if (!wszCharname)
			return NoCharSelected;

		if (!HKHKSUCCESS(HkGetAccountDirName(wszCharname, wscDir)))
			return CharDoesNotExist;
	}
	else
	{
		acc = HkExtractAccount(player);
		if (!acc)
			return CharDoesNotExist;
	}

	std::wstring wscFile;
	HkGetCharFileName(player, wscFile);
	std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	std::string scFileToRead;
	bool bDeleteAfter;
	if (HkIsEncoded(scCharFile))
	{
		std::string scCharFileNew = scCharFile + ".ini";
		if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return CouldNotDecodeCharFile;
		scFileToRead = scCharFileNew;
		bDeleteAfter = true;
	}
	else
	{
		scFileToRead = scCharFile;
		bDeleteAfter = false;
	}

	std::ifstream ifs;
	ifs.open(scFileToRead.c_str(), std::ios_base::in);
	if (!ifs.is_open())
		return UnknownError;

	std::string scLine;
	while (getline(ifs, scLine))
		lstOutput.push_back(stows(scLine));
	ifs.close();
	if (bDeleteAfter)
		DeleteFile(scFileToRead.c_str());
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkWriteCharFile(const std::variant<uint, std::wstring>& player, std::wstring wscData)
{
	const uint clientId = HkExtractClientId(player);

	std::wstring wscDir;
	CAccount* acc;
	if (clientId != -1)
	{
		acc = Players.FindAccountFromClientID(clientId);
		const wchar_t* wszCharname = (wchar_t*)Players.GetActiveCharacterName(clientId);
		if (!wszCharname)
			return NoCharSelected;

		if (!HKHKSUCCESS(HkGetAccountDirName(wszCharname, wscDir)))
			return CharDoesNotExist;
	}
	else
	{
		acc = HkExtractAccount(player);
		if (!acc)
			return CharDoesNotExist;
	}

	std::wstring wscFile;
	HkGetCharFileName(player, wscFile);
	std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	std::string scFileToWrite;
	bool bEncode;
	if (HkIsEncoded(scCharFile))
	{
		scFileToWrite = scCharFile + ".ini";
		bEncode = true;
	}
	else
	{
		scFileToWrite = scCharFile;
		bEncode = false;
	}

	std::ofstream ofs;
	ofs.open(scFileToWrite.c_str(), std::ios_base::out);
	if (!ofs.is_open())
		return UnknownError;

	size_t iPos;
	while ((iPos = wscData.find(L"\\n")) != -1)
	{
		std::wstring wscLine = wscData.substr(0, iPos);
		ofs << wstos(wscLine) << std::endl;
		wscData.erase(0, iPos + 2);
	}

	if (wscData.length())
		ofs << wstos(wscData);

	ofs.close();
	if (bEncode)
	{
		flc_encode(scFileToWrite.c_str(), scCharFile.c_str());
		DeleteFile(scFileToWrite.c_str());
	}
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkPlayerRecalculateCRC(uint clientId)
{
	try
	{
		PlayerData* pd = &Players[clientId];
		char* ACCalcCRC = (char*)hModServer + 0x6FAF0;
		__asm {
			pushad
			mov ecx, [pd]
			call[ACCalcCRC]
			mov ecx, [pd]
			mov[ecx + 320h], eax
			popad
		}
	}
	catch (...)
	{
		return InvalidClientId;
	}

	return HKE_OK;
}

/** Move the client to the specified location */
void HkRelocateClient(uint clientId, Vector vDestination, Matrix mOrientation)
{
	Quaternion qRotation = HkMatrixToQuaternion(mOrientation);

	FLPACKET_LAUNCH pLaunch;
	pLaunch.iShip = ClientInfo[clientId].iShip;
	pLaunch.iBase = 0;
	pLaunch.iState = 0xFFFFFFFF;
	pLaunch.fRotate[0] = qRotation.w;
	pLaunch.fRotate[1] = qRotation.x;
	pLaunch.fRotate[2] = qRotation.y;
	pLaunch.fRotate[3] = qRotation.z;
	pLaunch.fPos[0] = vDestination.x;
	pLaunch.fPos[1] = vDestination.y;
	pLaunch.fPos[2] = vDestination.z;

	HookClient->Send_FLPACKET_SERVER_LAUNCH(clientId, pLaunch);

	uint iSystem;
	pub::Player::GetSystem(clientId, iSystem);
	pub::SpaceObj::Relocate(ClientInfo[clientId].iShip, iSystem, vDestination, mOrientation);
}

/** Dock the client immediately */
HkError HkInstantDock(uint clientId, uint iDockObj)
{
	// check if logged in
	if (clientId == -1)
		return PlayerNotLoggedIn;

	uint iShip;
	pub::Player::GetShip(clientId, iShip);
	if (!iShip)
		return PlayerNotInSpace;

	uint iSystem, iSystem2;
	pub::SpaceObj::GetSystem(iShip, iSystem);
	pub::SpaceObj::GetSystem(iDockObj, iSystem2);
	if (iSystem != iSystem2)
	{
		return PlayerNotInSpace;
	}

	try
	{
		pub::SpaceObj::InstantDock(iShip, iDockObj, 1);
	}
	catch (...)
	{
		return PlayerNotInSpace;
	}

	return HKE_OK;
}

HkError HkGetRank(const std::variant<uint, std::wstring>& player, int& iRank)
{
	HkError err;
	std::wstring wscRet = L"";
	if ((err = HkFLIniGet(player, L"rank", wscRet)) != HKE_OK)
		return err;
	if (wscRet.length())
		iRank = ToInt(wscRet);
	else
		iRank = 0;
	return HKE_OK;
}

/// Get online time.
HkError HkGetOnlineTime(const std::variant<uint, std::wstring>& player, int& iSecs)
{
	std::wstring wscDir;
	if (!HKHKSUCCESS(HkGetAccountDirName(player, wscDir)))
		return CharDoesNotExist;

	std::wstring wscFile;
	HkGetCharFileName(player, wscFile);

	std::string scCharFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	if (HkIsEncoded(scCharFile))
	{
		std::string scCharFileNew = scCharFile + ".ini";
		if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return CouldNotDecodeCharFile;

		iSecs = (int)IniGetF(scCharFileNew, "mPlayer", "total_time_played", 0.0f);
		DeleteFile(scCharFileNew.c_str());
	}
	else
	{
		iSecs = (int)IniGetF(scCharFile, "mPlayer", "total_time_played", 0.0f);
	}

	return HKE_OK;
}

/// Return true if this player is within the specified distance of any other
/// player.
bool IsInRange(uint clientId, float fDistance)
{
	std::list<GROUP_MEMBER> lstMembers;
	HkGetGroupMembers((const wchar_t*)Players.GetActiveCharacterName(clientId), lstMembers);

	uint iShip;
	pub::Player::GetShip(clientId, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(clientId, iSystem);

	// For all players in system...
	struct PlayerData* playerDb = 0;
	while (playerDb = Players.traverse_active(playerDb))
	{
		// Get the this player's current system and location in the system.
		uint clientId2 = HkGetClientIdFromPD(playerDb);
		uint iSystem2 = 0;
		pub::Player::GetSystem(clientId2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(clientId2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Ignore players who are in your group.
		bool bGrouped = false;
		for (auto& gm : lstMembers)
		{
			if (gm.clientId == clientId2)
			{
				bGrouped = true;
				break;
			}
		}
		if (bGrouped)
			continue;

		// Is player within the specified range of the sending char.
		if (HkDistance3D(pos, pos2) < fDistance)
			return true;
	}
	return false;
}

/**
Delete a character.
*/
HkError HkDeleteCharacter(CAccount* acc, std::wstring& character)
{
	HkLockAccountAccess(acc, true);
	st6::wstring str((ushort*)character.c_str());
	Players.DeleteCharacterFromName(str);
	HkUnlockAccountAccess(acc);
	return HKE_OK;
}

/**
Create a new character in the specified account by emulating a
create character.
*/
HkError HkNewCharacter(CAccount* acc, std::wstring& character)
{
	HkLockAccountAccess(acc, true);
	HkUnlockAccountAccess(acc);

	INI_Reader ini;
	if (!ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
		return MpNewCharacterFileNotFoundOrInvalid;

	// Emulate char create by logging in.
	SLoginInfo logindata;
	wcsncpy_s(logindata.wszAccount, HkGetAccountID(acc).c_str(), 36);
	Players.login(logindata, Players.GetMaxPlayerCount() + 1);

	SCreateCharacterInfo newcharinfo;
	wcsncpy_s(newcharinfo.wszCharname, character.c_str(), 23);
	newcharinfo.wszCharname[23] = 0;

	newcharinfo.iNickName = 0;
	newcharinfo.iBase = 0;
	newcharinfo.iPackage = 0;
	newcharinfo.iPilot = 0;

	while (ini.read_header())
	{
		if (ini.is_header("Faction"))
		{
			while (ini.read_value())
			{
				if (ini.is_value("nickname"))
					newcharinfo.iNickName = CreateID(ini.get_value_string());
				else if (ini.is_value("base"))
					newcharinfo.iBase = CreateID(ini.get_value_string());
				else if (ini.is_value("package"))
					newcharinfo.iPackage = CreateID(ini.get_value_string());
				else if (ini.is_value("pilot"))
					newcharinfo.iPilot = CreateID(ini.get_value_string());
			}
			break;
		}
	}
	ini.close();

	if (newcharinfo.iNickName == 0)
		newcharinfo.iNickName = CreateID("new_player");
	if (newcharinfo.iBase == 0)
		newcharinfo.iBase = CreateID("Li01_01_Base");
	if (newcharinfo.iPackage == 0)
		newcharinfo.iPackage = CreateID("ge_fighter");
	if (newcharinfo.iPilot == 0)
		newcharinfo.iPilot = CreateID("trent");

	// Fill struct with valid data (though it isnt used it is needed)
	newcharinfo.iDunno[4] = 65536;
	newcharinfo.iDunno[5] = 65538;
	newcharinfo.iDunno[6] = 0;
	newcharinfo.iDunno[7] = 1058642330;
	newcharinfo.iDunno[8] = 3206125978;
	newcharinfo.iDunno[9] = 65537;
	newcharinfo.iDunno[10] = 0;
	newcharinfo.iDunno[11] = 3206125978;
	newcharinfo.iDunno[12] = 65539;
	newcharinfo.iDunno[13] = 65540;
	newcharinfo.iDunno[14] = 65536;
	newcharinfo.iDunno[15] = 65538;
	Server.CreateNewCharacter(newcharinfo, Players.GetMaxPlayerCount() + 1);
	HkSaveChar(character);
	Players.logout(Players.GetMaxPlayerCount() + 1);
	return HKE_OK;
}

typedef void(__stdcall* _FLAntiCheat)();
typedef void(__stdcall* _FLPossibleCheatingDetected)(int iReason);

/** Anti cheat checking code by mc_horst */
HkError HkAntiCheat(uint clientId)
{
#define ADDR_FL_ANTICHEAT_1                0x70120
#define ADDR_FL_ANTICHEAT_2                0x6FD20
#define ADDR_FL_ANTICHEAT_3                0x6FAF0
#define ADDR_FL_ANTICHEAT_4                0x6FAA0
#define ADDR_FL_POSSIBLE_CHEATING_DETECTED 0x6F570

	_FLAntiCheat FLAntiCheat1 = (_FLAntiCheat)((char*)hModServer + ADDR_FL_ANTICHEAT_1);
	_FLAntiCheat FLAntiCheat2 = (_FLAntiCheat)((char*)hModServer + ADDR_FL_ANTICHEAT_2);
	_FLAntiCheat FLAntiCheat3 = (_FLAntiCheat)((char*)hModServer + ADDR_FL_ANTICHEAT_3);
	_FLAntiCheat FLAntiCheat4 = (_FLAntiCheat)((char*)hModServer + ADDR_FL_ANTICHEAT_4);
	_FLPossibleCheatingDetected FLPossibleCheatingDetected =
		(_FLPossibleCheatingDetected)((char*)hModServer + ADDR_FL_POSSIBLE_CHEATING_DETECTED);

	// check if ship in space
	uint iShip = 0;
	pub::Player::GetShip(clientId, iShip);
	if (iShip)
		return HKE_OK;

	char* szObjPtr;
	memcpy(&szObjPtr, &Players, 4);
	szObjPtr += 0x418 * (clientId - 1);

	char cRes;

	////////////////////////// 1
	__asm {
		mov ecx, [szObjPtr]
		call [FLAntiCheat1]
		mov [cRes], al
	}

	if (cRes != 0)
	{ // kick
		HkKick(clientId);
		return UnknownError;
	}

	////////////////////////// 2
	__asm {
		mov ecx, [szObjPtr]
		call [FLAntiCheat2]
		mov [cRes], al
	}

	if (cRes != 0)
	{ // kick
		HkKick(clientId);
		return UnknownError;
	}

	////////////////////////// 3
	ulong lRet;
	ulong lCompare;
	__asm {
		mov ecx, [szObjPtr]
		mov eax, [ecx+0x320]
		mov [lCompare], eax
		call [FLAntiCheat3]
		mov [lRet], eax
	}

	if (lRet > lCompare)
	{ // kick
		HkKick(clientId);
		return UnknownError;
	}

	////////////////////////// 4
	__asm {
		mov ecx, [szObjPtr]
		call [FLAntiCheat4]
		mov [cRes], al
	}

	if (cRes != 0)
	{ // kick
		HkKick(clientId);
		return UnknownError;
	}

	return HKE_OK;
}

HkError HkSetEquip(const std::variant<uint, std::wstring>& player, const st6::list<EquipDesc>& equip)
{
	const uint clientId = HkExtractClientId(player);

	if ((clientId == -1) || HkIsInCharSelectMenu(clientId))
		return NoCharSelected;

	// Update FLHook's lists to make anticheat pleased.
	if (&equip != &Players[clientId].lShadowEquipDescList.equip)
		Players[clientId].lShadowEquipDescList.equip = equip;

	if (&equip != &Players[clientId].equipDescList.equip)
		Players[clientId].equipDescList.equip = equip;

	// Calculate packet size. First two bytes reserved for items count.
	uint itemBufSize = 2;
	for (const auto& item : equip)
	{
		itemBufSize += sizeof(SetEquipmentItem) + strlen(item.szHardPoint.value) + 1;
	}

	FLPACKET* packet = FLPACKET::Create(itemBufSize, FLPACKET::FLPACKET_SERVER_SETEQUIPMENT);
	FlPacketSetEquipment* pSetEquipment = reinterpret_cast<FlPacketSetEquipment*>(packet->content);

	// Add items to packet as array of variable size.
	uint index = 0;
	for (const auto& item : equip)
	{
		SetEquipmentItem setEquipItem;
		setEquipItem.iCount = item.iCount;
		setEquipItem.fHealth = item.fHealth;
		setEquipItem.iArchID = item.iArchID;
		setEquipItem.sID = item.sID;
		setEquipItem.bMounted = item.bMounted;
		setEquipItem.bMission = item.bMission;

		if (uint len = strlen(item.szHardPoint.value); len && item.szHardPoint.value != "BAY") {
			setEquipItem.szHardPointLen = len + 1; // add 1 for the null - char* is a null-terminated string in C++
		}
		else {
			setEquipItem.szHardPointLen = 0;
		}
		pSetEquipment->count++;

		byte* buf = (byte*)&setEquipItem;
		for (int i = 0; i < sizeof(SetEquipmentItem); i++)
			pSetEquipment->items[index++] = buf[i];

		byte* szHardPoint = (byte*)item.szHardPoint.value;
		for (int i = 0; i < setEquipItem.szHardPointLen; i++)
			pSetEquipment->items[index++] = szHardPoint[i];
	}

	return packet->SendTo(clientId) ? HKE_OK : UnknownError;
	
}

HkError HkAddEquip(const std::variant<uint, std::wstring>& player, uint iGoodID, const std::string& scHardpoint)
{
	const uint clientId = HkExtractClientId(player);

	if ((clientId == -1) || HkIsInCharSelectMenu(clientId))
		return NoCharSelected;

	if (!Players[clientId].iEnteredBase)
	{
		Players[clientId].iEnteredBase = Players[clientId].iBaseID;
		Server.ReqAddItem(iGoodID, scHardpoint.c_str(), 1, 1.0f, true, clientId);
		Players[clientId].iEnteredBase = 0;
	}
	else
	{
		Server.ReqAddItem(iGoodID, scHardpoint.c_str(), 1, 1.0f, true, clientId);
	}

	// Add to check-list which is being compared to the users equip-list when
	// saving char to fix "Ship or Equipment not sold on base" kick
	EquipDesc ed;
	ed.sID = Players[clientId].sLastEquipID;
	ed.iCount = 1;
	ed.iArchID = iGoodID;
	Players[clientId].lShadowEquipDescList.add_equipment_item(ed, false);

	return HKE_OK;
}

HkError HkAddEquip(
	const std::variant<uint, std::wstring>& player, uint iGoodID, const std::string& scHardpoint, bool bMounted)
{
	typedef bool(__stdcall * _AddCargoDocked)(
		uint iGoodID, CacheString * &hardpoint, int iNumItems, float fHealth, int bMounted, int bMission, uint iOne);
	static _AddCargoDocked AddCargoDocked = 0;
	if (!AddCargoDocked)
		AddCargoDocked = (_AddCargoDocked)((char*)hModServer + 0x6EFC0);

	const uint clientId = HkExtractClientId(player);
	if (clientId == -1 || HkIsInCharSelectMenu(clientId))
		return PlayerNotLoggedIn;

	uint iBase = 0;
	pub::Player::GetBase(clientId, iBase);
	uint iLocation = 0;
	pub::Player::GetLocation(clientId, iLocation);

	if (iLocation)
		Server.LocationExit(iLocation, clientId);
	if (iBase)
		Server.BaseExit(iBase, clientId);
	if (!HkIsValidClientID(clientId))
		return PlayerNotLoggedIn;

	PlayerData* pd = &Players[clientId];
	const char* p = scHardpoint.c_str();
	CacheString hardpoint;
	hardpoint.value = StringAlloc(p, false);

	int iOne = 1;
	int iMounted = bMounted;
	float fHealth = 1;
	CacheString* pHP = &hardpoint;
	__asm {
		push iOne
		push iMounted
		push iOne
		push fHealth
		push iOne
		push pHP
		push iGoodID
		mov ecx, pd
		call AddCargoDocked
	}

	if (iBase) Server.BaseEnter(iBase, clientId);
	if (iLocation)
		Server.LocationEnter(iLocation, clientId);

	return HKE_OK;
}

std::wstring GetLocation(unsigned int clientId)
{
	uint iSystemID = 0;
	uint iShip = 0;
	pub::Player::GetSystem(clientId, iSystemID);
	pub::Player::GetShip(clientId, iShip);
	if (!iSystemID || !iShip)
	{
		PrintUserCmdText(clientId, L"ERR Not in space");
		return false;
	}

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	return VectorToSectorCoord<std::wstring>(iSystemID, pos);
}

CAccount* HkGetAccountByClientID(uint clientId)
{
	if (!HkIsValidClientID(clientId))
		return 0;

	return Players.FindAccountFromClientID(clientId);
}

std::wstring HkGetAccountIDByClientID(uint clientId)
{
	if (HkIsValidClientID(clientId))
	{
		CAccount* acc = HkGetAccountByClientID(clientId);
		if (acc && acc->wszAccID)
		{
			return acc->wszAccID;
		}
	}
	return L"";
}

void HkDelayedKick(uint clientId, uint secs)
{
	mstime kick_time = timeInMS() + (secs * 1000);
	if (!ClientInfo[clientId].tmKickTime || ClientInfo[clientId].tmKickTime > kick_time)
		ClientInfo[clientId].tmKickTime = kick_time;
}

std::string HkGetPlayerSystemS(uint clientId)
{
	uint iSystemID;
	pub::Player::GetSystem(clientId, iSystemID);
	char szSystemname[1024] = "";
	pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystemID);
	return szSystemname;
}

HkError HKGetShipValue(const std::variant<uint, std::wstring>& player, float& fValue)
{
	uint clientId = HkExtractClientId(player);
	if (clientId != -1 && !HkIsInCharSelectMenu(clientId))
	{
		HkSaveChar(player);
		if (!HkIsValidClientID(clientId))
		{
			return UnknownError;
		}
	}

	fValue = 0.0f;

	uint iBaseID = 0;

	std::list<std::wstring> lstCharFile;
	HkError err = HkReadCharFile(player, lstCharFile);
	if (err != HKE_OK)
		return err;

	for (const auto& line : lstCharFile)
	{
		std::wstring wscKey = Trim(line.substr(0, line.find(L"=")));
		if (wscKey == L"base" || wscKey == L"last_base")
		{
			int iFindEqual = line.find(L"=");
			if (iFindEqual == -1)
			{
				continue;
			}

			if ((iFindEqual + 1) >= (int)line.size())
			{
				continue;
			}

			iBaseID = CreateID(wstos(Trim(line.substr(iFindEqual + 1))).c_str());
			break;
		}
	}

	for (const auto& line : lstCharFile)
	{
		std::wstring wscKey = Trim(line.substr(0, line.find(L"=")));
		if (wscKey == L"cargo" || wscKey == L"equip")
		{
			int iFindEqual = line.find(L"=");
			if (iFindEqual == -1)
			{
				continue;
			}
			int iFindComma = line.find(L",", iFindEqual);
			if (iFindComma == -1)
			{
				continue;
			}
			uint iGoodID = ToUInt(Trim(line.substr(iFindEqual + 1, iFindComma)));
			uint iGoodCount = ToUInt(Trim(line.substr(iFindComma + 1, line.find(L",", iFindComma + 1))));

			float fItemValue;
			if (pub::Market::GetPrice(iBaseID, Arch2Good(iGoodID), fItemValue) == 0)
			{
				if (arch_is_combinable(iGoodID))
				{
					fValue += fItemValue * iGoodCount;
				}
				else
				{
					float* fResaleFactor = (float*)((char*)hModServer + 0x8AE7C);
					fValue += fItemValue * (*fResaleFactor);
				}
			}
		}
		else if (wscKey == L"money")
		{
			int iFindEqual = line.find(L"=");
			if (iFindEqual == -1)
			{
				continue;
			}
			uint fItemValue = ToUInt(Trim(line.substr(iFindEqual + 1)));
			fValue += fItemValue;
		}
		else if (wscKey == L"ship_archetype")
		{
			uint iShipArchID = ToUInt(Trim(line.substr(line.find(L"=") + 1, line.length())));
			const GoodInfo* gi = GoodList_get()->find_by_ship_arch(iShipArchID);
			if (gi)
			{
				gi = GoodList::find_by_id(gi->iArchID);
				if (gi)
				{
					float* fResaleFactor = (float*)((char*)hModServer + 0x8AE78);
					float fItemValue = gi->fPrice * (*fResaleFactor);
					fValue += fItemValue;
				}
			}
		}
	}
	return HKE_OK;
}

void HkSaveChar(uint clientId)
{
	BYTE patch[] = { 0x90, 0x90 };
	WriteProcMem((char*)hModServer + 0x7EFA8, patch, sizeof(patch));
	pub::Save(clientId, 1);
}

HkError HkGetTarget(const std::variant<uint, std::wstring>& player, uint& target)
{
	uint ship;
	if (const HkError shipErr = HkGetShip(std::move(player), ship); shipErr != HKE_OK)
		return shipErr;

	pub::SpaceObj::GetTarget(ship, target);
	if (!ship)
		return NoTargetSelected;

	return HKE_OK;
}

HkError HkGetTargetClientId(const std::variant<uint, std::wstring>& player, uint& targetClientId)
{
	uint target;
	if (const HkError error = HkGetTarget(std::move(player), target); error != HKE_OK)
		return error;

	targetClientId = HkGetClientIDByShip(target);
	if (!targetClientId)
		return TargetIsNotPlayer;

	return HKE_OK;
}

HkError HkGetCurrentBase(const std::variant<uint, std::wstring>& player, uint& base)
{
	const uint clientId = HkExtractClientId(std::move(player));
	if (clientId == -1)
	{
		return PlayerNotLoggedIn;
	}

	pub::Player::GetBase(clientId, base);
	if (base)
	{
		return HKE_OK;
	}

	return PlayerNotDocked;
}

HkError HkGetSystem(const std::variant<uint, std::wstring>& player, uint& system)
{
	const uint clientId = HkExtractClientId(std::move(player));
	if (clientId == -1)
	{
		return PlayerNotLoggedIn;
	}

	pub::Player::GetSystem(clientId, system);
	if (!system)
		return InvalidSystem;

	return HKE_OK;
}

HkError HkGetShip(const std::variant<uint, std::wstring>& player, uint& ship)
{
	const uint clientId = HkExtractClientId(std::move(player));
	if (clientId == -1)
	{
		return PlayerNotLoggedIn;
	}

	pub::Player::GetShip(clientId, ship);
	if (!ship)
		return PlayerNotInSpace;

	return HKE_OK;
}