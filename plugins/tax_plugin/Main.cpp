// Tax Plugin
// By NekuraMew

#include "Main.h"

namespace Plugins::Tax
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();
	// Functions
	void RemoveTax(TAX b)
	{
		std::list<TAX>::iterator it = global->lsttax.begin();
		while (it != global->lsttax.end())
		{
			if (it->uiTargetID == b.uiTargetID && it->uiInitiatorID == b.uiInitiatorID)
			{
				global->lsttax.erase(it++);
			}
			else
			{
				++it;
			}
		}
		

	}

	void UserCmd_tax(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->config->EnableTax)
		{
			PRINT_DISABLED();
			return;
		}

		std::wstring wscCredits = GetParam(wscParam, ' ', 1);
		if (!wscCredits.length())
		{
			PrintUserCmdText(iClientID, L"Usage: /tax <credits in k>");
			return;
		}

	if (!global->config->EnableTax)
		{
			PRINT_DISABLED();
			return;
		}

		uint iSystem = 0;
		pub::Player::GetSystem(iClientID, iSystem);

		// no-pvp check
		for (auto& it : global->ExcludedSystemsIds)
		{
			if (iSystem == (it))
			{
				PrintUserCmdText(iClientID, L"Error: You cannot tax in a No-PvP system.");
				return;
			}
		}


		std::wstring wscTaxAmount = GetParam(wscParam, ' ', 0);

		if (!wscTaxAmount.length())
			PrintUserCmdText(iClientID, L"Error: No valid tax amount!");

		int iTaxValue = ToInt(wscTaxAmount);
		int iTaxValue_K = iTaxValue *= 1000;

		if (iTaxValue_K > global->config->iMaxTax)
		{
			PrintUserCmdText(iClientID, L"Error: Maximum tax value is %u Credits.", global->config->iMaxTax);
			return;
		}

		if (iTaxValue_K < 0)
		{
			PrintUserCmdText(iClientID, L"Error: The tax must be 0 or greater!");
			return;
		}

		uint iClientIDTarget = _GetTargetID(iClientID);
		if (!HkIsValidClientID(iClientIDTarget))
		{
			PrintUserCmdText(iClientID, L"Error: You must select the ship of the player you want to tax and it has to be a player.");
			return;
		}

		int secs = 0;
		std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
		std::wstring wscTargetCharname = (wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);
		HkGetOnlineTime(wscTargetCharname, secs);
		if (secs < global->config->MinPlayTimeSec)
		{
			PrintUserCmdText(iClientID, L"Error: This player dont have enough playtime.");
			return;
		}

		for (auto& it : global->lsttax)
		{
			if (it.uiTargetID == iClientIDTarget)
			{
				PrintUserCmdText(iClientID, L"Error: There already is a tax request pending for this player.");
				return;
			}
		}


		TAX tax;
		tax.uiInitiatorID = iClientID;
		tax.uiTargetID = iClientIDTarget;
		tax.iCash = iTaxValue;
		global->lsttax.push_back(tax);

		if (iTaxValue == 0)
			PrintUserCmd2(iClientIDTarget, L"You are being hunted by %s. Run for cover, he wants to kill you!", wscCharname.c_str());
		else
			PrintUserCmd2(
			    iClientIDTarget, L"You have received a tax request: Pay %d Credits to %s! Type \"/acc\" to pay the tax.", iTaxValue, wscCharname.c_str());

		// send confirmation msg
		if (iTaxValue > 0)
			PrintUserCmdText(iClientID, L"Tax request of %d credits sent to %s!", iTaxValue, wscTargetCharname.c_str());
		else
			PrintUserCmdText(iClientID, L"Unacceptable tax request sent to %s!", wscTargetCharname.c_str());


	}

	void UserCmd_acc(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->config->EnableTax)
		{
			PRINT_DISABLED();
			return;
		}

		for (auto& it : global->lsttax)
		{
			if (it.uiTargetID == iClientID)
			{
				if (it.iCash == 0)
				{
					PrintUserCmdText(iClientID, L"You cannot pay this rogue. Run for cover, he wants to kill you!");
					return;
				}

				int iCash;
				HkGetCash(iClientID, iCash);
				if (iCash < it.iCash)
				{
					PrintUserCmdText(iClientID, L"You have not enough money to pay the tax.");
					PrintUserCmdText(it.uiInitiatorID, L"The player has not enough money to pay the tax.");
					return;
				}
				HkAddCash(iClientID, (0 - it.iCash));
				PrintUserCmdText(iClientID, L"You paid the tax.");
				HkAddCash(it.uiInitiatorID, it.iCash);
				std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
				PrintUserCmdText(it.uiInitiatorID, L"%s paid the tax!", wscCharname.c_str());
				RemoveTax(it);
				HkSaveChar(iClientID);
				HkSaveChar(it.uiInitiatorID);
				return;
			}
		}

		PrintUserCmdText(iClientID, L"Error: No tax request found that could be accepted!");

	}

	void HkTimerF1Check()
	{
		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			if (ClientInfo[iClientID].tmF1TimeDisconnect)
				continue;

			if (ClientInfo[iClientID].tmF1Time && (timeInMS() >= ClientInfo[iClientID].tmF1Time)) // f1
			{
				// tax
				for (auto& it : global->lsttax)
				{
					if (it.uiTargetID == iClientID)
					{
						uint iShip;
						pub::Player::GetShip(iClientID, iShip);
						if (iShip)
						{
							//F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(iShip, 0.0);
						}
						std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(it.uiTargetID);
						PrintUserCmdText(it.uiInitiatorID, L"Tax request to %s aborted.", wscCharname.c_str());
						RemoveTax(it);
						break;
					}
				}
			}
			else if (ClientInfo[iClientID].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[iClientID].tmF1TimeDisconnect))
			{
				// tax
				for (auto& it : global->lsttax)
				{
					if (it.uiTargetID == iClientID)
					{
						uint iShip;
						pub::Player::GetShip(iClientID, iShip);
						if (iShip)
						{
							//F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(iShip, 0.0);
						}
						std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(it.uiTargetID);
						PrintUserCmdText(it.uiInitiatorID , L"Tax request to %s aborted.", wscCharname.c_str());
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

	EXPORT int __stdcall Update()
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

	void __stdcall DisConnect(uint& iClientID, enum EFLConnection& state)
	{ 
		if (global->config->EnableTax)
		{
			HkTimerF1Check();
		}
	}

	// Client command processing
	USERCMD UserCmds[] = {
	    {L"/tax", UserCmd_tax},
	    {L"/acc", UserCmd_acc},
	};

	// Process user input
	bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd) { DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, global->returncode); }

	EXPORT void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
	{
		PrintUserCmdText(iClientID, L"/tax <credits>");
		PrintUserCmdText(iClientID, L"/acc");
	}

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		for (auto& system : config.ExcludedSystems)
		{
			global->ExcludedSystemsIds.push_back(CreateID(system.c_str()));
		}
			

	}
} // namespace Tax

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::Tax;

REFL_AUTO(type(Config), field(EnableTax), field(ExcludedSystems), field(MinPlayTimeSec), field(iMaxTax))

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		LoadSettings();
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Tax Plugin");
	pi->shortName("Tax");
	pi->mayPause(false);
	pi->mayUnload(false);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}