#include "PCH.hpp"

#include "API/Types/RepId.hpp"

RepId::RepId(const ObjectId &spaceObj, const bool isSolar)
{
    if (isSolar)
    {
        pub::SpaceObj::GetSolarRep(spaceObj.GetValue(), value);
    }
    else
    {
        pub::SpaceObj::GetRep(spaceObj.GetValue(), value);
    }
}

RepId::operator bool() const { return value != 0; }

Action<RepGroupId, Error> RepId::GetAffiliation() const
{
    uint group;
    if (pub::Reputation::GetAffiliation(value, group) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::NoAffiliation) };
    }

    return { RepGroupId(group) };
}

Action<float, Error> RepId::GetAttitudeTowardsRepId(const RepId &target) const
{
    float ret;
    if (pub::Reputation::GetAttitude(value, target.value, ret) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { ret };
}

Action<float, Error> RepId::GetAttitudeTowardsFaction(const RepGroupId &group) const
{
    float ret;
    if (pub::Reputation::GetGroupFeelingsTowards(value, group.GetValue(), ret) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { ret };
}

Action<int, Error> RepId::GetRank() const
{
    float ret;
    if (pub::Reputation::GetRank(value, ret) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { static_cast<int>(ret) };
}

Action<std::pair<FmtStr, FmtStr>, Error> RepId::GetName() const
{
    FmtStr pilot, scanner;
    if (pub::Reputation::GetName(value, pilot, scanner) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { std::make_pair<FmtStr, FmtStr>(*pilot, *scanner) };
}

Action<void, Error> RepId::SetRank(const int rank) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetRank(value, static_cast<float>(rank)));
    if (ret == ResponseCode::InvalidInput)
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    if (ret == ResponseCode::Failure)
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { {} };
}

Action<void, Error> RepId::SetAttitudeTowardsRepId(RepId target, float newAttitude) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetAttitude(value, target.value, newAttitude));
    if (ret == ResponseCode::InvalidInput)
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    if (ret == ResponseCode::Failure)
    {
        return { cpp::fail(Error::InvalidReputation) };
    }

    return { {} };
}

Action<void, Error> RepId::SetAffiliation(RepGroupId group) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetAffiliation(value, group.GetValue()));
    if (ret == ResponseCode::InvalidInput)
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    if (ret == ResponseCode::Failure)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    return { {} };
}
