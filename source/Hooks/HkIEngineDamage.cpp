#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Core/IEngineHook.hpp"

#include <magic_enum.hpp>

using namespace magic_enum::bitwise_operators;

bool IEngineHook::AllowPlayerDamage(ClientId client, ClientId clientTarget)
{
    auto [rval, skip] = CallPlugins<bool>(&Plugin::OnAllowPlayerDamage, client, clientTarget);
    if (skip)
    {
        return rval;
    }

    const auto& config = FLHook::GetConfig();
    if (config.general.damageMode == DamageMode::None)
    {
        return false;
    }

    if (clientTarget)
    {
        if (!magic_enum::enum_flags_test(config.general.damageMode, DamageMode::PvP))
        {
            return false;
        }

        auto time = TimeUtils::UnixTime<std::chrono::milliseconds>();

        // anti-dockkill check
        if (auto& targetData = clientTarget.GetData(); targetData.spawnProtected)
        {
            if (time - targetData.spawnTime <= config.general.antiDockKill)
            {
                return false; // target is protected
            }

            targetData.spawnProtected = false;
        }

        if (auto& clientData = client.GetData(); clientData.spawnProtected)
        {
            if (time - clientData.spawnTime <= config.general.antiDockKill)
            {
                return false; // target may not shoot
            }

            clientData.spawnProtected = false;
        }

        // no-pvp check
        uint systemId;
        pub::Player::GetSystem(client.GetValue(), systemId);
        if (std::ranges::find(config.general.noPVPSystemsHashed, systemId) != config.general.noPVPSystemsHashed.end())
        {
            return false;
        }
    }

    return true;
}
