#pragma once

#include "../tempban_plugin/Main.h"

#include <FLHook.h>
#include <plugin.h>

namespace Plugins::AntiJumpDisconnect
{
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		TempBanCommunicator* tempBanCommunicator = nullptr;

		struct INFO
		{
			bool bInWrapGate;
		};

		std::map<uint, INFO> mapInfo;
	};
} // namespace Plugins::AntiJumpDisconnect