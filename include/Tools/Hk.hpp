#pragma once

#include "Typedefs.hpp"
#include "Enums.hpp"
#include "Constexpr.hpp"
#include "Macros.hpp"

extern DLL st6_malloc_t st6_malloc;
extern DLL st6_free_t st6_free;
#define ST6_ALLOCATION_DEFINED

#include "FLCore/FLCoreCommon.h"
#include "FLCore/FLCoreServer.h"
#include "FLCore/FLCoreRemoteClient.h"
#include "FLCore/FLCoreDALib.h"

#include <ext/Singleton.h>

#include "Structs.hpp"
#include "Concepts.hpp"

#include "Deps.hpp"
struct CARGO_INFO;

#pragma once

namespace Hk
{
	namespace Time
	{
		DLL uint GetUnixSeconds();
		DLL uint GetUnixMiliseconds();

		template<typename T>
		std::chrono::microseconds ToMicroseconds(T duration)
		{
			return std::chrono::duration_cast<std::chrono::microseconds>(duration);
		}

		template<typename T>
		std::chrono::milliseconds ToMilliseconds(T duration)
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		}

		template<typename T>
		std::chrono::seconds ToSeconds(T duration)
		{
			return std::chrono::duration_cast<std::chrono::seconds>(duration);
		}

		template<typename T>
		std::chrono::minutes ToMinutes(T duration)
		{
			return std::chrono::duration_cast<std::chrono::minutes>(duration);
		}

		template<typename T>
		std::chrono::hours ToHours(T duration)
		{
			return std::chrono::duration_cast<std::chrono::hours>(duration);
		}

		template<typename T>
		std::chrono::nanoseconds ToNanoseconds(T duration)
		{
			return std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
		}
	} // namespace Time

	namespace Client
	{
		/**
		 * Gets the current client id of the account.
		 * @param acc CAccount
		 * @returns On success : Client id of the active user of the account.
		 * @returns On fail : [PlayerNotLoggedIn] The function could not find a client id associated with the account id.
		 */
		DLL cpp::result<const uint, Error> GetClientIdFromAccount(const CAccount* acc);

		DLL std::wstring GetAccountIdByClientID(ClientId client);

		DLL CAccount* GetAccountByClientID(ClientId client);

		/**
		 * Gets the current client id of the character.
		 * @param character Wide string of the character name
		 * @returns On success : current client Id associated with that character name
		 * @returns On fail : [CharacterDoesNotExist] The function could not find a client id associated with this character name.
		 */
		DLL cpp::result<const uint, Error> GetClientIdFromCharName(const std::wstring& character);

		/**
		 * Checks to see if the client Id is valid
		 * @param client Client Id
		 * @returns If Valid: true
		 * @returns If Not Valid: false
		 */
		DLL bool IsValidClientID(ClientId client);

		/**
		 * Gets the account of the character
		 * @param character Wide string of the character name
		 * @returns On success : the account Id for that character
		 * @returns On fail : [CharacterDoesNotExist] The function could not find the account id associated with this character name.
		 */
		DLL cpp::result<CAccount*, Error> GetAccountByCharName(const std::wstring& character);

		/**
		 * Gets the account id in a wide string
		 * @param acc The account
		 * @returns On success : wide string of account Id
		 * @returns On fail : [CannotGetAccount] The function could not find the account.
		 */
		DLL cpp::result<const std::wstring, Error> GetAccountID(CAccount* acc);

		/**
		 * Gets the account id in a wide string
		 * @param fileName The account
		 * @returns On success : wide string of account Id
		 * @returns On fail : [CannotGetAccount] The function could not find the account.
		 */
		DLL bool IsEncoded(const std::string& fileName);

		DLL bool IsInCharSelectMenu(const uint& player);

		DLL cpp::result<const std::wstring, Error> GetCharacterNameByID(ClientId& client);
		DLL cpp::result<const uint, Error> ResolveID(const std::wstring& player);
		DLL cpp::result<ClientId, Error> ResolveShortCut(const std::wstring& wscShortcut);
		DLL cpp::result<ClientId, Error> GetClientIdByShip(ShipId ship);
		DLL std::wstring GetAccountDirName(const CAccount* acc);
		DLL cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring>& player, bool returnValueIfNoFile);
		DLL cpp::result<const std::wstring, Error> GetBaseNickByID(uint baseId);
		DLL cpp::result<const std::wstring, Error> GetPlayerSystem(ClientId client);
		DLL cpp::result<const std::wstring, Error> GetSystemNickByID(uint systemId);
		DLL std::vector<uint> getAllPlayersInSystem(SystemId system);
		DLL cpp::result<void, Error> LockAccountAccess(CAccount* acc, bool bKick);
		DLL cpp::result<void, Error> UnlockAccountAccess(CAccount* acc);
		DLL cpp::result<void, Error> PlaySoundEffect(ClientId client, uint soundId);
		DLL void GetItemsForSale(uint baseId, std::list<uint>& lstItems);
		DLL cpp::result<IObjInspectImpl*, Error> GetInspect(ClientId client);
		DLL EngineState GetEngineState(ClientId client);
		DLL EquipmentType GetEqType(Archetype::Equipment* eq);
	} // namespace Client

	namespace Math
	{
		/**
		 * Computes the difference between two points in 3d space,
		 * @param v1 3d vector 1
		 * @param v2 3d vector 2
		 * @returns a scalar of the distance between point v2 and v1
		 */
		DLL float Distance3D(Vector v1, Vector v2);

		/**
		 * See Distance3D for more information
		 * @see Distance3D
		 */
		DLL cpp::result<float, Error> Distance3DByShip(uint ship1, uint ship2);

		/**
		 * Converts a 3x3 rotation matrix into an equivalent quaternion.
		 */
		DLL Quaternion MatrixToQuaternion(const Matrix& m);
		template<typename Str>
		Str VectorToSectorCoord(uint systemId, Vector vPos);
		template DLL std::string VectorToSectorCoord(uint systemId, Vector vPos);
		template DLL std::wstring VectorToSectorCoord(uint systemId, Vector vPos);
		DLL float Degrees(float rad);
		DLL Vector MatrixToEuler(const Matrix& mat);
		DLL uint RgbToBgr(uint color);
		DLL std::wstring UintToHexString(uint number, uint width, bool addPrefix = false);
	} // namespace Math

	namespace Message
	{
		DLL cpp::result<void, Error> Msg(const std::variant<uint, std::wstring>& player, const std::wstring& wscMessage);
		DLL cpp::result<void, Error> MsgS(const std::variant<std::wstring, uint>& system, const std::wstring& wscMessage);
		DLL cpp::result<void, Error> MsgU(const std::wstring& wscMessage);
		DLL cpp::result<void, Error> FMsgEncodeXML(const std::wstring& wscXml, char* szBuf, uint iSize, uint& iRet);
		DLL void FMsgSendChat(ClientId client, char* szBuf, uint iSize);
		DLL cpp::result<void, Error> FMsg(ClientId client, const std::wstring& wscXML);
		DLL cpp::result<void, Error> FMsg(const std::variant<uint, std::wstring>& player, const std::wstring& wscXML);
		DLL cpp::result<void, Error> FMsgS(const std::variant<std::wstring, uint>& system, const std::wstring& wscXML);
		DLL cpp::result<void, Error> FMsgU(const std::wstring& wscXML);
		DLL std::wstring FormatMsg(MessageColor color, MessageFormat format, const std::wstring& msg);
		DLL std::wstring GetWStringFromIdS(uint iIdS);
		DLL void LoadStringDLLs();
		DLL void UnloadStringDLLs();
		DLL cpp::result<void, Error> FormatSendChat(uint iToClientId, const std::wstring& wscSender, const std::wstring& text, const std::wstring& textColor);
		DLL void SendGroupChat(uint iFromClientId, const std::wstring& text);
		DLL void SendLocalSystemChat(uint iFromClientId, const std::wstring& text);
		DLL cpp::result<void, Error> SendPrivateChat(uint iFromClientId, uint iToClientId, const std::wstring& text);
		DLL void SendSystemChat(uint iFromClientId, const std::wstring& text);
	} // namespace Message

	namespace Player
	{
		DLL cpp::result<void, Error> AddToGroup(ClientId client, uint iGroupId);
		DLL cpp::result<const uint, Error> GetGroupID(ClientId client);
		DLL cpp::result<const uint, Error> GetCash(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> AddCash(const std::variant<uint, std::wstring>& player, uint uAmount);
		DLL cpp::result<void, Error> RemoveCash(const std::variant<uint, std::wstring>& player, uint uAmount);
		DLL cpp::result<void, Error> AdjustCash(const std::variant<uint, std::wstring>& player, int iAmount);
		DLL cpp::result<void, Error> Kick(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> KickReason(const std::variant<uint, std::wstring>& player, const std::wstring& wscReason);
		DLL cpp::result<void, Error> Ban(const std::variant<uint, std::wstring>& player, bool bBan);
		DLL cpp::result<void, Error> TempBan(const std::variant<uint, std::wstring>& player, uint duration);
		DLL cpp::result<void, Error> Beam(const std::variant<uint, std::wstring>& player, const std::variant<uint, std::wstring>& base);
		DLL cpp::result<void, Error> SaveChar(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<const std::list<CARGO_INFO>, Error> EnumCargo(const std::variant<uint, std::wstring>& player, int& iRemainingHoldSize);
		DLL cpp::result<void, Error> RemoveCargo(const std::variant<uint, std::wstring>& player, ushort cargoId, int count);
		DLL cpp::result<void, Error> AddCargo(const std::variant<uint, std::wstring>& player, uint iGoodId, int iCount, bool bMission);
		DLL cpp::result<void, Error> AddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& wscGood, int iCount, bool bMission);
		DLL cpp::result<void, Error> Rename(const std::variant<uint, std::wstring>& player, const std::wstring& wscNewCharname, bool bOnlyDelete);
		DLL cpp::result<void, Error> MsgAndKick(ClientId client, const std::wstring& wscReason, uint iIntervall);
		DLL cpp::result<void, Error> Kill(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<bool, Error> GetReservedSlot(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> SetReservedSlot(const std::variant<uint, std::wstring>& player, bool bReservedSlot);
		DLL cpp::result<void, Error> ResetRep(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<std::vector<GroupMember>, Error> GetGroupMembers(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> SetRep(const std::variant<uint, std::wstring>& player, const std::wstring& wscRepGroup, float fValue);
		DLL cpp::result<float, Error> GetRep(const std::variant<uint, std::wstring>& player, const std::variant<uint, std::wstring>& repGroup);
		DLL cpp::result<std::list<std::wstring>, Error> ReadCharFile(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> WriteCharFile(const std::variant<uint, std::wstring>& player, std::wstring wscData);
		DLL cpp::result<void, Error> PlayerRecalculateCRC(ClientId client);
		DLL std::string GetPlayerSystemS(ClientId client);
		DLL bool IsInRange(ClientId client, float fDistance);
		DLL cpp::result<void, Error> SetEquip(const std::variant<uint, std::wstring>& player, const st6::list<EquipDesc>& equip);
		DLL cpp::result<void, Error> AddEquip(const std::variant<uint, std::wstring>& player, uint iGoodId, const std::string& scHardpoint);
		DLL cpp::result<void, Error> AntiCheat(ClientId client);
		DLL void DelayedKick(ClientId client, uint secs);
		DLL void DeleteCharacter(CAccount* acc, const std::wstring& character);
		DLL cpp::result<void, Error> NewCharacter(CAccount* acc, std::wstring& character);
		DLL cpp::result<int, Error> GetOnlineTime(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<int, Error> GetRank(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<const uint, Error> GetShipValue(const std::variant<uint, std::wstring>& player);
		DLL void RelocateClient(ClientId client, Vector vDestination, const Matrix& mOrientation);
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
	} // namespace Player

	namespace Solar
	{
		DLL cpp::result<const SystemId, Error> GetSystemBySpaceId(uint spaceObjId);
		DLL cpp::result<std::pair<Vector, Matrix>, Error> GetLocation(uint id, IdType type);
		DLL cpp::result<float, Error> GetMass(uint spaceObjId);
		DLL cpp::result<std::pair<Vector, Vector>, Error> GetMotion(uint spaceObjId);
		DLL cpp::result<uint, Error> GetType(uint spaceObjId);
		DLL cpp::result<Universe::IBase*, Error> GetBaseByWildcard(const std::wstring& targetBaseName);
		DLL cpp::result<uint, Error> GetAffiliation(BaseId solarId);
		DLL cpp::result<float, Error> GetCommodityPrice(BaseId baseId, GoodId goodId);
	} // namespace Solar

	namespace Ini
	{
		DLL cpp::result<std::wstring, Error> GetFromPlayerFile(const std::variant<uint, std::wstring>& player, const std::wstring& wscKey);
		DLL cpp::result<void, Error> WriteToPlayerFile(
		    const std::variant<uint, std::wstring>& player, const std::wstring& wscKey, const std::wstring& wscValue);

		DLL void SetCharacterIni(ClientId client, const std::wstring& name, std::wstring value);
		DLL std::wstring GetCharacterIniString(ClientId client, const std::wstring& name);
		DLL bool GetCharacterIniBool(ClientId client, const std::wstring& name);
		DLL int GetCharacterIniInt(ClientId client, const std::wstring& name);
		DLL uint GetCharacterIniUint(ClientId client, const std::wstring& name);
		DLL float GetCharacterIniFloat(ClientId client, const std::wstring& name);
		DLL double GetCharacterIniDouble(ClientId client, const std::wstring& name);
		DLL int64_t GetCharacterIniInt64(ClientId client, const std::wstring& name);
	} // namespace Ini

	namespace Admin
	{
		DLL std::wstring GetPlayerIP(ClientId client);
		DLL cpp::result<PlayerInfo, Error> GetPlayerInfo(const std::variant<uint, std::wstring>& player, bool bAlsoCharmenu);
		DLL std::list<PlayerInfo> GetPlayers();
		DLL cpp::result<DPN_CONNECTION_INFO, Error> GetConnectionStats(ClientId client);
		DLL cpp::result<void, Error> SetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& wscRights);
		DLL cpp::result<std::wstring, Error> GetAdmin(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> DelAdmin(const std::variant<uint, std::wstring>& player);
		DLL cpp::result<void, Error> ChangeNPCSpawn(bool bDisable);
		DLL cpp::result<BaseHealth, Error> GetBaseStatus(const std::wstring& wscBasename);
		DLL Fuse* GetFuseFromID(uint iFuseId);
		DLL bool LightFuse(IObjRW* ship, uint iFuseId, float fDelay, float fLifetime, float fSkip);
		DLL bool UnLightFuse(IObjRW* ship, uint iFuseId);
		DLL CEqObj* GetEqObjFromObjRW(struct IObjRW* objRW);
	} // namespace Admin

	namespace Err
	{
		DLL std::wstring ErrGetText(Error Err);
	}

	namespace Personalities
	{
		DLL cpp::result<pub::AI::Personality, Error> GetPersonality(const std::string& pilotNickname);
	}

	namespace ZoneUtilities
	{
		DLL void ReadUniverse(std::multimap<uint, LootableZone, std::less<>>* zones);
		DLL void ReadLootableZone(
		    std::multimap<uint, LootableZone, std::less<>>& zones, const std::string& systemNick, const std::string& defaultZoneNick, const std::string& file);
		DLL void ReadSystemLootableZones(std::multimap<uint, LootableZone, std::less<>>& zones, const std::string& systemNick, const std::string& file);
		DLL void ReadSystemZones(std::multimap<uint, LootableZone, std::less<>>& zones, const std::string& systemNick, const std::string& file);
		DLL bool InZone(uint systemId, const Vector& pos, Zone& rlz);
		DLL bool InDeathZone(uint systemId, const Vector& pos, Zone& rlz);
		DLL SystemInfo* GetSystemInfo(uint systemId);
		DLL void PrintZones();
	} // namespace ZoneUtilities
} // namespace Hk