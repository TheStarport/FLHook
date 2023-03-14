#include "Global.hpp"
#include "Features/TempBan.hpp"


namespace Hk::Player
{
	using _FLAntiCheat = void(__stdcall*)();
	using _FLPossibleCheatingDetected = void(__stdcall*)(int iReason);
	constexpr auto AddrAntiCheat1 = 0x70120;
	constexpr auto AddrAntiCheat2 = 0x6FD20;
	constexpr auto AddrAntiCheat3 = 0x6FAF0;
	constexpr auto AddrAntiCheat4 = 0x6FAA0;
	constexpr auto PossibleCheatingDetectedAddr = 0x6F570;

	cpp::result<void, Error> AddToGroup(ClientId client, uint groupId)
	{
		if (!Client::IsValidClientID(client))
			return cpp::fail(Error::InvalidClientId);

		if (const uint groupId = Players.GetGroupID(client); groupId == groupId)
			return cpp::fail(Error::InvalidGroupId);

		CPlayerGroup* group = CPlayerGroup::FromGroupID(groupId);
		if (!group)
			return cpp::fail(Error::InvalidGroupId);

		group->AddMember(client);
		return {};
	}

	cpp::result<const uint, Error> GetGroupID(ClientId client)
	{
		if (!Client::IsValidClientID(client))
			return cpp::fail(Error::InvalidClientId);

		return Players.GetGroupID(client);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, Error> GetCash(const std::variant<uint, std::wstring>& player)
	{
		if (ClientId client = Client::ExtractClientID(player); client != UINT_MAX)
		{
			if (Client::IsInCharSelectMenu(client))
				return cpp::fail(Error::CharacterNotSelected);

			int cash;
			pub::Player::InspectCash(client, cash);
			return static_cast<uint>(cash);
		}

		if (!player.index())
			return cpp::fail(Error::InvalidClientId);

		const auto acc = Client::GetAccountByCharName(std::get<std::wstring>(player));
		if (acc.has_error())
			return cpp::fail(acc.error());

		const auto dir = Client::GetAccountDirName(acc.value());

		auto file = Client::GetCharFileName(player);
		if (file.has_error())
			return cpp::fail(file.error());

		const std::string charFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl";

		// TODO: Remove capi references
		FILE* fTest;
		fopen_s(&fTest, charFile.c_str(), "r");
		if (!fTest)
			return cpp::fail(Error::CharacterDoesNotExist);
		fclose(fTest);

		if (Client::IsEncoded(charFile))
		{
			const std::string charFileNew = charFile + ".ini";
			if (!FlcDecodeFile(charFile.c_str(), charFileNew.c_str()))
				return cpp::fail(Error::CouldNotDecodeCharFile);

			auto cash = static_cast<uint>(IniGetI(charFileNew, "Player", "money", -1));
			DeleteFile(charFileNew.c_str());
			return cash;
		}

		return static_cast<uint>(IniGetI(charFile, "Player", "money", -1));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> AdjustCash(const std::variant<uint, std::wstring>& player, int iAmount)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client != UINT_MAX)
		{
			if (Client::IsInCharSelectMenu(client))
				return cpp::fail(Error::CharacterNotSelected);

			pub::Player::AdjustCash(client, iAmount);
			return {};
		}

		if (!player.index())
		{
			return cpp::fail(Error::InvalidClientId);
		}

		const auto& character = std::get<std::wstring>(player);
		const auto acc = Client::GetAccountByCharName(std::get<std::wstring>(player));
		if (acc.has_error())
			return cpp::fail(acc.error());

		const auto dir = Client::GetAccountDirName(acc.value());

		const auto file = Client::GetCharFileName(character);
		if (file.has_error())
			return cpp::fail(file.error());

		const std::string charFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl";
		int iRet;
		if (Client::IsEncoded(charFile))
		{
			const std::string charFileNew = charFile + ".ini";

			if (!FlcDecodeFile(charFile.c_str(), charFileNew.c_str()))
				return cpp::fail(Error::CouldNotDecodeCharFile);

			iRet = IniGetI(charFileNew, "Player", "money", -1);
			// Add a space to the value so the ini file line looks like "<key> =
			// <value>" otherwise IFSO can't decode the file correctly
			IniWrite(charFileNew, "Player", "money", " " + std::to_string(iRet + iAmount));

			if (!FLHookConfig::i()->general.disableCharfileEncryption && !FlcEncodeFile(charFileNew.c_str(), charFile.c_str()))
				return cpp::fail(Error::CouldNotEncodeCharFile);

			DeleteFile(charFileNew.c_str());
		}
		else
		{
			iRet = IniGetI(charFile, "Player", "money", -1);
			// Add a space to the value so the ini file line looks like "<key> =
			// <value>" otherwise IFSO can't decode the file correctly
			IniWrite(charFile, "Player", "money", " " + std::to_string(iRet + iAmount));
		}

		if (client != UINT_MAX)
		{
			// money fix in case player logs in with this account
			bool bFound = false;
			const std::wstring characterLower = ToLower(character);
			for (auto& money : ClientInfo[client].MoneyFix)
			{
				if (money.character == characterLower)
				{
					money.uAmount += iAmount;
					bFound = true;
					break;
				}
			}

			if (!bFound)
			{
				MONEY_FIX mf;
				mf.character = characterLower;
				mf.uAmount = iAmount;
				ClientInfo[client].MoneyFix.push_back(mf);
			}
		}

		return {};
	}

	cpp::result<void, Error> AddCash(const std::variant<uint, std::wstring>& player, uint uAmount)
	{
		return AdjustCash(player, static_cast<int>(uAmount));
	}

	cpp::result<void, Error> RemoveCash(const std::variant<uint, std::wstring>& player, uint uAmount)
	{
		return AdjustCash(player, -static_cast<int>(uAmount));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> Kick(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		CAccount* acc = Players.FindAccountFromClientID(client);
		acc->ForceLogout();
		return {};
	}

	cpp::result<void, Error> KickReason(const std::variant<uint, std::wstring>& player, const std::wstring& Reason)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		if (Reason.length())
			MsgAndKick(client, Reason, FLHookConfig::i()->messages.msgStyle.kickMsgPeriod);
		else
			Players.FindAccountFromClientID(client)->ForceLogout();

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> Ban(const std::variant<uint, std::wstring>& player, bool bBan)
	{
		auto acc = Client::ExtractAccount(player);
		if (acc.has_error())
			return cpp::fail(Error::CharacterDoesNotExist);

		auto id = Client::GetAccountID(acc.value());
		if (id.has_error())
			return cpp::fail(id.error());

		st6::wstring fr((ushort*)id.value().c_str());
		Players.BanAccount(fr, bBan);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> TempBan(const std::variant<uint, std::wstring>& player, uint duration)
	{
		uint clientId;
		if (player.index() != 0)
		{
			const auto& charName = std::get<std::wstring>(player);
			auto client = Client::GetClientIdFromCharName(charName);
			if (client.has_error())
			{
				return cpp::fail(client.error());
			}
			clientId = client.value();
		}
		else
		{
			clientId = std::get<uint>(player);
		}

		if (!Client::IsValidClientID(clientId))
			return cpp::fail(Error::InvalidClientId);

		TempBanManager::i()->AddTempBan(clientId, duration);

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Bases that will cause crashes if jumped to
	const std::array BannedBases = {
		CreateID("br_m_beryllium_miner"),
		CreateID("[br_m_hydrocarbon_miner]"),
		CreateID("[br_m_niobium_miner]"),
		CreateID("[co_khc_copper_miner]"),
		CreateID("[co_khc_cobalt_miner]"),
		CreateID("[co_kt_hydrocarbon_miner]"),
		CreateID("[co_shi_h-fuel_miner]"),
		CreateID("[co_shi_water_miner]"),
		CreateID("[co_ti_water_miner]"),
		CreateID("[gd_gm_h-fuel_miner]"),
		CreateID("[gd_im_oxygen_miner]"),
		CreateID("[gd_im_copper_miner]"),
		CreateID("[gd_im_silver_miner]"),
		CreateID("[gd_im_water_miner]"),
		CreateID("[rh_m_diamond_miner]"),
		CreateID("intro3_base"),
		CreateID("intro2_base"),
		CreateID("intro1_base"),
		CreateID("st03b_01_base"),
		CreateID("st02_01_base"),
		CreateID("st01_02_base"),
		CreateID("iw02_03_base"),
		CreateID("rh02_07_base"),
		CreateID("li04_06_base"),
		CreateID("li01_15_base"),
	};

	cpp::result<void, Error> Beam(const std::variant<uint, std::wstring>& player, const std::variant<uint, std::wstring>& baseVar)
	{
		ClientId client = Client::ExtractClientID(player);
		uint baseId;

		// check if logged in
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}
		// check if ship in space
		if (const auto ship = GetShip(client); ship.has_error())
		{
			return cpp::fail(ship.error());
		}

		// if basename was passed as string
		if (baseVar.index() == 1)
		{
			const std::string baseName = wstos(std::get<std::wstring>(baseVar));

			// get base id
			if (pub::GetBaseID(baseId, baseName.c_str()) == -4)
			{
				return cpp::fail(Error::InvalidBaseName);
			}
		}
		else
		{
			baseId = std::get<uint>(baseVar);
		}

		if (std::ranges::find(BannedBases, baseId) != BannedBases.end())
		{
			return cpp::fail(Error::InvalidBaseName);
		}

		uint iSysId;
		pub::Player::GetSystem(client, iSysId);
		const Universe::IBase* base = Universe::get_base(baseId);

		if (!base)
		{
			return cpp::fail(Error::InvalidBase);
		}

		pub::Player::ForceLand(client, baseId); // beam

		// if not in the same system, emulate F1 charload
		if (base->systemId != iSysId)
		{
			Server.BaseEnter(baseId, client);
			Server.BaseExit(baseId, client);
			auto fileName = Client::GetCharFileName(client);
			if (fileName.has_error())
			{
				return cpp::fail(fileName.error());
			}
			const std::wstring newFile = fileName.value() + L".fl";
			CHARACTER_ID cId;
			strcpy_s(cId.charFilename, wstos(newFile.substr(0, 14)).c_str());
			Server.CharacterSelect(cId, client);
		}

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> SaveChar(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		void* pJmp = (char*)server + 0x7EFA8;
		const char Nop[2] = {'\x90', '\x90'};
		const char TestAlAl[2] = {'\x74', '\x44'};
		WriteProcMem(pJmp, Nop, sizeof(Nop)); // nop the SinglePlayer() check
		pub::Save(client, 1);
		WriteProcMem(pJmp, TestAlAl, sizeof(TestAlAl)); // restore

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	struct EquipItem
	{
		EquipItem* next;
		uint i2;
		ushort s1;
		ushort id;
		uint goodId;
		CacheString hardpoint;
		bool mounted;
		char unk[3];
		float status;
		uint count;
		bool mission;
	};

	cpp::result<const std::list<CargoInfo>, Error> EnumCargo(const std::variant<uint, std::wstring>& player, int& iRemainingHoldSize)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
			return cpp::fail(Error::PlayerNotLoggedIn);

		std::list<CargoInfo> cargo;

		char* ClassPtr;
		memcpy(&ClassPtr, &Players, 4);
		ClassPtr += 0x418 * (client - 1);

		EquipItem* eqList;
		memcpy(&eqList, ClassPtr + 0x27C, 4);
		const EquipItem* eq = eqList->next;
		while (eq != eqList)
		{
			CargoInfo ci = {eq->id, static_cast<int>(eq->count), eq->goodId, eq->status, eq->mission, eq->mounted, eq->hardpoint};
			cargo.push_back(ci);

			eq = eq->next;
		}

		float fRemHold;
		pub::Player::GetRemainingHoldSize(client, fRemHold);
		iRemainingHoldSize = static_cast<int>(fRemHold);
		return cargo;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> RemoveCargo(const std::variant<uint, std::wstring>& player, ushort cargoId, int count)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
			return cpp::fail(Error::PlayerNotLoggedIn);

		int iHold;
		const auto cargo = EnumCargo(player, iHold);
		if (cargo.has_error())
		{
			return cpp::fail(cargo.error());
		}

		for (auto& item : cargo.value())
		{
			if ((item.id == cargoId) && (item.count < count))
				count = item.count; // trying to remove more than actually there
		}

		pub::Player::RemoveCargo(client, cargoId, count);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> AddCargo(const std::variant<uint, std::wstring>& player, uint iGoodId, int iCount, bool bMission)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
			return cpp::fail(Error::PlayerNotLoggedIn);

		// add
		const GoodInfo* gi;
		if (!(gi = GoodList::find_by_id(iGoodId)))
			return cpp::fail(Error::InvalidGood);

		bool bMultiCount;
		memcpy(&bMultiCount, (char*)gi + 0x70, 1);

		uint iBase = 0;
		pub::Player::GetBase(client, iBase);
		uint iLocation = 0;
		pub::Player::GetLocation(client, iLocation);

		// trick cheat detection
		if (iBase)
		{
			if (iLocation)
				Server.LocationExit(iLocation, client);
			Server.BaseExit(iBase, client);
			if (!Client::IsValidClientID(client)) // got cheat kicked
				return cpp::fail(Error::PlayerNotLoggedIn);
		}

		if (bMultiCount)
		{
			// it's a good that can have multiple units(commodities missile ammo, etc)
			int ret;

			// we need to do this, else server or client may crash
			for (const auto cargo = EnumCargo(player, ret); auto& item : cargo.value())
			{
				if ((item.archId == iGoodId) && (item.mission != bMission))
				{
					RemoveCargo(player, static_cast<ushort>(item.id), item.count);
					iCount += item.count;
				}
			}

			pub::Player::AddCargo(client, iGoodId, iCount, 1, bMission);
		}
		else
		{
			for (int i = 0; (i < iCount); i++)
				pub::Player::AddCargo(client, iGoodId, 1, 1, bMission);
		}

		if (iBase)
		{
			// player docked on base
			///////////////////////////////////////////////////
			// fix, else we get anti-cheat msg when undocking
			// this DOES NOT disable anti-cheat-detection, we're
			// just making some adjustments so that we dont get kicked

			Server.BaseEnter(iBase, client);
			if (iLocation)
				Server.LocationEnter(iLocation, client);
		}

		return {};
	}

	cpp::result<void, Error> AddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& Good, int iCount, bool bMission)
	{
		uint iGoodId = ToInt(Good.c_str());
		if (!iGoodId)
			pub::GetGoodID(iGoodId, wstos(Good).c_str());
		if (!iGoodId)
			return cpp::fail(Error::InvalidGood);

		return AddCargo(player, iGoodId, iCount, bMission);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> Rename(const std::variant<uint, std::wstring>& player, const std::wstring& NewCharname, bool bOnlyDelete)
	{
		ClientId client = Client::ExtractClientID(player);

		if ((client == UINT_MAX) && player.index() && !Client::GetAccountByClientID(client))
			return cpp::fail(Error::CharacterDoesNotExist);

		if (!bOnlyDelete && Client::GetAccountByCharName(NewCharname))
			return cpp::fail(Error::AlreadyExists);

		if (!bOnlyDelete && (NewCharname.length() > 23))
			return cpp::fail(Error::CharacterNameTooLong);

		if (!bOnlyDelete && !NewCharname.length())
			return cpp::fail(Error::CharacterNameTooShort);

		INI_Reader ini;
		if (!bOnlyDelete && !(ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false)))
			return cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid);

		CAccount* acc;
		std::wstring oldCharName;
		if (client != UINT_MAX)
		{
			acc = Players.FindAccountFromClientID(client);
			oldCharName = (wchar_t*)Players.GetActiveCharacterName(client);
		}
		else
		{
			oldCharName = std::get<std::wstring>(player);
			acc = Client::GetAccountByCharName(std::get<std::wstring>(player)).value();
		}

		const std::wstring AccountDirname = Client::GetAccountDirName(acc);
		const auto newFileName = Client::GetCharFileName(NewCharname);
		if (newFileName.has_error())
		{
			return cpp::fail(newFileName.error());
		}

		const auto oldFileName = Client::GetCharFileName(oldCharName);
		if (oldFileName.has_error())
		{
			return cpp::fail(oldFileName.error());
		}

		std::string NewCharfilePath = CoreGlobals::c()->accPath + wstos(AccountDirname) + "\\" + wstos(newFileName.value()) + ".fl";
		std::string OldCharfilePath = CoreGlobals::c()->accPath + wstos(AccountDirname) + "\\" + wstos(oldFileName.value()) + ".fl";

		if (bOnlyDelete)
		{
			// delete character
			st6::wstring str((ushort*)oldCharName.c_str());
			Client::LockAccountAccess(acc, true); // also kicks player on this account
			Players.DeleteCharacterFromName(str);
			Client::UnlockAccountAccess(acc);
			return {};
		}

		Client::LockAccountAccess(acc, true); // kick player if online
		Client::UnlockAccountAccess(acc);

		// Copy existing char file into tmp
		std::string TmpPath = OldCharfilePath + ".tmp";
		DeleteFile(TmpPath.c_str());
		CopyFile(OldCharfilePath.c_str(), TmpPath.c_str(), FALSE);

		// Delete existing char otherwise a rename of the char in slot 5 fails.
		st6::wstring str((ushort*)oldCharName.c_str());
		Players.DeleteCharacterFromName(str);

		// Emulate char create
		SLoginInfo logindata;
		wcsncpy_s(logindata.account, Client::GetAccountID(acc).value().c_str(), 36);
		Players.login(logindata, MaxClientId + 1);

		SCreateCharacterInfo newcharinfo;
		wcsncpy_s(newcharinfo.charname, NewCharname.c_str(), 23);
		newcharinfo.charname[23] = 0;

		newcharinfo.nickName = 0;
		newcharinfo.base = 0;
		newcharinfo.package = 0;
		newcharinfo.pilot = 0;

		while (ini.read_header())
		{
			if (ini.is_header("Faction"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
						newcharinfo.nickName = CreateID(ini.get_value_string());
					else if (ini.is_value("base"))
						newcharinfo.base = CreateID(ini.get_value_string());
					else if (ini.is_value("Package"))
						newcharinfo.package = CreateID(ini.get_value_string());
					else if (ini.is_value("Pilot"))
						newcharinfo.pilot = CreateID(ini.get_value_string());
				}
				break;
			}
		}
		ini.close();

		if (newcharinfo.nickName == 0)
			newcharinfo.nickName = CreateID("new_player");
		if (newcharinfo.base == 0)
			newcharinfo.base = CreateID("Li01_01_Base");
		if (newcharinfo.package == 0)
			newcharinfo.package = CreateID("ge_fighter");
		if (newcharinfo.pilot == 0)
			newcharinfo.pilot = CreateID("trent");

		// Fill struct with valid data (though it isnt used it is needed)
		newcharinfo.dunno[4] = 65536;
		newcharinfo.dunno[5] = 65538;
		newcharinfo.dunno[6] = 0;
		newcharinfo.dunno[7] = 1058642330;
		newcharinfo.dunno[8] = 3206125978;
		newcharinfo.dunno[9] = 65537;
		newcharinfo.dunno[10] = 0;
		newcharinfo.dunno[11] = 3206125978;
		newcharinfo.dunno[12] = 65539;
		newcharinfo.dunno[13] = 65540;
		newcharinfo.dunno[14] = 65536;
		newcharinfo.dunno[15] = 65538;
		Server.CreateNewCharacter(newcharinfo, MaxClientId + 1);
		SaveChar(NewCharname);
		Players.logout(MaxClientId + 1);

		// Decode the backup of the old char and overwrite the new char file
		if (!FlcDecodeFile(TmpPath.c_str(), NewCharfilePath.c_str()))
		{
			// file wasn't encoded, thus
			// simply rename it
			DeleteFile(NewCharfilePath.c_str()); // just to get sure...
			CopyFile(TmpPath.c_str(), NewCharfilePath.c_str(), FALSE);
		}
		DeleteFile(TmpPath.c_str());

		// Update the char name in the new char file.
		// Add a space to the value so the ini file line looks like "<key> =
		// <value>" otherwise Ioncross Server Operator can't decode the file
		// correctly
		std::string value = " ";
		for (uint i = 0; (i < NewCharname.length()); i++)
		{
			char cHiByte = NewCharname[i] >> 8;
			char cLoByte = NewCharname[i] & 0xFF;
			char Buf[8];
			sprintf_s(Buf, "%02X%02X", static_cast<uint>(cHiByte) & 0xFF, static_cast<uint>(cLoByte) & 0xFF);
			value += Buf;
		}
		IniWrite(NewCharfilePath, "Player", "Name", value);

		// Re-encode the char file if needed.
		if (!FLHookConfig::i()->general.disableCharfileEncryption && !FlcEncodeFile(NewCharfilePath.c_str(), NewCharfilePath.c_str()))
			return cpp::fail(Error::CouldNotEncodeCharFile);

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> MsgAndKick(ClientId client, const std::wstring& Reason, uint iIntervall)
	{
		if (!ClientInfo[client].tmKickTime)
		{
			const std::wstring Msg = ReplaceStr(FLHookConfig::i()->messages.msgStyle.kickMsg, L"%reason", XMLText(Reason));
			Message::FMsg(client, Msg);
			ClientInfo[client].tmKickTime = Time::GetUnixMiliseconds() + iIntervall;
		}

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> Kill(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);

		// check if logged in
		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
			return cpp::fail(Error::PlayerNotInSpace);

		pub::SpaceObj::SetRelativeHealth(ship, 0.0f);

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<bool, Error> GetReservedSlot(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);

		const CAccount* acc;
		if (client != UINT_MAX)
			acc = Players.FindAccountFromClientID(client);
		else
			acc = player.index() ? Client::GetAccountByCharName(std::get<std::wstring>(player)).value() : nullptr;

		if (!acc)
			return cpp::fail(Error::CharacterDoesNotExist);

		const auto dir = Client::GetAccountDirName(acc);
		const std::string userFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";

		return IniGetB(userFile, "Settings", "ReservedSlot", false);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> SetReservedSlot(const std::variant<uint, std::wstring>& player, bool bReservedSlot)
	{
		auto acc = Client::ExtractAccount(player);
		if (acc.has_error())
			return cpp::fail(Error::CharacterDoesNotExist);

		const auto dir = Client::GetAccountDirName(acc.value());
		const std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";

		if (bReservedSlot)
			IniWrite(UserFile, "Settings", "ReservedSlot", "yes");
		else
			IniWrite(UserFile, "Settings", "ReservedSlot", "no");

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> ResetRep(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);

		// check if logged in
		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		INI_Reader ini;
		if (!ini.open("mpnewcharacter.fl", false))
			return cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid);

		ini.read_header();
		if (!ini.is_header("Player"))
		{
			ini.close();
			return cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid);
		}

		int iPlayerRep;
		pub::Player::GetRep(client, iPlayerRep);
		while (ini.read_value())
		{
			if (ini.is_value("house"))
			{
				const float fRep = ini.get_value_float(0);
				const char* RepGroupName = ini.get_value_string(1);

				uint iRepGroupId;
				pub::Reputation::GetReputationGroup(iRepGroupId, RepGroupName);
				pub::Reputation::SetReputation(iPlayerRep, iRepGroupId, fRep);
			}
		}

		ini.close();
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> SetRep(const std::variant<uint, std::wstring>& player, const std::wstring& RepGroup, float fValue)
	{
		ClientId client = Client::ExtractClientID(player);
		// check if logged in
		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		uint iRepGroupId;
		pub::Reputation::GetReputationGroup(iRepGroupId, wstos(RepGroup).c_str());
		if (iRepGroupId == -1)
			return cpp::fail(Error::InvalidRepGroup);

		int iPlayerRep;
		pub::Player::GetRep(client, iPlayerRep);
		pub::Reputation::SetReputation(iPlayerRep, iRepGroupId, fValue);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<float, Error> GetRep(const std::variant<uint, std::wstring>& player, const std::variant<uint, std::wstring>& repGroup)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		uint repGroupId;
		if (repGroup.index() == 1)
		{
			pub::Reputation::GetReputationGroup(repGroupId, wstos(std::get<std::wstring>(repGroup)).c_str());
			if (repGroupId == UINT_MAX)
				return cpp::fail(Error::InvalidRepGroup);
		}
		else
		{
			repGroupId = std::get<uint>(repGroup);
		}

		int playerRep;
		pub::Player::GetRep(client, playerRep);
		float playerFactionRep;
		pub::Reputation::GetGroupFeelingsTowards(playerRep, repGroupId, playerFactionRep);
		return playerFactionRep;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<std::vector<GroupMember>, Error> GetGroupMembers(const std::variant<uint, std::wstring>& player)
	{
		std::vector<GroupMember> members;
		ClientId client = Client::ExtractClientID(player);

		// check if logged in
		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		// hey, at least it works! beware of the VC optimiser.
		st6::vector<uint> vMembers;
		pub::Player::GetGroupMembers(client, vMembers);

		for (const uint i : vMembers)
		{
			GroupMember gm = {i, (wchar_t*)Players.GetActiveCharacterName(i)};
			members.push_back(gm);
		}

		return members;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<std::list<std::wstring>, Error> ReadCharFile(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);

		std::wstring dir;
		cpp::result<CAccount*, Error> acc;
		if (client != UINT_MAX)
		{
			acc = Players.FindAccountFromClientID(client);
			if (const wchar_t* wCharname = (wchar_t*)Players.GetActiveCharacterName(client); !wCharname)
				return cpp::fail(Error::CharacterNotSelected);

			dir = Client::GetAccountDirName(acc.value());
		}
		else
		{
			acc = Client::ExtractAccount(player);
			if (!acc)
				return cpp::fail(Error::CharacterDoesNotExist);
		}

		auto file = Client::GetCharFileName(player);
		if (file.has_error())
		{
			return cpp::fail(file.error());
		}

		std::string charFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl";
		std::string fileToRead;
		bool bDeleteAfter;
		if (Client::IsEncoded(charFile))
		{
			std::string charFileNew = charFile + ".ini";
			if (!FlcDecodeFile(charFile.c_str(), charFileNew.c_str()))
				return cpp::fail(Error::CouldNotDecodeCharFile);
			fileToRead = charFileNew;
			bDeleteAfter = true;
		}
		else
		{
			fileToRead = charFile;
			bDeleteAfter = false;
		}

		std::ifstream ifs;
		ifs.open(fileToRead.c_str(), std::ios_base::in);
		if (!ifs.is_open())
			return cpp::fail(Error::unknownError);

		std::list<std::wstring> output;
		std::string Line;
		while (getline(ifs, Line))
			output.emplace_back(stows(Line));
		ifs.close();
		if (bDeleteAfter)
			DeleteFile(fileToRead.c_str());

		return output;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> WriteCharFile(const std::variant<uint, std::wstring>& player, std::wstring Data)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX)
		{
			return cpp::fail(Error::InvalidClientId);
		}

		const auto acc = Players.FindAccountFromClientID(client);
		if (const wchar_t* charname = (wchar_t*)Players.GetActiveCharacterName(client); !charname)
			return cpp::fail(Error::CharacterNotSelected);

		auto dir = Client::GetAccountDirName(acc);

		const auto file = Client::GetCharFileName(player);
		if (file.has_error())
		{
			return cpp::fail(file.error());
		}

		std::string charFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl";
		std::string fileToWrite;
		bool bEncode;
		if (Client::IsEncoded(charFile))
		{
			fileToWrite = charFile + ".ini";
			bEncode = true;
		}
		else
		{
			fileToWrite = charFile;
			bEncode = false;
		}

		std::ofstream ofs;
		ofs.open(fileToWrite.c_str(), std::ios_base::out);
		if (!ofs.is_open())
			return cpp::fail(Error::unknownError);

		size_t iPos;
		while ((iPos = Data.find(L"\\n")) != -1)
		{
			std::wstring Line = Data.substr(0, iPos);
			ofs << wstos(Line) << std::endl;
			Data.erase(0, iPos + 2);
		}

		if (Data.length())
			ofs << wstos(Data);

		ofs.close();
		if (bEncode)
		{
			FlcEncodeFile(fileToWrite.c_str(), charFile.c_str());
			DeleteFile(fileToWrite.c_str());
		}
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> PlayerRecalculateCRC(ClientId client)
	{
		try
		{
			const PlayerData* pd = &Players[client];
			char* ACCalcCRC = (char*)server + 0x6FAF0;
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
			return cpp::fail(Error::InvalidClientId);
		}

		return {};
	}

	/** Move the client to the specified location */
	void RelocateClient(ClientId client, Vector vDestination, const Matrix& mOrientation)
	{
		const Quaternion rotation = Math::MatrixToQuaternion(mOrientation);

		FLPACKET_LAUNCH launchPacket;
		launchPacket.ship = ClientInfo[client].ship;
		launchPacket.iBase = 0;
		launchPacket.iState = 0xFFFFFFFF;
		launchPacket.fRotate[0] = rotation.w;
		launchPacket.fRotate[1] = rotation.x;
		launchPacket.fRotate[2] = rotation.y;
		launchPacket.fRotate[3] = rotation.z;
		launchPacket.fPos[0] = vDestination.x;
		launchPacket.fPos[1] = vDestination.y;
		launchPacket.fPos[2] = vDestination.z;

		HookClient->Send_FLPACKET_SERVER_LAUNCH(client, launchPacket);

		uint system;
		pub::Player::GetSystem(client, system);
		pub::SpaceObj::Relocate(ClientInfo[client].ship, system, vDestination, mOrientation);
	}

	/** Dock the client immediately */
	cpp::result<void, Error> InstantDock(ClientId client, uint iDockObj)
	{
		// check if logged in
		if (client == UINT_MAX)
			return cpp::fail(Error::PlayerNotLoggedIn);

		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
			return cpp::fail(Error::PlayerNotInSpace);

		uint system;
		uint iSystem2;
		pub::SpaceObj::GetSystem(ship, system);
		pub::SpaceObj::GetSystem(iDockObj, iSystem2);
		if (system != iSystem2)
		{
			return cpp::fail(Error::PlayerNotInSpace);
		}

		try
		{
			pub::SpaceObj::InstantDock(ship, iDockObj, 1);
		}
		catch (...)
		{
			return cpp::fail(Error::PlayerNotInSpace);
		}

		return {};
	}

	cpp::result<int, Error> GetRank(const std::variant<uint, std::wstring>& player)
	{
		auto rank = Ini::GetFromPlayerFile(player, L"rank");
		if (rank.has_error())
		{
			return cpp::fail(rank.error());
		}

		return rank.value().length() ? ToInt(rank.value()) : 0;
	}

	/// Get online time.
	cpp::result<int, Error> GetOnlineTime(const std::variant<uint, std::wstring>& player)
	{
		const auto client = Client::ExtractClientID(player);
		const auto acc = Client::GetAccountByClientID(client);
		const auto dir = Client::GetAccountDirName(acc);

		const auto file = Client::GetCharFileName(player);
		if (file.has_error())
		{
			return cpp::fail(file.error());
		}

		const std::string CharFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + wstos(file.value()) + ".fl";
		if (Client::IsEncoded(CharFile))
		{
			const std::string CharFileNew = CharFile + ".ini";
			if (!FlcDecodeFile(CharFile.c_str(), CharFileNew.c_str()))
				return cpp::fail(Error::CouldNotDecodeCharFile);

			int secs = IniGetI(CharFileNew, "mPlayer", "total_time_played", 0);
			DeleteFile(CharFileNew.c_str());
			return secs;
		}
		return IniGetI(CharFile, "mPlayer", "total_time_played", 0);
	}

	cpp::result<const uint, Error> GetSystemByNickname(std::variant<std::string, std::wstring> nickname)
	{
		uint system = 0;
		const std::string nick = nickname.index() == 0 ? std::get<std::string>(nickname) : wstos(std::get<std::wstring>(nickname));
		pub::GetSystemID(system, nick.c_str());
		if (!system)
			return cpp::fail(Error::InvalidSystem);

		return system;
	}

	CShip* CShipFromShipDestroyed(const DWORD** ecx)
	{
		return reinterpret_cast<CShip*>((*ecx)[4]); // NOLINT(performance-no-int-to-ptr)
	}

	/// Return true if this player is within the specified distance of any other player.
	bool IsInRange(ClientId client, float fDistance)
	{
		const auto Members = GetGroupMembers((const wchar_t*)Players.GetActiveCharacterName(client));
		if (Members.has_error())
		{
			return false;
		}

		uint ship;
		pub::Player::GetShip(client, ship);

		Vector pos;
		Matrix rot;
		pub::SpaceObj::GetLocation(ship, pos, rot);

		uint system;
		pub::Player::GetSystem(client, system);

		// For all players in system...
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			// Get the this player's current system and location in the system.
			ClientId client2 = playerDb->onlineId;
			uint iSystem2 = 0;
			pub::Player::GetSystem(client2, iSystem2);
			if (system != iSystem2)
				continue;

			uint ship2;
			pub::Player::GetShip(client2, ship2);

			Vector pos2;
			Matrix rot2;
			pub::SpaceObj::GetLocation(ship2, pos2, rot2);

			// Ignore players who are in your group.
			bool bGrouped = false;
			for (auto& gm : Members.value())
			{
				if (gm.client == client2)
				{
					bGrouped = true;
					break;
				}
			}
			if (bGrouped)
				continue;

			// Is player within the specified range of the sending char.
			if (Math::Distance3D(pos, pos2) < fDistance)
				return true;
		}
		return false;
	}

	/**
	Delete a character.
	*/
	void DeleteCharacter(CAccount* acc, const std::wstring& character)
	{
		Client::LockAccountAccess(acc, true);
		st6::wstring str((ushort*)character.c_str());
		Players.DeleteCharacterFromName(str);
		Client::UnlockAccountAccess(acc);
	}

	/**
	Create a new character in the specified account by emulating a
	create character.
	*/
	cpp::result<void, Error> NewCharacter(CAccount* acc, std::wstring& character)
	{
		Client::LockAccountAccess(acc, true);
		Client::UnlockAccountAccess(acc);

		INI_Reader ini;
		if (!ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
			return cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid);

		// Emulate char create by logging in.
		SLoginInfo logindata;
		wcsncpy_s(logindata.account, Client::GetAccountID(acc).value().c_str(), 36);
		Players.login(logindata, Players.GetMaxPlayerCount() + 1);

		SCreateCharacterInfo newcharinfo;
		wcsncpy_s(newcharinfo.charname, character.c_str(), 23);
		newcharinfo.charname[23] = 0;

		newcharinfo.nickName = 0;
		newcharinfo.base = 0;
		newcharinfo.package = 0;
		newcharinfo.pilot = 0;

		while (ini.read_header())
		{
			if (ini.is_header("Faction"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("nickname"))
						newcharinfo.nickName = CreateID(ini.get_value_string());
					else if (ini.is_value("base"))
						newcharinfo.base = CreateID(ini.get_value_string());
					else if (ini.is_value("package"))
						newcharinfo.package = CreateID(ini.get_value_string());
					else if (ini.is_value("pilot"))
						newcharinfo.pilot = CreateID(ini.get_value_string());
				}
				break;
			}
		}
		ini.close();

		if (newcharinfo.nickName == 0)
			newcharinfo.nickName = CreateID("new_player");
		if (newcharinfo.base == 0)
			newcharinfo.base = CreateID("Li01_01_Base");
		if (newcharinfo.package == 0)
			newcharinfo.package = CreateID("ge_fighter");
		if (newcharinfo.pilot == 0)
			newcharinfo.pilot = CreateID("trent");

		// Fill struct with valid data (though it isnt used it is needed)
		newcharinfo.dunno[4] = 65536;
		newcharinfo.dunno[5] = 65538;
		newcharinfo.dunno[6] = 0;
		newcharinfo.dunno[7] = 1058642330;
		newcharinfo.dunno[8] = 3206125978;
		newcharinfo.dunno[9] = 65537;
		newcharinfo.dunno[10] = 0;
		newcharinfo.dunno[11] = 3206125978;
		newcharinfo.dunno[12] = 65539;
		newcharinfo.dunno[13] = 65540;
		newcharinfo.dunno[14] = 65536;
		newcharinfo.dunno[15] = 65538;
		Server.CreateNewCharacter(newcharinfo, Players.GetMaxPlayerCount() + 1);
		SaveChar(character);
		Players.logout(Players.GetMaxPlayerCount() + 1);

		return {};
	}

	// Anti cheat checking code by mc_horst. Will always return okay if the user is in space.
	cpp::result<void, Error> AntiCheat(ClientId client)
	{
		const auto AntiCheat1 = (_FLAntiCheat)((char*)server + AddrAntiCheat1);
		const auto AntiCheat2 = (_FLAntiCheat)((char*)server + AddrAntiCheat2);
		const auto AntiCheat3 = (_FLAntiCheat)((char*)server + AddrAntiCheat3);
		const auto AntiCheat4 = (_FLAntiCheat)((char*)server + AddrAntiCheat4);

		// Hack to make the linter happy
		(void)AntiCheat1;
		(void)AntiCheat2;
		(void)AntiCheat3;
		(void)AntiCheat4;

		// Check if ship in space
		if (const auto ship = GetShip(client); ship.has_value())
		{
			return {};
		}

		char* ObjPtr;
		memcpy(&ObjPtr, &Players, 4);
		ObjPtr += 0x418 * (client - 1);

		char cRes = 0;

		__asm {
			mov ecx, [ObjPtr]
			call [AntiCheat1]
			mov [cRes], al
			}

		if (cRes != 0)
		{
			// kick
			Kick(client);
			return cpp::fail(Error::unknownError);
		}

		__asm {
			mov ecx, [ObjPtr]
			call [AntiCheat2]
			mov [cRes], al
			}

		if (cRes != 0)
		{
			Kick(client);
			return cpp::fail(Error::unknownError);
		}

		ulong lRet = 0;
		ulong lCompare = 0;
		__asm {
			mov ecx, [ObjPtr]
			mov eax, [ecx+0x320]
			mov [lCompare], eax
			call [AntiCheat3]
			mov [lRet], eax
			}

		if (lRet > lCompare)
		{
			Kick(client);
			return cpp::fail(Error::unknownError);
		}

		__asm {
			mov ecx, [ObjPtr]
			call [AntiCheat4]
			mov [cRes], al
			}

		if (cRes != 0)
		{
			Kick(client);
			return cpp::fail(Error::unknownError);
		}
	}

	cpp::result<void, Error> SetEquip(const std::variant<uint, std::wstring>& player, const st6::list<EquipDesc>& equip)
	{
		ClientId client = Client::ExtractClientID(player);

		if ((client == UINT_MAX) || Client::IsInCharSelectMenu(client))
			return cpp::fail(Error::CharacterNotSelected);

		// Update FLHook's lists to make anticheat pleased.
		if (&equip != &Players[client].lShadowEquipDescList.equip)
			Players[client].lShadowEquipDescList.equip = equip;

		if (&equip != &Players[client].equipDescList.equip)
			Players[client].equipDescList.equip = equip;

		// Calculate packet size. First two bytes reserved for items count.
		uint itemBufSize = 2;
		for (const auto& item : equip)
		{
			itemBufSize += sizeof(SetEquipmentItem) + strlen(item.hardPoint.value) + 1;
		}

		FlPacket* packet = FlPacket::Create(itemBufSize, FlPacket::FLPACKET_SERVER_SETEQUIPMENT);
		const auto pSetEquipment = reinterpret_cast<FlPacketSetEquipment*>(packet->content);

		// Add items to packet as array of variable size.
		uint index = 0;
		for (const auto& item : equip)
		{
			SetEquipmentItem setEquipItem;
			setEquipItem.count = item.count;
			setEquipItem.health = item.health;
			setEquipItem.archId = item.archId;
			setEquipItem.id = item.id;
			setEquipItem.mounted = item.mounted;
			setEquipItem.mission = item.mission;

			if (const uint len = strlen(item.hardPoint.value); len && item.hardPoint.value != "BAY")
			{
				setEquipItem.hardPointLen = static_cast<ushort>(len + 1); // add 1 for the null - char* is a null-terminated string in C++
			}
			else
			{
				setEquipItem.hardPointLen = 0;
			}
			pSetEquipment->count++;

			const byte* buf = (byte*)&setEquipItem;
			for (int i = 0; i < sizeof(SetEquipmentItem); i++)
				pSetEquipment->items[index++] = buf[i];

			const byte* HardPoint = (byte*)item.hardPoint.value;
			for (int i = 0; i < setEquipItem.hardPointLen; i++)
				pSetEquipment->items[index++] = HardPoint[i];
		}

		if (packet->SendTo(client))
		{
			return {};
		}
		return cpp::fail(Error::unknownError);
	}

	cpp::result<void, Error> AddEquip(const std::variant<uint, std::wstring>& player, uint iGoodId, const std::string& Hardpoint)
	{
		ClientId client = Client::ExtractClientID(player);

		if ((client == UINT_MAX) || Client::IsInCharSelectMenu(client))
			return cpp::fail(Error::CharacterNotSelected);

		if (!Players[client].enteredBase)
		{
			Players[client].enteredBase = Players[client].baseId;
			Server.ReqAddItem(iGoodId, Hardpoint.c_str(), 1, 1.0f, true, client);
			Players[client].enteredBase = 0;
		}
		else
		{
			Server.ReqAddItem(iGoodId, Hardpoint.c_str(), 1, 1.0f, true, client);
		}

		// Add to check-list which is being compared to the users equip-list when
		// saving char to fix "Ship or Equipment not sold on base" kick
		EquipDesc ed;
		ed.id = Players[client].lastEquipId;
		ed.count = 1;
		ed.archId = iGoodId;
		Players[client].lShadowEquipDescList.add_equipment_item(ed, false);

		return {};
	}

	cpp::result<void, Error> AddEquip(const std::variant<uint, std::wstring>& player, uint iGoodId, const std::string& Hardpoint, bool bMounted)
	{
		using _AddCargoDocked = bool(__stdcall *)(uint iGoodId, CacheString* & hardpoint, int iNumItems, float fHealth, int bMounted, int bMission, uint iOne);
		static _AddCargoDocked AddCargoDocked = nullptr;
		if (!AddCargoDocked)
			AddCargoDocked = (_AddCargoDocked)((char*)server + 0x6EFC0);

		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
			return cpp::fail(Error::PlayerNotLoggedIn);

		uint iBase = 0;
		pub::Player::GetBase(client, iBase);
		uint iLocation = 0;
		pub::Player::GetLocation(client, iLocation);

		if (iLocation)
			Server.LocationExit(iLocation, client);
		if (iBase)
			Server.BaseExit(iBase, client);
		if (!Client::IsValidClientID(client))
			return cpp::fail(Error::PlayerNotLoggedIn);

		PlayerData* pd = &Players[client];
		const char* p = Hardpoint.c_str();
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
			push iGoodId
			mov ecx, pd
			call AddCargoDocked
			}

		if (iBase)
			Server.BaseEnter(iBase, client);
		if (iLocation)
			Server.LocationEnter(iLocation, client);

		return {};
	}

	void DelayedKick(ClientId client, uint secs)
	{
		const mstime kick_time = Time::GetUnixMiliseconds() + (secs * 1000);
		if (!ClientInfo[client].tmKickTime || ClientInfo[client].tmKickTime > kick_time)
			ClientInfo[client].tmKickTime = kick_time;
	}

	std::string GetPlayerSystemS(ClientId client)
	{
		uint iSystemId;
		pub::Player::GetSystem(client, iSystemId);
		char Systemname[1024] = "";
		pub::GetSystemNickname(Systemname, sizeof(Systemname), iSystemId);
		return Systemname;
	}

	cpp::result<const uint, Error> GetShipValue(const std::variant<uint, std::wstring>& player)
	{
		if (ClientId client = Client::ExtractClientID(player); client != UINT_MAX && !Client::IsInCharSelectMenu(client))
		{
			SaveChar(player);
			if (!Client::IsValidClientID(client))
			{
				return cpp::fail(Error::unknownError);
			}
		}

		float fValue = 0.0f;

		uint iBaseId = 0;

		const auto CharFile = ReadCharFile(player);
		if (CharFile.has_error())
		{
			return cpp::fail(CharFile.error());
		}

		for (const auto& line : CharFile.value())
		{
			std::wstring Key = Trim(line.substr(0, line.find(L"=")));
			if (Key == L"base" || Key == L"last_base")
			{
				const int iFindEqual = line.find(L"=");
				if (iFindEqual == -1)
				{
					continue;
				}

				if ((iFindEqual + 1) >= static_cast<int>(line.size()))
				{
					continue;
				}

				iBaseId = CreateID(wstos(Trim(line.substr(iFindEqual + 1))).c_str());
				break;
			}
		}

		for (const auto& line : CharFile.value())
		{
			std::wstring Key = Trim(line.substr(0, line.find(L"=")));
			if (Key == L"cargo" || Key == L"equip")
			{
				const int iFindEqual = line.find(L"=");
				if (iFindEqual == -1)
				{
					continue;
				}
				const int iFindComma = line.find(L",", iFindEqual);
				if (iFindComma == -1)
				{
					continue;
				}
				const uint iGoodId = ToUInt(Trim(line.substr(iFindEqual + 1, iFindComma)));
				const uint iGoodCount = ToUInt(Trim(line.substr(iFindComma + 1, line.find(L",", iFindComma + 1))));

				float fItemValue;
				if (pub::Market::GetPrice(iBaseId, Arch2Good(iGoodId), fItemValue) == 0)
				{
					if (arch_is_combinable(iGoodId))
					{
						fValue += fItemValue * static_cast<float>(iGoodCount);
					}
					else
					{
						const float* fResaleFactor = (float*)((char*)server + 0x8AE7C);
						fValue += fItemValue * (*fResaleFactor);
					}
				}
			}
			else if (Key == L"money")
			{
				const int iFindEqual = line.find(L"=");
				if (iFindEqual == -1)
				{
					continue;
				}
				const uint fItemValue = ToUInt(Trim(line.substr(iFindEqual + 1)));
				fValue += fItemValue;
			}
			else if (Key == L"ship_archetype")
			{
				const uint shipArchId = ToUInt(Trim(line.substr(line.find(L"=") + 1, line.length())));
				const GoodInfo* gi = GoodList_get()->find_by_ship_arch(shipArchId);
				if (gi)
				{
					gi = GoodList::find_by_id(gi->archId);
					if (gi)
					{
						const auto fResaleFactor = (float*)((char*)server + 0x8AE78);
						const float fItemValue = gi->price * (*fResaleFactor);
						fValue += fItemValue;
					}
				}
			}
		}

		return static_cast<uint>(fValue);
	}

	void SaveChar(ClientId client)
	{
		const BYTE patch[] = {0x90, 0x90};
		WriteProcMem((char*)server + 0x7EFA8, patch, sizeof(patch));
		pub::Save(client, 1);
	}

	cpp::result<const ShipId, Error> GetTarget(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		const auto ship = GetShip(client);
		if (ship.has_error())
			return cpp::fail(ship.error());

		uint target;
		pub::SpaceObj::GetTarget(ship.value(), target);
		if (!target)
			return cpp::fail(Error::NoTargetSelected);

		return target;
	}

	cpp::result<ClientId, Error> GetTargetClientID(const std::variant<uint, std::wstring>& player)
	{
		const auto target = GetTarget(player);
		if (target.has_error())
			return cpp::fail(target.error());

		auto targetClientId = Client::GetClientIdByShip(target.value());
		if (targetClientId.has_error())
			return cpp::fail(Error::TargetIsNotPlayer);

		return targetClientId;
	}

	cpp::result<const BaseId, Error> GetCurrentBase(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}

		uint base;
		pub::Player::GetBase(client, base);
		if (base)
		{
			return base;
		}

		return cpp::fail(Error::PlayerNotDocked);
	}

	cpp::result<const SystemId, Error> GetSystem(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}

		uint system;
		pub::Player::GetSystem(client, system);
		if (!system)
			return cpp::fail(Error::InvalidSystem);

		return system;
	}

	// returns ship instance ID
	cpp::result<const ShipId, Error> GetShip(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}

		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
			return cpp::fail(Error::PlayerNotInSpace);

		return ship;
	}

	// returns Ship type
	cpp::result<const uint, Error> GetShipID(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}

		uint shipId;
		pub::Player::GetShipID(client, shipId);
		if (!shipId)
			return cpp::fail(Error::PlayerNotInSpace);

		return shipId;
	}

	cpp::result<void, Error> MarkObj(const std::variant<uint, std::wstring>& player, uint objId, int markStatus)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}
		pub::Player::MarkObj(client, objId, markStatus);

		return {};
	}

	cpp::result<int, Error> GetPvpKills(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}
		int kills;
		pub::Player::GetNumKills(client, kills);
		return kills;
	}

	cpp::result<void, Error> SetPvpKills(const std::variant<uint, std::wstring>& player, int killAmount)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}

		pub::Player::SetNumKills(client, killAmount);

		return {};
	}

	cpp::result<int, Error> IncrementPvpKills(const std::variant<uint, std::wstring>& player)
	{
		ClientId client = Client::ExtractClientID(player);
		if (client == UINT_MAX)
		{
			return cpp::fail(Error::PlayerNotLoggedIn);
		}

		int kills;
		pub::Player::GetNumKills(client, kills);
		kills++;
		pub::Player::SetNumKills(client, kills);

		return {};
	}
} // namespace Hk::Player
