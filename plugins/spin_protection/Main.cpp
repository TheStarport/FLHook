// Spin Protection plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::SpinProtection
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// Load configuration file
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
	}

	void __stdcall SPObjCollision(struct SSPObjCollisionInfo const& ci, ClientId& client)
	{
		// If spin protection is off, do nothing.
		if (global->config->spinProtectionMass == -1.0f)
			return;

		float targetMass;
		pub::SpaceObj::GetMass(ci.iColliderObjectId, targetMass);

		ClientId clientShip;
		pub::Player::GetShip(client, clientShip);

		float clientMass;
		pub::SpaceObj::GetMass(clientShip, clientMass);

		// Don't do spin protect unless the hit ship is big
		if (targetMass < global->config->spinProtectionMass)
			return;

		// Don't do spin protect unless the hit ship is 2 times larger than the
		// hitter
		if (targetMass < clientMass * 2)
			return;

		Vector V1, V2;
		pub::SpaceObj::GetMotion(ci.iColliderObjectId, V1, V2);
		V1.x *= global->config->spinImpulseMultiplier * clientMass;
		V1.y *= global->config->spinImpulseMultiplier * clientMass;
		V1.z *= global->config->spinImpulseMultiplier * clientMass;
		V2.x *= global->config->spinImpulseMultiplier * clientMass;
		V2.y *= global->config->spinImpulseMultiplier * clientMass;
		V2.z *= global->config->spinImpulseMultiplier * clientMass;
		pub::SpaceObj::AddImpulse(ci.iColliderObjectId, V1, V2);
	}
} // namespace Plugins::SpinProtection

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::SpinProtection;

DefaultDllMainSettings(LoadSettings)

REFL_AUTO(type(Config), field(spinProtectionMass), field(spinImpulseMultiplier))

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Spin Protection");
	pi->shortName("spin_protection");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjCollision, &SPObjCollision);
}
