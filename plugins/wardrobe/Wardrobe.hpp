#pragma once
#include "API/FLHook/Plugin.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date Jan, 2023
     * @author Raikkonen, ported by Nen
     * @brief
     * The Wardrobe plugin allows players to change their body and head models from a defined list of allowed models.
     *
     * @par configuration Configuration
     * @code
     * {
     *     "bodies": {
     *         "ExampleBody": "ku_edo_body"
     *     },
     *     "heads": {
     *         "ExampleHead": "ku_edo_head"
     *     }
     * }
     * @endcode
     * @paragraph cmds Player Commands
     * -wardrobe show <head/body> - lists available heads or bodies
     * -wardrobe change <head/body> - changes your character model to selected head or body
     *
     * @par adminCmds Admin Commands
     * None
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    //! Struct that holds a pending wardrobe change
    class WardrobePlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            //! Config data for this plugin
            struct Config final
            {
                    //! A map containing the user friendly name of a head and it's actual name in the FL files
                    std::map<std::string, std::string> heads = {
                        { "ExampleHead", "ku_edo_head" }
                    };
                    //! A map containing the user friendly name of a body and it's actual name in the FL files
                    std::map<std::string, std::string> bodies = {
                        { "ExampleBody", "ku_edo_body" }
                    };
            };

            Config config;

            bool OnLoadSettings() override;
            concurrencpp::result<void> UserCmdShowWardrobe(ClientId client, std::wstring_view param);
            concurrencpp::result<void> UserCmdChangeCostume(ClientId client, std::wstring_view type, std::wstring_view costume);
            concurrencpp::result<void> UserCmdHandle(ClientId client, std::wstring_view command, std::wstring_view param, std::wstring_view param2);

            // clang-format off
            inline static const std::array<CommandInfo<WardrobePlugin>, 1> commands = {
            {
                AddCommand(WardrobePlugin, Cmds(L"/wardrobe"), UserCmdHandle, L"/wardrobe <list/change>", L"Allows you to change the outfit of your character on bases.")}
            };
            // clang-format on

            SetupUserCommandHandler(WardrobePlugin, commands);

        public:
            explicit WardrobePlugin(const PluginInfo& info);
    };
} // namespace Plugins
