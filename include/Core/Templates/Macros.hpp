#pragma once

/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B)  A##B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B)     PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A)    STRINGIZE_NX(A)

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
    catch ([[maybe_unused]] SehException & exc)                \
    {                                                          \
        ExceptionHandler::LogException(exc);                   \
        e;                                                     \
    }                                                          \
    catch ([[maybe_unused]] const StopProcessingException&) {} \
    catch (const GameException& ex)                            \
    {                                                          \
        Logger::Info(ex.Msg());                                \
        e;                                                     \
    }                                                          \
    catch ([[maybe_unused]] std::exception & exc) { e; }       \
    catch (...) { e; }

#define DefaultDllMain() \
    BOOL WINAPI DllMain([[maybe_unused]] HINSTANCE dll, [[maybe_unused]] DWORD reason, [[maybe_unused]] LPVOID reserved) { return true; }

#define DeduceClassType(variableName, type, value)                         \
private:                                                                   \
    static auto DeduceType##variableName() { return value; }               \
    using type = std::invoke_result_t<decltype(DeduceType##variableName)>; \
    type variableName = DeduceType##variableName()

#define AddCommand(class, cmds, func, usage, description)                                                    \
    {                                                                                                        \
        cmds, ClassFunctionWrapper<decltype(&class ::func), &class ::func>::ProcessParam, usage, description \
    }

#define Cmds(...) std::vector<std::wstring_view> {__VA_ARGS__}

#define GetCommandsFunc(commands)                                                                           \
    std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> GetCommands() override \
    {                                                                                                       \
        std::vector<std::tuple<std::vector<std::wstring_view>, std::wstring_view, std::wstring_view>> info;              \
        for (const auto& cmd : commands)                                                                    \
        {                                                                                                   \
            info.emplace_back(cmd.cmd, cmd.usage, cmd.description);                                         \
        }                                                                                                   \
                                                                                                            \
        return info;                                                                                        \
    };

#define SetupUserCommandHandler(class, commandArray)                                                                                                  \
    template <int N>                                                                                                                                  \
    bool MatchCommand(class* processor, ClientId triggeringClient, const std::wstring_view cmd, std::vector<std::wstring>& paramVector) \
    {                                                                                                                                                 \
        const CommandInfo<class> command = std::get<N - 1>(class ::commandArray);                                                                     \
        for (auto& str : command.cmd)                                                                                                                 \
        {                                                                                                                                             \
            if (str == cmd)                                                                                                                   \
            {                                                                                                                                         \
                command.func(processor, paramVector);                                                                                                 \
                return true;                                                                                                                          \
            }                                                                                                                                         \
        }                                                                                                                                             \
                                                                                                                                                      \
        return MatchCommand<N - 1>(processor, triggeringClient, cmd, paramVector);                                                                    \
    }                                                                                                                                                 \
                                                                                                                                                      \
    template <>                                                                                                                                       \
    bool MatchCommand<0>(class * processor, ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring> & paramVector)                \
    {                                                                                                                                                 \
        return false;                                                                                                                                 \
    }                                                                                                                                                 \
                                                                                                                                                      \
    bool ProcessCommand(ClientId triggeringClient, std::wstring_view cmd, std::vector<std::wstring>& paramVector) override                            \
    {                                                                                                                                                 \
        userCmdClient = triggeringClient;                                                                                                             \
        return MatchCommand<commandArray.size()>(this, userCmdClient, cmd, paramVector);                                                              \
    }                                                                                                                                                 \
                                                                                                                                                      \
public:                                                                                                                                               \
    GetCommandsFunc(commandArray);                                                                                                                    \
                                                                                                                                                      \
private:

#define FUNCTION   reinterpret_cast<const char*>(__FUNCTION__)
#define FUNCTION_W reinterpret_cast<const wchar_t*>(__FUNCTIONW__)
