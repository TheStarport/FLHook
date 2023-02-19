/**
 * @date Unknown
 * @author ||KOS||Acid (Ported by Raikkonen 2022)
 * @defgroup BountyHunt Bounty Hunt
 * @brief
 * The "Bounty Hunt" plugin allows players to put bounties on each other that can be collected by destroying that player.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - bountyhunt <player> <amount> [timelimit] - Places a bounty on the specified player. When another player kills them, they gain <credits>.
 * - bountyhuntid <id> <amount> [timelimit] - Same as above but with an id instead of a player name. Use /ids
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "enableBountyHunt": true,
 *     "levelProtect": 0,
 *     "minimalHuntTime": 1,
 *     "maximumHuntTime": 240,
 *     "defaultHuntTime": 30
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * None
 */

#include "Main.h"

namespace Plugins::BountyHunt
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup BountyHunt
	 * @brief Removed an active bounty hunt
	 */
	void RemoveBountyHunt(const BountyHunt& bounty)
	{
		auto it = global->bountyHunt.begin();
		while (it != global->bountyHunt.end())
		{
			if (it->targetId == bounty.targetId && it->initiatorId == bounty.initiatorId)
			{
				it = global->bountyHunt.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	/** @ingroup BountyHunt
	 * @brief Print all the active bounty hunts to the player
	 */
	void PrintBountyHunts(ClientId& client)
	{
		if (global->bountyHunt.begin() != global->bountyHunt.end())
		{
			PrintUserCmdText(client, L"Offered Bounty Hunts:");
			for (auto const& [targetId, initiatorId, target, initiator, cash, end] : global->bountyHunt)
			{
				PrintUserCmdText(client, std::format(L"Kill {} and earn {} credits ({} minutes left)", target, cash, ((end - GetTimeInMS()) / 60000)));
			}
		}
	}

	/** @ingroup BountyHunt
	 * @brief User Command for /bountyhunt. Creates a bounty against a specified player.
	 */
	void UserCmdBountyHunt(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableBountyHunt)
			return;

		const std::wstring target = GetParam(param, ' ', 0);
		const uint prize = MultiplyUIntBySuffix(GetParam(param, ' ', 1));
		const std::wstring timeString = GetParam(param, ' ', 2);
		if (!target.length() || prize == 0)
		{
			PrintUserCmdText(client, L"Usage: /bountyhunt <playername> <credits> <time>");
			PrintBountyHunts(client);
			return;
		}

		uint time = wcstol(timeString.c_str(), nullptr, 10);
		const auto targetId = Hk::Client::GetClientIdFromCharName(target);

		int rankTarget = Hk::Player::GetRank(targetId.value()).value();

		if (targetId == UINT_MAX || Hk::Client::IsInCharSelectMenu(targetId.value()))
		{
			PrintUserCmdText(client, std::format(L"{} is not online.", target));
			return;
		}

		if (rankTarget < global->config->levelProtect)
		{
			PrintUserCmdText(client, L"Low level players may not be hunted.");
			return;
		}

		// clamp the hunting time to configured range, or set default if not specified
		if (time)
		{
			time = std::min(global->config->maximumHuntTime, std::max(global->config->minimalHuntTime, time));
		}
		else
		{
			time = global->config->defaultHuntTime;
		}

		if (uint clientCash = Hk::Player::GetCash(client).value(); clientCash < prize)
		{
			PrintUserCmdText(client, L"You do not possess enough credits.");
			return;
		}

		for (const auto& bh : global->bountyHunt)
		{
			if (bh.initiatorId == client && bh.targetId == targetId)
			{
				PrintUserCmdText(client, L"You already have a bounty on this player.");
				return;
			}
		}

		Hk::Player::RemoveCash(client, prize);
		const std::wstring initiator = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		BountyHunt bh;
		bh.initiatorId = client;
		bh.end = GetTimeInMS() + (static_cast<mstime>(time) * 60000);
		bh.initiator = initiator;
		bh.cash = prize;
		bh.target = target;
		bh.targetId = targetId.value();

		global->bountyHunt.push_back(bh);

		Hk::Message::MsgU(
		    bh.initiator + L" offers " + std::to_wstring(bh.cash) + L" credits for killing " + bh.target + L" in " + std::to_wstring(time) + L" minutes.");
	}

	/** @ingroup BountyHunt
	 * @brief User Command for /bountyhuntid. Creates a bounty against a specified player.
	 */
	void UserCmdBountyHuntID(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableBountyHunt)
			return;

		const std::wstring target = GetParam(param, ' ', 0);
		const std::wstring credits = GetParam(param, ' ', 1);
		const std::wstring time = GetParam(param, ' ', 2);
		if (!target.length() || !credits.length())
		{
			PrintUserCmdText(client, L"Usage: /bountyhuntid <id> <credits> <time>");
			PrintBountyHunts(client);
			return;
		}

		ClientId clientTarget = ToInt(target);
		if (!Hk::Client::IsValidClientID(clientTarget) || Hk::Client::IsInCharSelectMenu(clientTarget))
		{
			PrintUserCmdText(client, L"Error: Invalid client id.");
			return;
		}

		const std::wstring charName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientTarget));
		const auto paramNew = std::wstring(charName + L" " + credits + L" " + time);
		UserCmdBountyHunt(client, paramNew);
	}

	/** @ingroup BountyHunt
	 * @brief Checks for expired bounties.
	 */
	void BhTimeOutCheck()
	{
		auto bounty = global->bountyHunt.begin();

		while (bounty != global->bountyHunt.end())
		{
			if (bounty->end < timeInMS())
			{
				if (const auto cashError = Hk::Player::AddCash(bounty->target, bounty->cash); cashError.has_error())
				{
					Console::ConWarn(wstos(Hk::Err::ErrGetText(cashError.error())));
					return;
				}

				Hk::Message::MsgU(bounty->target + L" was not hunted down and earned " + std::to_wstring(bounty->cash) + L" credits.");
				bounty = global->bountyHunt.erase(bounty);
			}
			else
			{
				++bounty;
			}
		}
	}

	/** @ingroup BountyHunt
	 * @brief Processes a ship death to see if it was part of a bounty.
	 */
	void BillCheck(ClientId& client, ClientId& killer)
	{
		for (auto& bounty : global->bountyHunt)
		{
			if (bounty.targetId == client)
			{
				if (killer == 0 || client == killer)
				{
					Hk::Message::MsgU(L"The hunt for " + bounty.target + L" still goes on.");
					continue;
				}

				if (std::wstring winnerCharacterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(killer)); !winnerCharacterName.empty())
				{
					if (const auto cashError = Hk::Player::AddCash(winnerCharacterName, bounty.cash); cashError.has_error())
					{
						Console::ConWarn(wstos(Hk::Err::ErrGetText(cashError.error())));
						return;
					}
					Hk::Message::MsgU(winnerCharacterName + L" has killed " + bounty.target + L" and earned " + std::to_wstring(bounty.cash) + L" credits.");
				}
				else
				{
					if (const auto cashError = Hk::Player::AddCash(bounty.initiator, bounty.cash); cashError.has_error())
					{
						Console::ConWarn(wstos(Hk::Err::ErrGetText(cashError.error())));
						return;
					}
				}
				RemoveBountyHunt(bounty);
				BillCheck(killer, killer);
				break;
			}
		}
	}

	// Timer Hook
	const std::vector<Timer> timers = {{BhTimeOutCheck, 60}};

	/** @ingroup BountyHunt
	 * @brief Hook for SendDeathMsg to call BillCheck
	 */
	void SendDeathMsg([[maybe_unused]] const std::wstring& msg, [[maybe_unused]] const SystemId& system, ClientId& clientVictim, ClientId& clientKiller)
	{
		if (global->config->enableBountyHunt)
		{
			BillCheck(clientVictim, clientKiller);
		}
	}

	void checkIfPlayerFled(ClientId& client)
	{
		for (auto& it : global->bountyHunt)
		{
			if (it.targetId == client)
			{
				if (const auto cashError = Hk::Player::AddCash(it.initiator, it.cash); cashError.has_error())
				{
					Console::ConWarn(wstos(Hk::Err::ErrGetText(cashError.error())));
					return;
				}
				Hk::Message::MsgU(L"The coward " + it.target + L" has fled. " + it.initiator + L" has been refunded.");
				RemoveBountyHunt(it);
				return;
			}
		}
	}

	/** @ingroup BountyHunt
	 * @brief Hook for Disconnect to see if the player had a bounty on them
	 */
	void DisConnect(ClientId& client, [[maybe_unused]] const enum EFLConnection& state)
	{
		checkIfPlayerFled(client);
	}

	/** @ingroup BountyHunt
	 * @brief Hook for CharacterSelect to see if the player had a bounty on them
	 */
	void CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client)
	{
		checkIfPlayerFled(client);
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/bountyhunt", L"<charname> <credits> [minutes]", UserCmdBountyHunt,
	        L"Places a bounty on the specified player. When another player kills them, they gain <credits>."),
	    CreateUserCommand(
	        L"/bountyhuntid", L"<id> <credits> [minutes]", UserCmdBountyHuntID, L"Same as above but with an id instead of a player name. Use /ids"),
	}};

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
	}
} // namespace Plugins::BountyHunt

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::BountyHunt;

REFL_AUTO(type(Config), field(enableBountyHunt), field(levelProtect, AttrMin {0u}, AttrMax {100u}), field(minimalHuntTime, AttrMin {1u}, AttrMax {1400u}),
    field(maximumHuntTime, AttrMin {1u}, AttrMax {1400u}), field(defaultHuntTime, AttrMin {1u}, AttrMax {1400u}))

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Bounty Hunt");
	pi->shortName("bountyhunt");
	pi->mayUnload(false);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect, HookStep::After);
}