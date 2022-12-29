/**
 * @date August, 2022
 * @author Ported from 88Flak by Raikkonen
 * @defgroup DeathPenalty Death Penalty
 * @brief
 * This plugin charges players credits for dying based on their ship worth. If the killer was a player it also rewards them.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - dp - Shows the credits you would be charged if you died.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "DeathPenaltyFraction": 1.0,
 *     "DeathPenaltyFractionKiller": 1.0,
 *     "ExcludedSystems": ["li01"],
 *     "FractionOverridesByShip": {{"ge_fighter",1.0}}
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "Main.h"

namespace Plugins::DeathPenalty
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// Load configuration file
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		for (auto& system : config.ExcludedSystems)
			global->ExcludedSystemsIds.push_back(CreateID(system.c_str()));

		for (auto& override : config.FractionOverridesByShip)
			global->FractionOverridesByShipIds[CreateID(override.first.c_str())] = override.second;
	}

	void ClearClientInfo(ClientId& client) { global->MapClients.erase(client); }

	/** @ingroup DeathPenalty
	 * @brief Is the player is a system that is excluded from death penalty?
	 */
	bool bExcludedSystem(ClientId client)
	{
		// Get System Id
		uint iSystemId;
		pub::Player::GetSystem(client, iSystemId);
		// Search list for system
		return (std::find(global->ExcludedSystemsIds.begin(), global->ExcludedSystemsIds.end(), iSystemId) != global->ExcludedSystemsIds.end());
	}

	
	/** @ingroup DeathPenalty
	 * @brief This returns the override for the specific ship as defined in the json file.
	 * If there is not override it returns the default value defined as
	 * "DeathPenaltyFraction" in the json file
	 */
	float fShipFractionOverride(ClientId client)
	{
		// Get ShipArchId
		uint shipArchId;
		pub::Player::GetShipID(client, shipArchId);

		// Default return value is the default death penalty fraction
		float fOverrideValue = global->config->DeathPenaltyFraction;

		// See if the ship has an override fraction
		if (global->FractionOverridesByShipIds.find(shipArchId) != global->FractionOverridesByShipIds.end())
			fOverrideValue = global->FractionOverridesByShipIds[shipArchId];

		return fOverrideValue;
	}

	/** @ingroup DeathPenalty
	 * @brief Hook on Player Launch. Used to work out the death penalty and display a message to the player warning them of such
	 */
	void __stdcall PlayerLaunch(uint& ship, ClientId& client)
	{
		// No point in processing anything if there is no death penalty
		if (global->config->DeathPenaltyFraction > 0.00001f)
		{
			// Check to see if the player is in a system that doesn't have death
			// penalty
			if (!bExcludedSystem(client))
			{
				// Get the players net worth
				float fValue;
				pub::Player::GetAssetValue(client, fValue);

				const auto cash = Hk::Player::GetCash(client);
				if (cash.has_error())
				{
					Console::ConWarn(L"Unable to get cash of undocking player: %s", Hk::Err::ErrGetText(cash.error()));
					return;
				}

				auto dpCredits = static_cast<uint>(fValue * fShipFractionOverride(client));
				if (cash < dpCredits)
					dpCredits = cash.value();

				// Calculate what the death penalty would be upon death
				global->MapClients[client].DeathPenaltyCredits = dpCredits;

				// Should we print a death penalty notice?
				if (global->MapClients[client].bDisplayDPOnLaunch)
					PrintUserCmdText(client, L"Notice: the death penalty for your ship will be " + ToMoneyStr(global->MapClients[client].DeathPenaltyCredits) +
					        L" credits.  Type /dp for more information.");
			}
			else
				global->MapClients[client].DeathPenaltyCredits = 0;
		}
	}

	/** @ingroup DeathPenalty
	 * @brief Load settings directly from the player's save directory
	 */
	void LoadUserCharSettings(ClientId& client)
	{
		// Get Account directory then flhookuser.ini file
		CAccount* acc = Players.FindAccountFromClientID(client);
		std::wstring dir = Hk::Client::GetAccountDirName(acc);
		std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";

		// Get char filename and save setting to flhookuser.ini
		const auto wscFilename = Hk::Client::GetCharFileName(client);
		std::string scFilename = wstos(wscFilename.value());
		std::string scSection = "general_" + scFilename;

		// read death penalty settings
		CLIENT_DATA c;
		c.bDisplayDPOnLaunch = IniGetB(scUserFile, scSection, "DPnotice", true);
		global->MapClients[client] = c;
	}

	/** @ingroup DeathPenalty
	 * @brief Apply the death penalty on a player death
	 */
	void PenalizeDeath(ClientId client, uint iKillerId)
	{
		if (global->config->DeathPenaltyFraction < 0.00001f)
			return;

		// Valid client and the ShipArch or System isnt in the excluded list?
		if (client != -1 && !bExcludedSystem(client))
		{
			// Get the players cash
			const auto cash = Hk::Player::GetCash(client);
			if (cash.has_error())
			{
				Console::ConWarn(L"Unable to get cash from client.");
				return;
			}

			// Get how much the player owes
			uint uOwed = global->MapClients[client].DeathPenaltyCredits;

			// If the amount the player owes is more than they have, set the
			// amount to their total cash
			if (uOwed > cash.value())
				uOwed = cash.value();

			// If another player has killed the player
			if (iKillerId != client && global->config->DeathPenaltyFractionKiller)
			{
				uint uGive = (uOwed * global->config->DeathPenaltyFractionKiller);
				if (uGive)
				{
					// Reward the killer, print message to them
					Hk::Player::AddCash(iKillerId, uGive);
					PrintUserCmdText(iKillerId, L"Death penalty: given " + ToMoneyStr(uGive) + L" credits from %s's death penalty.",
					    Players.GetActiveCharacterName(client));
				}
			}

			if (uOwed)
			{
				// Print message to the player and remove cash
				PrintUserCmdText(client, L"Death penalty: charged " + ToMoneyStr(uOwed) + L" credits.");
				Hk::Player::RemoveCash(client, uOwed);
			}
		}
	}

	/** @ingroup DeathPenalty
	 * @brief Hook on ShipDestroyed to kick off PenalizeDeath
	 */
	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
	{
		if (iKill)
		{
			// Get client
			const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
			ClientId client = cShip->GetOwnerPlayer();

			// Get Killer Id if there is one
			uint iKillerId = 0;
			if (client)
			{
				const DamageList* dmg = *_dmg;
				const auto inflictor = dmg->get_cause() == DamageCause::Unknown ? Hk::Client::GetClientIdByShip(ClientInfo[client].dmgLast.get_inflictor_id())
				                                                     : Hk::Client::GetClientIdByShip(dmg->get_inflictor_id());
				if (inflictor.has_value())
				{
					iKillerId = inflictor.value();
				}
			}

			// Call function to penalize player and reward killer
			PenalizeDeath(client, iKillerId);
		}
	}

	/** @ingroup DeathPenalty
	 * @brief This will save whether the player wants to receieve the /dp notice or not to the flhookuser.ini file
	 */
	void SaveDPNoticeToCharFile(ClientId client, std::string value)
	{
		CAccount* acc = Players.FindAccountFromClientID(client);
		std::wstring dir = Hk::Client::GetAccountDirName(acc);
		if (const auto file = Hk::Client::GetCharFileName(client); file.has_value())
		{
			std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
			std::string scSection = "general_" + wstos(file.value());
			IniWrite(scUserFile, scSection, "DPnotice", value);
		}
	}

	/** @ingroup DeathPenalty
	 * @brief /dp command. Shows information about death penalty
	 */
	void UserCmd_DP(ClientId& client, const std::wstring& wscParam)
	{
		// If there is no death penalty, no point in having death penalty commands
		if (std::abs(global->config->DeathPenaltyFraction) < 0.0001f)
		{
			Console::ConWarn(L"DP Plugin active, but no/too low death penalty fraction is set.");
			return;
		}

		const auto param = ViewToWString(wscParam);
		if (wscParam.length()) // Arguments passed
		{
			if (ToLower(Trim(param)) == L"off")
			{
				global->MapClients[client].bDisplayDPOnLaunch = false;
				SaveDPNoticeToCharFile(client, "no");
				PrintUserCmdText(client, L"Death penalty notices disabled.");
			}
			else if (ToLower(Trim(param)) == L"on")
			{
				global->MapClients[client].bDisplayDPOnLaunch = true;
				SaveDPNoticeToCharFile(client, "yes");
				PrintUserCmdText(client, L"Death penalty notices enabled.");
			}
			else
			{
				PrintUserCmdText(client, L"ERR Invalid parameters");
				PrintUserCmdText(client, L"/dp on | /dp off");
			}
		}
		else
		{
			PrintUserCmdText(client, L"The death penalty is charged immediately when you die.");
			if (!bExcludedSystem(client))
			{
				float fValue;
				pub::Player::GetAssetValue(client, fValue);
				uint uOwed = static_cast<uint>(fValue * fShipFractionOverride(global->config->DeathPenaltyFraction));
				PrintUserCmdText(client, L"The death penalty for your ship will be " + ToMoneyStr(uOwed) + L" credits.");
				PrintUserCmdText(client,
				    L"If you would like to turn off the death penalty notices, run "
				    L"this command with the argument \"off\".");
			}
			else
			{
				PrintUserCmdText(client,
				    L"You don't have to pay the death penalty "
				    L"because you are in a specific system.");
			}
		}
	}

	// Define usable chat commands here
	const std::vector commands = {{
	    CreateUserCommand(L"/dp", L"", UserCmd_DP, L"Shows the credits you would be charged if you died."),
	}};
} // namespace Plugins::DeathPenalty

using namespace Plugins::DeathPenalty;

REFL_AUTO(type(Config), field(DeathPenaltyFraction), field(DeathPenaltyFractionKiller), field(ExcludedSystems), field(FractionOverridesByShip))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Death Penalty");
	pi->shortName("death_penalty");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::FLHook__LoadCharacterSettings, &LoadUserCharSettings);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
}
