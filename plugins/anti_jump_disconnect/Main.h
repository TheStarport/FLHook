#pragma once

#include "../tempban_plugin/Main.h"

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::AntiJumpDisconnect
{
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		Tempban::TempBanCommunicator* tempBanCommunicator = nullptr;

		struct INFO
		{
			bool bInWrapGate;
		};

		std::map<uint, INFO> mapInfo;
	};
} // namespace Plugins::AntiJumpDisconnect