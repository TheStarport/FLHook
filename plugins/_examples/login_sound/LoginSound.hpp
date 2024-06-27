#pragma once

#include "API/FLHook/Plugin.hpp"

namespace Plugins
{
    /**
     * @date 2022
     * @author Raikkonen
     * @defgroup SoundManager Sound Manager
     * @brief
     * The plugin plays a random sound upon player login. To be expanded upon.
     *
     * @par Player Commands
     * None
     *
     * @par Admin Commands
     * None
     *
     * @par Configuration
     * @code
     * {
     *	"sounds": ["dock_not_allowed", "dock_granted"]
     * }
     * @endcode
     */
    class LoginSound final : Plugin
    {
        void OnLoadSettings() override;
        void OnLoginAfter(ClientId client, const SLoginInfo& li) override;

        struct Config final
        {
            std::vector<UnknownId> sounds;
        };

        Config config;

        public:
            explicit LoginSound(const PluginInfo& info);
    };
} // namespace Plugins
