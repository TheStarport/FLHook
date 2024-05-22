#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void IServerImplHook::NpcSpinProtection(const SSPObjCollisionInfo& oci, ClientId client)
{
    static uint dummy;
    // static
    static EqObj* iObj;
    uint spaceObjId = oci.colliderObjectId;
    FLHook::getShipInspect(spaceObjId, reinterpret_cast<IObjInspectImpl*&>(iObj), dummy);
    const float targetMass = iObj->get_mass();
    // Don't do spin protect unless the hit ship is big
    const auto& config = FLHook::GetConfig();
    if (targetMass < config.npc.spinProtectionMass)
    {
        return;
    }

    const float clientMass = client.GetShip().Handle()->get_mass();

    // Don't do spin protect unless the hit ship is 2 times larger than the hitter
    if (targetMass < clientMass * 2)
    {
        return;
    }

    auto linearVelocity = iObj->get_velocity();
    auto angularVelocity = iObj->get_angular_velocity();

    // crash prevention in case of null vectors
    if (linearVelocity.x == 0.0f && linearVelocity.y == 0.0f && linearVelocity.z == 0.0f && angularVelocity.x == 0.0f && angularVelocity.y == 0.0f &&
        angularVelocity.z == 0.0f)
    {
        return;
    }

    linearVelocity.x *= config.npc.spinImpulseMultiplier * clientMass;
    linearVelocity.y *= config.npc.spinImpulseMultiplier * clientMass;
    linearVelocity.z *= config.npc.spinImpulseMultiplier * clientMass;
    angularVelocity.x *= config.npc.spinImpulseMultiplier * clientMass;
    angularVelocity.y *= config.npc.spinImpulseMultiplier * clientMass;
    angularVelocity.z *= config.npc.spinImpulseMultiplier * clientMass;

    iObj->cobject()->add_impulse(linearVelocity, angularVelocity);
}

void __stdcall IServerImplHook::SpObjCollision(const SSPObjCollisionInfo& oci, ClientId client)
{
    Logger::Trace(std::format(L"SPObjCollision(\n\tClientId client = {}\n)", client));

    if (FLHook::GetConfig().npc.enableNpcSpinProtection && oci.colliderObjectId)
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
