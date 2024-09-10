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

const PluginInfo Info(L"Login Sound", L"login_sound", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(LoginSound, Info);
