#include "PCH.hpp"

#include "Global.hpp"
#include "API/FLServer/Admin.hpp"
#include "API/FLServer/Client.hpp"


bool g_bNPCDisabled;

namespace Hk::Admin
{
    std::wstring GetPlayerIP(ClientId client)
    {
        const CDPClientProxy* cdpClient = clientProxyArray[client - 1];
        if (!cdpClient)
        {
            return L"";
        }

        // get ip
        char* P1;
        char* IdirectPlay8Address;
        wchar_t hostname[] = L"hostname";
        memcpy(&P1, (char*)cdpSrv + 4, 4);

        wchar_t wIP[1024] = L"";
        long sizeofIP = sizeof wIP;
        long dataType = 1;
        __asm {
			push 0 ; flags
			lea edx, IdirectPlay8Address
			push edx ; address
			mov edx, [cdpClient]
			mov edx, [edx+8]
			push edx ; dpnid
			mov eax, [P1]
			push eax
			mov ecx, [eax]
			call dword ptr[ecx + 0x28] ; GetClientAddress
			cmp eax, 0
			jnz some_error

			lea eax, dataType
			push eax
			lea eax, sizeofIP
			push eax
			lea eax, wIP
			push eax
			lea eax, hostname
			push eax
			mov ecx, [IdirectPlay8Address]
			push ecx
			mov ecx, [ecx]
			call dword ptr[ecx+0x40] ; GetComponentByName

			mov ecx, [IdirectPlay8Address]
			push ecx
			mov ecx, [ecx]
			call dword ptr[ecx+0x08] ; Release
			some_error:
        }

        return wIP;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<PlayerInfo> GetPlayerInfo(const std::variant<uint, std::wstring_view>& player, bool alsoCharmenu)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX || (Client::IsInCharSelectMenu(client) && !alsoCharmenu))
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        PlayerInfo pi;
        const wchar_t* activeCharname = (wchar_t*)Players.GetActiveCharacterName(client);

        pi.client = client;
        pi.character = activeCharname ? activeCharname : L"";
        pi.baseName = pi.systemName = L"";

        uint base = 0;
        uint system = 0;
        pub::Player::GetBase(client, base);
        pub::Player::GetSystem(client, system);
        pub::Player::GetShip(client, pi.ship);

        if (base)
        {
            char Basename[1024] = "";
            pub::GetBaseNickname(Basename, sizeof Basename, base);
            pi.baseName = StringUtils::stows(Basename);
        }

        if (system)
        {
            char Systemname[1024] = "";
            pub::GetSystemNickname(Systemname, sizeof Systemname, system);
            pi.systemName = StringUtils::stows(Systemname);
            pi.system = system;
        }

        // get ping
        auto ci = GetConnectionStats(client).Raw();
        if (ci.has_error())
        {
            Logger::i()->Log(LogLevel::Warn, L"Invalid client ID provided when getting connection stats");
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }
        pi.connectionInfo = ci.value();

        // get ip
        pi.IP = GetPlayerIP(client);

        pi.hostname = ClientInfo[client].hostname;

        return { pi };
    }

    std::list<PlayerInfo> GetPlayers()
    {
        std::list<PlayerInfo> ret;

        PlayerData* playerDb = nullptr;
        while ((playerDb = Players.traverse_active(playerDb)))
        {
            ClientId client = playerDb->onlineId;

            if (Client::IsInCharSelectMenu(client))
            {
                continue;
            }

            ret.emplace_back(GetPlayerInfo(client, false).Unwrap());
        }
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<DPN_CONNECTION_INFO> GetConnectionStats(ClientId client)
    {
        if (client < 1 || client > MaxClientId)
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        CDPClientProxy* cdpClient = clientProxyArray[client - 1];

        DPN_CONNECTION_INFO ci;
        if (!cdpClient || !cdpClient->GetConnectionStats(&ci))
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        return { ci };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void> ChangeNPCSpawn(bool disable)
    {
        if (CoreGlobals::c()->disableNpcs && disable)
        {
            return { {} };
        }
        if (!CoreGlobals::c()->disableNpcs && !disable)
        {
            return { {} };
        }

        char jump[1];
        char cmp[1];
        if (disable)
        {
            jump[0] = '\xEB';
            cmp[0] = '\xFF';
        }
        else
        {
            jump[0] = '\x75';
            cmp[0] = '\xF9';
        }

        void* address = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS1);
        MemUtils::WriteProcMem(address, &jump, 1);
        address = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS2);
        MemUtils::WriteProcMem(address, &cmp, 1);
        g_bNPCDisabled = disable;
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<BaseHealth> GetBaseStatus(std::wstring_view basename)
    {
        uint baseId = 0;
        pub::GetBaseID(baseId, StringUtils::wstos(std::wstring(basename)).c_str());
        if (!baseId)
        {
            return { cpp::fail(Error::InvalidBaseName) };
        }

        float curHealth;
        float maxHealth;

        const Universe::IBase* base = Universe::get_base(baseId);
        pub::SpaceObj::GetHealth(base->spaceObjId, curHealth, maxHealth);
        BaseHealth bh = { curHealth, maxHealth };
        return { bh };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Fuse* GetFuseFromID(uint fuseId)
    {
        int dunno = 0;
        Fuse* fuse = nullptr;
        __asm {
			mov edx, 0x6CFD390
			call edx

			lea ecx, fuseId
			push ecx
			lea ecx, dunno
			push ecx
			mov ecx, eax
			mov edx, 0x6D15D10
			call edx
			mov edx, [dunno]
			mov edi, [edx+0x10]
			mov fuse, edi
        }
        return fuse;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /// Return the CEqObj from the IObjRW
    __declspec(naked) CEqObj* __stdcall GetEqObjFromObjRW_(IObjRW* objRW)
    {
        __asm {
			push ecx
			push edx
			mov ecx, [esp+12]
			mov edx, [ecx]
			call dword ptr[edx+0x150]
			pop edx
			pop ecx
			ret 4
        }
    }

    CEqObj* GetEqObjFromObjRW(IObjRW* objRW) { return GetEqObjFromObjRW_(objRW); }

    // TODO: implement role based commands
    Action<void> AddRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles)
    {
        if (ClientId client = Client::GetClientIdFromCharName(characterName).Unwrap())
        {
            const auto info = &ClientInfo[client];
            const auto adminObj = info->accountData.value("admin", nlohmann::json::object());
            auto roleArray = adminObj.value("roles", nlohmann::json::array());

            for (const auto& role : roles)
            {
                roleArray.emplace_back(StringUtils::wstos(std::wstring(role)));
            }

            info->SaveAccountData();

            return { {} };
        }

        const auto acc = Hk::Client::GetAccountByCharName(characterName).Raw();
        if (acc.has_error())
        {
            return { cpp::fail(acc.error()) };
        }

        const std::wstring dir = Client::GetAccountDirName(acc.value());
        const std::wstring userFile = std::format(L"{}{}\\accData.json", CoreGlobals::c()->accPath, dir);

        // TODO: Move all file IO to FileUtils class

        std::fstream file;
        file.open(userFile, std::ios::in | std::ios::out | std::ios::app);
        file.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file.tellg());

        std::string buffer(size, ' ');

        file.seekg(0);
        file.read(buffer.data(), size);

        nlohmann::basic_json<> accountData;
        try
        {
            accountData = nlohmann::json::parse(buffer);
        }
        catch (nlohmann::json::parse_error&)
        {
            // TODO: Log error character error to specific file
            Logger::i()->Log(LogLevel::Err, std::format(L"Unable to parse account data for character: {}.", characterName));
            accountData = nlohmann::json::object();
        }

        if (!accountData.contains("admin"))
        {
            accountData["admin"] = nlohmann::json::object();
        }

        auto& admin = accountData["admin"];
        if (!admin.contains("roles"))
        {
            admin["roles"] = nlohmann::json::array();
        }

        auto& roleArray = admin["roles"];

        for (const auto& role : roles)
        {
            if (std::ranges::all_of(roleArray,
                                    [role](const auto& existing)
                                    {
                                        if (!existing.is_string())
                                        {
                                            throw GameException(L"Account data is malformed. Admin block corrupted.", Error::MalformedData);
                                        }
                                        return StringUtils::stows(existing.template get<std::string>()) != role;
                                    }))
            {
                roleArray.emplace_back(StringUtils::wstos(std::wstring(role)));
            }
        }

        const auto content = accountData.dump(4);
        std::ofstream of(userFile);
        of.write(content.c_str(), content.size());

        return { {} };
    }

    Action<void> RemoveRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles, bool clear)
    {
        if (ClientId client = Client::GetClientIdFromCharName(characterName).Unwrap())
        {
            const auto info = &ClientInfo[client];
            const auto adminObj = info->accountData.value("admin", nlohmann::json::object());
            auto roleArray = adminObj.value("roles", nlohmann::json::array());

            if (clear)
            {
                roleArray = nlohmann::json::array();
            }
            else
            {
                for (const auto& role : roles)
                {
                    if (const auto foundRole = std::ranges::find_if(roleArray,
                                                                    [role](const auto& existing)
                                                                    {
                                                                        if (!existing.is_string())
                                                                        {
                                                                            throw GameException(L"Account data is malformed. Admin block corrupted.",
                                                                                                Error::MalformedData);
                                                                        }
                                                                        return StringUtils::stows(existing.template get<std::string>()) != role;
                                                                    }) == roleArray.end())
                    {
                        roleArray.erase(foundRole);
                    }
                }
            }

            info->SaveAccountData();

            return { {} };
        }

        const auto acc = Hk::Client::GetAccountByCharName(characterName).Raw();
        if (acc.has_error())
        {
            return { cpp::fail(acc.error()) };
        }

        const std::wstring dir = Client::GetAccountDirName(acc.value());
        const std::wstring userFile = std::format(L"{}{}\\accData.json", CoreGlobals::c()->accPath, dir);

        // TODO: Move all file IO to FileUtils class

        std::fstream file;
        file.open(userFile, std::ios::in | std::ios::out | std::ios::app);
        file.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file.tellg());

        std::string buffer(size, ' ');

        file.seekg(0);
        file.read(buffer.data(), size);

        nlohmann::basic_json<> accountData;
        try
        {
            accountData = nlohmann::json::parse(buffer);
        }
        catch (nlohmann::json::parse_error&)
        {
            // TODO: Log error character error to specific file
            Logger::i()->Log(LogLevel::Err, std::format(L"Unable to parse account data for character: {}.", characterName));
            accountData = nlohmann::json::object();
        }

        if (!accountData.contains("admin"))
        {
            accountData["admin"] = nlohmann::json::object();
        }

        auto& admin = accountData["admin"];
        if (!admin.contains("roles"))
        {
            admin["roles"] = nlohmann::json::array();
        }

        auto& roleArray = admin["roles"];

        for (const auto& role : roles)
        {
            if (const auto foundRole = std::ranges::find_if(roleArray,
                                                            [role](const auto& existing)
                                                            {
                                                                if (!existing.is_string())
                                                                {
                                                                    throw GameException(L"Account data is malformed. Admin block corrupted.",
                                                                                        Error::MalformedData);
                                                                }
                                                                return StringUtils::stows(existing.template get<std::string>()) != role;
                                                            }) == roleArray.end())
            {
                roleArray.erase(foundRole);
            }
        }

        const auto content = accountData.dump(4);
        std::ofstream of(userFile);
        of.write(content.c_str(), content.size());
        return { {} };
    }

    Action<void> SetRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles)
    {
        if (const auto removed = RemoveRoles(characterName, roles, true).Raw(); removed.has_error())
        {
            return { cpp::fail(removed.error()) };
        }

        if (const auto added = AddRoles(characterName, roles).Raw(); added.has_error())
        {
            return { cpp::fail(added.error()) };
        }

        return { {} };
    }

    __declspec(naked) bool __stdcall LightFuse_(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip)
    {
        __asm {
			lea eax, [esp+8] // fuseId
			push [esp+20] // skip
			push [esp+16] // delay
			push 0 // SUBOBJ_Id_NONE
			push eax
			push [esp+32] // lifetime
			mov ecx, [esp+24]
			mov eax, [ecx]
			call [eax+0x1E4]
			ret 20
        }
    }

    bool LightFuse(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip) { return LightFuse_(ship, fuseId, delay, lifetime, skip); }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Returns true if a fuse was unlit
    __declspec(naked) bool __stdcall UnLightFuse_([[maybe_unused]] const IObjRW* ship, [[maybe_unused]] uint fuseId, [[maybe_unused]] float dunno)
    {
        __asm {
			mov ecx, [esp+4]
			lea eax, [esp+8] // fuseId
			push [esp+12] // fdunno
			push 0 // SUBOBJ_Id_NONE
			push eax // fuseId
			mov eax, [ecx]
			call [eax+0x1E8]
			ret 12
        }
    }

    bool UnLightFuse(IObjRW* ship, uint fuseId) { return UnLightFuse_(ship, fuseId, 0.f); }
} // namespace Hk::Admin
