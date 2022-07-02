// Tax Plugin
// By NekuraMew

#include "Main.h"

namespace Plugins::Tax
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();
	// Functions
	void RemoveTax(Tax b)
	{
		std::list<Tax>::iterator it = global->lsttax.begin();
		while (it != global->lsttax.end())
		{
			if (it->targetId == b.targetId && it->initiatorId == b.initiatorId)
			{
				global->lsttax.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void UserCmdTax(const uint& clientId, const std::wstring_view& param)
	{
		std::wstring credits = GetParam(param, ' ', 1);
		if (!credits.length())
		{
			PrintUserCmdText(clientId, L"Usage: /tax <credits in k>");
			return;
		}

		uint system = 0;
		pub::Player::GetSystem(clientId, system);

		// no-pvp check
		for (auto& it : global->excludedsystemsIds)
		{
			if (system == (it))
			{
				PrintUserCmdText(clientId, L"Error: You cannot tax in a No-PvP system.");
				return;
			}
		}

		std::wstring taxAmount = GetParam(param, ' ', 0);

		if (!taxAmount.length())
			PrintUserCmdText(clientId, L"Error: No valid tax amount!");

		int taxValue = ToInt(taxAmount);
		int taxValueKM = taxValue *= 1000;

		if (taxValueKM > global->config->maxTax)
		{
			PrintUserCmdText(clientId, L"Error: Maximum tax value is %u Credits.", global->config->maxTax);
			return;
		}

		if (taxValueKM < 0)
		{
			PrintUserCmdText(clientId, L"Error: The tax must be 0 or greater!");
			return;
		}

		std::wstring charname = (wchar_t*)Players.GetActiveCharacterName(clientId);
		uint clientIdTarget;
		HkGetTargetClientId(charname, clientIdTarget);
		if (!HkIsValidClientID(clientIdTarget))
		{
			PrintUserCmdText(clientId, L"Error: You must select the ship of the player you want to tax and it has to be a player.");
			return;
		}

		int secs = 0;
		std::wstring targetCharname = (wchar_t*)Players.GetActiveCharacterName(clientIdTarget);
		HkGetOnlineTime(targetCharname, secs);
		if (secs < global->config->minplaytimeSec)
		{
			PrintUserCmdText(clientId, L"Error: This player dont have enough playtime.");
			return;
		}

		for (auto& it : global->lsttax)
		{
			if (it.targetId == clientIdTarget)
			{
				PrintUserCmdText(clientId, L"Error: There already is a tax request pending for this player.");
				return;
			}
		}

		Tax tax;
		tax.initiatorId = clientId;
		tax.targetId = clientIdTarget;
		tax.cash = taxValue;
		global->lsttax.push_back(tax);

		if (taxValue == 0)
			HkFormatMessage(clientIdTarget,  global->config->customColor, global->config->customFormat, L"You are being hunted by %s. Run for cover, he wants to kill you!", charname.c_str());
		else
			HkFormatMessage(clientIdTarget,  global->config->customColor, global->config->customFormat, L"You have received a tax request: Pay %d Credits to %s! Type \"/acc\" to pay the tax.", taxValue, charname.c_str());

		// send confirmation msg
		if (taxValue > 0)
			PrintUserCmdText(clientId, L"Tax request of %d credits sent to %s!", taxValue, targetCharname.c_str());
		else
			PrintUserCmdText(clientId, L"Unacceptable tax request sent to %s!", targetCharname.c_str());
	}

	void UserCmdPay(const uint& clientId, const std::wstring_view& param)
	{
		for (auto& it : global->lsttax)
		{
			if (it.targetId == clientId)
			{
				if (it.cash == 0)
				{
					PrintUserCmdText(clientId, L"You cannot pay this rogue. Run for cover, he wants to kill you!");
					return;
				}

				int cash;
				HkGetCash(clientId, cash);
				if (cash < it.cash)
				{
					PrintUserCmdText(clientId, L"You have not enough money to pay the tax.");
					PrintUserCmdText(it.initiatorId, L"The player has not enough money to pay the tax.");
					return;
				}
				HkAddCash(clientId, (0 - it.cash));
				PrintUserCmdText(clientId, L"You paid the tax.");
				HkAddCash(it.initiatorId, it.cash);
				std::wstring charname = (wchar_t*)Players.GetActiveCharacterName(clientId);
				PrintUserCmdText(it.initiatorId, L"%s paid the tax!", charname.c_str());
				RemoveTax(it);
				HkSaveChar(clientId);
				HkSaveChar(it.initiatorId);
				return;
			}
		}

		PrintUserCmdText(clientId, L"Error: No tax request found that could be accepted!");
	}

	void HkTimerF1Check()
	{
		struct PlayerData* pPd = 0;
		while (pPd = Players.traverse_active(pPd))
		{
			uint clientId = HkGetClientIdFromPD(pPd);

			if (ClientInfo[clientId].tmF1TimeDisconnect)
				continue;

			if (ClientInfo[clientId].tmF1Time && (timeInMS() >= ClientInfo[clientId].tmF1Time)) // f1
			{
				// tax
				for (auto& it : global->lsttax)
				{
					if (it.targetId == clientId)
					{
						uint ship;
						pub::Player::GetShip(clientId, ship);
						if (ship)
						{
							// F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(ship, 0.0);
						}
						std::wstring charname = (wchar_t*)Players.GetActiveCharacterName(it.targetId);
						PrintUserCmdText(it.initiatorId, L"Tax request to %s aborted.", charname.c_str());
						RemoveTax(it);
						break;
					}
				}
			}
			else if (ClientInfo[clientId].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[clientId].tmF1TimeDisconnect))
			{
				// tax
				for (auto& it : global->lsttax)
				{
					if (it.targetId == clientId)
					{
						uint ship;
						pub::Player::GetShip(clientId, ship);
						if (ship)
						{
							// F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(ship, 0.0);
						}
						std::wstring charname = (wchar_t*)Players.GetActiveCharacterName(it.targetId);
						PrintUserCmdText(it.initiatorId, L"Tax request to %s aborted.", charname.c_str());
						RemoveTax(it);
						break;
					}
				}
				continue;
			}
		}
	}

	// Hooks
	typedef void (*_TimerFunc)();
	struct TIMER
	{
		_TimerFunc proc;
		mstime tmIntervallMS;
		mstime tmLastCall;
	};

	TIMER Timers[] = {
	    {HkTimerF1Check, 1000, 0},
	};

	int __stdcall Update()
	{
		for (uint i = 0; (i < sizeof(Timers) / sizeof(TIMER)); i++)
		{
			if ((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS)
			{
				Timers[i].tmLastCall = timeInMS();
				Timers[i].proc();
			}
		}
		return 0;
	}

	void __stdcall DisConnect(uint& clientId, enum EFLConnection& state)
	{
		HkTimerF1Check();
	}

	void UserCmdHelp(uint& clientId, const std::wstring& param)
	{
		PrintUserCmdText(clientId, L"/pay <credits>");
		PrintUserCmdText(clientId, L"/acc");
	}

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		for (auto& system : config.excludedSystems)
		{
			global->excludedsystemsIds.push_back(CreateID(system.c_str()));
		}
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/tax", L"", UserCmdTax, L""),
	    CreateUserCommand(L"/pay", L"", UserCmdPay, L""),
	}};

} // namespace Plugins::Tax

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::Tax;

REFL_AUTO(type(Config), field(excludedSystems), field(minplaytimeSec), field(maxTax), field(customColor), field(customFormat)) 

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Tax Plugin");
	pi->shortName("Tax");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}