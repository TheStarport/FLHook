/**
 * @date Dec, 2015
 * @author Discovery Devs, Raikkonen
 * @defgroup NPCControl NPC Control
 * @brief
 * The NPC Control plugin allows admins to spawn and control NPC ships using any ship loadout in the server files.
 * This is especially useful during events.
 *
 * @paragraph cmds Player Commands
 * None
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - aicreate [number] [name] - Creates X amount of the specified NPCs. The name for the NPC is configured in the json file.
 * - aidestroy - Destroys the targeted ship if it was spawned using this plugin. Otherwise, destroy all spawned ships.
 * - aicancel - Cancels the current command given to the npcs.
 * - aifollow [player] - Instructs all spawned ships to follow the targeted player, or the character specified in the command.
 * - aicome - Make the spawned ships fly to your position.
 * - aifleet [name] - Spawn a fleet of ships specified in the config file.
 * - fleetlist - List available fleets to spawn.
 * - npclist - List available singular NPCs.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "fleetInfo": {
 *         "example": {
 *             "member": {
 *                 "example": 5
 *             },
 *             "name": "example"
 *         }
 *     },
 *     "npcInfo": {
 *         "example": {
 *             "graph": "FIGHTER",
 *             "iff": "fc_fl_grp",
 *             "infocard2Id": 197809,
 *             "infocardId": 197808,
 *             "loadout": "MP_ge_fighter",
 *             "pilot": "pilot_pirate_ace",
 *             "shipArch": "ge_fighter"
 *         }
 *     },
 *     "npcInfocardIds": [
 *         197808
 *     ],
 *     "startupNpcs": [
 *         {
 *             "name": "example",
 *             "position": [
 *                 -33367.0,
 *                 120.0,
 *                 -28810.0
 *             ],
 *             "spawnChance": 1.0,
 *             "rotation": [
 *                 0.0,
 *                 0.0,
 *                 0.0
 *             ],
 *             "system": "li01"
 *         }
 *     ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * NpcCommunicator: exposes CreateNpc method with parameters (const std::wstring& name, Vector position, const Matrix& rotation, SystemId systemId, bool
 * varyPosition)
 */

#define SPDLOG_USE_STD_FORMAT
#include "NPCControl.h"

namespace Plugins::Npc
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup NPCControl
	 * @brief Returns a random float between two numbers
	 */
	float RandomFloatRange(float a, float b)
	{
		return ((b - a) * (static_cast<float>(rand()) / RAND_MAX)) + a;
	}

	/** @ingroup NPCControl
	 * @brief Return random infocard Id from list that was loaded in
	 */
	uint RandomInfocardID()
	{
		int randomIndex = rand() % global->config->npcInfocardIds.size();
		return global->config->npcInfocardIds.at(randomIndex);
	}

	/** @ingroup NPCControl
	 * @brief Checks if a ship is one of our spawned NPCs and if so, removes it from our data
	 */
	bool IsHookNPC(CShip* ship)
	{
		// If it's a player do nothing
		if (ship->is_player() == true)
		{
			return false;
		}

		// Is it a FLHook NPC?
		auto iter = global->spawnedNpcs.begin();
		while (iter != global->spawnedNpcs.end())
		{
			if (*iter == ship->get_id())
			{
				ship->clear_equip_and_cargo();
				global->spawnedNpcs.erase(iter);
				return true;
			}
			iter++;
		}
		return false;
	}

	/** @ingroup NPCControl
	 * @brief Hook on ship destroyed to remove from our data
	 */
	void ShipDestroyed([[maybe_unused]] DamageList** _dmg, [[maybe_unused]] const DWORD** ecx, const uint& kill)
	{
		if (kill)
		{
			CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
			IsHookNPC(cShip);
		}
	}

	/** @ingroup NPCControl
	 * @brief Checks to ensure an NPC is valid before attempting to spawn it. loadout, iff and pilot are all optional, but do require valid values to
	 * avoid a crash if populated
	 */
	bool CheckNpc(const std::wstring& npcName)
	{
		auto npc = global->config->npcInfo.find(npcName);
		if (npc == global->config->npcInfo.end())
		{
			Console::ConErr(std::format("The provided NPC name, '{}' was not found.", wstos(npcName)));
			return false;
		}

		bool validity = true;

		// Check solar solarArch is valid
		if (!Archetype::GetShip(CreateID(npc->second.shipArch.c_str())))
		{
			Console::ConErr(std::format("The shipArch '{}' for '{}' is invalid. Spawning this NPC may cause a crash", npc->second.shipArch, wstos(npcName)));
			validity = false;
		}

		// Check the loadout is valid
		EquipDescVector loadout;
		pub::GetLoadout(loadout, npc->second.loadoutId);

		if (!npc->second.loadout.empty() && loadout.equip.empty())
		{
			Console::ConErr(
			    std::format("The loadout '{}' loaded for '{}' is invalid. Spawning this NPC may cause a crash", npc->second.loadout, wstos(npcName)));
			validity = false;
		}

		// Check if solar iff is valid
		uint npcIff;
		pub::Reputation::GetReputationGroup(npcIff, npc->second.iff.c_str());
		if (!npc->second.iff.empty() && npcIff == UINT_MAX)
		{
			Console::ConErr(
			    std::format("The reputation '{}' loaded for '{}' is invalid. Spawning this NPC may cause a crash", npc->second.iff, wstos(npcName)));
			validity = false;
		}

		// Check solar pilot is valid
		if (!npc->second.pilot.empty() && !Hk::Personalities::GetPersonality(npc->second.pilot).has_value())
		{
			Console::ConErr(std::format("The pilot '{}' loaded for '{}' is invalid. Spawning this NPC may cause a crash", npc->second.pilot, wstos(npcName)));
			validity = false;
		}
		return validity;
	}

	/** @ingroup NPCControl
	 * @brief Function to spawn an NPC
	 */
	uint CreateNPC(const std::wstring& name, Vector position, const Matrix& rotation, SystemId systemId, bool varyPosition)
	{
		if (!CheckNpc(name))
		{
			Console::ConWarn(std::format("Unable to spawn '{}', invalid data was found in the npcInfo", wstos(name)));
			return 0;
		}

		Console::ConDebug(std::format("Spawning npc '{}'", wstos(name)));
		Npc arch = global->config->npcInfo[name];

		pub::SpaceObj::ShipInfo si;
		memset(&si, 0, sizeof(si));
		si.iFlag = 1;
		si.iSystem = systemId;
		si.shipArchetype = arch.shipArchId;
		si.mOrientation = rotation;
		si.iLoadout = arch.loadoutId;
		si.iLook1 = CreateID("li_newscaster_head_gen_hat");
		si.iLook2 = CreateID("pl_female1_journeyman_body");
		si.iComm = CreateID("comm_br_darcy_female");
		si.iPilotVoice = CreateID("pilot_f_leg_f01a");
		si.iHealth = -1;
		si.iLevel = 19;

		if (varyPosition)
		{
			si.vPos.x = position.x + RandomFloatRange(0, 1000);
			si.vPos.y = position.y + RandomFloatRange(0, 1000);
			si.vPos.z = position.z + RandomFloatRange(0, 2000);
		}
		else
		{
			si.vPos.x = position.x;
			si.vPos.y = position.y;
			si.vPos.z = position.z;
		}

		// Define the string used for the scanner name. Because the
		// following entry is empty, the pilot_name is used. This
		// can be overriden to display the ship type instead.
		FmtStr scanner_name(0, nullptr);
		scanner_name.begin_mad_lib(0);
		scanner_name.end_mad_lib();

		// Define the string used for the pilot name. The example
		// below shows the use of multiple part names.
		FmtStr pilot_name(0, nullptr);
		pilot_name.begin_mad_lib(16163); // ids of "%s0 %s1"
		if (arch.infocardId != 0)
		{
			pilot_name.append_string(arch.infocardId);
			if (arch.infocard2Id != 0)
			{
				pilot_name.append_string(arch.infocard2Id);
			}
		}
		else
		{
			pilot_name.append_string(RandomInfocardID()); // ids that replaces %s0
			pilot_name.append_string(RandomInfocardID()); // ids that replaces %s1
		}
		pilot_name.end_mad_lib();

		pub::Reputation::Alloc(si.iRep, scanner_name, pilot_name);
		pub::Reputation::SetAffiliation(si.iRep, arch.iffId);

		// Get personality
		pub::AI::SetPersonalityParams personalityParams;
		personalityParams.iStateGraph = pub::StateGraph::get_state_graph(arch.graph.c_str(), pub::StateGraph::TYPE_STANDARD);
		personalityParams.bStateId = true;
		const auto personality = Hk::Personalities::GetPersonality(arch.pilot);

		if (personality.has_error())
		{
			std::string errorMessage = arch.pilot + " is not recognised as a pilot name.";
			AddLog(LogType::Normal, LogLevel::Err, errorMessage);
			return 0;
		}

		personalityParams.personality = personality.value();

		// Create the ship in space
		uint spaceObj;
		pub::SpaceObj::Create(spaceObj, si);

		// Add the personality to the space obj
		pub::AI::SubmitState(spaceObj, &personalityParams);

		global->spawnedNpcs.push_back(spaceObj);

		constexpr auto level = static_cast<spdlog::level::level_enum>(LogLevel::Info);
		std::string logMessage = "Created " + wstos(name);
		global->log->log(level, logMessage);

		return spaceObj;
	}

	/** @ingroup NPCControl
	 * @brief Load plugin settings into memory
	 */
	void LoadSettings()
	{
		global->log = spdlog::basic_logger_mt<spdlog::async_factory>("npcs", "logs/npc.log");
		auto config = Serializer::JsonToObject<Config>();

		for (auto& [name, npc] : config.npcInfo)
		{
			npc.shipArchId = CreateID(npc.shipArch.c_str());
			npc.loadoutId = CreateID(npc.loadout.c_str());
			pub::Reputation::GetReputationGroup(npc.iffId, npc.iff.c_str());
		}

		for (auto& npc : config.startupNpcs)
		{
			// Check if defined startupNpcs are valid before spawning them
			if (!config.npcInfo.contains(npc.name))
			{
				Console::ConErr(std::format("Attempted to load an NPC that was not defined in npcInfo: {}", wstos(npc.name)));
				continue;
			}

			npc.systemId = CreateID(npc.system.c_str());

			npc.positionVector.x = npc.position[0];
			npc.positionVector.y = npc.position[1];
			npc.positionVector.z = npc.position[2];

			npc.rotationMatrix = EulerMatrix({npc.rotation[0], npc.rotation[1], npc.rotation[2]});
		}

		global->config = std::make_unique<Config>(config);
	}

	/** @ingroup NPCControl
	 * @brief Main Load Settings function, calls the one above. Had to use this hook instead of LoadSettings otherwise NPCs wouldnt appear on server startup
	 */
	void AfterStartup()
	{
		LoadSettings();

		// Check validity of NPCs on server startup and print any problems to the console
		for (const auto& [key, value] : global->config->npcInfo)
		{
			CheckNpc(key);
		}

		// Initalize for random number generator (new C++ 11 standard)
		std::random_device rd; // Used to obtain a seed
		std::mt19937 mt(rd()); //  Mersenne Twister algorithm seeded with the variable above
		std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		int spawned = 0;

		// Some checks on load, to ensure that values provided for NPCs are valid. Unfortunately we can't easily check loadouts or the state graph here
		for (const auto& [key, value] : global->config->npcInfo)
		{
			// Check if NPC iff is valid
			uint npcIff;
			pub::Reputation::GetReputationGroup(npcIff, value.iff.c_str());
			if (npcIff == UINT_MAX)
			{
				Console::ConErr(std::format("Loaded invalid reputation {}", value.iff));
			}

			// Check NPC pilot is valid
			if (!Hk::Personalities::GetPersonality(value.pilot).has_value())
			{
				Console::ConErr(std::format("Loaded invalid pilot {}", value.pilot));
			}

			// Check NPC shipArch is valid
			if (!Archetype::GetShip(CreateID(value.shipArch.c_str())))
			{
				Console::ConErr(std::format("Attempted to load invalid shipArch {}", value.shipArch));
			}
		}

		for (const auto& npc : global->config->startupNpcs)
		{
			// Check spawn chance is valid
			if (npc.spawnChance < 0 || npc.spawnChance > 1)
			{
				Console::ConErr(std::format("Spawn chance must be between 0 and 1 for NPC '{}'", wstos(npc.name)));
				continue;
			}

			// Spawn NPC if spawn chance allows it
			if (dist(mt) <= npc.spawnChance)
			{
				CreateNPC(npc.name, npc.positionVector, npc.rotationMatrix, npc.systemId, false);
				spawned++;
			}
		}
		Console::ConInfo(std::format("{} NPCs loaded on startup", spawned));
	}

	/** @ingroup NPCControl
	 * @brief Admin command to make NPCs
	 */
	void AdminCmdAIMake(CCmds* cmds, int amount, const std::wstring& NpcType)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		if (!amount)
			amount = 1;

		if (const auto& iter = global->config->npcInfo.find(NpcType); iter == global->config->npcInfo.end())
		{
			cmds->Print("ERR Wrong NPC name");
			return;
		}

		const auto ship = Hk::Player::GetShip(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value());
		if (!ship.has_value())
			return;

		SystemId system = Hk::Player::GetSystem(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value()).value();

		auto [position, rotation] = Hk::Solar::GetLocation(ship.value(), IdType::Ship).value();

		// Creation counter
		for (int i = 0; i < amount; i++)
		{
			CreateNPC(NpcType, position, rotation, system, true);
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command to destroy the AI
	 */
	void AdminCmdAIKill(CCmds* commands)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission");
			return;
		}

		// Destroy targeted ship
		if (commands->IsPlayer())
		{
			if (auto const target = Hk::Player::GetTarget(commands->GetAdminName()); target.has_value())
			{
				if (const auto it = std::ranges::find(global->spawnedNpcs, target.value()); target.value() && it != global->spawnedNpcs.end())
				{
					pub::SpaceObj::Destroy(target.value(), DestroyType::FUSE);
					commands->Print("OK");
					return;
				}
			}
		}

		// Destroy all ships - Copy into new vector since the old one will be modified in ShipDestroy hook
		for (std::vector<uint> tempSpawnedNpcs = global->spawnedNpcs; const auto& npc : tempSpawnedNpcs)
			pub::SpaceObj::Destroy(npc, DestroyType::FUSE);

		commands->Print("OK");
	}

	/** @ingroup NPCControl
	 * @brief Admin command to make AI come to your position
	 */
	void AdminCmdAICome(CCmds* commands)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission");
			return;
		}

		if (auto ship = Hk::Player::GetShip(Hk::Client::GetClientIdFromCharName(commands->GetAdminName()).value()); ship.has_value())
		{
			auto [pos, rot] = Hk::Solar::GetLocation(ship.value(), IdType::Ship).value();

			for (const auto& npc : global->spawnedNpcs)
			{
				pub::AI::DirectiveCancelOp cancelOP;
				pub::AI::SubmitDirective(npc, &cancelOP);

				pub::AI::DirectiveGotoOp go;
				go.iGotoType = 1;
				go.vPos = pos;
				go.vPos.x = pos.x + RandomFloatRange(0, 500);
				go.vPos.y = pos.y + RandomFloatRange(0, 500);
				go.vPos.z = pos.z + RandomFloatRange(0, 500);
				go.fRange = 0;
				pub::AI::SubmitDirective(npc, &go);
			}
		}
		commands->Print("OK");
		return;
	}

	void AiFollow(uint ship, uint npc)
	{
		pub::AI::DirectiveCancelOp cancelOP;
		pub::AI::SubmitDirective(npc, &cancelOP);
		pub::AI::DirectiveFollowOp testOP;
		testOP.iFollowSpaceObj = ship;
		testOP.fMaxDistance = 100;
		pub::AI::SubmitDirective(npc, &testOP);
	}

	/** @ingroup NPCControl
	 * @brief Admin command to make AI follow target (or admin) until death
	 */
	void AdminCmdAIFollow(CCmds* commands, std::wstring characterName)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission");
			return;
		}

		// If no player specified follow the admin
		uint client;
		if (characterName == L"")
		{
			client = Hk::Client::GetClientIdFromCharName(commands->GetAdminName()).value();
			characterName = commands->GetAdminName();
		}
		// Follow the player specified
		else
			client = Hk::Client::GetClientIdFromCharName(characterName).value();

		if (client == UINT_MAX)
			commands->Print(std::format("{} is not online", wstos(characterName)));

		else
		{
			const auto ship = Hk::Player::GetShip(client);
			if (ship.has_value())
			{
				if (const auto target = Hk::Player::GetTarget(client); target.has_value())
				{
					if (const auto it = std::ranges::find(global->spawnedNpcs, target.value()); target.value() && it != global->spawnedNpcs.end())
					{
						AiFollow(ship.value(), target.value());
					}
					else
					{
						for (const auto& npc : global->spawnedNpcs)
							AiFollow(ship.value(), npc);
					}
					commands->Print(std::format("Following {}", wstos(characterName)));
				}
			}
			else
			{
				commands->Print(std::format("{} is not in space", wstos(characterName)));
			}
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command to cancel the current operation
	 */
	void AdminCmdAICancel(CCmds* commands)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission");
			return;
		}

		// Is the admin targeting an NPC?
		if (const auto target = Hk::Player::GetTarget(commands->GetAdminName()); target.has_value())
		{
			if (const auto it = std::ranges::find(global->spawnedNpcs, target.value()); target.value() && it != global->spawnedNpcs.end())
			{
				pub::AI::DirectiveCancelOp cancelOp;
				pub::AI::SubmitDirective(target.value(), &cancelOp);
			}
		}
		// Cancel all NPC actions
		else
		{
			for (const auto& npc : global->spawnedNpcs)
			{
				pub::AI::DirectiveCancelOp cancelOp;
				pub::AI::SubmitDirective(npc, &cancelOp);
			}
		}
		commands->Print("OK");
	}

	/** @ingroup NPCControl
	 * @brief Admin command to list NPCs
	 */
	void AdminCmdListNPCs(CCmds* commands)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission");
			return;
		}

		commands->Print(std::format("Available NPCs: {}", global->config->npcInfo.size()));

		for (auto const& [name, npc] : global->config->npcInfo)
			commands->Print(std::format("|{}", wstos(name)));
	}

	/** @ingroup NPCControl
	 * @brief Admin command to list NPC fleets
	 */
	void AdminCmdListNPCFleets(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		cmds->Print(std::format("Available fleets: {}", global->config->fleetInfo.size()));

		for (auto const& [name, npc] : global->config->fleetInfo)
			cmds->Print(std::format("|{}", wstos(name)));
	}

	/** @ingroup NPCControl
	 * @brief Admin command to spawn a Fleet
	 */
	void AdminCmdAIFleet(CCmds* commands, const std::wstring& fleetName)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission");
			return;
		}

		if (const auto& iter = global->config->fleetInfo.find(fleetName); iter != global->config->fleetInfo.end())
			for (auto const& [name, amount] : iter->second.member)
				AdminCmdAIMake(commands, amount, name);
		else
		{
			commands->Print("ERR Wrong Fleet name");
			return;
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command processing
	 */
	bool ExecuteCommandString(CCmds* commands, const std::wstring& cmd)
	{
		global->returnCode = ReturnCode::SkipAll;

		if (cmd == L"aicreate")
			AdminCmdAIMake(commands, commands->ArgInt(1), commands->ArgStr(2));
		else if (cmd == L"aidestroy")
			AdminCmdAIKill(commands);
		else if (cmd == L"aicancel")
			AdminCmdAICancel(commands);
		else if (cmd == L"aifollow")
			AdminCmdAIFollow(commands, commands->ArgCharname(1));
		else if (cmd == L"aicome")
			AdminCmdAICome(commands);
		else if (cmd == L"aifleet")
			AdminCmdAIFleet(commands, commands->ArgStr(1));
		else if (cmd == L"fleetlist")
			AdminCmdListNPCFleets(commands);
		else if (cmd == L"npclist")
			AdminCmdListNPCs(commands);
		else
		{
			global->returnCode = ReturnCode::Default;
			return false;
		}

		return true;
	}

	NpcCommunicator::NpcCommunicator(const std::string& plug) : PluginCommunicator(plug)
	{
		this->CreateNpc = CreateNPC;
	}
} // namespace Plugins::Npc

using namespace Plugins::Npc;

DefaultDllMainSettings(AfterStartup);

REFL_AUTO(type(Npc), field(shipArch), field(loadout), field(iff), field(infocardId), field(infocard2Id), field(pilot), field(graph));
REFL_AUTO(type(Fleet), field(name), field(member));
REFL_AUTO(type(StartupNpc), field(name), field(system), field(position), field(rotation), field(spawnChance));
REFL_AUTO(type(Config), field(npcInfo), field(fleetInfo), field(startupNpcs), field(npcInfocardIds));

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(NpcCommunicator::pluginName);
	pi->shortName("npc");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Startup, &AfterStartup, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);

	// Register IPC
	global->communicator = new NpcCommunicator(NpcCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}