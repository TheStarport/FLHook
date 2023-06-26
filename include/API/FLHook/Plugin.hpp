#pragma once

struct Timer
{
        std::function<void()> func;
        mstime intervalInSeconds;
        mstime lastTime = 0;

#ifdef FLHOOK
        static std::vector<std::unique_ptr<Timer>> timers;
#endif

        DLL static void Add(std::function<void()> function, uint interval);
        DLL static void Remove(const std::function<void()>& func);
};

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

#define PluginCall(name, ...) (*(name))(__VA_ARGS__)

// Inherit from this to define a IPC (Inter-Plugin Communication) class.
class DLL PluginCommunicator
{
    public:
        using EventSubscription = void (*)(int id, void* dataPack);
        void Dispatch(int id, void* dataPack) const;
        void AddListener(std::wstring plugin, EventSubscription event);

        std::wstring plugin;

        explicit PluginCommunicator(std::wstring plugin) : plugin(std::move(plugin)) {}

        static void ExportPluginCommunicator(PluginCommunicator* communicator);
        static PluginCommunicator* ImportPluginCommunicator(std::wstring plugin, EventSubscription subscription = nullptr);

    private:
        std::map<std::wstring, EventSubscription> listeners;
};

struct PluginInfo
{
        std::wstring name;
        std::wstring shortName;
        PluginMajorVersion versionMajor = PluginMajorVersion::UNDEFINED;
        PluginMinorVersion versionMinor = PluginMinorVersion::UNDEFINED;
        bool mayUnload;

        PluginInfo() = delete;
        PluginInfo(std::wstring name, std::wstring shortName, const PluginMajorVersion major, const PluginMinorVersion minor, const bool mayUnload = true)
            : name(std::move(name)), shortName(std::move(shortName)), versionMajor(major), versionMinor(minor), mayUnload(mayUnload)
        {}
};

class DLL Plugin
{
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

    protected:
        ReturnCode returnCode = ReturnCode::Default;

        std::vector<Timer> timers;

    public:
        explicit Plugin(const PluginInfo& info)
        {
            name = info.name;
            shortName = info.shortName;
            mayUnload = info.mayUnload;
            versionMajor = info.versionMajor;
            versionMinor = info.versionMinor;
        }

        virtual ~Plugin() = default;
        Plugin& operator=(const Plugin&&) = delete;
        Plugin& operator=(const Plugin&) = delete;

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

        [[nodiscard]]
        auto& GetTimers()
        {
            return timers;
        }
};

#define SetupPlugin(type, info)                                               \
    EXPORT std::shared_ptr<type> PluginFactory()                              \
    {                                                                         \
        __pragma(comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)); \
        return std::move(std::make_shared<type>(info));                       \
    }
