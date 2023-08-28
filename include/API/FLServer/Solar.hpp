#pragma once

namespace Hk::Solar
{
    DLL Action<SystemId, Error> GetSystemBySpaceId(uint spaceObjId);
    DLL Action<std::pair<Vector, Matrix>, Error> GetLocation(uint id, IdType type);
    DLL Action<float, Error> GetMass(uint spaceObjId);
    DLL Action<std::pair<Vector, Vector>, Error> GetMotion(uint spaceObjId);
    DLL Action<uint, Error> GetType(uint spaceObjId);
    DLL Action<Universe::IBase*, Error> GetBaseByWildcard(std::wstring_view targetBaseName);
    DLL Action<uint, Error> GetAffiliation(BaseId solarId);
    DLL Action<float, Error> GetCommodityPrice(BaseId baseId, GoodId goodId);
    DLL Action<bool, Error> ExistsAndAlive(uint id);
} // namespace Hk::Solar
