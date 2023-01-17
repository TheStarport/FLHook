#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Autobuy
{
	//! A struct to represent each client
	class AutobuyInfo
	{
	  public:
		AutobuyInfo() = default;

		bool missiles;
		bool mines;
		bool torps;
		bool cd;
		bool cm;
		bool repairs;
	};

	struct AutobuyCartItem
	{
		uint archId = 0;
		uint count = 0;
		std::wstring description;
	};

	struct Global final
	{
		std::map<uint, AutobuyInfo> autobuyInfo;
		ReturnCode returnCode = ReturnCode::Default;
	};
}