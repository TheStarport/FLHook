#pragma once

namespace Hk::Solar
{
    DLL Action<SystemId> GetSystemBySpaceId(uint spaceObjId);
    DLL Action<std::pair<Vector, Matrix>> GetLocation(uint id, IdType type);
    DLL Action<float> GetMass(uint spaceObjId);
    DLL Action<std::pair<Vector, Vector>> GetMotion(uint spaceObjId);
    DLL Action<uint> GetType(uint spaceObjId);
    DLL Action<Universe::IBase*> GetBaseByWildcard(const std::wstring& targetBaseName);
    DLL Action<uint> GetAffiliation(BaseId solarId);
    DLL Action<float> GetCommodityPrice(BaseId baseId, GoodId goodId);
} // namespace Hk::Solar
