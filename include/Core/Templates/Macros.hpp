#pragma once

#ifndef DLL
    #ifndef FLHOOK
        #define DLL __declspec(dllimport)
    #else
        #define DLL __declspec(dllexport)
    #endif
#endif

#define TryHook \
    try         \
    {           \
        _set_se_translator(SehException::Translator);
#define CatchHook(e)                                           \
    }                                                          \
    catch ([[maybe_unused]] SehException & exc) { e; }         \
    catch ([[maybe_unused]] const StopProcessingException&) {} \
    catch (const GameException& ex)                            \
    {                                                          \
        FLHook::GetLogger().Log(LogLevel::Info, ex.Msg());     \
        e;                                                     \
    }                                                          \
    catch ([[maybe_unused]] std::exception & exc) { e; }       \
    catch (...) { e; }

#define LOG_EXCEPTION \
    {}

#define DefaultDllMain(x, xx)                                                                                            \
    BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE dll, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved) \
    {                                                                                                                    \
        if (xx reason == DLL_PROCESS_ATTACH)                                                                             \
        {                                                                                                                \
            x;                                                                                                           \
        }                                                                                                                \
        return true;                                                                                                     \
    }

#define DeduceClassType(variableName, type, value)                         \
private:                                                                   \
    static auto DeduceType##variableName() { return value; }               \
    using type = std::invoke_result_t<decltype(DeduceType##variableName)>; \
    type variableName = DeduceType##variableName()

#define AddCommand(class, str, func, usage, description)                                                                       \
    {                                                                                                                          \
        std::wstring_view(str), ClassFunctionWrapper<decltype(&class ::func), &class ::func>::ProcessParam, usage, description \
    }

#define GetCommandsFunc(commands)                                                                           \
    std::vector<std::tuple<std::wstring_view, std::wstring_view, std::wstring_view>> GetCommands() override \
    {                                                                                                       \
        std::vector<std::tuple<std::wstring_view, std::wstring_view, std::wstring_view>> info;              \
        for (const auto& cmd : commands)                                                                    \
        {                                                                                                   \
            info.emplace_back(cmd.cmd, cmd.usage, cmd.description);                                         \
        }                                                                                                   \
                                                                                                            \
        return info;                                                                                        \
    };

#define SetupUserCommandHandler(class, commandArray)                                                                                    \
    template <int N>                                                                                                                    \
    bool MatchCommand(class* processor, ClientId triggeringClient, const std::wstring_view cmd, std::vector<std::wstring>& paramVector) \
    {                                                                                                                                   \
        if (const CommandInfo<class> command = std::get<N - 1>(class ::commandArray); command.cmd == cmd)                               \
        {                                                                                                                               \
            command.func(processor, paramVector);                                                                                       \
            return true;                                                                                                                \
        }                                                                                                                               \
                                                                                                                                        \
        return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);                                                      \
    }                                                                                                                                   \
                                                                                                                                        \
    template <>                                                                                                                         \
    bool MatchCommand<0>(class * processor, ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring> & paramVector)  \
    {                                                                                                                                   \
        return false;                                                                                                                   \
    }                                                                                                                                   \
                                                                                                                                        \
    bool ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring>& paramVector) override              \
    {                                                                                                                                   \
        client = triggeringClient;                                                                                                      \
        return MatchCommand<commandArray.size()>(this, client, cmd, paramVector);                                                       \
    }                                                                                                                                   \
                                                                                                                                        \
public:                                                                                                                                 \
    GetCommandsFunc(commandArray);                                                                                                      \
                                                                                                                                        \
private:

#define Serialize(type, ...) NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(type, __VA_ARGS__)
