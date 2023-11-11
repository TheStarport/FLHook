#include "PCH.hpp"

#include "API/Types/RepGroupId.hpp"

Action<std::wstring_view, Error> RepGroupId::GetName() const
{
    uint ids;
    if (pub::Reputation::GetGroupName(value, ids) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    return { FLHook::GetInfocardManager().GetInfocard(ids) };
}

Action<std::wstring_view, Error> RepGroupId::GetShortName() const
{
    uint ids;
    if (pub::Reputation::GetShortGroupName(value, ids) != (int)ResponseCode::Success)
    {
        return { cpp::fail(Error::InvalidRepGroup) };
    }

    return { FLHook::GetInfocardManager().GetInfocard(ids) };
}
