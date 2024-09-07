#pragma once

#include "API/FLHook/Plugin.hpp"
#include "FLHook.hpp"

#ifdef FLHOOK
    #include "Commands/AdminCommandProcessor.hpp"
    #include "Commands/ExternalCommandProcessor.hpp"
    #include "Commands/UserCommandProcessor.hpp"
    #include "Core/ExceptionHandler.hpp"
    #include "Utils/TemplateHelpers.hpp"
#endif

class DLL PluginManager final : public Singleton<PluginManager>
{
#ifdef FLHOOK
        friend AdminCommandProcessor;
        friend UserCommandProcessor;
        friend ExternalCommandProcessor;
        friend FLHook;

        std::vector<std::shared_ptr<Plugin>> plugins;
        std::vector<std::weak_ptr<AbstractUserCommandProcessor>> userCommands;
        std::vector<std::weak_ptr<AbstractAdminCommandProcessor>> adminCommands;
        std::vector<std::weak_ptr<AbstractExternalCommandProcessor>> externalCommands;

        void ClearData(bool free);

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

        template <typename ReturnType, typename FuncPtr, typename... Args>
        ReturnType CallPlugins(FuncPtr target, bool& skipFunctionCall, Args&&... args) const
        {
            constexpr bool returnTypeIsVoid = std::is_same_v<ReturnType, void>;
            using NoVoidReturnType = std::conditional_t<returnTypeIsVoid, int, ReturnType>;

            NoVoidReturnType ret{};
            TryHook
            {
                for (auto plugin : plugins)
                {
                    plugin->returnCode = ReturnCode::Default;

                    TryHook
                    {
                        using ClassType = typename MemberFunctionPointerClassType<FuncPtr>::type;
                        if constexpr (std::is_same_v<ClassType, PacketInterface>)
                        {
                            if (const auto packetInterface = dynamic_cast<PacketInterface*>(plugin.get()); packetInterface != nullptr)
                            {
                                auto& pluginRef = *packetInterface;
                                if constexpr (returnTypeIsVoid)
                                {
                                    (pluginRef.*target)(std::forward<Args>(args)...);
                                }
                                else
                                {
                                    ret = (pluginRef.*target)(std::forward<Args>(args)...);
                                }
                            }
                        }
                        else
                        {
                            auto& pluginRef = *plugin;
                            if constexpr (returnTypeIsVoid)
                            {
                                (pluginRef.*target)(std::forward<Args>(args)...);
                            }
                            else
                            {
                                ret = (pluginRef.*target)(std::forward<Args>(args)...);
                            }
                        }
                    }
                    CatchHook({
                        auto targetName = typeid(FuncPtr).name();
                        Logger::Err(std::format(L"Exception in plugin '{}' in {}", plugin->name, StringUtils::stows(targetName)));
                    });

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
            CatchHook({ Logger::Err(std::format(L"Exception {}", StringUtils::stows(FUNCTION))); });

            if constexpr (!returnTypeIsVoid)
            {
                return ret;
            }
            else
            {
                return void();
            }
        }
#endif
        // ReSharper disable once CppRedundantAccessSpecifier
    public:
        std::optional<std::weak_ptr<Plugin>> GetPlugin(std::wstring_view shortName);
};

#ifdef FLHOOK
template <typename ReturnType = void, typename FuncPtr, typename... Args>
auto CallPlugins(FuncPtr target, Args&&... args)
{
    bool skip = false;
    if constexpr (std::is_same_v<ReturnType, void>)
    {
        PluginManager::i()->CallPlugins<void>(target, skip, std::forward<Args>(args)...);
        return skip;
    }
    else
    {
        auto ret = PluginManager::i()->CallPlugins<ReturnType>(target, skip, std::forward<Args>(args)...);
        return std::make_tuple(ret, skip);
    }
}
#endif

using PluginFactoryT = std::shared_ptr<Plugin> (*)();

inline std::optional<std::weak_ptr<Plugin>> Plugin::GetPluginFromManager(const std::wstring_view shortName) { return PluginManager::i()->GetPlugin(shortName); }
