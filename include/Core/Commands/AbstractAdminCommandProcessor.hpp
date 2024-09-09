#pragma once

#include "API/FLHook/TaskScheduler.hpp"

using namespace std::string_view_literals;

enum class DefaultRoles
{
    SuperAdmin,
    Any,
    Cash,
    Expel,
    Reputation,
    Info,
    Cargo,
    Message,
    Character,
    Plugin,
    Movement
};

enum class AllowedContext
{
    Reset = 0,
    GameOnly = 1,
    ConsoleOnly = 2,
    GameAndConsole = GameOnly | ConsoleOnly,
};

template <class Processor>
struct AdminCommandInfo
{
        std::vector<std::wstring_view> cmd;
        Task (*func)(Processor* cl, std::vector<std::wstring_view>& params);
        AllowedContext allowedContext;
        std::wstring_view requiredRole;
        std::wstring_view usage;
        std::wstring_view description;
};

class AbstractAdminCommandProcessor
{
    protected:
        // Current user, changes every command invocation.
        AllowedContext currentContext = AllowedContext::GameOnly;

        [[nodiscard]]
        cpp::result<void, std::wstring> Validate(ClientId client, const AllowedContext context, std::wstring_view requiredRole) const
        {
            using namespace magic_enum::bitwise_operators;
            const static std::wstring invalidPerms = L"ERR: No permission.";
            const static std::wstring invalidCommand = L"ERR: Command not found or invalid context.";
            static constexpr std::wstring_view superAdminRole = magic_enum::enum_name(DefaultRoles::SuperAdmin);

            // If the current context does not allow command
            if (static_cast<int>(currentContext & context) == 0)
            {
                return cpp::fail(invalidCommand);
            }

            auto& credMap = FLHook::GetAdmins();
            const auto credentials = credMap.find(std::wstring(client.GetCharacterName().Handle()));
            if (credentials == credMap.end())
            {
                // Some how got here and not authenticated!
                return cpp::fail(invalidPerms);
            }

            // If they have a role and the required role is 'any' then we are all good
            if (requiredRole == magic_enum::enum_name(DefaultRoles::Any))
            {
                return {};
            }

            if (std::ranges::find(credentials->second, requiredRole) == credentials->second.end() &&
                std::ranges::find(credentials->second, superAdminRole) == credentials->second.end())
            {
                return cpp::fail(invalidPerms);
            }

            // All good!
            return {};
        }

    public:
        virtual ~AbstractAdminCommandProcessor() = default;
        virtual std::optional<Task> ProcessCommand(ClientId user, AllowedContext currentContext, std::wstring_view cmd,
                                                   std::vector<std::wstring_view>& paramVector) = 0;
        virtual const std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>>& GetAdminCommands() const = 0;
};

template <class T>
concept IsAdminCommandProcessor = std::is_base_of_v<AbstractAdminCommandProcessor, T>;

#define AddAdminCommand(cls, str, func, context, requiredRole, usage, description)                                                                             \
    {                                                                                                                                                          \
        str, ClassFunctionWrapper<decltype(&cls::func), &cls::func>::ProcessParam, AllowedContext::context, magic_enum::enum_name(DefaultRoles::requiredRole), \
            usage, description                                                                                                                                 \
    }

#define GetAdminCommandsFunc(commands)                                                                                                     \
    const std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>>& GetAdminCommands() const override \
    {                                                                                                                                      \
        static std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> info;                         \
        if (info.empty())                                                                                                                  \
        {                                                                                                                                  \
            for (const auto& cmd : commands)                                                                                               \
            {                                                                                                                              \
                info.emplace_back(cmd.cmd, cmd.usage, cmd.description);                                                                    \
            }                                                                                                                              \
        }                                                                                                                                  \
                                                                                                                                           \
        return info;                                                                                                                       \
    };

#define SetupAdminCommandHandler(class, commandArray)                                                                                                        \
    template <int N>                                                                                                                                         \
    std::optional<Task> MatchCommand(class* processor, const std::wstring_view cmd, std::vector<std::wstring_view>& paramVector)                             \
    {                                                                                                                                                        \
        const AdminCommandInfo<class>& command = std::get<N - 1>(class ::commandArray);                                                                      \
        for (auto& str : command.cmd)                                                                                                                        \
        {                                                                                                                                                    \
            if (cmd.starts_with(str))                                                                                                                        \
            {                                                                                                                                                \
                paramVector.erase(paramVector.begin(), paramVector.begin() + (std::clamp(std::ranges::count(str, L' '), 1, 5)));                             \
                return command.func(processor, paramVector);                                                                                                 \
            }                                                                                                                                                \
        }                                                                                                                                                    \
                                                                                                                                                             \
        return MatchCommand<N - 1>(processor, cmd, paramVector);                                                                                             \
    }                                                                                                                                                        \
                                                                                                                                                             \
    template <>                                                                                                                                              \
    std::optional<Task> MatchCommand<0>(class * processor, std::wstring_view cmd, std::vector<std::wstring_view> & paramVector)                              \
    {                                                                                                                                                        \
        return std::nullopt;                                                                                                                                 \
    }                                                                                                                                                        \
                                                                                                                                                             \
    std::optional<Task> ProcessCommand(ClientId client, AllowedContext context, std::wstring_view cmd, std::vector<std::wstring_view>& paramVector) override \
    {                                                                                                                                                        \
        currentContext = context;                                                                                                                            \
        return MatchCommand<commandArray.size()>(this, cmd, paramVector);                                                                                    \
    }                                                                                                                                                        \
                                                                                                                                                             \
public:                                                                                                                                                      \
    GetAdminCommandsFunc(commandArray);                                                                                                                      \
                                                                                                                                                             \
private:
