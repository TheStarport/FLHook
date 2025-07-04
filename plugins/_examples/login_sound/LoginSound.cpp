#include "PCH.hpp"

#include "LoginSound.hpp"

#include "API/Utils/Random.hpp"

using namespace Plugins;

bool LoginSound::OnLoadSettings()
{
    LoadJsonWithValidation(Config, config, "config/login_sound.json");

    return true;
}

void LoginSound::OnLoginAfter(const ClientId client, [[maybe_unused]] const SLoginInfo& li)
{
    if (!config.sounds.empty())
    {
        const auto sound = config.sounds[Random::Uniform(0u, config.sounds.size() - 1)];
        client.PlaySound(sound.GetValue());
    }
}

LoginSound::LoginSound(const PluginInfo& info) : Plugin(info) {}

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Login Sound",
	    .shortName = L"login_sound",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(LoginSound);
