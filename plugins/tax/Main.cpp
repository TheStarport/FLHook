// Tax plugin - July 2022 by Nekura Mew
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::Tax
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();
	// Functions
	void RemoveTax(const Tax& b)
	{
		auto it = global->lsttax.begin();
		while (it != global->lsttax.end())
		{
			if (it->targetId == b.targetId && it->initiatorId == b.initiatorId)
				global->lsttax.erase(it++);
			else
				++it;
		}
	}

	void UserCmdTax(const uint& clientId, const std::wstring_view& param)
	{
		uint system = 0;
		pub::Player::GetSystem(clientId, system);

		// no-pvp check
		for (auto const& it : global->excludedsystemsIds)
		{
			if (system == it)
			{
				PrintUserCmdText(clientId, L"Error: You cannot tax in a No-PvP system.");
				return;
			}
		}

		const std::wstring taxAmount = GetParam(param, ' ', 0);

		if (!taxAmount.length())
			PrintUserCmdText(clientId, L"Error: No valid tax amount!");

		const int taxValue = ToInt(taxAmount);

		if (taxValue > global->config->maxTax)
		{
			PrintUserCmdText(clientId, L"Error: Maximum tax value is %u credits.", global->config->maxTax);
			return;
		}

		if (taxValue < 0)
		{
			PrintUserCmdText(clientId, L"Error: The tax must be 0 or greater!");
			return;
		}

		std::wstring characterName = GetCharacterNameById(clientId);
		uint clientIdTarget;
		if (const Error error = GetTargetClientId(characterName, clientIdTarget); error != E_OK || !IsValidClientID(clientIdTarget))
		{
			PrintUserCmdText(clientId, L"Error: You are not targeting a player.");
			return;
		}

		int secs = 0;
		std::wstring targetCharacterName = GetCharacterNameById(clientIdTarget);
		if (const Error error = GetOnlineTime(targetCharacterName, secs); error != E_OK || secs < global->config->minplaytimeSec)
		{
			PrintUserCmdText(clientId, L"Error: This player doesn't have enough playtime.");
			return;
		}

		for (const auto& [targetId, initiatorId, target, initiator, cash, f1] : global->lsttax)
		{
			if (targetId == clientIdTarget)
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
			FormatMessage(clientIdTarget,  global->config->customColor, global->config->customFormat, global->config->huntingMessage, characterName.c_str());
		else
			FormatMessage(clientIdTarget,  global->config->customColor, global->config->customFormat, global->config->taxRequestReceived, taxValue, characterName.c_str());

		// send confirmation msg
		if (taxValue > 0)
			PrintUserCmdText(clientId, L"Tax request of %d credits sent to %s!", taxValue, targetCharacterName.c_str());
		else
			PrintUserCmdText(clientId, global->config->huntingMessageOriginator, targetCharacterName.c_str());
	}

	void UserCmdPay(const uint& clientId, const std::wstring_view& param)
	{
		for (auto& it : global->lsttax)
			if (it.targetId == clientId)
			{
				if (it.cash == 0)
				{
					PrintUserCmdText(clientId, global->config->cannotPay);
					return;
				}

				int cash;
				GetCash(clientId, cash);
				if (cash < it.cash)
				{
					PrintUserCmdText(clientId, L"You have not enough money to pay the tax.");
					PrintUserCmdText(it.initiatorId, L"The player does not have enough money to pay the tax.");
					return;
				}
				AddCash(clientId, (0 - it.cash));
				PrintUserCmdText(clientId, L"You paid the tax.");
				AddCash(it.initiatorId, it.cash);
				const std::wstring characterName = GetCharacterNameById(clientId);
				PrintUserCmdText(it.initiatorId, L"%s paid the tax!", characterName.c_str());
				RemoveTax(it);
				SaveChar(clientId);
				SaveChar(it.initiatorId);
				return;
			}

		PrintUserCmdText(clientId, L"Error: No tax request was found that could be accepted!");
	}

	void TimerF1Check()
	{
		struct PlayerData* pPd = nullptr;
		while (pPd = Players.traverse_active(pPd))
		{
			uint clientId = GetClientIdFromPD(pPd);

			if (ClientInfo[clientId].tmF1TimeDisconnect)
				continue;

			if (ClientInfo[clientId].tmF1Time && (timeInMS() >= ClientInfo[clientId].tmF1Time)) // f1
			{
				// tax
				for (const auto& it : global->lsttax)
				{
					if (it.targetId == clientId)
					{
						uint ship;
						pub::Player::GetShip(clientId, ship);
						if (ship && global->config->killDisconnectingPlayers)
						{
							// F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(ship, 0.0);
						}
						std::wstring characterName = GetCharacterNameById(it.targetId);
						PrintUserCmdText(it.initiatorId, L"Tax request to %s aborted.", characterName.c_str());
						RemoveTax(it);
						break;
					}
				}
			}
			else if (ClientInfo[clientId].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[clientId].tmF1TimeDisconnect))
			{
				// tax
				for (const auto& it : global->lsttax)
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
						std::wstring characterName = GetCharacterNameById(it.targetId);
						PrintUserCmdText(it.initiatorId, L"Tax request to %s aborted.", characterName.c_str());
						RemoveTax(it);
						break;
					}
				}
				continue;
			}
		}
	}

	// Hooks
	typedef void (*TimerFunc)();
	struct TIMER
	{
		TimerFunc proc;
		mstime tmIntervallMS;
		mstime tmLastCall;
	};

	TIMER Timers[] = {
	    {TimerF1Check, 1000, 0},
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

	void __stdcall DisConnect(ClientId& clientId, enum EFLConnection& state)
	{
		TimerF1Check();
	}

	void UserCmdHelp(ClientId& clientId, const std::wstring& param)
	{
		PrintUserCmdText(clientId, L"/pay <credits>");
		PrintUserCmdText(clientId, L"/acc");
	}

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		for (auto const& system : config.excludedSystems)
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

REFL_AUTO(type(Config), field(excludedSystems), field(minplaytimeSec), field(maxTax), field(customColor), field(customFormat), field(taxRequestReceived),
    field(huntingMessage), field(huntingMessageOriginator), field(cannotPay), field(killDisconnectingPlayers)) 

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Tax");
	pi->shortName("Tax");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}