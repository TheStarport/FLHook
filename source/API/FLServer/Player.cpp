#include "PCH.hpp"

#include "Global.hpp"

namespace Hk::Player
{
    using _FLAntiCheat = void(__stdcall*)();
    using _FLPossibleCheatingDetected = void(__stdcall*)(int reason);
    constexpr auto AddrAntiCheat1 = 0x70120;
    constexpr auto AddrAntiCheat2 = 0x6FD20;
    constexpr auto AddrAntiCheat3 = 0x6FAF0;
    constexpr auto AddrAntiCheat4 = 0x6FAA0;
    constexpr auto PossibleCheatingDetectedAddr = 0x6F570;

    Action<void> AddToGroup(ClientId client, uint groupId)
    {
        if (!Client::IsValidClientID(client))
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        if (const uint groupId = Players.GetGroupID(client); groupId == groupId)
        {
            return { cpp::fail(Error::InvalidGroupId) };
        }

        CPlayerGroup* group = CPlayerGroup::FromGroupID(groupId);
        if (!group)
        {
            return { cpp::fail(Error::InvalidGroupId) };
        }

        group->AddMember(client);
        return { {} };
    }

    Action<uint> GetGroupID(ClientId client)
    {
        if (!Client::IsValidClientID(client))
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        return { Players.GetGroupID(client) };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<uint> GetCash(const std::variant<uint, std::wstring_view>& player)
    {
        if (ClientId client = Client::ExtractClientID(player); client != UINT_MAX)
        {
            if (Client::IsInCharSelectMenu(client))
            {
                return { cpp::fail(Error::CharacterNotSelected) };
            }

            int cash;
            pub::Player::InspectCash(client, cash);
            return { cash };
        }

        if (!player.index())
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        const auto acc = Client::GetAccountByCharName(std::get<std::wstring_view>(player)).Raw();
        if (acc.has_error())
        {
            return { cpp::fail(acc.error()) };
        }

        const auto dir = Client::GetAccountDirName(acc.value());

        auto file = Client::GetCharFileName(player).Raw();
        if (file.has_error())
        {
            return { cpp::fail(file.error()) };
        }

        const std::wstring charFile = std::format(L"{}{}\\{}.fl", CoreGlobals::c()->accPath, dir, file.value());
        if (std::filesystem::exists(charFile))
        {
            return { cpp::fail(Error::CharacterDoesNotExist) };
        }

        if (Client::IsEncoded(charFile))
        {
            const std::wstring charFileNew = std::format(L"{}.ini", charFile);
            if (FlCodec::Decode(charFile).empty())
            {
                return { cpp::fail(Error::CouldNotDecodeCharFile) };
            }

            auto cash = StringUtils::Cast<uint>(IniUtils::GetFromFile(charFileNew, L"Player", L"money").value());
            DeleteFileW(charFileNew.c_str());
            return { cash };
        }

        return { StringUtils::Cast<int>(IniUtils::GetFromFile(charFile, L"Player", L"money").value()) };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> AdjustCash(const std::variant<uint, std::wstring_view>& player, int amount)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client != UINT_MAX)
        {
            if (Client::IsInCharSelectMenu(client))
            {
                return { cpp::fail(Error::CharacterNotSelected) };
            }

            pub::Player::AdjustCash(client, amount);
            return { {} };
        }

        if (!player.index())
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        const auto& character = std::get<std::wstring_view>(player);
        const int money = StringUtils::Cast<int>(IniUtils::i()->GetFromPlayerFile(player, L"money").value());
        IniUtils::i()->WriteToPlayerFile(player, L"money", std::to_wstring(money + amount)).Handle();

        if (client != UINT_MAX)
        {
            // money fix in case player logs in with this account
            bool found = false;
            const std::wstring characterLower = StringUtils::ToLower(character);
            for (auto& money : ClientInfo[client].moneyFix)
            {
                if (money.character == characterLower)
                {
                    money.amount += amount;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                MONEY_FIX mf;
                mf.character = characterLower;
                mf.amount = amount;
                ClientInfo[client].moneyFix.push_back(mf);
            }
        }

        return { {} };
    }

    Action<void> AddCash(const std::variant<uint, std::wstring_view>& player, uint amount) { return AdjustCash(player, static_cast<int>(amount)); }

    Action<void> RemoveCash(const std::variant<uint, std::wstring_view>& player, uint amount) { return AdjustCash(player, -static_cast<int>(amount)); }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> Kick(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        CAccount* acc = Players.FindAccountFromClientID(client);
        acc->ForceLogout();
        return { {} };
    }

    Action<void> KickReason(const std::variant<uint, std::wstring_view>& player, std::wstring_view reason)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        if (reason.length())
        {
            MsgAndKick(client, reason, FLHookConfig::i()->chatConfig.msgStyle.kickMsgPeriod);
        }
        else
        {
            Players.FindAccountFromClientID(client)->ForceLogout();
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> Ban(const std::variant<uint, std::wstring_view>& player, bool ban)
    {
        auto acc = Client::ExtractAccount(player);
        if (acc.has_error())
        {
            return { cpp::fail(Error::CharacterDoesNotExist) };
        }

        auto id = Client::GetAccountID(acc.value()).Raw();
        if (id.has_error())
        {
            return { cpp::fail(id.error()) };
        }

        st6::wstring fr((ushort*)id.value().c_str());
        Players.BanAccount(fr, ban);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> TempBan(const std::variant<uint, std::wstring_view>& player, uint duration)
    {
        uint clientId;
        if (player.index() != 0)
        {
            const auto& charName = std::get<std::wstring_view>(player);
            auto client = Client::GetClientIdFromCharName(charName).Raw();
            if (client.has_error())
            {
                return { cpp::fail(client.error()) };
            }
            clientId = client.value();
        }
        else
        {
            clientId = std::get<uint>(player);
        }

        if (!Client::IsValidClientID(clientId))
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        TempBanManager::i()->AddTempBan(clientId, duration);

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Bases that will cause crashes if jumped to
    const std::array BannedBases = {
        CreateID("br_m_beryllium_miner"),  CreateID("[br_m_hydrocarbon_miner]"),
        CreateID("[br_m_niobium_miner]"),  CreateID("[co_khc_copper_miner]"),
        CreateID("[co_khc_cobalt_miner]"), CreateID("[co_kt_hydrocarbon_miner]"),
        CreateID("[co_shi_h-fuel_miner]"), CreateID("[co_shi_water_miner]"),
        CreateID("[co_ti_water_miner]"),   CreateID("[gd_gm_h-fuel_miner]"),
        CreateID("[gd_im_oxygen_miner]"),  CreateID("[gd_im_copper_miner]"),
        CreateID("[gd_im_silver_miner]"),  CreateID("[gd_im_water_miner]"),
        CreateID("[rh_m_diamond_miner]"),  CreateID("intro3_base"),
        CreateID("intro2_base"),           CreateID("intro1_base"),
        CreateID("st03b_01_base"),         CreateID("st02_01_base"),
        CreateID("st01_02_base"),          CreateID("iw02_03_base"),
        CreateID("rh02_07_base"),          CreateID("li04_06_base"),
        CreateID("li01_15_base"),
    };

    Action<void> Beam(const std::variant<uint, std::wstring_view>& player, const std::variant<uint, std::wstring_view>& baseVar)
    {
        ClientId client = Client::ExtractClientID(player);
        uint baseId;

        // check if logged in
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }
        // check if ship in space
        if (const auto ship = GetShip(client).Raw(); ship.has_error())
        {
            return { cpp::fail(ship.error()) };
        }

        // if basename was passed as string
        if (baseVar.index() == 1)
        {
            const std::string baseName = StringUtils::wstos(std::wstring(std::get<std::wstring_view>(baseVar)));

            // get base id
            if (pub::GetBaseID(baseId, baseName.c_str()) == -4)
            {
                return { cpp::fail(Error::InvalidBaseName) };
            }
        }
        else
        {
            baseId = std::get<uint>(baseVar);
        }

        if (std::ranges::find(BannedBases, baseId) != BannedBases.end())
        {
            return { cpp::fail(Error::InvalidBaseName) };
        }

        uint sysId;
        pub::Player::GetSystem(client, sysId);
        const Universe::IBase* base = Universe::get_base(baseId);

        if (!base)
        {
            return { cpp::fail(Error::InvalidBase) };
        }

        pub::Player::ForceLand(client, baseId); // beam

        // if not in the same system, emulate F1 charload
        if (base->systemId != sysId)
        {
            Server.BaseEnter(baseId, client);
            Server.BaseExit(baseId, client);
            auto fileName = Client::GetCharFileName(client).Raw();
            if (fileName.has_error())
            {
                return { cpp::fail(fileName.error()) };
            }
            const std::wstring newFile = std::format(L"{}.fl", fileName.value());
            CHARACTER_ID charId;
            strcpy_s(charId.charFilename, StringUtils::wstos(newFile.substr(0, 14)).c_str());
            Server.CharacterSelect(charId, client);
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> SaveChar(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        void* jmp = (char*)server + 0x7EFA8;
        const char Nop[2] = { '\x90', '\x90' };
        const char TestAlAl[2] = { '\x74', '\x44' };
        MemUtils::WriteProcMem(jmp, Nop, sizeof Nop); // nop the SinglePlayer() check
        pub::Save(client, 1);
        MemUtils::WriteProcMem(jmp, TestAlAl, sizeof TestAlAl); // restore

        return { {} };
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

    Action<std::list<CargoInfo>> EnumCargo(const std::variant<uint, std::wstring_view>& player, int& remainingHoldSize)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
        {
            return { (cpp::fail(Error::PlayerNotLoggedIn)) };
        }

        std::list<CargoInfo> cargo;

        char* classPtr;
        memcpy(&classPtr, &Players, 4);
        classPtr += 0x418 * (client - 1);

        EquipItem* eqList;
        memcpy(&eqList, classPtr + 0x27C, 4);
        const EquipItem* eq = eqList->next;
        while (eq != eqList)
        {
            CargoInfo ci = { eq->id, static_cast<int>(eq->count), eq->goodId, eq->status, eq->mission, eq->mounted, eq->hardpoint };
            cargo.push_back(ci);

            eq = eq->next;
        }

        float remainingHold;
        pub::Player::GetRemainingHoldSize(client, remainingHold);
        remainingHoldSize = static_cast<int>(remainingHold);
        return { cargo };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> RemoveCargo(const std::variant<uint, std::wstring_view>& player, ushort cargoId, int count)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        int hold;
        const auto cargo = EnumCargo(player, hold).Raw();
        if (cargo.has_error())
        {
            return { cpp::fail(cargo.error()) };
        }

        for (auto& item : cargo.value())
        {
            if (item.id == cargoId && item.count < count)
            {
                count = item.count; // trying to remove more than actually there
            }
        }

        pub::Player::RemoveCargo(client, cargoId, count);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> AddCargo(const std::variant<uint, std::wstring_view>& player, uint goodId, int count, bool mission)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        // add
        const GoodInfo* gi;
        if (!(gi = GoodList::find_by_id(goodId)))
        {
            return { cpp::fail(Error::InvalidGood) };
        }

        bool multiCount;
        memcpy(&multiCount, (char*)gi + 0x70, 1);

        uint base = 0;
        pub::Player::GetBase(client, base);
        uint location = 0;
        pub::Player::GetLocation(client, location);

        // trick cheat detection
        if (base)
        {
            if (location)
            {
                Server.LocationExit(location, client);
            }
            Server.BaseExit(base, client);
            if (!Client::IsValidClientID(client)) // got cheat kicked
            {
                return { cpp::fail(Error::PlayerNotLoggedIn) };
            }
        }

        if (multiCount)
        {
            // it's a good that can have multiple units(commodities missile ammo, etc)
            int ret;

            // we need to do this, else server or client may crash
            for (const auto cargo = EnumCargo(player, ret).Raw(); auto& item : cargo.value())
            {
                if (item.archId == goodId && item.mission != mission)
                {
                    RemoveCargo(player, static_cast<ushort>(item.id), item.count);
                    count += item.count;
                }
            }

            pub::Player::AddCargo(client, goodId, count, 1, mission);
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                pub::Player::AddCargo(client, goodId, 1, 1, mission);
            }
        }

        if (base)
        {
            // player docked on base
            ///////////////////////////////////////////////////
            // fix, else we get anti-cheat msg when undocking
            // this DOES NOT disable anti-cheat-detection, we're
            // just making some adjustments so that we dont get kicked

            Server.BaseEnter(base, client);
            if (location)
            {
                Server.LocationEnter(location, client);
            }
        }

        return { {} };
    }

    Action<void> AddCargo(const std::variant<uint, std::wstring_view>& player, std::wstring_view good, int count, bool mission)
    {
        uint goodId = StringUtils::Cast<int>(good);
        if (!goodId)
        {
            pub::GetGoodID(goodId, StringUtils::wstos(std::wstring(good)).c_str());
        }
        if (!goodId)
        {
            return { cpp::fail(Error::InvalidGood) };
        }

        return AddCargo(player, goodId, count, mission);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> Rename(const std::variant<uint, std::wstring_view>& player, std::wstring_view newCharname, bool onlyDelete)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX && player.index() && !Client::GetAccountByClientID(client))
        {
            return { cpp::fail(Error::CharacterDoesNotExist) };
        }

        if (!onlyDelete && Client::GetAccountByCharName(newCharname).Raw())
        {
            return { cpp::fail(Error::AlreadyExists) };
        }

        if (!onlyDelete && newCharname.length() > 23)
        {
            return { cpp::fail(Error::CharacterNameTooLong) };
        }

        if (!onlyDelete && !newCharname.length())
        {
            return { cpp::fail(Error::CharacterNameTooShort) };
        }

        INI_Reader ini;
        if (!onlyDelete && !ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
        {
            return { cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid) };
        }

        CAccount* acc;
        std::wstring oldCharName;
        if (client != UINT_MAX)
        {
            acc = Players.FindAccountFromClientID(client);
            oldCharName = (wchar_t*)Players.GetActiveCharacterName(client);
        }
        else
        {
            oldCharName = std::get<std::wstring_view>(player);
            acc = Client::GetAccountByCharName(std::get<std::wstring_view>(player)).Raw().value();
        }

        const std::wstring AccountDirname = Client::GetAccountDirName(acc);
        const auto newFileName = Client::GetCharFileName(newCharname).Raw();
        if (newFileName.has_error())
        {
            return { cpp::fail(newFileName.error()) };
        }

        const auto oldFileName = Client::GetCharFileName(oldCharName).Raw();
        if (oldFileName.has_error())
        {
            return { cpp::fail(oldFileName.error()) };
        }

        std::wstring NewCharfilePath = CoreGlobals::c()->accPath + (AccountDirname) + L"\\" + (newFileName.value()) + L".fl";
        std::wstring OldCharfilePath = CoreGlobals::c()->accPath + (AccountDirname) + L"\\" + (oldFileName.value()) + L".fl";

        if (onlyDelete)
        {
            // delete character
            st6::wstring str((ushort*)oldCharName.c_str());
            Client::LockAccountAccess(acc, true); // also kicks player on this account
            Players.DeleteCharacterFromName(str);
            Client::UnlockAccountAccess(acc);
            return { {} };
        }

        Client::LockAccountAccess(acc, true); // kick player if online
        Client::UnlockAccountAccess(acc);

        // Copy existing char file into tmp
        std::wstring tmpPath = OldCharfilePath + L".tmp";
        DeleteFileW(tmpPath.c_str());
        CopyFileW(OldCharfilePath.c_str(), tmpPath.c_str(), FALSE);

        // Delete existing char otherwise a rename of the char in slot 5 fails.
        st6::wstring str((ushort*)oldCharName.c_str());
        Players.DeleteCharacterFromName(str);

        // Emulate char create
        SLoginInfo logindata;
        wcsncpy_s(logindata.account, Client::GetAccountID(acc).Raw().value().c_str(), 36);
        Players.login(logindata, MaxClientId + 1);

        SCreateCharacterInfo newcharinfo;
        wcsncpy_s(newcharinfo.charname, newCharname.data(), 23);
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
                    {
                        newcharinfo.nickName = CreateID(ini.get_value_string());
                    }
                    else if (ini.is_value("base"))
                    {
                        newcharinfo.base = CreateID(ini.get_value_string());
                    }
                    else if (ini.is_value("Package"))
                    {
                        newcharinfo.package = CreateID(ini.get_value_string());
                    }
                    else if (ini.is_value("Pilot"))
                    {
                        newcharinfo.pilot = CreateID(ini.get_value_string());
                    }
                }
                break;
            }
        }
        ini.close();

        if (newcharinfo.nickName == 0)
        {
            newcharinfo.nickName = CreateID("new_player");
        }
        if (newcharinfo.base == 0)
        {
            newcharinfo.base = CreateID("Li01_01_Base");
        }
        if (newcharinfo.package == 0)
        {
            newcharinfo.package = CreateID("ge_fighter");
        }
        if (newcharinfo.pilot == 0)
        {
            newcharinfo.pilot = CreateID("trent");
        }

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
        SaveChar(newCharname);
        Players.logout(MaxClientId + 1);

        // Decode the backup of the old char and overwrite the new char file
        if (!FlCodec::DecodeFile(tmpPath, NewCharfilePath))
        {
            // file wasn't encoded, thus
            // simply rename it
            DeleteFileW(NewCharfilePath.c_str()); // just to get sure...
            CopyFileW(tmpPath.c_str(), NewCharfilePath.c_str(), FALSE);
        }
        DeleteFileW(tmpPath.c_str());

        std::wstring value = std::format(L" {}", StringUtils::ToHex(newCharname));

        IniUtils::i()->WriteToPlayerFile(player, L"Name", value).Handle();

        // Re-encode the char file if needed.
        if (!FLHookConfig::i()->general.disableCharfileEncryption && !FlCodec::EncodeFile(NewCharfilePath, NewCharfilePath))
        {
            return { cpp::fail(Error::CouldNotEncodeCharFile) };
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> MsgAndKick(ClientId client, const std::wstring_view reason, uint interval)
    {
        if (!ClientInfo[client].tmKickTime)
        {
            const std::wstring msg = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.kickMsg, L"%reason", StringUtils::XmlText(reason));
            Chat::FMsg(client, msg);
            ClientInfo[client].tmKickTime = TimeUtils::UnixMilliseconds() + interval;
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> Kill(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);

        // check if logged in
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint ship;
        pub::Player::GetShip(client, ship);
        if (!ship)
        {
            return { cpp::fail(Error::PlayerNotInSpace) };
        }

        pub::SpaceObj::SetRelativeHealth(ship, 0.0f);

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> ResetRep(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);

        // check if logged in
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        INI_Reader ini;
        if (!ini.open("mpnewcharacter.fl", false))
        {
            return { cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid) };
        }

        ini.read_header();
        if (!ini.is_header("Player"))
        {
            ini.close();
            return { cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid) };
        }

        int playerRep;
        pub::Player::GetRep(client, playerRep);
        while (ini.read_value())
        {
            if (ini.is_value("house"))
            {
                const float rep = ini.get_value_float(0);
                const char* repGroupName = ini.get_value_string(1);

                uint repGroupId;
                pub::Reputation::GetReputationGroup(repGroupId, repGroupName);
                pub::Reputation::SetReputation(playerRep, repGroupId, rep);
            }
        }

        ini.close();
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> SetRep(const std::variant<uint, std::wstring_view>& player, std::wstring_view repGroup, float value)
    {
        ClientId client = Client::ExtractClientID(player);
        // check if logged in
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint repGroupId;
        pub::Reputation::GetReputationGroup(repGroupId, StringUtils::wstos(std::wstring(repGroup)).c_str());
        if (repGroupId == -1)
        {
            return { cpp::fail(Error::InvalidRepGroup) };
        }

        int playerRep;
        pub::Player::GetRep(client, playerRep);
        pub::Reputation::SetReputation(playerRep, repGroupId, value);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<float> GetRep(const std::variant<uint, std::wstring_view>& player, const std::variant<uint, std::wstring_view>& repGroup)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint repGroupId;
        if (repGroup.index() == 1)
        {
            pub::Reputation::GetReputationGroup(repGroupId, StringUtils::wstos(std::wstring(std::get<std::wstring_view>(repGroup))).c_str());
            if (repGroupId == UINT_MAX)
            {
                return { cpp::fail(Error::InvalidRepGroup) };
            }
        }
        else
        {
            repGroupId = std::get<uint>(repGroup);
        }

        int playerRep;
        pub::Player::GetRep(client, playerRep);
        float playerFactionRep;
        pub::Reputation::GetGroupFeelingsTowards(playerRep, repGroupId, playerFactionRep);
        return { playerFactionRep };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<std::vector<GroupMember>> GetGroupMembers(const std::variant<uint, std::wstring_view>& player)
    {
        std::vector<GroupMember> members;
        ClientId client = Client::ExtractClientID(player);

        // check if logged in
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        // hey, at least it works! beware of the VC optimiser.
        st6::vector<uint> vMembers;
        pub::Player::GetGroupMembers(client, vMembers);

        for (const uint i : vMembers)
        {
            GroupMember gm = { i, (wchar_t*)Players.GetActiveCharacterName(i) };
            members.push_back(gm);
        }

        return { members };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: Make this return just a single string.
    Action<std::list<std::wstring>> ReadCharFile(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);

        std::wstring dir;
        cpp::result<CAccount*, Error> acc;
        if (client != UINT_MAX)
        {
            acc = Players.FindAccountFromClientID(client);
            if (auto charName = Client::GetCharacterNameByID(client); charName.Raw().has_error())
            {
                return { cpp::fail(Error::CharacterNotSelected) };
            }

            dir = Client::GetAccountDirName(acc.value());
        }
        else
        {
            acc = Client::ExtractAccount(player);
            if (!acc)
            {
                return { cpp::fail(Error::CharacterDoesNotExist) };
            }
        }

        auto file = Client::GetCharFileName(player).Raw();
        if (file.has_error())
        {
            return { cpp::fail(file.error()) };
        }

        std::wstring charFile = CoreGlobals::c()->accPath + dir + L"\\" + file.value() + L".fl";
        std::wstring fileToRead;
        bool deleteAfter;
        if (Client::IsEncoded(charFile))
        {
            std::wstring charFileNew = charFile + L".ini";
            if (!FlCodec::DecodeFile(charFile, charFileNew))
            {
                return { cpp::fail(Error::CouldNotDecodeCharFile) };
            }

            fileToRead = charFileNew;
            deleteAfter = true;
        }
        else
        {
            fileToRead = charFile;
            deleteAfter = false;
        }

        std::wifstream ifs;
        ifs.open(fileToRead.c_str(), std::ios_base::in);
        if (!ifs.is_open())
        {
            return { cpp::fail(Error::UnknownError) };
        }

        std::list<std::wstring> output;
        std::wstring Line;
        while (getline(ifs, Line))
        {
            output.emplace_back(Line);
        }
        ifs.close();
        if (deleteAfter)
        {
            DeleteFileW(fileToRead.c_str());
        }

        return { output };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> WriteCharFile(const std::variant<uint, std::wstring_view>& player, std::wstring data)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        const auto acc = Players.FindAccountFromClientID(client);
        if (const wchar_t* charname = (wchar_t*)Players.GetActiveCharacterName(client); !charname)
        {
            return { cpp::fail(Error::CharacterNotSelected) };
        }

        auto dir = Client::GetAccountDirName(acc);

        const auto file = Client::GetCharFileName(player).Raw();
        if (file.has_error())
        {
            return { cpp::fail(file.error()) };
        }

        std::wstring charFile = CoreGlobals::c()->accPath + dir + L"\\" + file.value() + L".fl";
        std::wstring fileToWrite;
        bool encode;
        if (Client::IsEncoded(charFile))
        {
            fileToWrite = charFile + L".ini";
            encode = true;
        }
        else
        {
            fileToWrite = charFile;
            encode = false;
        }

        std::ofstream ofs;
        ofs.open(fileToWrite.c_str(), std::ios_base::out);
        if (!ofs.is_open())
        {
            return { cpp::fail(Error::UnknownError) };
        }

        size_t pos;
        while ((pos = data.find(L"\\n")) != -1)
        {
            std::wstring line = data.substr(0, pos);
            ofs << StringUtils::wstos(line) << std::endl;
            data.erase(0, pos + 2);
        }

        if (data.length())
        {
            ofs << StringUtils::wstos(data);
        }

        ofs.close();
        if (encode)
        {
            FlCodec::EncodeFile(fileToWrite, charFile);
            DeleteFileW(fileToWrite.c_str());
        }
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> PlayerRecalculateCRC(ClientId client)
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
            return { cpp::fail(Error::InvalidClientId) };
        }

        return { {} };
    }

    /** Move the client to the specified location */
    void RelocateClient(ClientId client, Vector destination, const Matrix& orientation)
    {
        const Quaternion rotation = Math::MatrixToQuaternion(orientation);

        FLPACKET_LAUNCH launchPacket;
        launchPacket.ship = ClientInfo[client].ship;
        launchPacket.base = 0;
        launchPacket.state = 0xFFFFFFFF;
        launchPacket.rotate[0] = rotation.w;
        launchPacket.rotate[1] = rotation.x;
        launchPacket.rotate[2] = rotation.y;
        launchPacket.rotate[3] = rotation.z;
        launchPacket.pos[0] = destination.x;
        launchPacket.pos[1] = destination.y;
        launchPacket.pos[2] = destination.z;

        HookClient->Send_FLPACKET_SERVER_LAUNCH(client, launchPacket);

        uint system;
        pub::Player::GetSystem(client, system);
        pub::SpaceObj::Relocate(ClientInfo[client].ship, system, destination, orientation);
    }

    /** Dock the client immediately */
    Action<void> InstantDock(ClientId client, uint dockObj)
    {
        // check if logged in
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint ship;
        pub::Player::GetShip(client, ship);
        if (!ship)
        {
            return { cpp::fail(Error::PlayerNotInSpace) };
        }

        uint system;
        uint system2;
        pub::SpaceObj::GetSystem(ship, system);
        pub::SpaceObj::GetSystem(dockObj, system2);
        if (system != system2)
        {
            return { cpp::fail(Error::PlayerNotInSpace) };
        }

        try
        {
            pub::SpaceObj::InstantDock(ship, dockObj, 1);
        }
        catch (...)
        {
            return { cpp::fail(Error::PlayerNotInSpace) };
        }

        return { {} };
    }

    Action<int> GetRank(const std::variant<uint, std::wstring_view>& player)
    {
        auto rank = IniUtils::i()->GetFromPlayerFile(player, L"rank");
        if (rank.has_error())
        {
            return { cpp::fail(rank.error()) };
        }

        return { rank.value().length() ? StringUtils::Cast<int>(rank.value()) : 0 };
    }

    /// Get online time.
    Action<int> GetOnlineTime(const std::variant<uint, std::wstring_view>& player)
    {
        const auto client = Client::ExtractClientID(player);
        const auto acc = Client::GetAccountByClientID(client);
        const auto dir = Client::GetAccountDirName(acc);

        const auto file = Client::GetCharFileName(player).Raw();
        if (file.has_error())
        {
            return { cpp::fail(file.error()) };
        }

        const std::wstring charFile = CoreGlobals::c()->accPath + dir + L"\\" + file.value() + L".fl";
        if (Client::IsEncoded(charFile))
        {
            const std::wstring charFileNew = charFile + L".ini";
            if (!FlCodec::DecodeFile(charFile, charFileNew))
            {
                return { cpp::fail(Error::CouldNotDecodeCharFile) };
            }

            int secs = StringUtils::Cast<int>(IniUtils::GetFromFile(charFileNew, L"mPlayer", L"total_time_played").value());
            DeleteFileW(charFileNew.c_str());
            return { secs };
        }
        return { StringUtils::Cast<int>(IniUtils::GetFromFile(charFile, L"mPlayer", L"total_time_played").value()) };
    }

    Action<uint> GetSystemByNickname(std::variant<std::wstring_view, std::string_view> nickname)
    {
        uint system = 0;
        const std::string nick =
            nickname.index() == 0 ? StringUtils::wstos(std::wstring(std::get<std::wstring_view>(nickname))) : std::string(std::get<std::string_view>(nickname));
        pub::GetSystemID(system, nick.c_str());
        if (!system)
        {
            return { cpp::fail(Error::InvalidSystem) };
        }

        return { system };
    }

    CShip* CShipFromShipDestroyed(const DWORD** ecx)
    {
        return reinterpret_cast<CShip*>((*ecx)[4]); // NOLINT(performance-no-int-to-ptr)
    }

    /// Return true if this player is within the specified distance of any other player.
    bool IsInRange(ClientId client, float distance)
    {
        const auto Members = GetGroupMembers((const wchar_t*)Players.GetActiveCharacterName(client)).Raw();
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
            uint system2 = 0;
            pub::Player::GetSystem(client2, system2);
            if (system != system2)
            {
                continue;
            }

            uint ship2;
            pub::Player::GetShip(client2, ship2);

            Vector pos2;
            Matrix rot2;
            pub::SpaceObj::GetLocation(ship2, pos2, rot2);

            // Ignore players who are in your group.
            bool grouped = false;
            for (auto& gm : Members.value())
            {
                if (gm.client == client2)
                {
                    grouped = true;
                    break;
                }
            }
            if (grouped)
            {
                continue;
            }

            // Is player within the specified range of the sending char.
            if (Math::Distance3D(pos, pos2) < distance)
            {
                return true;
            }
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
    Action<void> NewCharacter(CAccount* acc, std::wstring& character)
    {
        Client::LockAccountAccess(acc, true);
        Client::UnlockAccountAccess(acc);

        INI_Reader ini;
        if (!ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
        {
            return { cpp::fail(Error::MpNewCharacterFileNotFoundOrInvalid) };
        }

        // Emulate char create by logging in.
        SLoginInfo logindata;
        wcsncpy_s(logindata.account, Client::GetAccountID(acc).Raw().value().c_str(), 36);
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
                    {
                        newcharinfo.nickName = CreateID(ini.get_value_string());
                    }
                    else if (ini.is_value("base"))
                    {
                        newcharinfo.base = CreateID(ini.get_value_string());
                    }
                    else if (ini.is_value("package"))
                    {
                        newcharinfo.package = CreateID(ini.get_value_string());
                    }
                    else if (ini.is_value("pilot"))
                    {
                        newcharinfo.pilot = CreateID(ini.get_value_string());
                    }
                }
                break;
            }
        }
        ini.close();

        if (newcharinfo.nickName == 0)
        {
            newcharinfo.nickName = CreateID("new_player");
        }
        if (newcharinfo.base == 0)
        {
            newcharinfo.base = CreateID("Li01_01_Base");
        }
        if (newcharinfo.package == 0)
        {
            newcharinfo.package = CreateID("ge_fighter");
        }
        if (newcharinfo.pilot == 0)
        {
            newcharinfo.pilot = CreateID("trent");
        }

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

        return { {} };
    }

    // Anti cheat checking code by mc_horst. Will always return okay if the user is in space.
    Action<void> AntiCheat(ClientId client)
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
        if (const auto ship = GetShip(client).Raw(); ship.has_value())
        {
            return { {} };
        }

        char* objPtr;
        memcpy(&objPtr, &Players, 4);
        objPtr += 0x418 * (client - 1);

        char res = 0;

        __asm {
			mov ecx, [objPtr]
			call [AntiCheat1]
			mov [res], al
        }

        if (res != 0)
        {
            // kick
            Kick(client);
            return { cpp::fail(Error::UnknownError) };
        }

        __asm {
			mov ecx, [objPtr]
			call [AntiCheat2]
			mov [res], al
        }

        if (res != 0)
        {
            Kick(client);
            return { cpp::fail(Error::UnknownError) };
        }

        ulong retVal = 0;
        ulong compare = 0;
        __asm {
			mov ecx, [objPtr]
			mov eax, [ecx+0x320]
			mov [compare], eax
			call [AntiCheat3]
			mov [retVal], eax
        }

        if (retVal > compare)
        {
            Kick(client);
            return { cpp::fail(Error::UnknownError) };
        }

        __asm {
			mov ecx, [objPtr]
			call [AntiCheat4]
			mov [res], al
        }

        if (res != 0)
        {
            Kick(client);
            return { cpp::fail(Error::UnknownError) };
        }
    }

    Action<void> SetEquip(const std::variant<uint, std::wstring_view>& player, const st6::list<EquipDesc>& equip)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
        {
            return { cpp::fail(Error::CharacterNotSelected) };
        }

        // Update FLHook's lists to make anticheat pleased.
        if (&equip != &Players[client].lShadowEquipDescList.equip)
        {
            Players[client].lShadowEquipDescList.equip = equip;
        }

        if (&equip != &Players[client].equipDescList.equip)
        {
            Players[client].equipDescList.equip = equip;
        }

        // Calculate packet size. First two bytes reserved for items count.
        uint itemBufSize = 2;
        for (const auto& item : equip)
        {
            itemBufSize += sizeof(SetEquipmentItem) + strlen(item.hardPoint.value) + 1;
        }

        FlPacket* packet = FlPacket::Create(itemBufSize, FlPacket::FLPACKET_SERVER_SETEQUIPMENT);
        const auto setEquipmentPacket = reinterpret_cast<FlPacketSetEquipment*>(packet->content);

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
            setEquipmentPacket->count++;

            const byte* buf = (byte*)&setEquipItem;
            for (int i = 0; i < sizeof(SetEquipmentItem); i++)
            {
                setEquipmentPacket->items[index++] = buf[i];
            }

            const byte* hardPoint = (byte*)item.hardPoint.value;
            for (int i = 0; i < setEquipItem.hardPointLen; i++)
            {
                setEquipmentPacket->items[index++] = hardPoint[i];
            }
        }

        if (packet->SendTo(client))
        {
            return { {} };
        }
        return { cpp::fail(Error::UnknownError) };
    }

    Action<void> AddEquip(const std::variant<uint, std::wstring_view>& player, uint goodId, const std::wstring& hardpoint)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
        {
            return { cpp::fail(Error::CharacterNotSelected) };
        }

        if (!Players[client].enteredBase)
        {
            Players[client].enteredBase = Players[client].baseId;
            Server.ReqAddItem(goodId, StringUtils::wstos(hardpoint).c_str(), 1, 1.0f, true, client);
            Players[client].enteredBase = 0;
        }
        else
        {
            Server.ReqAddItem(goodId, StringUtils::wstos(hardpoint).c_str(), 1, 1.0f, true, client);
        }

        // Add to check-list which is being compared to the users equip-list when
        // saving char to fix "Ship or Equipment not sold on base" kick
        EquipDesc ed;
        ed.id = Players[client].lastEquipId;
        ed.count = 1;
        ed.archId = goodId;
        Players[client].lShadowEquipDescList.add_equipment_item(ed, false);

        return { {} };
    }

    Action<void> AddEquip(const std::variant<uint, std::wstring_view>& player, [[maybe_unused]] uint goodId, const std::wstring& hardpoint, bool mounted)
    {
        using _AddCargoDocked = bool(__stdcall*)(uint goodId, CacheString * &hardpoint, int numItems, float health, int mounted, int mission, uint one);
        static _AddCargoDocked addCargoDocked = nullptr;
        if (!addCargoDocked)
        {
            addCargoDocked = (_AddCargoDocked)((char*)server + 0x6EFC0);
        }

        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX || Client::IsInCharSelectMenu(client))
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint base = 0;
        pub::Player::GetBase(client, base);
        uint location = 0;
        pub::Player::GetLocation(client, location);

        if (location)
        {
            Server.LocationExit(location, client);
        }
        if (base)
        {
            Server.BaseExit(base, client);
        }
        if (!Client::IsValidClientID(client))
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        PlayerData* pd = &Players[client];
        const auto p = StringUtils::wstos(hardpoint);
        CacheString hardpointCache;
        hardpointCache.value = StringAlloc(p.c_str(), false);

        int one = 1;
        int mountedAsInt = mounted;
        float health = 1;
        CacheString* hardpointPointer = &hardpointCache;
        __asm {
			push one
			push mountedAsInt
			push one
			push health
			push one
			push hardpointPointer
			push goodId
			mov ecx, pd
			call addCargoDocked
        }

        if (base) Server.BaseEnter(base, client);
        if (location)
        {
            Server.LocationEnter(location, client);
        }

        return { {} };
    }

    void DelayedKick(ClientId client, uint secs)
    {
        const mstime kick_time = TimeUtils::UnixSeconds() + secs;
        if (!ClientInfo[client].tmKickTime || ClientInfo[client].tmKickTime > kick_time)
        {
            ClientInfo[client].tmKickTime = kick_time;
        }
    }

    std::wstring GetPlayerSystemS(ClientId client)
    {
        uint systemId;
        pub::Player::GetSystem(client, systemId);
        char Systemname[1024] = "";
        pub::GetSystemNickname(Systemname, sizeof Systemname, systemId);
        return StringUtils::stows(Systemname);
    }

    Action<uint> GetShipValue(const std::variant<uint, std::wstring_view>& player)
    {
        if (ClientId client = Client::ExtractClientID(player); client != UINT_MAX && !Client::IsInCharSelectMenu(client))
        {
            SaveChar(player);
            if (!Client::IsValidClientID(client))
            {
                return { cpp::fail(Error::UnknownError) };
            }
        }

        float value = 0.0f;

        uint baseId = 0;

        const auto CharFile = ReadCharFile(player).Raw();
        if (CharFile.has_error())
        {
            return { cpp::fail(CharFile.error()) };
        }

        for (const auto& line : CharFile.value())
        {
            std::wstring key = StringUtils::Trim(line.substr(0, line.find(L"=")));
            if (key == L"base" || key == L"last_base")
            {
                const int findEqual = line.find(L"=");
                if (findEqual == -1)
                {
                    continue;
                }

                if (findEqual + 1 >= static_cast<int>(line.size()))
                {
                    continue;
                }

                baseId = CreateID(StringUtils::wstos(StringUtils::Trim(line.substr(findEqual + 1))).c_str());
                break;
            }
        }

        for (const auto& line : CharFile.value())
        {
            std::wstring key = StringUtils::Trim(line.substr(0, line.find(L"=")));
            if (key == L"cargo" || key == L"equip")
            {
                const int findEqual = line.find(L"=");
                if (findEqual == -1)
                {
                    continue;
                }
                const int findComma = line.find(L",", findEqual);
                if (findComma == -1)
                {
                    continue;
                }
                const uint goodId = StringUtils::Cast<uint>(StringUtils::Trim(line.substr(findEqual + 1, findComma)));
                const uint goodCount = StringUtils::Cast<uint>(StringUtils::Trim(line.substr(findComma + 1, line.find(L",", findComma + 1))));

                float itemValue;
                if (pub::Market::GetPrice(baseId, Arch2Good(goodId), itemValue) == 0)
                {
                    if (arch_is_combinable(goodId))
                    {
                        value += itemValue * static_cast<float>(goodCount);
                    }
                    else
                    {
                        const float* resaleFactor = (float*)((char*)server + 0x8AE7C);
                        value += itemValue * *resaleFactor;
                    }
                }
            }
            else if (key == L"money")
            {
                const int findEqual = line.find(L"=");
                if (findEqual == -1)
                {
                    continue;
                }
                const uint itemValue = StringUtils::Cast<uint>(StringUtils::Trim(line.substr(findEqual + 1)));
                value += itemValue;
            }
            else if (key == L"ship_archetype")
            {
                const uint shipArchId = StringUtils::Cast<uint>(StringUtils::Trim(line.substr(line.find(L"=") + 1, line.length())));
                const GoodInfo* gi = GoodList_get()->find_by_ship_arch(shipArchId);
                if (gi)
                {
                    gi = GoodList::find_by_id(gi->archId);
                    if (gi)
                    {
                        const auto resaleFactor = (float*)((char*)server + 0x8AE78);
                        const float itemValue = gi->price * *resaleFactor;
                        value += itemValue;
                    }
                }
            }
        }

        return { value };
    }

    void SaveChar(ClientId client)
    {
        const BYTE patch[] = { 0x90, 0x90 };
        MemUtils::WriteProcMem((char*)server + 0x7EFA8, patch, sizeof patch);
        pub::Save(client, 1);
    }

    Action<ShipId> GetTarget(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        const auto ship = GetShip(client).Raw();
        if (ship.has_error())
        {
            return { cpp::fail(ship.error()) };
        }

        uint target;
        pub::SpaceObj::GetTarget(ship.value(), target);
        if (!target)
        {
            return { cpp::fail(Error::NoTargetSelected) };
        }

        return { target };
    }

    Action<ClientId> GetTargetClientID(const std::variant<uint, std::wstring_view>& player)
    {
        const auto target = GetTarget(player).Raw();
        if (target.has_error())
        {
            return { cpp::fail(target.error()) };
        }

        auto targetClientId = Client::GetClientIdByShip(target.value()).Raw();
        if (targetClientId.has_error())
        {
            return { cpp::fail(Error::TargetIsNotPlayer) };
        }

        return { targetClientId };
    }

    Action<BaseId> GetCurrentBase(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint base;
        pub::Player::GetBase(client, base);
        if (base)
        {
            return { base };
        }

        return { cpp::fail(Error::PlayerNotDocked) };
    }

    Action<SystemId> GetSystem(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint system;
        pub::Player::GetSystem(client, system);
        if (!system)
        {
            return { cpp::fail(Error::InvalidSystem) };
        }

        return { system };
    }

    // returns ship instance ID
    Action<ShipId> GetShip(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint ship;
        pub::Player::GetShip(client, ship);
        if (!ship)
        {
            return { cpp::fail(Error::PlayerNotInSpace) };
        }

        return { ship };
    }

    // returns Ship type
    Action<uint> GetShipID(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        uint shipId;
        pub::Player::GetShipID(client, shipId);
        if (!shipId)
        {
            return { cpp::fail(Error::PlayerNotInSpace) };
        }

        return { shipId };
    }

    Action<void> MarkObj(const std::variant<uint, std::wstring_view>& player, uint objId, int markStatus)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }
        pub::Player::MarkObj(client, objId, markStatus);

        return { {} };
    }

    Action<int> GetPvpKills(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }
        int kills;
        pub::Player::GetNumKills(client, kills);
        return { kills };
    }

    Action<void> SetPvpKills(const std::variant<uint, std::wstring_view>& player, int killAmount)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        pub::Player::SetNumKills(client, killAmount);

        return { {} };
    }

    Action<int> IncrementPvpKills(const std::variant<uint, std::wstring_view>& player)
    {
        ClientId client = Client::ExtractClientID(player);
        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        int kills;
        pub::Player::GetNumKills(client, kills);
        kills++;
        pub::Player::SetNumKills(client, kills);

        return { 0 };
    }
} // namespace Hk::Player
