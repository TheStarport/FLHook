/**
 * @date July, 2022
 * @author Nekura Mew
 * @defgroup Tax Tax
 * @brief
 * The Tax plugin allows players to issue 'formally' make credit demands and declare hostilities.
 *
 * @paragraph cmds Player Commands
 * -tax <amount> - demand listed amount from the player, for amount equal zero, it declares hostilities.
 * -pay - submits the demanded payment to the issuing player
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "cannotPay": "This rogue isn't interested in money. Run for cover, they want to kill you!",
 *     "customColor": 9498256,
 *     "customFormat": 144,
 *     "huntingMessage": "You are being hunted by %s. Run for cover, they want to kill you!",
 *     "huntingMessageOriginator": "Good luck hunting %s !",
 *     "killDisconnectingPlayers": true,
 *     "maxTax": 300,
 *     "minplaytimeSec": 0,
 *     "taxRequestReceived": "You have received a tax request: Pay %d credits to %s! Type \"/pay\" to pay the tax."
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "Tax.h"
#include <Tools/Serialization/Attributes.hpp>

namespace Plugins::Tax
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();
	// Functions
	void RemoveTax(const Tax& toRemove)
	{
		auto taxToRemove = std::ranges::find_if(
		    global->lsttax, [&toRemove](const Tax& tax) { return tax.targetId == toRemove.targetId && tax.initiatorId == toRemove.initiatorId; });
		global->lsttax.erase(taxToRemove);
	}

	void UserCmdTax(ClientId& client, const std::wstring& param)
	{
		const auto& noPvpSystems = FLHookConfig::c()->general.noPVPSystemsHashed;
		// no-pvp check
		if (SystemId system = Hk::Player::GetSystem(client).value(); std::ranges::find(noPvpSystems, system) == noPvpSystems.end())
		{
			PrintUserCmdText(client, L"Error: You cannot tax in a No-PvP system.");
			return;
		}

		const std::wstring taxAmount = GetParam(param, ' ', 0);

		if (taxAmount.empty())
		{
			PrintUserCmdText(client, L"Usage:");
			PrintUserCmdText(client, L"/tax <credits>");
		}

		const uint taxValue = MultiplyUIntBySuffix(taxAmount);

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

		for (const auto& [targetId, initiatorId, target, initiator, cash, f1] : global->lsttax)
		{
			if (targetId == clientTarget)
			{
				PrintUserCmdText(client, L"Error: There already is a tax request pending for this player.");
				return;
			}
		}

		Tax tax;
		tax.initiatorId = client;
		tax.targetId = clientTarget;
		tax.cash = taxValue;
		global->lsttax.push_back(tax);

		std::wstring msg;

		if (taxValue == 0)
			msg = Hk::Message::FormatMsg(global->config->customColor,
			    global->config->customFormat,
			    std::vformat(global->config->huntingMessage, std::make_wformat_args(characterName.value())));
		else
			msg = Hk::Message::FormatMsg(global->config->customColor,
			    global->config->customFormat,
			    std::vformat(global->config->taxRequestReceived, std::make_wformat_args(taxValue, characterName.value())));

		Hk::Message::FMsg(clientTarget, msg);

		// send confirmation msg
		if (taxValue > 0)
			PrintUserCmdText(client, std::format(L"Tax request of {} credits sent to {}!", taxValue, targetCharacterName.value()));
		else
			PrintUserCmdText(client, std::vformat(global->config->huntingMessageOriginator, std::make_wformat_args(targetCharacterName.value())));
	}

	void UserCmdPay(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		for (auto& it : global->lsttax)
			if (it.targetId == client)
			{
				if (it.cash == 0)
				{
					PrintUserCmdText(client, global->config->cannotPay);
					return;
				}

				if (const auto cash = Hk::Player::GetCash(client); cash.has_error() || cash.value() < it.cash)
				{
					PrintUserCmdText(client, L"You have not enough money to pay the tax.");
					PrintUserCmdText(it.initiatorId, L"The player does not have enough money to pay the tax.");
					return;
				}
				Hk::Player::RemoveCash(client, it.cash);
				PrintUserCmdText(client, L"You paid the tax.");
				Hk::Player::AddCash(it.initiatorId, it.cash);
				const auto characterName = Hk::Client::GetCharacterNameByID(client);
				PrintUserCmdText(it.initiatorId, std::format(L"{} paid the tax!", characterName.value()));
				RemoveTax(it);
				Hk::Player::SaveChar(client);
				Hk::Player::SaveChar(it.initiatorId);
				return;
			}

		PrintUserCmdText(client, L"Error: No tax request was found that could be accepted!");
	}

	void TimerF1Check()
	{
		PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			ClientId client = playerData->iOnlineId;

			if (ClientInfo[client].tmF1TimeDisconnect)
				continue;

			if (ClientInfo[client].tmF1Time && (Hk::Time::GetUnixMiliseconds() >= ClientInfo[client].tmF1Time)) // f1
			{
				// tax
				for (const auto& it : global->lsttax)
				{
					if (it.targetId == client)
					{
						if (uint ship = Hk::Player::GetShip(client).value(); ship && global->config->killDisconnectingPlayers)
						{
							// F1 -> Kill
							pub::SpaceObj::SetRelativeHealth(ship, 0.0);
						}
						const auto characterName = Hk::Client::GetCharacterNameByID(it.targetId);
						PrintUserCmdText(it.initiatorId, std::format(L"Tax request to {} aborted.", characterName.value()));
						RemoveTax(it);
						break;
					}
				}
			}
			else if (ClientInfo[client].tmF1TimeDisconnect && (Hk::Time::GetUnixMiliseconds() >= ClientInfo[client].tmF1TimeDisconnect))
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
						PrintUserCmdText(it.initiatorId, std::format(L"Tax request to {} aborted.", characterName.value()));
						RemoveTax(it);
						break;
					}
				}
			}
		}
	}

	// Hooks

	const std::vector<Timer> timers = {{TimerF1Check, 1}};

	void DisConnect([[maybe_unused]] ClientId& client, [[maybe_unused]] const enum EFLConnection& state)
	{
		TimerF1Check();
	}

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/tax", L"<amount>", UserCmdTax, L"Demand listed amount from your current target."),
	    CreateUserCommand(L"/pay", L"", UserCmdPay, L"Pays a tax request that has been issued to you."),
	}};

} // namespace Plugins::Tax

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::Tax;

REFL_AUTO(type(Config), field(minplaytimeSec, AttrMin {0u}), field(maxTax, AttrMin {0u}), field(customColor), field(customFormat),
    field(taxRequestReceived, AttrNotEmpty<std::wstring> {}), field(huntingMessage, AttrNotEmpty<std::wstring> {}),
    field(huntingMessageOriginator, AttrNotEmpty<std::wstring> {}), field(cannotPay, AttrNotEmpty<std::wstring> {}), field(killDisconnectingPlayers))

DefaultDllMainSettings(LoadSettings);

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
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}