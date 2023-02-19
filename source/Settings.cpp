#include "Global.hpp"
#include <refl.hpp>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// setting variables

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string FLHookConfig::File()
{
	return "FLHook.json";
}


void LoadSettings()
{
	auto config = Serializer::JsonToObject<FLHookConfig>();

	if (!config.socket.encryptionKey.empty())
	{
		if (!config.socket.bfCTX)
			config.socket.bfCTX = static_cast<BLOWFISH_CTX*>(malloc(sizeof(BLOWFISH_CTX)));
		Blowfish_Init(static_cast<BLOWFISH_CTX*>(config.socket.bfCTX), reinterpret_cast<unsigned char*>(config.socket.encryptionKey.data()),
		    static_cast<int>(config.socket.encryptionKey.length()));
	}

	// NoPVP
	config.general.noPVPSystemsHashed.clear();
	for (const auto& system : config.general.noPVPSystems)
	{
		uint systemId;
		pub::GetSystemID(systemId, system.c_str());
		config.general.noPVPSystemsHashed.emplace_back(systemId);
	}

	auto ptr = std::make_unique<FLHookConfig>(config);
	FLHookConfig::i(&ptr);	
}

#ifndef CORE_REFL
#define CORE_REFL
REFL_AUTO(type(FLHookConfig::General), field(antiDockKill), field(antiF1), field(changeCruiseDisruptorBehaviour), field(debugMode), 
    field(disableCharfileEncryption), field(disconnectDelay), field(disableNPCSpawns), field(localTime), field(maxGroupSize),
    field(persistGroup), field(reservedSlots), field(torpMissileBaseDamageMultiplier), field(logPerformanceTimers), field(chatSuppressList),
    field(noPVPSystems), field(antiBaseIdle), field(antiCharMenuIdle))
REFL_AUTO(type(FLHookConfig::Plugins), field(loadAllPlugins), field(plugins))
REFL_AUTO(type(FLHookConfig::Socket), field(activated), field(port), field(wPort), field(ePort), field(eWPort), field(encryptionKey), field(passRightsMap))
REFL_AUTO(type(FLHookConfig::Message), field(defaultLocalChat), field(echoCommands), field(suppressInvalidCommands), field(dieMsg), field(dockingMessages))
REFL_AUTO(type(FLHookConfig::MsgStyle), field(msgEchoStyle), field(deathMsgStyle), field(deathMsgStyleSys), field(kickMsgPeriod), field(kickMsg),
    field(userCmdStyle), field(adminCmdStyle), field(deathMsgTextAdminKill), field(deathMsgTextPlayerKill), field(deathMsgTextSelfKill), field(deathMsgTextNPC),
    field(deathMsgTextSuicide))
REFL_AUTO(type(FLHookConfig::UserCommands), field(userCmdSetDieMsg), field(userCmdSetDieMsgSize), field(userCmdSetChatFont), field(userCmdIgnore),
    field(userCmdHelp), field(userCmdMaxIgnoreList), field(defaultLocalChat))
REFL_AUTO(type(FLHookConfig::MultiKillMessages), field(active), field(multiKillMessageStyle)
//	, field(multiKillMessageTemplates)
)
REFL_AUTO(type(FLHookConfig::Bans), field(banAccountOnMatch), field(banWildcardsAndIPs))
REFL_AUTO(type(FLHookConfig), field(general), field(plugins), field(socket), field(messages), field(userCommands), field(multiKillMessages), field(bans))
#endif