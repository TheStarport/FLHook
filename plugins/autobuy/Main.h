#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Autobuy
{
	//! A struct to represent each client
	class ClientInfo
	{
	  public:
		ClientInfo() = default;

		bool autoBuyMissiles;
		bool autoBuyMines;
		bool autoBuyTorps;
		bool autoBuyCD;
		bool autoBuyCM;
		bool autoBuyRepairs;
	};

	struct AUTOBUY_CARTITEM
	{
		uint iArchId;
		uint iCount;
		std::wstring wscDescription;
	};

	// Loadable json configuration
	struct Config final : Reflectable
	{
		std::string File() override { return "config/autobuy.json"; }
		bool enableAutobuy = true;
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		std::map<uint, ClientInfo> autobuyInfo;
		ReturnCode returnCode = ReturnCode::Default;
	};
}