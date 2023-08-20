#pragma once

constexpr PluginMajorVersion CurrentMajorVersion = PluginMajorVersion::VERSION_04;
constexpr PluginMinorVersion CurrentMinorVersion = PluginMinorVersion::VERSION_01;

const std::wstring VersionInformation = std::to_wstring(static_cast<int>(CurrentMajorVersion)) + L"." + std::to_wstring(static_cast<int>(CurrentMinorVersion));

struct PluginHook
{
        using FunctionType = void (*)(void*);

        HookedCall targetFunction;
        FunctionType hookFunction;
        HookStep step;
        int priority;

        template <typename Func>
        PluginHook(const HookedCall targetFunction, Func hookFunc, const HookStep step = HookStep::Before, const int priority = 0)
            : targetFunction(targetFunction), step(step), priority(priority)
        {
            // This is dumb. We have to cast it to a pointer reference, then dereference it. If we don't we get a type error.
            hookFunction = *reinterpret_cast<FunctionType*>(&hookFunc);
            switch (step)
            {
                case HookStep::Before:
                    if (targetFunction == HookedCall::FLHook__LoadSettings)
                    {
                        throw std::invalid_argument("Load settings can only be called HookStep::After.");
                    }
                    break;
                case HookStep::After: break;
                default:;
            }
        }

        friend class PluginManager;
};

struct Timer
{
        std::function<void()> func;
        DWORD funcAddr;
        mstime intervalInSeconds;
        mstime lastTime = 0;

        static std::vector<std::shared_ptr<Timer>> timers;
        static std::shared_ptr<Timer> Add(std::function<void()> function, void* funcAddr, uint interval);
        static void Remove(DWORD funcAddr);
};

struct PluginInfo
{
        std::wstring name;
        std::wstring shortName;
        PluginMajorVersion versionMajor = PluginMajorVersion::UNDEFINED;
        PluginMinorVersion versionMinor = PluginMinorVersion::UNDEFINED;
        bool mayUnload;

        PluginInfo() = delete;
        PluginInfo(std::wstring_view name, std::wstring_view shortName, const PluginMajorVersion major, const PluginMinorVersion minor,
                   const bool mayUnload = true)
            : name(name), shortName(shortName), versionMajor(major), versionMinor(minor), mayUnload(mayUnload)
        {}
};

class DLL Plugin
{
        static std::optional<std::weak_ptr<Plugin>> GetPluginFromManager(std::wstring_view shortName);
#ifdef FLHOOK
        friend PluginManager;
#endif

        PluginMajorVersion versionMajor = PluginMajorVersion::UNDEFINED;
        PluginMinorVersion versionMinor = PluginMinorVersion::UNDEFINED;

        std::wstring name;
        std::wstring shortName;
        bool mayUnload;

        std::vector<PluginHook> hooks;
        HMODULE dll = nullptr;
        std::wstring dllName;
        std::vector<std::shared_ptr<Timer>> timers;

    protected:
        ReturnCode returnCode = ReturnCode::Default;

        template <typename... Args>
        void EmplaceHook(Args&&... args)
        {
            PluginHook ph(std::forward<Args>(args)...);
            if (std::ranges::find_if(hooks, [ph](const PluginHook& hook) { return hook.targetFunction == ph.targetFunction && ph.step == hook.step; }) !=
                hooks.end())
            {
                return;
            }

            hooks.emplace_back(ph);
        }

        void RemoveHook(HookedCall target, HookStep step)
        {
            std::erase_if(hooks, [target, step](const PluginHook& hook) { return hook.targetFunction == target && step == hook.step; });
        }

        void AddTimer(void (Plugin::*func)(), const int frequencyInSeconds)
        {
            if (auto timer = Timer::Add([this, func] { (this->*func)(); }, &func, frequencyInSeconds))
            {
                timers.emplace_back(timer);
            }
        }

        template <typename T>
        std::optional<std::weak_ptr<T>> GetPlugin(const std::wstring_view shortName)
            requires std::derived_from<T, Plugin>
        {
            auto plugin = GetPluginFromManager(shortName);
            if (!plugin.has_value())
            {
                return std::nullopt;
            }

            auto weakBase = plugin.value();
            std::weak_ptr<T> weakPlugin = std::static_pointer_cast<T>(weakBase.lock());
            return weakPlugin;
        }

    public:
        explicit Plugin(const PluginInfo& info)
        {
            name = info.name;
            shortName = info.shortName;
            mayUnload = info.mayUnload;
            versionMajor = info.versionMajor;
            versionMinor = info.versionMinor;
        }

        virtual ~Plugin()
        {
            for (int i = static_cast<int>(timers.size()) - 1; i >= 0; --i)
            {
                const auto& timer = timers[i];
                Timer::Remove(timer->funcAddr);
                timers.erase(timers.begin() + static_cast<uint>(i));
            }
        };

        Plugin& operator=(const Plugin&&) = delete;
        Plugin& operator=(const Plugin&) = delete;

        [[nodiscard]]
        std::wstring_view GetName() const
        {
            return name;
        }

        [[nodiscard]]
        std::wstring_view GetShortName() const
        {
            return shortName;
        }

        const auto& GetTimers() { return timers; }
};

#define SetupPlugin(type, info)                                               \
    EXPORT std::shared_ptr<type> PluginFactory()                              \
    {                                                                         \
        __pragma(comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)); \
        return std::move(std::make_shared<type>(info));                       \
    }

#define AddPluginTimer(func, time) AddTimer(static_cast<void (Plugin::*)()>(func), time)
