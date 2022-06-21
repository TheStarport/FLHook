// BountyHunt Plugin
// Originally by ||KOS||Acid
// Modified by Raikkonen

#include "Main.h"

namespace Plugins::BountyHunt
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void RemoveBountyHunt(const BountyHunt& bounty)
	{
		auto it = global->bountyHunt.begin();
		while (it != global->bountyHunt.end())
		{
			if (it->targetId == bounty.targetId && it->initiatorId == bounty.initiatorId)
			{
				global->bountyHunt.erase(it++);
			}
			else
			{
				++it;
			}
		}
	}

	void PrintBountyHunts(uint clientId)
	{
		if (global->bountyHunt.begin() != global->bountyHunt.end())
		{
			PrintUserCmdText(clientId, L"Offered Bounty Hunts:");
			for (auto const& [targetId, initiatorId, target, initiator, cash, end] : global->bountyHunt)
			{
				PrintUserCmdText(
					clientId,
					L"Kill %s and earn %u credits (%u minutes left)",
					target.c_str(),
					cash,
				    ((end - GetTimeInMS()) / 60000));
			}
		}
	}

	void UserCmdBountyHunt(uint clientId, const std::wstring& wscParam)
	{
		if (!global->config->enableBountyHunt)
			return;

		const std::wstring target = GetParam(wscParam, ' ', 0);
		const std::wstring credits = GetParam(wscParam, ' ', 1);
		const std::wstring timeString = GetParam(wscParam, ' ', 2);
		if (!target.length() || !credits.length())
		{
			PrintUserCmdText(clientId, L"Usage: /bountyhunt <playername> <credits> <time>");
			PrintBountyHunts(clientId);
			return;
		}

		const int prize = wcstol(credits.c_str(), nullptr, 10);
		uint time = wcstol(timeString.c_str(), nullptr, 10);
		uint targetId = HkGetClientIdFromCharname(target);

		int rankTarget;
		pub::Player::GetRank(targetId, rankTarget);

		if (targetId == -1 || HkIsInCharSelectMenu(targetId))
		{
			PrintUserCmdText(clientId, L"%s is not online.", target.c_str());
			return;
		}

		if (rankTarget < global->config->levelProtect)
		{
			PrintUserCmdText(clientId, L"Low level players may not be hunted.");
			return;
		}
		if (time < 1 || time > 240)
		{
			PrintUserCmdText(clientId, L"Hunting time: 30 minutes.");
			time = 30;
		}

		int clientCash;
		pub::Player::InspectCash(clientId, clientCash);
		if (clientCash < prize)
		{
			PrintUserCmdText(clientId, L"You do not possess enough credits.");
			return;
		}

		for (const auto& bh : global->bountyHunt)
		{
			if (bh.initiatorId == clientId && bh.targetId == targetId)
			{
				PrintUserCmdText(clientId, L"You already have a bounty on this player.");
				return;
			}
		}

		pub::Player::AdjustCash(clientId, -prize);
		const std::wstring initiator = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));

		BountyHunt bh;
		bh.initiatorId = clientId;
		bh.end = GetTimeInMS() + (static_cast<mstime>(time) * 60000);
		bh.initiator = initiator;
		bh.cash = prize;
		bh.target = target;
		bh.targetId = targetId;

		global->bountyHunt.push_back(bh);

		HkMsgU(bh.initiator + L" offers " + std::to_wstring(bh.cash) + L" credits for killing " + bh.target + L" in " + std::to_wstring(time) +
		    L" minutes.");
	}

	void UserCmdBountyHuntId(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->config->enableBountyHunt)
			return;

		const std::wstring target = GetParam(wscParam, ' ', 0);
		const std::wstring credits = GetParam(wscParam, ' ', 1);
		const std::wstring time = GetParam(wscParam, ' ', 2);
		if (!target.length() || !credits.length())
		{
			PrintUserCmdText(iClientID, L"Usage: /bountyhuntid <id> <credits> <time>");
			PrintBountyHunts(iClientID);
			return;
		}

		uint clientIdTarget = ToInt(target);
		if (!HkIsValidClientID(clientIdTarget) || HkIsInCharSelectMenu(clientIdTarget))
		{
			PrintUserCmdText(iClientID, L"Error: Invalid client id.");
			return;
		}

		const std::wstring charName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientIdTarget));
		const auto paramNew = std::wstring(charName + L" " + credits + L" " + time);
		UserCmdBountyHunt(iClientID, paramNew);
	}

	void BhTimeOutCheck()
	{
		for (auto& bounty : global->bountyHunt)
		{
			if (bounty.end < timeInMS())
			{
				HkAddCash(bounty.target, bounty.cash);
				HkMsgU(bounty.target + L" was not hunted down and earned " + std::to_wstring(bounty.cash) + L" credits.");
				RemoveBountyHunt(bounty);
				BhTimeOutCheck();
				break;
			}
		}
	}

	void BhKillCheck(uint clientId, uint killerId)
	{
		for (auto& bounty : global->bountyHunt)
		{
			if (bounty.targetId == clientId)
			{
				if (killerId == 0 || clientId == killerId)
				{
					HkMsgU(L"The hunt for " + bounty.target + L" still goes on.");
					continue;
				}
				
				if (std::wstring winnerCharacterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(killerId)); !winnerCharacterName.empty())
				{
					HkAddCash(winnerCharacterName, bounty.cash);
					HkMsgU(winnerCharacterName + L" has killed " + bounty.target + L" and earned " + std::to_wstring(bounty.cash) + L" credits.");
				}
				else 
					HkAddCash(bounty.initiator, bounty.cash);

				RemoveBountyHunt(bounty);
				BhKillCheck(clientId, killerId);
				break;
			}
		}
	}

	// Hooks
	using TimerFunc = void (*)();
	struct Timer
	{
		TimerFunc proc;
		mstime intervalMs;
		mstime lastCall;
	};

	Timer Timers[] = {
	    {BhTimeOutCheck, 2017, 0},
	};

	int __stdcall Update()
	{
		for (uint i = 0; (i < sizeof(Timers) / sizeof(Timer)); i++)
		{
			if ((timeInMS() - Timers[i].lastCall) >= Timers[i].intervalMs)
			{
				Timers[i].lastCall = timeInMS();
				Timers[i].proc();
			}
		}
		return 0;
	}

	void SendDeathMsg(const std::wstring& wscMsg, uint& iSystemID, uint& iClientIDVictim, uint& iClientIDKiller)
	{
		if (global->config->enableBountyHunt)
		{
			BhKillCheck(iClientIDVictim, iClientIDKiller);
		}
	}

	void __stdcall DisConnect(uint& iClientID, enum EFLConnection& state)
	{
		for (auto& it : global->bountyHunt)
		{
			if (it.targetId == iClientID)
			{
				HkMsgU(L"The coward " + it.target + L" has fled. " + it.initiator + L" has been refunded.");
				HkAddCash(it.initiator, it.cash);
				RemoveBountyHunt(it);
				return;
			}
		}
	}

	// Client command processing
	const std::array<USERCMD, 2> UserCmds = {{
	    {L"/bountyhunt", UserCmdBountyHunt},
	    {L"/bountyhuntid", UserCmdBountyHuntId},
	}};

	void UserCmdHelp(uint& iClientID, const std::wstring& wscParam)
	{
		PrintUserCmdText(iClientID, L"/bountyhunt <charname> <credits> [<minutes>]");
		PrintUserCmdText(iClientID, L"/bountyhuntid <id> <credits> [<minutes>]");
	}

	// Load Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

	}
} // namespace BountyHunt

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::BountyHunt;

bool ProcessUserCmds(uint& clientId, const std::wstring& param)
{
	return DefaultUserCommandHandling(clientId, param, UserCmds, global->returnCode);
}

REFL_AUTO(type(Config), field(enableBountyHunt), field(levelProtect))

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Bounty Hunt");
	pi->shortName("bountyhunt");
	pi->mayPause(false);
	pi->mayUnload(false);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &ProcessUserCmds);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmdHelp);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
}