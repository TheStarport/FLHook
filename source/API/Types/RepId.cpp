#include "PCH.hpp"

#include "API/Types/RepId.hpp"

RepId::operator bool() const { return value != 0; }
int RepId::GetValue() const { return value; }

Action<RepGroupId> RepId::GetAffiliation() const
{
    uint group;
    if (pub::Reputation::GetAffiliation(value, group) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::NoAffiliation) };
    }

    return { RepGroupId(group) };
}

Action<float> RepId::GetAttitudeTowardsRepId(const RepId& target) const
{
    float ret;
    if (pub::Reputation::GetAttitude(value, target.value, ret) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { ret };
}

Action<float> RepId::GetAttitudeTowardsFaction(const RepGroupId& group) const
{
    float ret;
    if (pub::Reputation::GetGroupFeelingsTowards(value, group.GetValue(), ret) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { ret };
}

Action<int> RepId::GetRank() const
{
    float ret;
    if (pub::Reputation::GetRank(value, ret) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { static_cast<int>(ret) };
}

Action<std::pair<FmtStr, FmtStr>> RepId::GetName() const
{
    FmtStr pilot, scanner;
    if (pub::Reputation::GetName(value, pilot, scanner) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { std::make_pair(pilot, scanner) };
}

Action<void> RepId::SetRank(const int rank) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetRank(value, static_cast<float>(rank)));
    if (ret != ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { {} };
}

Action<void> RepId::SetAttitudeTowardsRepId(RepId target, float newAttitude) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetAttitude(value, target.value, newAttitude));
    if (ret != ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { {} };
}

Action<void> RepId::SetAttitudeTowardsRepGroupId(RepGroupId target, float newAttitude) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetReputation(value, target.GetValue(), newAttitude));
    if (ret == ResponseCode::InvalidInput)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    if (ret == ResponseCode::Failure)
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }
    return { {} };
}

Action<void> RepId::SetAffiliation(RepGroupId group) const
{
    const auto ret = static_cast<ResponseCode>(pub::Reputation::SetAffiliation(value, group.GetValue()));
    if (ret == ResponseCode::InvalidInput)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    if (ret == ResponseCode::Failure)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    return { {} };
}
