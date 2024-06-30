#pragma once

using namespace std::string_view_literals;

/**
 * @brief Interface used for processing user commands. Include if your plugin contains user commands.
 */
class AbstractUserCommandProcessor
{
    protected:
        ClientId userCmdClient;
        void PrintOk() const { (void)userCmdClient.Message(L"OK"); }

    public:
        virtual ~AbstractUserCommandProcessor() = default;
        virtual bool ProcessCommand(ClientId client, std::wstring_view cmd, std::vector<std::wstring_view>& paramVector) = 0;
        virtual const std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>>& GetUserCommands() const = 0;
};

template <class Processor>
struct CommandInfo
{
        std::vector<std::wstring_view> cmd;
        void (*func)(Processor* cl, std::vector<std::wstring_view>& params);
        std::wstring_view usage;
        std::wstring_view description;
};

#define AddCommand(class, cmds, func, usage, description)                                                    \
    {                                                                                                        \
        cmds, ClassFunctionWrapper<decltype(&class ::func), &class ::func>::ProcessParam, usage, description \
    }

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

#define SetupUserCommandHandler(class, commandArray)                                                                                         \
    template <int N>                                                                                                                         \
    bool MatchCommand(class* processor, ClientId triggeringClient, const std::wstring_view cmd, std::vector<std::wstring_view>& paramVector) \
    {                                                                                                                                        \
        const CommandInfo<class> command = std::get<N - 1>(class ::commandArray);                                                            \
        for (auto& str : command.cmd)                                                                                                        \
        {                                                                                                                                    \
            if (cmd.starts_with(str))                                                                                                        \
            {                                                                                                                                \
                paramVector.erase(paramVector.begin(), paramVector.begin() + (std::clamp(std::ranges::count(str, L' '), 1, 5)));                               \
                command.func(processor, paramVector);                                                                                        \
                return true;                                                                                                                 \
            }                                                                                                                                \
        }                                                                                                                                    \
                                                                                                                                             \
        return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);                                                           \
    }                                                                                                                                        \
                                                                                                                                             \
    template <>                                                                                                                              \
    bool MatchCommand<0>(class * processor, ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring_view> & paramVector)  \
    {                                                                                                                                        \
        return false;                                                                                                                        \
    }                                                                                                                                        \
                                                                                                                                             \
    bool ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring_view>& paramVector) override              \
    {                                                                                                                                        \
        userCmdClient = triggeringClient;                                                                                                    \
        return MatchCommand<commandArray.size()>(this, userCmdClient, cmd, paramVector);                                                     \
    }                                                                                                                                        \
                                                                                                                                             \
public:                                                                                                                                      \
    GetUserCommandsFunc(commandArray);                                                                                                       \
                                                                                                                                             \
private:

template <class T>
concept IsUserCommandProcessor = std::is_base_of_v<AbstractUserCommandProcessor, T>;
