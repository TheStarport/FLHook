#include "PCH.hpp"

#include "API/Types/RepGroupId.hpp"

#include "API/FLHook/InfocardManager.hpp"
#include "API/Types/RepId.hpp"

RepGroupId::RepGroupId(std::wstring_view nickName) { pub::Reputation::GetReputationGroup(value, StringUtils::wstos(std::wstring(nickName)).c_str()); }

Action<std::wstring_view> RepGroupId::GetName() const
{
    uint ids;
    if (pub::Reputation::GetGroupName(value, ids) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    return { FLHook::GetInfocardManager()->GetInfocard(ids) };
}

Action<std::wstring_view> RepGroupId::GetShortName() const
{
    uint ids;
    if (static_cast<ResponseCode>(pub::Reputation::GetShortGroupName(value, ids)) != ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    return { FLHook::GetInfocardManager()->GetInfocard(ids) };
}

Action<float> RepGroupId::GetAttitudeTowardsRepId(RepId target) const
{
    float attitude;

    const auto ret = static_cast<ResponseCode>(pub::Reputation::GetGroupFeelingsTowards(value, target.GetValue(), attitude));
    if (ret == ResponseCode::InvalidInput)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    if (ret == ResponseCode::Failure)
    {
        return { cpp::fail(Error::InvalidRepInstance) };
    }

    return { attitude };
}
