#pragma once

namespace Hk::Player
{
	DLL cpp::result<void, Error> AddToGroup(ClientId client, uint groupId);
	DLL cpp::result<const uint, Error> GetGroupID(ClientId client);
	DLL cpp::result<const uint, Error> GetCash(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> AddCash(const std::variant<uint, std::wstring>& player, uint amount);
	DLL cpp::result<void, Error> RemoveCash(const std::variant<uint, std::wstring>& player, uint amount);
	DLL cpp::result<void, Error> AdjustCash(const std::variant<uint, std::wstring>& player, int amount);
	DLL cpp::result<void, Error> Kick(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> KickReason(const std::variant<uint, std::wstring>& player, const std::wstring& reason);
	DLL cpp::result<void, Error> Ban(const std::variant<uint, std::wstring>& player, bool ban);
	DLL cpp::result<void, Error> TempBan(const std::variant<uint, std::wstring>& player, uint duration);
	DLL cpp::result<void, Error> Beam(const std::variant<uint, std::wstring>& player, const std::variant<uint, std::wstring>& base);
	DLL cpp::result<void, Error> SaveChar(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const std::list<CargoInfo>, Error> EnumCargo(const std::variant<uint, std::wstring>& player, int& remainingHoldSize);
	DLL cpp::result<void, Error> RemoveCargo(const std::variant<uint, std::wstring>& player, ushort cargoId, int count);
	DLL cpp::result<void, Error> AddCargo(const std::variant<uint, std::wstring>& player, uint goodId, int count, bool mission);
	DLL cpp::result<void, Error> AddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& good, int count, bool mission);
	DLL cpp::result<void, Error> Rename(const std::variant<uint, std::wstring>& player, const std::wstring& newCharname, bool onlyDelete);
	DLL cpp::result<void, Error> MsgAndKick(ClientId client, const std::wstring& Reason, uint interval);
	DLL cpp::result<void, Error> Kill(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<bool, Error> GetReservedSlot(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> SetReservedSlot(const std::variant<uint, std::wstring>& player, bool reservedSlot);
	DLL cpp::result<void, Error> ResetRep(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<std::vector<GroupMember>, Error> GetGroupMembers(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> SetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup, float value);
	DLL cpp::result<float, Error> GetRep(const std::variant<uint, std::wstring>& player, const std::variant<uint, std::wstring>& repGroup);
	DLL cpp::result<std::list<std::wstring>, Error> ReadCharFile(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> WriteCharFile(const std::variant<uint, std::wstring>& player, std::wstring data);
	DLL cpp::result<void, Error> PlayerRecalculateCRC(ClientId client);
	DLL std::string GetPlayerSystemS(ClientId client);
	DLL bool IsInRange(ClientId client, float distance);
	DLL cpp::result<void, Error> SetEquip(const std::variant<uint, std::wstring>& player, const st6::list<EquipDesc>& equip);
	DLL cpp::result<void, Error> AddEquip(const std::variant<uint, std::wstring>& player, uint goodId, const std::string& hardpoint);
	DLL cpp::result<void, Error> AntiCheat(ClientId client);
	DLL void DelayedKick(ClientId client, uint secs);
	DLL void DeleteCharacter(CAccount* acc, const std::wstring& character);
	DLL cpp::result<void, Error> NewCharacter(CAccount* acc, std::wstring& character);
	DLL cpp::result<int, Error> GetOnlineTime(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<int, Error> GetRank(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const uint, Error> GetShipValue(const std::variant<uint, std::wstring>& player);
	DLL void RelocateClient(ClientId client, Vector destination, const Matrix& orientation);
	DLL void SaveChar(ClientId client);
	DLL cpp::result<const ShipId, Error> GetTarget(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<ClientId, Error> GetTargetClientID(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const BaseId, Error> GetCurrentBase(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const SystemId, Error> GetSystem(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const ShipId, Error> GetShip(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const uint, Error> GetShipID(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> MarkObj(const std::variant<uint, std::wstring>& player, uint objId, int markStatus);
	DLL cpp::result<int, Error> GetPvpKills(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<void, Error> SetPvpKills(const std::variant<uint, std::wstring>& player, int killAmount);
	DLL cpp::result<int, Error> IncrementPvpKills(const std::variant<uint, std::wstring>& player);
	DLL cpp::result<const uint, Error> GetSystemByNickname(std::variant<std::string, std::wstring> nickname);
	DLL CShip* CShipFromShipDestroyed(const DWORD** ecx);
}