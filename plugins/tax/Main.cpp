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

	void UserCmdTax(ClientId& client, const std::wstring& param)
	{
		// no-pvp check
		if (SystemId system = Hk::Player::GetSystem(client).value(); 
			std::ranges::find(global->excludedsystemsIds, system) != global->excludedsystemsIds.end())
		{
			PrintUserCmdText(client, L"Error: You cannot tax in a No-PvP system.");
			return;
		}

		const uint taxValue = ToUInt(GetParam(param, ' ', 0));

		if (!taxValue)
		{
			PrintUserCmdText(client, L"Error: No valid tax amount!");
		}

		if (taxValue > global->config->maxTax)
		{
			PrintUserCmdText(client, std::format(L"Error: Maximum tax value is {} credits.", global->config->maxTax));
			return;
		}

		const auto characterName = Hk::Client::GetCharacterNameByID(client);

		const auto clientTargetObject = Hk::Player::GetTargetClientID(client);
		
		if (clientTargetObject.has_error())
		{
			PrintUserCmdText(client, L"Error: You are not targeting a player.");
			return;
		}

		const auto clientTarget = clientTargetObject.value();
		const auto secs = Hk::Player::GetOnlineTime(client);
		const auto targetCharacterName = Hk::Client::GetCharacterNameByID(clientTarget);
		if (secs.has_error() || secs.value() < global->config->minplaytimeSec)
		{
			PrintUserCmdText(client, L"Error: This player doesn't have enough playtime.");
			return;
		}

		if (std::ranges::find_if(global->lsttax, [clientTarget](const Tax& tax) { return tax.targetId == clientTarget; }) != global->lsttax.end())
		{
			PrintUserCmdText(client, L"Error: There already is a tax request pending for this player.");
			return;
		}

		Tax tax;
		tax.initiatorId = client;
		tax.targetId = clientTarget;
		tax.cash = taxValue;
		global->lsttax.push_back(tax);

		std::wstring msg;

		if (taxValue == 0)
		{
			msg = Hk::Message::FormatMsg(global->config->customColor, global->config->customFormat, global->config->huntingMessage, characterName.value().c_str());
			PrintUserCmdText(client, global->config->huntingMessageOriginator, targetCharacterName.value().c_str());
		}
		else
		{
			msg = Hk::Message::FormatMsg(global->config->customColor, global->config->customFormat, global->config->taxRequestReceived, taxValue, characterName.value().c_str());
			PrintUserCmdText(client, std::format(L"Tax request of {} credits sent to {}!", taxValue, targetCharacterName.value().c_str()));
		}
		Hk::Message::FMsg(clientTarget, msg);
	}

	void UserCmdPay(ClientId& client, const std::wstring& param)
	{
		for (auto& it : global->lsttax)
			if (it.targetId == client)
			{
				if (it.cash == 0)
				{
					PrintUserCmdText(client, global->config->cannotPay);
					return;
				}

				if (const auto cash = Hk::Player::GetCash(client); 
					cash.has_error() || cash.value() < it.cash)
				{
					PrintUserCmdText(client, L"You have not enough money to pay the tax.");
					PrintUserCmdText(it.initiatorId, L"The player does not have enough money to pay the tax.");
					return;
				}
				Hk::Player::RemoveCash(client, it.cash);
				PrintUserCmdText(client, L"You paid the tax.");
				Hk::Player::AddCash(it.initiatorId, it.cash);
				const auto characterName = Hk::Client::GetCharacterNameByID(client);
				PrintUserCmdText(it.initiatorId, std::format(L"{} paid the tax!", characterName.value().c_str()));
				RemoveTax(it);
				Hk::Player::SaveChar(client);
				Hk::Player::SaveChar(it.initiatorId);
				return;
			}

		PrintUserCmdText(client, L"Error: No tax request was found that could be accepted!");
	}

	void timerCheck()
	{
		struct PlayerData* playerData = nullptr;
		playerData = Players.traverse_active(playerData);
		while (playerData)
		{
			ClientId client = playerData->iOnlineId;

			if (ClientInfo[client].tmF1TimeDisconnect)
				continue;

			if (ClientInfo[client].tmF1Time && (timeInMS() >= ClientInfo[client].tmF1Time)) // f1
			{
				// tax
				for (const auto& it : global->lsttax)
				{
					if (it.targetId == client)
					{
						if (uint ship = Hk::Player::GetShip(client).value(); 
							ship && global->config->killDisconnectingPlayers)
						{
							// F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(ship, 0.0);
						}
						const auto characterName = Hk::Client::GetCharacterNameByID(it.targetId);
						PrintUserCmdText(it.initiatorId, L"Tax request to %s aborted.", characterName.value().c_str());
						RemoveTax(it);
						break;
					}
				}
			}
			else if (ClientInfo[client].tmF1TimeDisconnect && (timeInMS() >= ClientInfo[client].tmF1TimeDisconnect))
			{
				// tax
				for (const auto& it : global->lsttax)
				{
					if (it.targetId == client)
					{
						if (uint ship = Hk::Player::GetShip(client).value())
						{
							// F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(ship, 0.0);
						}
						const auto characterName = Hk::Client::GetCharacterNameByID(it.targetId);
						PrintUserCmdText(it.initiatorId, L"Tax request to %s aborted.", characterName.value().c_str());
						RemoveTax(it);
						break;
					}
				}
				continue;
			}

			playerData = Players.traverse_active(playerData);
		}
	}
	// Timer Hook
	const std::vector<Timer> timers = {
		{timerCheck, 1}
	};

	void UserCmdHelp(ClientId& client, const std::wstring& param)
	{
		PrintUserCmdText(client, L"/pay <credits>");
		PrintUserCmdText(client, L"/acc");
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
	pi->name("tax");
	pi->shortName("tax");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}