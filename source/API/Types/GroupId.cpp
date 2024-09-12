#include "PCH.hpp"

#include "API/Types/GroupId.hpp"

Action<void> GroupId::ForEachGroupMember(const std::function<std::optional<Error>(ClientId client)> &func, const bool stopIfErr) const
{
    auto members = GetGroupMembers();
    if (members.Raw().has_error())
    {
        return { cpp::fail(members.Raw().error()) };
    }

    for (const auto& member : members.Unwrap())
    {
        if (auto err = func(ClientId(member)); err.has_value() && stopIfErr)
        {
            return { cpp::fail(err.value()) };
        }
    }

    return { {} };
}

Action<std::vector<ClientId>> GroupId::GetGroupMembers() const
{
    const auto group = CPlayerGroup::FromGroupID(value);
    if (!group)
    {
        return { cpp::fail(Error::InvalidGroupId) };
    }

    st6::vector<uint> members;
    group->StoreMemberList(members);

    std::vector<ClientId> ret(members.begin(), members.end());
    return { ret };
}

Action<uint> GroupId::GetGroupSize() const
{
    const auto group = CPlayerGroup::FromGroupID(value);
    if (!group)
    {
        return { cpp::fail(Error::InvalidGroupId) };
    }

    return { group->GetMemberCount() };
}

Action<void> GroupId::InviteMember(ClientId client)
{
    const auto group = CPlayerGroup::FromGroupID(value);
    if (!group)
    {
        return { cpp::fail(Error::InvalidGroupId) };
    }

    group->AddInvite(client.GetValue());
    return { {} };
}

Action<void> GroupId::AddMember(ClientId client)
{
    const auto group = CPlayerGroup::FromGroupID(value);
    if (!group)
    {
        return { cpp::fail(Error::InvalidGroupId) };
    }

    group->AddMember(client.GetValue());
    return { {} };
}
