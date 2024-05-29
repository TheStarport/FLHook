#pragma once

using namespace std::string_view_literals;

enum class DefaultRoles
{
    SuperAdmin,
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
        std::wstring (*func)(Processor* cl, std::vector<std::wstring>& params);
        AllowedContext allowedContext;
        std::wstring_view requiredRole;
        std::wstring_view usage;
        std::wstring_view description;
};

class AbstractAdminCommandProcessor
{
    protected:
        // Current user, changes every command invocation.
        std::wstring_view currentUser;
        AllowedContext currentContext = AllowedContext::GameOnly;

        cpp::result<void, std::wstring> Validate(const AllowedContext context, std::wstring_view requiredRole) const
        {
            using namespace magic_enum::bitwise_operators;
            const static std::wstring invalidPerms = L"ERR: No permission.";
            const static std::wstring invalidCommand = L"ERR: Command not found.";
            static constexpr std::wstring_view superAdminRole = magic_enum::enum_name(DefaultRoles::SuperAdmin);

            // If the current context does not allow command
            if (static_cast<int>(currentContext & context) == 0)
            {
                return cpp::fail(invalidCommand);
            }

            auto& credMap = FLHook::GetAdmins();
            const auto credentials = credMap.find(currentUser.data());
            if (credentials == credMap.end())
            {
                // Some how got here and not authenticated!
                return cpp::fail(invalidPerms);
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
        virtual std::wstring ProcessCommand(std::wstring_view user, AllowedContext currentContext, std::wstring_view cmd,
                                            std::vector<std::wstring>& paramVector) = 0;
        virtual std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> GetAdminCommands() = 0;
};

template <class T>
concept IsAdminCommandProcessor = std::is_base_of_v<AbstractAdminCommandProcessor, T>;

#define AddAdminCommand(cls, str, func, context, requiredRole, usage, description)                                                                             \
    {                                                                                                                                                          \
        str, ClassFunctionWrapper<decltype(&cls::func), &cls::func>::ProcessParam, AllowedContext::context, magic_enum::enum_name(DefaultRoles::requiredRole), \
            usage, description                                                                                                                                 \
    }

#define GetAdminCommandsFunc(commands)                                                                                        \
    std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> GetAdminCommands() override \
    {                                                                                                                         \
        std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> info;                   \
        for (const auto& cmd : commands)                                                                                      \
        {                                                                                                                     \
            info.emplace_back(cmd.cmd, cmd.usage, cmd.description);                                                           \
        }                                                                                                                     \
                                                                                                                              \
        return info;                                                                                                          \
    };

#define SetupAdminCommandHandler(class, commandArray)                                                                                                   \
    template <int N>                                                                                                                                    \
    std::wstring MatchCommand(class* processor, const std::wstring_view cmd, std::vector<std::wstring>& paramVector)                                    \
    {                                                                                                                                                   \
        const AdminCommandInfo<class>& command = std::get<N - 1>(class ::commandArray);                                                                       \
        for (auto& str : command.cmd)                                                                                                                   \
        {                                                                                                                                               \
            if (str == cmd)                                                                                                                             \
            {                                                                                                                                           \
                return command.func(processor, paramVector);                                                                                            \
            }                                                                                                                                           \
        }                                                                                                                                               \
                                                                                                                                                        \
        return MatchCommand<N - 1>(processor, cmd, paramVector);                                                                                        \
    }                                                                                                                                                   \
                                                                                                                                                        \
    template <>                                                                                                                                         \
    std::wstring MatchCommand<0>(class * processor, std::wstring_view cmd, std::vector<std::wstring> & paramVector)                                     \
    {                                                                                                                                                   \
        return L"";                                                                                                                                   \
    }                                                                                                                                                   \
                                                                                                                                                        \
    std::wstring ProcessCommand(std::wstring_view user, AllowedContext context, std::wstring_view cmd, std::vector<std::wstring>& paramVector) override \
    {                                                                                                                                                   \
        currentUser = user;                                                                                                                             \
        currentContext = context;                                                                                                                       \
        return MatchCommand<commandArray.size()>(this, cmd, paramVector);                                                                               \
    }                                                                                                                                                   \
                                                                                                                                                        \
public:                                                                                                                                                 \
    GetAdminCommandsFunc(commandArray);                                                                                                                 \
                                                                                                                                                        \
private:
