#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void IServerImplHook::NpcSpinProtection(const SSPObjCollisionInfo& oci, ClientId client)
{
    static uint dummy;
    // static
    static GameObject* collidingIObject = FLHook::GetObjInspect(oci.colliderObjectId);
    const float targetMass = collidingIObject->get_mass();
    // Don't do spin protect unless the hit ship is big
    const auto config = FLHook::GetConfig();
    if (targetMass < config->gameFixes.spinProtectionMass)
    {
        return;
    }

    const float clientMass = client.GetShip().Handle().GetValue().lock()->get_mass();

    // Don't do spin protect unless the hit ship is 2 times larger than the hitter
    if (targetMass < clientMass * 2)
    {
        return;
    }

    auto linearVelocity = collidingIObject->get_velocity();
    auto angularVelocity = collidingIObject->get_angular_velocity();

    // crash prevention in case of null vectors
    if (linearVelocity.x == 0.0f && linearVelocity.y == 0.0f && linearVelocity.z == 0.0f && angularVelocity.x == 0.0f && angularVelocity.y == 0.0f &&
        angularVelocity.z == 0.0f)
    {
        return;
    }

    linearVelocity.x *= config->gameFixes.spinImpulseMultiplier * clientMass;
    linearVelocity.y *= config->gameFixes.spinImpulseMultiplier * clientMass;
    linearVelocity.z *= config->gameFixes.spinImpulseMultiplier * clientMass;
    angularVelocity.x *= config->gameFixes.spinImpulseMultiplier * clientMass;
    angularVelocity.y *= config->gameFixes.spinImpulseMultiplier * clientMass;
    angularVelocity.z *= config->gameFixes.spinImpulseMultiplier * clientMass;

    collidingIObject->cobject()->add_impulse(linearVelocity, angularVelocity);
}

void __stdcall IServerImplHook::SpObjCollision(const SSPObjCollisionInfo& oci, ClientId client)
{
    TRACE("IServerImplHook::SpObjCollision client={{client}}", { "client", client });

    if (FLHook::GetConfig()->gameFixes.enableNpcSpinProtection && oci.colliderObjectId)
    {
        NpcSpinProtection(oci, client);
    }

    const auto skip = CallPlugins(&Plugin::OnSpObjectCollision, client, oci);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.SPObjCollision(oci, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpObjectCollisionAfter, client, oci);
}
