#include "PCH.hpp"

#include "API/FLServer/Chat.hpp"
#include "API/FLServer/Solar.hpp"

namespace Hk::Solar
{
    Action<SystemId, Error> GetSystemBySpaceId(uint spaceObjId)
    {
        uint system;
        pub::SpaceObj::GetSystem(spaceObjId, system);
        if (!system)
        {
            return { cpp::fail(Error::InvalidSystem) };
        }

        return { system };
    }

    Action<std::pair<Vector, Matrix>, Error> GetLocation(uint id, IdType type)
    {
        switch (type)
        {
            case IdType::Client:
                {
                    uint ship = 0;
                    pub::Player::GetShip(id, ship);
                    if (!ship)
                    {
                        return { cpp::fail(Error::PlayerNotInSpace) };
                    }
                    id = ship;
                }
                [[fallthrough]];
            case IdType::Solar:
                {
                    Vector pos;
                    Matrix rot;
                    pub::SpaceObj::GetLocation(id, pos, rot);
                    return { std::make_pair(pos, rot) };
                }
            default: return { cpp::fail(Error::InvalidIdType) };
        }
    }

    Action<float, Error> GetMass(uint spaceObjId)
    {
        uint system;
        pub::SpaceObj::GetSystem(spaceObjId, system);
        if (!system)
        {
            return { cpp::fail(Error::InvalidSpaceObjId) };
        }
        float mass;
        pub::SpaceObj::GetMass(spaceObjId, mass);

        return { mass };
    }

    Action<std::pair<Vector, Vector>, Error> GetMotion(uint spaceObjId)
    {
        uint system;
        pub::SpaceObj::GetSystem(spaceObjId, system);
        if (!system)
        {
            return { cpp::fail(Error::InvalidSpaceObjId) };
        }
        Vector v1;
        Vector v2;
        pub::SpaceObj::GetMotion(spaceObjId, v1, v2);
        return { std::make_pair(v1, v2) };
    }

    Action<uint, Error> GetType(uint spaceObjId)
    {
        uint system;
        pub::SpaceObj::GetSystem(spaceObjId, system);
        if (!system)
        {
            return { cpp::fail(Error::InvalidSpaceObjId) };
        }
        uint type;
        pub::SpaceObj::GetType(spaceObjId, type);
        return { type };
    }

    Action<Universe::IBase*, Error> GetBaseByWildcard(std::wstring_view targetBaseName)
    {
        // Search for an exact match at the start of the name
        Universe::IBase* baseinfo = Universe::GetFirstBase();
        while (baseinfo)
        {
            char baseNickname[1024] = "";
            pub::GetBaseNickname(baseNickname, sizeof baseNickname, baseinfo->baseId);

            if (const std::wstring basename = Chat::GetWStringFromIdS(baseinfo->baseIdS);
                StringUtils::ToLower(StringUtils::stows(baseNickname)) == StringUtils::ToLower(targetBaseName) ||
                StringUtils::ToLower(basename).find(StringUtils::ToLower(targetBaseName)) == 0)
            {
                return { baseinfo };
            }
            baseinfo = Universe::GetNextBase();
        }

        // Exact match failed, try a for an partial match
        baseinfo = Universe::GetFirstBase();
        while (baseinfo)
        {
            if (const std::wstring basename = Chat::GetWStringFromIdS(baseinfo->baseIdS);
                StringUtils::ToLower(basename).find(StringUtils::ToLower(targetBaseName)) != -1)
            {
                return { baseinfo };
            }
            baseinfo = Universe::GetNextBase();
        }
        return { cpp::fail(Error::InvalidBase) };
    }

    Action<uint, Error> GetAffiliation(BaseId solarId)
    {
        int solarRep;
        pub::SpaceObj::GetSolarRep(solarId, solarRep);
        if (solarRep == -1)
        {
            return { cpp::fail(Error::InvalidBase) };
        }

        uint baseAff;
        pub::Reputation::GetAffiliation(solarRep, baseAff);
        if (baseAff == UINT_MAX)
        {
            return { cpp::fail(Error::InvalidRepGroup) };
        }
        return { baseAff };
    }

    Action<float, Error> GetCommodityPrice(BaseId baseId, GoodId goodId)
    {
        float nomPrice;
        pub::Market::GetNominalPrice(goodId, nomPrice);
        if (nomPrice == 0.0f)
        {
            return { cpp::fail(Error::InvalidGood) };
        }

        float price;
        pub::Market::GetPrice(baseId, goodId, price);
        if (price == -1.0f)
        {
            return { cpp::fail(Error::InvalidBase) };
        }
        return { price };
    }
} // namespace Hk::Solar
