#pragma once

#include "API/FLHook/TaskScheduler.hpp"
#include "Utils/TemplateHelpers.hpp"

using namespace std::string_view_literals;

/**
 * @brief Interface used for processing user commands. Include if your plugin contains user commands.
 */
class AbstractUserCommandProcessor
{
    public:
        virtual ~AbstractUserCommandProcessor() = default;
        virtual std::optional<concurrencpp::result<void>> ProcessCommand(ClientId client, std::wstring_view cmd,
                                                                         std::vector<std::wstring_view>& paramVector) = 0;
        virtual const std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>>& GetUserCommands() const = 0;
};

template <class Processor>
struct CommandInfo
{
        std::vector<std::wstring_view> cmd;
        concurrencpp::result<void> (*func)(Processor* cl, std::vector<std::wstring_view>& params);
        std::wstring_view usage;
        std::wstring_view description;
};

#define AddCommand(class, cmds, func, usage, description) \
    { cmds, ClassFunctionWrapper<decltype(&class ::func), &class ::func>::ProcessParam, usage, description }

#define GetUserCommandsFunc(commands)                                                                                                     \
    const std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>>& GetUserCommands() const override \
    {                                                                                                                                     \
        static std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> info;                        \
        if (info.empty())                                                                                                                 \
        {                                                                                                                                 \
            for (const auto& cmd : commands)                                                                                              \
            {                                                                                                                             \
                info.emplace_back(cmd.cmd, cmd.usage, cmd.description);                                                                   \
            }                                                                                                                             \
        }                                                                                                                                 \
                                                                                                                                          \
        return info;                                                                                                                      \
    };

#define SetupUserCommandHandler(class, commandArray)                                                                                                        \
    template <int N>                                                                                                                                        \
    std::optional<concurrencpp::result<void>> MatchCommand(                                                                                                 \
        class* processor, ClientId triggeringClient, const std::wstring_view cmd, std::vector<std::wstring_view>& paramVector)                              \
    {                                                                                                                                                       \
        const CommandInfo<class> command = std::get<N - 1>(class ::commandArray);                                                                           \
        for (auto& str : command.cmd)                                                                                                                       \
        {                                                                                                                                                   \
            if (cmd.starts_with(std::wstring(str) + L' ') || cmd == str)                                                                                    \
            {                                                                                                                                               \
                const auto countVal = std::ranges::count(str, L' ');                                                                                        \
                paramVector.erase(paramVector.begin() + 1, paramVector.begin() + std::clamp(countVal + 2, 2, 6));                                           \
                return command.func(processor, paramVector);                                                                                                \
            }                                                                                                                                               \
        }                                                                                                                                                   \
                                                                                                                                                            \
        return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);                                                                          \
    }                                                                                                                                                       \
                                                                                                                                                            \
    template <>                                                                                                                                             \
    std::optional<concurrencpp::result<void>> MatchCommand<0>(                                                                                              \
        class * processor, ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring_view> & paramVector)                                  \
    {                                                                                                                                                       \
        return std::nullopt;                                                                                                                                \
    }                                                                                                                                                       \
                                                                                                                                                            \
    std::optional<concurrencpp::result<void>> ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring_view>& paramVector) \
        override                                                                                                                                            \
    {                                                                                                                                                       \
        return MatchCommand<commandArray.size()>(this, triggeringClient, cmd, paramVector);                                                                 \
    }                                                                                                                                                       \
                                                                                                                                                            \
public:                                                                                                                                                     \
    GetUserCommandsFunc(commandArray);                                                                                                                      \
                                                                                                                                                            \
    template <typename T>                                                                                                                                   \
    static std::wstring_view GetUserCommandUsage(T func)                                                                                                    \
    {                                                                                                                                                       \
        for (auto& command : commands)                                                                                                                      \
        {                                                                                                                                                   \
            if (command.func == func)                                                                                                                       \
            {                                                                                                                                               \
                return command.usage;                                                                                                                       \
            }                                                                                                                                               \
        }                                                                                                                                                   \
        return L"Unknown Command";                                                                                                                          \
    }                                                                                                                                                       \
                                                                                                                                                            \
private:

template <class T>
concept IsUserCommandProcessor = std::is_base_of_v<AbstractUserCommandProcessor, T>;

#define GET_USER_CMD_USAGE(x) GetUserCommandUsage(ClassFunctionWrapper<decltype(&x), &x>::ProcessParam)
