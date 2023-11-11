#pragma once

class GroupId final
{
        uint value = 0;

        Action<void, Error> ForEachGroupMember(const std::function<std::optional<Error>(ClientId client)>& func, bool stopIfErr = true) const;

        template <typename T, T>
        struct Proxy;

        template <typename T, typename R, typename... Args, R (T::*MemberFunc)(Args...)>
        struct Proxy<R (T::*)(Args...), MemberFunc>
        {
                static R Call(T& obj, Args&&... args) { return (obj.*MemberFunc)(std::forward<Args>(args)...); }
        };

    public:
        explicit GroupId(const uint val) : value(val) {}
        explicit GroupId() = default;
        ~GroupId() = default;
        GroupId(const GroupId&) = default;
        GroupId& operator=(GroupId) = default;
        GroupId(GroupId&&) = default;
        GroupId& operator=(GroupId&&) = delete;

        bool operator==(const GroupId& next) const { return value == next.value; }
        explicit operator bool() const { return value != 0; }

        uint GetValue() const { return value; }

        Action<std::vector<ClientId>, Error> GetGroupMembers() const;
        Action<uint, Error> GetGroupSize() const;

        template <typename FunctionPtr, typename... Args>
            requires std::is_same_v<typename MemberFunctionPointerClassType<FunctionPtr>::type, ClientId>
        Action<void, Error> GroupAction(FunctionPtr ptr, bool stopIfErr, Args... args) const
        {
            return ForEachGroupMember([ptr, args](ClientId client) { Proxy<void (ClientId::*)(), ptr>::Call(client, args...); }, stopIfErr);
        }
};
