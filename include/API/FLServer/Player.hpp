#pragma once

namespace Hk::Player
{
    DLL Action<void, Error> AddToGroup(ClientId client, uint groupId);
    DLL Action<uint, Error> GetGroupID(ClientId client);
    DLL Action<uint, Error> GetCash(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> AddCash(const std::variant<uint, std::wstring_view>& player, uint amount);
    DLL Action<void, Error> RemoveCash(const std::variant<uint, std::wstring_view>& player, uint amount);
    DLL Action<void, Error> AdjustCash(const std::variant<uint, std::wstring_view>& player, int amount);
    DLL Action<void, Error> Kick(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> KickReason(const std::variant<uint, std::wstring_view>& player, std::wstring_view reason);
    DLL Action<void, Error> Ban(const std::variant<uint, std::wstring_view>& player, bool ban);
    DLL Action<void, Error> TempBan(const std::variant<uint, std::wstring_view>& player, uint duration);
    DLL Action<void, Error> Beam(const std::variant<uint, std::wstring_view>& player, const std::variant<uint, std::wstring_view>& base);
    DLL Action<void, Error> SaveChar(const std::variant<uint, std::wstring_view>& player);
    DLL Action<std::list<CargoInfo>, Error> EnumCargo(const std::variant<uint, std::wstring_view>& player, int& remainingHoldSize);
    DLL Action<void, Error> RemoveCargo(const std::variant<uint, std::wstring_view>& player, ushort cargoId, int count);
    DLL Action<void, Error> AddCargo(const std::variant<uint, std::wstring_view>& player, uint goodId, int count, bool mission);
    DLL Action<void, Error> AddCargo(const std::variant<uint, std::wstring_view>& player, std::wstring_view good, int count, bool mission);
    DLL Action<void, Error> Rename(const std::variant<uint, std::wstring_view>& player, std::wstring_view newCharname, bool onlyDelete);
    DLL Action<void, Error> MsgAndKick(ClientId client, std::wstring_view Reason, uint interval);
    DLL Action<void, Error> Kill(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> ResetRep(const std::variant<uint, std::wstring_view>& player);
    DLL Action<std::vector<GroupMember>, Error> GetGroupMembers(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> SetRep(const std::variant<uint, std::wstring_view>& player, std::wstring_view repGroup, float value);
    DLL Action<float, Error> GetRep(const std::variant<uint, std::wstring_view>& player, const std::variant<uint, std::wstring_view>& repGroup);
    DLL Action<std::list<std::wstring>, Error> ReadCharFile(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> WriteCharFile(const std::variant<uint, std::wstring_view>& player, std::wstring data);
    DLL Action<void, Error> PlayerRecalculateCRC(ClientId client);
    DLL std::wstring GetPlayerSystemS(ClientId client);
    DLL bool IsInRange(ClientId client, float distance);
    DLL Action<void, Error> SetEquip(const std::variant<uint, std::wstring_view>& player, const st6::list<EquipDesc>& equip);
    DLL Action<void, Error> AddEquip(const std::variant<uint, std::wstring_view>& player, uint goodId, const std::wstring& hardpoint);
    DLL Action<void, Error> AntiCheat(ClientId client);
    DLL void DelayedKick(ClientId client, uint secs);
    DLL void DeleteCharacter(CAccount* acc, const std::wstring& character);
    DLL Action<void, Error> NewCharacter(CAccount* acc, std::wstring& character);
    DLL Action<int, Error> GetOnlineTime(const std::variant<uint, std::wstring_view>& player);
    DLL Action<int, Error> GetRank(const std::variant<uint, std::wstring_view>& player);
    DLL Action<uint, Error> GetShipValue(const std::variant<uint, std::wstring_view>& player);
    DLL void RelocateClient(ClientId client, Vector destination, const Matrix& orientation);
    DLL void SaveChar(ClientId client);
    DLL Action<ShipId, Error> GetTarget(const std::variant<uint, std::wstring_view>& player);
    DLL Action<ClientId, Error> GetTargetClientID(const std::variant<uint, std::wstring_view>& player);
    DLL Action<BaseId, Error> GetCurrentBase(const std::variant<uint, std::wstring_view>& player);
    DLL Action<SystemId, Error> GetSystem(const std::variant<uint, std::wstring_view>& player);
    DLL Action<ShipId, Error> GetShip(const std::variant<uint, std::wstring_view>& player);
    DLL Action<uint, Error> GetShipID(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> MarkObj(const std::variant<uint, std::wstring_view>& player, uint objId, int markStatus);
    DLL Action<int, Error> GetPvpKills(const std::variant<uint, std::wstring_view>& player);
    DLL Action<void, Error> SetPvpKills(const std::variant<uint, std::wstring_view>& player, int killAmount);
    DLL Action<int, Error> IncrementPvpKills(const std::variant<uint, std::wstring_view>& player);
    DLL Action<uint, Error> GetSystemByNickname(std::variant<std::wstring_view, std::string_view> nickname);
    DLL CShip* CShipFromShipDestroyed(const DWORD** ecx);
} // namespace Hk::Player
