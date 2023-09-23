#pragma once
#include "API/FlHook/Plugin.hpp"
#include "Commands/AdminCommandProcessor.hpp"
#include "Commands/UserCommandProcessor.hpp"
#include <Singleton.hpp>
#include <Exceptions/ErrorInfo.hpp>

struct PluginHookData
{
        HookedCall targetFunction;
        PluginHook::FunctionType hookFunction;
        HookStep step;
        int priority;
        std::weak_ptr<Plugin> plugin;
};

inline bool operator<(const PluginHookData& lhs, const PluginHookData& rhs) { return lhs.priority > rhs.priority; }


class PluginManager : public Singleton<PluginManager>
{
        friend AdminCommandProcessor;
        friend UserCommandProcessor;

    public:
        struct FunctionHookProps
        {
                bool callBefore = false;
                bool callAfter = false;

                bool matches(HookStep s) const
                {
                    switch (s)
                    {
                        case HookStep::Before: return callBefore;
                        case HookStep::After: return callAfter;
                        default: return false;
                    }
                }
        };

    private:
        std::array<std::vector<PluginHookData>, static_cast<uint>(HookedCall::Count) * magic_enum::enum_count<HookStep>()> pluginHooks;
        // TODO: Add a getter function of a const ref so other classes can look at thi list of plugins.
        std::vector<std::shared_ptr<Plugin>> plugins;
        std::vector<std::weak_ptr<AbstractUserCommandProcessor>> userCommands;
        std::vector<std::weak_ptr<AbstractAdminCommandProcessor>> adminCommands;
        std::unordered_map<HookedCall, FunctionHookProps> hookProps;

        void ClearData(bool free);
        void setupProps();
        void SetProps(HookedCall c, bool before, bool after);

    public:
        PluginManager();
        ~PluginManager();

        void LoadAll(bool);
        void UnloadAll();

        bool Load(std::wstring_view fileName, bool);
        cpp::result<std::wstring, Error> Unload(std::wstring_view shortName);

        auto begin() { return plugins.begin(); }
        auto end() { return plugins.end(); }

        [[nodiscard]]
        auto begin() const
        {
            return plugins.begin();
        }

        [[nodiscard]]
        auto end() const
        {
            return plugins.end();
        }

        std::optional<std::weak_ptr<Plugin>> GetPlugin(std::wstring_view shortName);

        template <typename ReturnType, typename... Args>
        ReturnType CallPlugins(HookedCall target, HookStep step, bool& skipFunctionCall, Args&&... args) const
        {
            using PluginCallType = ReturnType(__thiscall*)(void*, Args...);
            constexpr bool returnTypeIsVoid = std::is_same_v<ReturnType, void>;
            using NoVoidReturnType = std::conditional_t<returnTypeIsVoid, int, ReturnType>;

            NoVoidReturnType ret{};
            TRY_HOOK
            {
                for (const PluginHookData& hook : pluginHooks[static_cast<uint>(target) * magic_enum::enum_count<HookStep>() + static_cast<uint>(step)])
                {
                    if (hook.plugin.expired())
                    {
                        continue;
                    }

                    const auto& plugin = hook.plugin.lock();

                    plugin->returnCode = ReturnCode::Default;

                    TRY_HOOK
                    {
                        void* pluginRaw = plugin.get();
                        if constexpr (returnTypeIsVoid)
                        {
                            reinterpret_cast<PluginCallType>(hook.hookFunction)(pluginRaw, std::forward<Args>(args)...);
                        }
                        else
                        {
                            ret = reinterpret_cast<PluginCallType>(hook.hookFunction)(pluginRaw, std::forward<Args>(args)...);
                        }
                    }
                    CATCH_HOOK({
                        Logger::i()->Log(
                            LogLevel::Err,
                            std::format(L"Exception in plugin '{}' in {}-{}", plugin->name, magic_enum::enum_name(target), magic_enum::enum_name(step)));
                    })

                    const auto code = plugin->returnCode;

                    if ((code & ReturnCode::SkipFunctionCall) != ReturnCode::Default)
                    {
                        skipFunctionCall = true;
                    }

                    if ((code & ReturnCode::SkipPlugins) != ReturnCode::Default)
                    {
                        break;
                    }
                }
            }
            CATCH_HOOK({ Logger::i()->Log(LogLevel::Err, std::format(L"Exception {}", StringUtils::stows(__FUNCTION__))); });

            if constexpr (!returnTypeIsVoid)
            {
                return ret;
            }
        }
};

template <typename ReturnType = void, typename... Args>
auto CallPluginsBefore(HookedCall target, Args&&... args)
{
    bool skip = false;
    if constexpr (std::is_same_v<ReturnType, void>)
    {
        PluginManager::i()->CallPlugins<void>(target, HookStep::Before, skip, std::forward<Args>(args)...);
        return skip;
    }
    else
    {
        auto ret = PluginManager::i()->CallPlugins<ReturnType>(target, HookStep::Before, skip, std::forward<Args>(args)...);
        return std::make_tuple(ret, skip);
    }
}

template <typename... Args>
void CallPluginsAfter(HookedCall target, Args&&... args)
{
    bool _ = false;
    PluginManager::i()->CallPlugins<void>(target, HookStep::After, _, std::forward<Args>(args)...);
}

template <typename... Args>
bool CallPluginsOther(HookedCall target, HookStep step, Args&&... args)
{
    bool skip = false;
    PluginManager::i()->CallPlugins<void>(target, step, skip, std::forward<Args>(args)...);
    return skip;
}

using PluginFactoryT = std::shared_ptr<Plugin> (*)();

inline std::optional<std::weak_ptr<Plugin>> Plugin::GetPluginFromManager(const std::wstring_view shortName) { return PluginManager::i()->GetPlugin(shortName); }