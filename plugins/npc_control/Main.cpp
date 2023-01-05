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
 * This plugin does not expose any functionality.
 */

#include "Main.h"

namespace Plugins::Npc
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup NPCControl
	 * @brief Returns the specified personality
	 */
	pub::AI::SetPersonalityParams MakePersonality(std::string graph, const std::string& personality)
	{
		pub::AI::SetPersonalityParams p;
		p.iStateGraph = pub::StateGraph::get_state_graph(graph.c_str(), pub::StateGraph::TYPE_STANDARD);
		p.bStateId = true;
		const auto err = Hk::Personalities::GetPersonality(personality);

		if (err.has_error())
		{
			std::wstring errorMessage = stows(personality) + L" is not recognised as a pilot name.";
			Console::ConErr(errorMessage);
			AddLog(LogType::Normal, LogLevel::Critical, wstos(errorMessage));
		}

		p.personality = err.value();
		return p;
	}

	/** @ingroup NPCControl
	 * @brief Returns a random float between two numbers
	 */
	float RandomFloatRange(float a, float b) { return ((b - a) * (static_cast<float>(rand()) / RAND_MAX)) + a; }

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
	bool IsFLHookNPC(CShip* ship)
	{
		// If it's a player do nothing
		if (ship->is_player() == true)
		{
			return false;
		}

		// Is it a FLHook NPC?
		std::vector<uint>::iterator iter = global->spawnedNpcs.begin();
		while (iter != global->spawnedNpcs.end())
		{
			if (*iter == ship->get_id())
			{
				ship->clear_equip_and_cargo();
				global->spawnedNpcs.erase(iter);
				return true;
				break;
			}
			iter++;
		}
		return false;
	}

	/** @ingroup NPCControl
	 * @brief Hook on ship destroyed to remove from our data
	 */
	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
	{
		if (iKill)
		{
			CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
			IsFLHookNPC(cShip);
		}
	}

	/** @ingroup NPCControl
	 * @brief Function to spawn an NPC
	 */
	void CreateNPC(std::wstring& name, Vector position, Matrix rotation, uint systemId, bool varyPosition)
	{
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
		FmtStr scanner_name(0, 0);
		scanner_name.begin_mad_lib(0);
		scanner_name.end_mad_lib();

		// Define the string used for the pilot name. The example
		// below shows the use of multiple part names.
		FmtStr pilot_name(0, 0);
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

		uint spaceObj;
		pub::SpaceObj::Create(spaceObj, si);

		pub::AI::SetPersonalityParams personality = MakePersonality(arch.graph, arch.pilot);
		pub::AI::SubmitState(spaceObj, &personality);

		global->spawnedNpcs.push_back(spaceObj);

		constexpr auto level = static_cast<spdlog::level::level_enum>(LogLevel::Info);
		std::string logMessage = "Created " + wstos(name);
		global->Log->log(level, logMessage);

		return;
	}

	/** @ingroup NPCControl
	 * @brief Load plugin settings into memory
	 */
	void LoadSettings()
	{
		global->Log = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_npcs", "flhook_logs/npc.log");
		auto config = Serializer::JsonToObject<Config>();

		for (auto& [name, npc] : config.npcInfo)
		{
			npc.shipArchId = CreateID(npc.shipArch.c_str());
			npc.loadoutId = CreateID(npc.loadout.c_str());
			pub::Reputation::GetReputationGroup(npc.iffId, npc.iff.c_str());
		}

		for (auto& npc : config.startupNpcs)
		{
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

		for (auto& npc : global->config->startupNpcs)
			CreateNPC(npc.name, npc.positionVector, npc.rotationMatrix, npc.systemId, false);
	}

	/** @ingroup NPCControl
	 * @brief Admin command to make NPCs
	 */
	void AdminCmdAIMake(CCmds* cmds, int Amount, std::wstring NpcType)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		if (!Amount)
			Amount = 1;

		if (const std::map<std::wstring, Npc>::iterator iter = global->config->npcInfo.find(NpcType); iter == global->config->npcInfo.end())
		{
			cmds->Print(L"ERR Wrong NPC name");
			return;
		}

		uint shipId;
		pub::Player::GetShip(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value(), shipId);
		if (!shipId)
			return;

		SystemId iSystem = Hk::Player::GetSystem(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value()).value();

		Vector position;
		Matrix rotation;
		pub::SpaceObj::GetLocation(shipId, position, rotation);

		// Creation counter
		for (int i = 0; i < Amount; i++)
		{
			CreateNPC(NpcType, position, rotation, iSystem, true);
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command to destroy the AI
	 */
	void AdminCmdAIKill(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		// Destroy targeted ship
		if (cmds->IsPlayer())
		{
			uint ship;
			pub::Player::GetShip(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value(), ship);
			uint target = Hk::Player::GetTarget(ship).value();
			if (const auto it = std::find(global->spawnedNpcs.begin(), global->spawnedNpcs.end(), target); target && it != global->spawnedNpcs.end())
			{
				pub::SpaceObj::Destroy(target, DestroyType::FUSE);
				global->spawnedNpcs.erase(it);
				cmds->Print(L"OK");
				return;
			}
		}

		// Destroy all ships
		for (const auto& npc : global->spawnedNpcs)
			pub::SpaceObj::Destroy(npc, DestroyType::FUSE);

		global->spawnedNpcs.clear();
		cmds->Print(L"OK");
	}

	/** @ingroup NPCControl
	 * @brief Admin command to make AI come to your position
	 */
	void AdminCmdAICome(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		uint ship1;
		pub::Player::GetShip(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value(), ship1);
		if (ship1)
		{
			Vector pos;
			Matrix rot;
			pub::SpaceObj::GetLocation(ship1, pos, rot);

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
		cmds->Print(L"OK");
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
	void AdminCmdAIFollow(CCmds* cmds, std::wstring& wscCharname)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		// If no player specified follow the admin
		uint client;
		if (wscCharname == L"")
		{
			client = Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value();
			wscCharname = cmds->GetAdminName();
		}
		// Follow the player specified
		else
			client = Hk::Client::GetClientIdFromCharName(wscCharname).value();

		if (client == -1)
			cmds->Print(L"%s is not online", wscCharname.c_str());

		else
		{
			uint ship;
			pub::Player::GetShip(client, ship);
			if (ship)
			{
				uint target = Hk::Player::GetTarget(ship).value();
				if (const auto it = std::find(global->spawnedNpcs.begin(), global->spawnedNpcs.end(), target); target && it != global->spawnedNpcs.end())
				{
					AiFollow(ship, target);
					global->spawnedNpcs.erase(it);
				}
				else
				{
					for (const auto& npc : global->spawnedNpcs)
						AiFollow(ship, npc);
				}
				cmds->Print(L"Following %s", wscCharname.c_str());
			}
			else
			{
				cmds->Print(L"%s is not in space", wscCharname.c_str());
			}
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command to cancel the current operation
	 */
	void AdminCmdAICancel(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		uint ship;
		pub::Player::GetShip(Hk::Client::GetClientIdFromCharName(cmds->GetAdminName()).value(), ship);
		if (ship)
		{
			// Is the admin targeting an NPC?
			uint target = Hk::Player::GetTarget(ship).value();
			if (const auto it = std::find(global->spawnedNpcs.begin(), global->spawnedNpcs.end(), target); target && it != global->spawnedNpcs.end())
			{
				pub::AI::DirectiveCancelOp cancelOp;
				pub::AI::SubmitDirective(target, &cancelOp);
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
			cmds->Print(L"OK");
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command to list NPCs
	 */
	void AdminCmdListNPCs(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		cmds->Print(L"Available NPCs: %d", global->config->npcInfo.size());

		for (auto& [name, npc] : global->config->npcInfo)
			cmds->Print(L"|%s", name.c_str());
	}

	/** @ingroup NPCControl
	 * @brief Admin command to list NPC fleets
	 */
	void AdminCmdListNPCFleets(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		cmds->Print(L"Available fleets: %d", global->config->fleetInfo.size());

		for (auto& [name, npc] : global->config->fleetInfo)
			cmds->Print(L"|%s", name.c_str());
	}

	/** @ingroup NPCControl
	 * @brief Admin command to spawn a Fleet
	 */
	void AdminCmdAIFleet(CCmds* cmds, std::wstring FleetName)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		if (const std::map<std::wstring, Fleet>::iterator iter = global->config->fleetInfo.find(FleetName); iter != global->config->fleetInfo.end())
			for (auto& [name, amount] : iter->second.member)
				AdminCmdAIMake(cmds, amount, name);
		else
		{
			cmds->Print(L"ERR Wrong Fleet name");
			return;
		}
	}

	/** @ingroup NPCControl
	 * @brief Admin command processing
	 */
	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		global->returnCode = ReturnCode::SkipAll;

		if (wscCmd == L"aicreate")
			AdminCmdAIMake(cmds, cmds->ArgInt(1), cmds->ArgStr(2));
		else if (wscCmd == L"aidestroy")
			AdminCmdAIKill(cmds);
		else if (wscCmd == L"aicancel")
			AdminCmdAICancel(cmds);
		else if (wscCmd == L"aifollow")
			AdminCmdAIFollow(cmds, cmds->ArgCharname(1));
		else if (wscCmd == L"aicome")
			AdminCmdAICome(cmds);
		else if (wscCmd == L"aifleet")
			AdminCmdAIFleet(cmds, cmds->ArgStr(1));
		else if (wscCmd == L"fleetlist")
			AdminCmdListNPCFleets(cmds);
		else if (wscCmd == L"npclist")
			AdminCmdListNPCs(cmds);
		else
		{
			global->returnCode = ReturnCode::Default;
			return false;
		}

		return true;
	}
} // namespace Plugins::Npc

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::Npc;

DefaultDllMainSettings(AfterStartup);

REFL_AUTO(type(Npc), field(shipArch), field(loadout), field(iff), field(infocardId), field(infocard2Id), field(pilot), field(graph));
REFL_AUTO(type(Fleet), field(name), field(member)) REFL_AUTO(type(StartupNpc), field(name), field(system), field(position), field(rotation));
REFL_AUTO(type(Config), field(npcInfo), field(fleetInfo), field(startupNpcs), field(npcInfocardIds));

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("NPC Control");
	pi->shortName("npc");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Startup, &AfterStartup, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}