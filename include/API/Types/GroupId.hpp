#pragma once

class DLL GroupId final
{
        uint value = 0;

    public:
        explicit GroupId(const uint val) : value(val) {}
        explicit GroupId() = default;
        ~GroupId() = default;

        bool operator==(const GroupId& next) const { return value == next.value; }
        explicit operator bool() const { return value != 0; }

        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        [[nodiscard]]
        Action<std::vector<ClientId>> GetGroupMembers() const;
        [[nodiscard]]
        Action<uint> GetGroupSize() const;
        Action<void> ForEachGroupMember(const std::function<std::optional<Error>(ClientId client)>& func, bool stopIfErr = true) const;
        Action<void> InviteMember(ClientId client) const;
        Action<void> AddMember(ClientId client) const;
        Action<void> RemoveMember(ClientId client) const;
};

template <>
struct std::formatter<GroupId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const GroupId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
