#pragma once
#include "API/FlHook/Plugin.hpp"
#include "Commands/AdminCommandProcessor.hpp"
#include "Commands/UserCommandProcessor.hpp"
#include <Exceptions/ErrorInfo.hpp>
#include <Singleton.hpp>
#include <Utils/TemplateHelpers.hpp>

class PluginManager : public Singleton<PluginManager>
{
        friend AdminCommandProcessor;
        friend UserCommandProcessor;

        // TODO: Add a getter function of a const ref so other classes can look at thi list of plugins.
        std::vector<std::shared_ptr<Plugin>> plugins;
        std::vector<std::weak_ptr<AbstractUserCommandProcessor>> userCommands;
        std::vector<std::weak_ptr<AbstractAdminCommandProcessor>> adminCommands;

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

        std::optional<std::weak_ptr<Plugin>> GetPlugin(std::wstring_view shortName);

        template <typename ReturnType, typename FuncPtr, typename... Args>
        ReturnType CallPlugins(FuncPtr target, bool& skipFunctionCall, Args... args) const
        {
            constexpr bool returnTypeIsVoid = std::is_same_v<ReturnType, void>;
            using NoVoidReturnType = std::conditional_t<returnTypeIsVoid, int, ReturnType>;

            NoVoidReturnType ret{};
            TRY_HOOK
            {
                for (auto plugin : plugins)
                {
                    plugin->returnCode = ReturnCode::Default;

                    TRY_HOOK
                    {
                        using ClassType = typename MemberFunctionPointerClassType<FuncPtr>::type;
                        if constexpr (std::is_same_v<ClassType, PacketInterface>)
                        {
                            if (const auto packetInterface = dynamic_cast<PacketInterface*>(plugin.get()); packetInterface != nullptr)
                            {
                                auto& pluginRef = *packetInterface;
                                if constexpr (returnTypeIsVoid)
                                {
                                    (pluginRef.*target)(args...);
                                }
                                else
                                {
                                    ret = (pluginRef.*target)(args...);
                                }
                            }
                        }
                        else
                        {
                            auto& pluginRef = *plugin;
                            if constexpr (returnTypeIsVoid)
                            {
                                (pluginRef.*target)(args...);
                            }
                            else
                            {
                                ret = (pluginRef.*target)(args...);
                            }
                        }
                    }
                    CATCH_HOOK({
                        auto targetName = typeid(FuncPtr).name();
                        Logger::i()->Log(LogLevel::Err, std::format(L"Exception in plugin '{}' in {}", plugin->name, StringUtils::stows(targetName)));
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
            else
            {
                return void();
            }
        }
};

template <typename ReturnType = void, typename FuncPtr, typename... Args>
auto CallPlugins(FuncPtr target, Args... args)
{
    bool skip = false;
    if constexpr (std::is_same_v<ReturnType, void>)
    {
        PluginManager::i()->CallPlugins<void>(target, skip, args...);
        return skip;
    }
    else
    {
        auto ret = PluginManager::i()->CallPlugins<ReturnType>(target, skip, args...);
        return std::make_tuple(ret, skip);
    }
}

using PluginFactoryT = std::shared_ptr<Plugin> (*)();

inline std::optional<std::weak_ptr<Plugin>> Plugin::GetPluginFromManager(const std::wstring_view shortName) { return PluginManager::i()->GetPlugin(shortName); }
