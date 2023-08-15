#pragma once

namespace Hk::Admin
{
    DLL std::wstring GetPlayerIP(ClientId client);
    DLL Action<PlayerInfo, Error> GetPlayerInfo(const std::variant<uint, std::wstring_view>& player, bool alsoCharmenu);
    DLL std::list<PlayerInfo> GetPlayers();
    DLL Action<DPN_CONNECTION_INFO, Error> GetConnectionStats(ClientId client);
    DLL Action<void, Error> ChangeNPCSpawn(bool disable);
    DLL Action<BaseHealth, Error> GetBaseStatus(std::wstring_view basename);
    DLL Fuse* GetFuseFromID(uint fuseId);
    DLL bool LightFuse(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip);
    DLL bool UnLightFuse(IObjRW* ship, uint fuseId);
    DLL CEqObj* GetEqObjFromObjRW(IObjRW* objRW);
    DLL Action<void, Error> AddRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
    DLL Action<void, Error> RemoveRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles, bool clear);
    DLL Action<void, Error> SetRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
} // namespace Hk::Admin
