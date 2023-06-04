#include "PCH.hpp"
#include "Global.hpp"
#include <refl.hpp>

#include "Defs/FLHookConfig.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setting variables

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring FLHookConfig::File()
{
	return L"FLHook.json";
}

void LoadSettings()
{
	auto config = Serializer::JsonToObject<FLHookConfig>();

	// NoPVP
	config.general.noPVPSystemsHashed.clear();
	for (const auto& system : config.general.noPVPSystems)
	{
		uint systemId;
		pub::GetSystemID(systemId, StringUtils::wstos(system).c_str());
		config.general.noPVPSystemsHashed.emplace_back(systemId);
	}

	auto ptr = std::make_unique<FLHookConfig>(config);
	FLHookConfig::i(&ptr);
}

#ifndef CORE_REFL
#define CORE_REFL
REFL_AUTO(type(FLHookConfig::Debug),
	field(debugMode),
	field(logTraceLevel));

REFL_AUTO(type(FLHookConfig::General),
	field(antiDockKill),
	field(antiF1),
	field(changeCruiseDisruptorBehaviour),
	field(disableCharfileEncryption),
	field(disconnectDelay),
	field(disableNPCSpawns),
	field(localTime),
	field(maxGroupSize),
	field(persistGroup),
	field(torpMissileBaseDamageMultiplier),
	field(logPerformanceTimers),
	field(chatSuppressList),
	field(noPVPSystems),
	field(antiBaseIdle),
	field(antiCharMenuIdle));

REFL_AUTO(type(FLHookConfig::Plugins), field(loadAllPlugins), field(plugins));

REFL_AUTO(type(FLHookConfig::MessageQueue), field(enableQueues), field(ensureSecureConnection), field(username), field(password), field(port));

REFL_AUTO(type(FLHookConfig::ChatConfig), field(defaultLocalChat), field(echoCommands), field(suppressInvalidCommands), field(dieMsg), field(dockingMessages));

REFL_AUTO(type(FLHookConfig::MsgStyle),
	field(msgEchoStyle),
	field(deathMsgStyle),
	field(deathMsgStyleSys),
	field(kickMsgPeriod),
	field(kickMsg),
	field(userCmdStyle),
	field(adminCmdStyle),
	field(deathMsgTextAdminKill),
	field(deathMsgTextPlayerKill),
	field(deathMsgTextSelfKill),
	field(deathMsgTextNPC),
	field(deathMsgTextSuicide));

REFL_AUTO(type(FLHookConfig::UserCommands),
	field(userCmdSetDieMsg),
	field(userCmdSetDieMsgSize),
	field(userCmdSetChatFont),
	field(userCmdIgnore),
	field(userCmdHelp),
	field(userCmdMaxIgnoreList),
	field(defaultLocalChat));

REFL_AUTO(type(FLHookConfig::Bans), field(banAccountOnMatch), field(banWildcardsAndIPs));

REFL_AUTO(type(FLHookConfig::Callsign), field(allowedFormations), field(disableRandomisedFormations), field(disableUsingAffiliationForCallsign));

REFL_AUTO(type(FLHookConfig), field(general), field(plugins), field(messageQueue), field(userCommands), field(bans), field(chatConfig), field(callsign));
#endif

