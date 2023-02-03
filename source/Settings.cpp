#include "Global.hpp"

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
