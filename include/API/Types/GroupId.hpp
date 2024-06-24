#pragma once

class DLL GroupId final
{
        uint value = 0;

    public:
        explicit GroupId(const uint val) : value(val) {}
        explicit GroupId() = default;
        ~GroupId() = default;
        GroupId(const GroupId&) = default;
        GroupId& operator=(GroupId) = delete;
        GroupId(GroupId&&) = default;
        GroupId& operator=(GroupId&&) = delete;

        bool operator==(const GroupId& next) const { return value == next.value; }
        explicit operator bool() const { return value != 0; }

        uint GetValue() const { return value; }

        Action<std::vector<ClientId>, Error> GetGroupMembers() const;
        Action<uint, Error> GetGroupSize() const;
        Action<void, Error> ForEachGroupMember(const std::function<std::optional<Error>(ClientId client)>& func, bool stopIfErr = true) const;
        Action<void, Error> InviteMember(ClientId client);
        Action<void, Error> AddMember(ClientId client);
};

template <>
struct std::formatter<GroupId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const GroupId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
