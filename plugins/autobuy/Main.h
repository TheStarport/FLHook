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
		uint archId;
		uint count;
		std::wstring description;
	};

	struct Global final
	{
		std::map<uint, ClientInfo> autobuyInfo;
		ReturnCode returnCode = ReturnCode::Default;
	};
}