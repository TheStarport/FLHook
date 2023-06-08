#pragma once

namespace Hk::Admin
{
	DLL std::wstring GetPlayerIP(ClientId client);
	DLL Action<PlayerInfo> GetPlayerInfo(const std::variant<uint, std::wstring_view>& player, bool alsoCharmenu);
	DLL std::list<PlayerInfo> GetPlayers();
	DLL Action<DPN_CONNECTION_INFO> GetConnectionStats(ClientId client);
	DLL Action<void> ChangeNPCSpawn(bool disable);
	DLL Action<BaseHealth> GetBaseStatus(const std::wstring& basename);
	DLL Fuse* GetFuseFromID(uint fuseId);
	DLL bool LightFuse(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip);
	DLL bool UnLightFuse(IObjRW* ship, uint fuseId);
	DLL CEqObj* GetEqObjFromObjRW(IObjRW* objRW);
	DLL Action<void> AddRole(const std::wstring& characterName, const std::wstring& role);
	DLL Action<void> RemoveRole(const std::wstring& characterName, const std::wstring& role);
	DLL Action<void> SetRoles(const std::wstring& characterName, const std::vector<std::wstring>& roles);
}