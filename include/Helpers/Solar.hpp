#pragma once

namespace Hk::Solar
{
	DLL cpp::result<const SystemId, Error> GetSystemBySpaceId(uint spaceObjId);
	DLL cpp::result<std::pair<Vector, Matrix>, Error> GetLocation(uint id, IdType type);
	DLL cpp::result<float, Error> GetMass(uint spaceObjId);
	DLL cpp::result<std::pair<Vector, Vector>, Error> GetMotion(uint spaceObjId);
	DLL cpp::result<uint, Error> GetType(uint spaceObjId);
	DLL cpp::result<Universe::IBase*, Error> GetBaseByWildcard(const std::wstring& targetBaseName);
	DLL cpp::result<uint, Error> GetAffiliation(BaseId solarId);
	DLL cpp::result<float, Error> GetCommodityPrice(BaseId baseId, GoodId goodId);
}