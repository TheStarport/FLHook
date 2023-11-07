#pragma once

class DLL Admin
{
        static std::wstring GetPlayerIP(ClientId client);
        static Action<PlayerInfo, Error> GetPlayerInfo(const std::variant<uint, std::wstring_view>& player, bool alsoCharmenu);
        static std::list<PlayerInfo> GetPlayers();
        static Action<DPN_CONNECTION_INFO, Error> GetConnectionStats(ClientId client);
        static Action<void, Error> ChangeNPCSpawn(bool disable);
        static Action<BaseHealth, Error> GetBaseStatus(std::wstring_view basename);
        static Fuse* GetFuseFromID(uint fuseId);
        static bool LightFuse(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip);
        static bool UnLightFuse(IObjRW* ship, uint fuseId);
        static CEqObj* GetEqObjFromObjRW(IObjRW* objRW);
        static Action<void, Error> AddRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
        static Action<void, Error> RemoveRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles, bool clear);
        static Action<void, Error> SetRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
};
