#pragma once

namespace Hk::Player
{
	DLL Action<void> AddToGroup(ClientId client, uint groupId);
	DLL Action<uint> GetGroupID(ClientId client);
	DLL Action<uint> GetCash(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> AddCash(const std::variant<uint, std::wstring_view>& player, uint amount);
	DLL Action<void> RemoveCash(const std::variant<uint, std::wstring_view>& player, uint amount);
	DLL Action<void> AdjustCash(const std::variant<uint, std::wstring_view>& player, int amount);
	DLL Action<void> Kick(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> KickReason(const std::variant<uint, std::wstring_view>& player, std::wstring_view reason);
	DLL Action<void> Ban(const std::variant<uint, std::wstring_view>& player, bool ban);
	DLL Action<void> TempBan(const std::variant<uint, std::wstring_view>& player, uint duration);
	DLL Action<void> Beam(const std::variant<uint, std::wstring_view>& player, const std::variant<uint, std::wstring_view>& base);
	DLL Action<void> SaveChar(const std::variant<uint, std::wstring_view>& player);
	DLL Action<std::list<CargoInfo>> EnumCargo(const std::variant<uint, std::wstring_view>& player, int& remainingHoldSize);
	DLL Action<void> RemoveCargo(const std::variant<uint, std::wstring_view>& player, ushort cargoId, int count);
	DLL Action<void> AddCargo(const std::variant<uint, std::wstring_view>& player, uint goodId, int count, bool mission);
	DLL Action<void> AddCargo(const std::variant<uint, std::wstring_view>& player, const std::wstring& good, int count, bool mission);
	DLL Action<void> Rename(const std::variant<uint, std::wstring_view>& player, const std::wstring& newCharname, bool onlyDelete);
	DLL Action<void> MsgAndKick(ClientId client, std::wstring_view Reason, uint interval);
	DLL Action<void> Kill(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> ResetRep(const std::variant<uint, std::wstring_view>& player);
	DLL Action<std::vector<GroupMember>> GetGroupMembers(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> SetRep(const std::variant<uint, std::wstring_view>& player, const std::wstring& repGroup, float value);
	DLL Action<float> GetRep(const std::variant<uint, std::wstring_view>& player, const std::variant<uint, std::wstring_view>& repGroup);
	DLL Action<std::list<std::wstring>> ReadCharFile(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> WriteCharFile(const std::variant<uint, std::wstring_view>& player, std::wstring data);
	DLL Action<void> PlayerRecalculateCRC(ClientId client);
	DLL std::wstring GetPlayerSystemS(ClientId client);
	DLL bool IsInRange(ClientId client, float distance);
	DLL Action<void> SetEquip(const std::variant<uint, std::wstring_view>& player, const st6::list<EquipDesc>& equip);
	DLL Action<void> AddEquip(const std::variant<uint, std::wstring_view>& player, uint goodId, const std::wstring& hardpoint);
	DLL Action<void> AntiCheat(ClientId client);
	DLL void DelayedKick(ClientId client, uint secs);
	DLL void DeleteCharacter(CAccount* acc, const std::wstring& character);
	DLL Action<void> NewCharacter(CAccount* acc, std::wstring& character);
	DLL Action<int> GetOnlineTime(const std::variant<uint, std::wstring_view>& player);
	DLL Action<int> GetRank(const std::variant<uint, std::wstring_view>& player);
	DLL Action<uint> GetShipValue(const std::variant<uint, std::wstring_view>& player);
	DLL void RelocateClient(ClientId client, Vector destination, const Matrix& orientation);
	DLL void SaveChar(ClientId client);
	DLL Action<ShipId> GetTarget(const std::variant<uint, std::wstring_view>& player);
	DLL Action<ClientId> GetTargetClientID(const std::variant<uint, std::wstring_view>& player);
	DLL Action<BaseId> GetCurrentBase(const std::variant<uint, std::wstring_view>& player);
	DLL Action<SystemId> GetSystem(const std::variant<uint, std::wstring_view>& player);
	DLL Action<ShipId> GetShip(const std::variant<uint, std::wstring_view>& player);
	DLL Action<uint> GetShipID(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> MarkObj(const std::variant<uint, std::wstring_view>& player, uint objId, int markStatus);
	DLL Action<int> GetPvpKills(const std::variant<uint, std::wstring_view>& player);
	DLL Action<void> SetPvpKills(const std::variant<uint, std::wstring_view>& player, int killAmount);
	DLL Action<int> IncrementPvpKills(const std::variant<uint, std::wstring_view>& player);
	DLL Action<uint> GetSystemByNickname(std::variant<std::wstring_view, std::string_view> nickname);
	DLL CShip* CShipFromShipDestroyed(const DWORD** ecx);
}