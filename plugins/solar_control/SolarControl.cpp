/**
 * @date August, 2023
 * @author Raikkonen
 * @defgroup SolarControl Solar Control
 * @brief
 * The Solar Control plugin allows Solar Objects to be spawned.
 * It also contains fixes for Nomad Airlocks not being dockable, and mission solars missing a loadout
 *
 * @paragraph cmds Player Commands
 * None
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - solarcreate [number] [name] - Creates X amount of the specified Solars. The name for the Solar is configured in the json file.
 * - solardestroy - Destroys all spawned solars.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "solarArches": {
 *         "osiris": {
 *             "base": "Li01_15_base",
 *             "iff": "fc_or_grp",
 *             "infocard": 217114,
 *             "loadout": "or_osiris",
 *             "pilot": "pilot_solar_hardest",
 *             "solarArch": "o_osiris"
 *         },
 *         "osirisb": {
 *             "base": "St01_02_base",
 *             "iff": "fc_or_grp",
 *             "infocard": 217114,
 *             "loadout": "or_osiris",
 *             "pilot": "pilot_solar_hardest",
 *             "solarArch": "o_osiris"
 *         }
 *     },
 *     "startupSolars": [
 *         {
 *             "name": "osiris",
 *             "position": [
 *                 -30367.0,
 *                 120.0,
 *                 -25810.0
 *             ],
 *             "rotation": [
 *                 0.0,
 *                 0.0,
 *                 0.0
 *             ],
 *             "system": "li01"
 *         },
 *         {
 *             "name": "osirisb",
 *             "position": [
 *                 5000.0,
 *                 0.0,
 *                 0.0
 *             ],
 *             "rotation": [
 *                 0.0,
 *                 0.0,
 *                 0.0
 *             ],
 *             "system": "st01"
 *         }
 *     ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * SolarCommunicator: exposes CreateUserDefinedSolar method with parameters (const std::wstring& name, Vector position, const Matrix& rotation, SystemId systemId, bool
 * varyPosition, bool mission)
 */

#include "SolarControl.h"

namespace Plugins::SolarControl
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup SolarControl
	 * @brief Returns a random float between two numbers
	 */
	float RandomFloatRange(const float firstNumber, const float secondNumber)
	{
		return ((secondNumber - firstNumber) * (static_cast<float>(rand()) / RAND_MAX)) + firstNumber;
	}

	/** @ingroup SolarControl
	 * @brief Sends a custom Solar Packet for the spaceId using the solarInfo struct. A modified packet is needed because vanilla misses the loadout
	 */
	void SendSolarPacket(uint& spaceId, pub::SpaceObj::SolarInfo& solarInfo)
	{
		uint unknown;
		if (IObjInspectImpl * inspect; GetShipInspect(spaceId, inspect, unknown))
		{
			auto const* solar = reinterpret_cast<CSolar const*>(inspect->cobject());
			
			solar->launch_pos(solarInfo.vPos, solarInfo.mOrientation, 1);

			struct SolarStruct
			{
				std::byte unknown[0x100];
			};

			SolarStruct solarPacket {};

			const std::byte* address1 = reinterpret_cast<std::byte*>(hModServer) + 0x163F0;
			const std::byte* address2 = reinterpret_cast<std::byte*>(hModServer) + 0x27950;

			// fill struct
			__asm
			{
			lea ecx, solarPacket
			mov eax, address1
			call eax
			push solar
			lea ecx, solarPacket
			push ecx
			mov eax, address2
			call eax
			add esp, 8
			}

			// Send packet to every client in the system
			PlayerData* playerData = nullptr;
			while (playerData = Players.traverse_active(playerData))
			{
				if (playerData->systemId == solarInfo.systemId)
					GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(playerData->iOnlineId, reinterpret_cast<FLPACKET_CREATESOLAR&>(solarPacket));
			}
		}
	}

	/** @ingroup SolarControl
	 * @brief Creates a solar from a solarInfo struct
	 */
	int CreateSolar(uint& spaceId, pub::SpaceObj::SolarInfo& solarInfo)
	{
		// Hack server.dll so it does not call create solar packet send
		char* serverHackAddress = reinterpret_cast<char*>(hModServer) + 0x2A62A;
		constexpr char serverHack[] = {'\xEB'};
		WriteProcMem(serverHackAddress, &serverHack, 1);

		// Create the Solar
		const int returnValue = pub::SpaceObj::CreateSolar(spaceId, solarInfo);

		// Send solar creation packet
		SendSolarPacket(spaceId, solarInfo);

		// Undo the server.dll hack
		constexpr char serverUnHack[] = {'\x74'};
		WriteProcMem(serverHackAddress, &serverUnHack, 1);

		return returnValue;
	}

	/** @ingroup SolarControl
	 * @brief Returns a loaded personality
	 */
	pub::AI::SetPersonalityParams GetPersonality(const std::string& personalityString)
	{
		pub::AI::SetPersonalityParams p;
		p.iStateGraph = pub::StateGraph::get_state_graph("STATION", pub::StateGraph::TYPE_STANDARD);
		p.bStateId = true;

		if (const auto personality = Hk::Personalities::GetPersonality(personalityString); personality.has_error())
		{
			AddLog(LogType::Normal, LogLevel::Critical, std::format("{} is not recognised as a pilot name.", personalityString));
		}
		else
		{
			p.personality = personality.value();
		}

		return p;
	}

	/** @ingroup SolarControl
	 * @brief Creates a solar defined in the solar json file
	 */
	uint CreateUserDefinedSolar(const std::wstring& name, Vector position, const Matrix& rotation, SystemId system, bool varyPosition, bool mission)
	{
		SolarArch arch = global->config->solarArches[name];

		pub::SpaceObj::SolarInfo si {};
		memset(&si, 0, sizeof(si));
		si.iFlag = 4;

		// Prepare the settings for the space object
		si.iArchId = arch.solarArchId;
		si.iLoadoutId = arch.loadoutId;
		si.iHitPointsLeft = 1000;
		si.systemId = system;
		si.mOrientation = rotation;
		si.Costume.head = CreateID("benchmark_male_head");
		si.Costume.body = CreateID("benchmark_male_body");
		si.Costume.lefthand = 0;
		si.Costume.righthand = 0;
		si.Costume.accessories = 0;
		si.iVoiceId = CreateID("atc_leg_m01");
		si.iRep = MakeId("fc_lr_grp");
		std::string npcId = wstos(name) + std::to_string(global->spawnedSolars.size());
		strncpy_s(si.cNickName, sizeof(si.cNickName), npcId.c_str(), name.size() + global->spawnedSolars.size());

		// Do we need to vary the starting position slightly? Useful when spawning multiple objects
		si.vPos = position;
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

		// Which base this links to
		si.baseId = arch.baseId;

		// Mission base?
		if (mission)
		{
			si.mission = 1;
		}

		// Define the string used for the scanner name.
		// Because this is empty, the solar_name will be used instead.
		FmtStr scanner_name(0, nullptr);
		scanner_name.begin_mad_lib(0);
		scanner_name.end_mad_lib();

		// Define the string used for the solar name.
		FmtStr solar_name(0, nullptr);
		solar_name.begin_mad_lib(16163); // ids of "%s0 %s1"
		solar_name.append_string(arch.infocard);
		solar_name.end_mad_lib();

		// Set Reputation
		pub::Reputation::Alloc(si.iRep, scanner_name, solar_name);
		uint iff;
		pub::Reputation::GetReputationGroup(iff, arch.iff.c_str());
		pub::Reputation::SetAffiliation(si.iRep, iff);

		// Spawn the solar object
		uint iSpaceObj;
		CreateSolar(iSpaceObj, si);

		pub::AI::SetPersonalityParams personalityParams = GetPersonality(arch.pilot);
		pub::AI::SubmitState(iSpaceObj, &personalityParams);

		// Set the visible health for the Space Object
		pub::SpaceObj::SetRelativeHealth(iSpaceObj, 1);

		global->spawnedSolars[iSpaceObj] = si;

		return iSpaceObj;
	}

	/** @ingroup SolarControl
	 * @brief Admin command to create a user defined solar
	 */
	void AdminCmd_SolarMake(CCmds* commands, int amount, const std::wstring& solarType)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission\n");
			return;
		}

		if (amount == 0)
			amount = 1;

		if (const auto iter = global->config->solarArches.find(solarType); iter != global->config->solarArches.end())
			SolarArch arch = iter->second;
		else
		{
			commands->Print("ERR Wrong Solar name\n");
			return;
		}

		const uint client = Hk::Client::GetClientIdFromCharName(commands->GetAdminName()).value();
		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
			return;

		uint system;
		pub::Player::GetSystem(client, system);

		Vector pos {};
		Matrix rot {};
		pub::SpaceObj::GetLocation(ship, pos, rot);

		for (int i = 0; i < amount; i++)
			CreateUserDefinedSolar(solarType, pos, rot, system, true, false);
	}

	/** @ingroup SolarControl
	 * @brief Admin command to delete all spawned solars
	 */
	void AdminCmd_SolarKill(CCmds* commands)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission\n");
			return;
		}

		for (auto const& [id, name] : global->spawnedSolars)
			pub::SpaceObj::SetRelativeHealth(id, 0.0f);

		global->spawnedSolars.clear();
		commands->Print("OK\n");

		return;
	}

	/** @ingroup SolarControl
	 * @brief Proceessing for admin commands
	 */
	bool AdminCommandProcessing(CCmds* commands, const std::wstring& command)
	{
		if (command == L"solarcreate")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_SolarMake(commands, commands->ArgInt(1), commands->ArgStr(2));
			return true;
		}
		else if (command == L"solardestroy")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_SolarKill(commands);
			return true;
		}
		else
		{
			global->returnCode = ReturnCode::Default;
			return false;
		}
	}

	/** @ingroup SolarControl
	 * @brief Load settings hook
	 */
	void LoadSettings()
	{
		// Hook solar creation to fix fl-bug in MP where loadout is not sent
		char* pAddressCreateSolar = (reinterpret_cast<char*>(GetModuleHandle("content.dll")) + 0x1134D4);
		auto const fpHkCreateSolar = reinterpret_cast <FARPROC>(CreateSolar);
		WriteProcMem(pAddressCreateSolar, &fpHkCreateSolar, 4);

		auto config = Serializer::JsonToObject<Config>();
		global->spawnedSolars.clear();
		global->Log = spdlog::basic_logger_mt<spdlog::async_factory>("solars", "logs/solar.log");

		for (auto& [name, solar] : config.solarArches)
		{
			solar.baseId = CreateID(solar.base.c_str());
			solar.loadoutId = CreateID(solar.loadout.c_str());
			solar.solarArchId = CreateID(solar.solarArch.c_str());
		}

		for (auto& solar : config.startupSolars)
		{
			solar.systemId = CreateID(solar.system.c_str());

			solar.pos.x = solar.position[0];
			solar.pos.y = solar.position[1];
			solar.pos.z = solar.position[2];

			solar.rot = EulerMatrix({solar.rotation[0], solar.rotation[1], solar.rotation[2]});
		}

		for (auto& [baseFrom, baseTo] : config.baseRedirects)
		{
			config.hashedBaseRedirects[CreateID(baseFrom.c_str())] = CreateID(baseTo.c_str());
		}

		global->config = std::make_unique<Config>(config);
	}

	/** @ingroup SolarControl
	 * @brief We have to spawn here since the Startup/LoadSettings hooks are too early
	 */
	void Login([[maybe_unused]] struct SLoginInfo const& li, [[maybe_unused]] uint& iClientID)
	{
		if (global->firstRun)
		{
			for (const auto& solar : global->config->startupSolars)
			{
				CreateUserDefinedSolar(solar.name, solar.pos, solar.rot, solar.systemId, false, false);
			}
			global->firstRun = false;
		}
	}

	/** @ingroup SolarControl
	 * @brief Set target hook. Used to send a docking request if an airlock is selected since vanilla doesn't make this dockable by default
	 */
	void SetTarget(const ClientId& client, struct XSetTarget const& target)
	{
		uint type;
		pub::SpaceObj::GetType(target.iSpaceId, type);
		
		if ((type & OBJ_AIRLOCK_GATE) && std::ranges::find(global->pendingDockingRequests, client) == global->pendingDockingRequests.end())
		{
			pub::SpaceObj::DockRequest(Hk::Player::GetShip(client).value(), target.iSpaceId);
			global->pendingDockingRequests.emplace_back(client);
		}
	}

	/** @ingroup SolarControl
	 * @brief Timer to set the relative health of spawned solars. This fixes the glitch where spawned solars are not dockable
	 */
	void RelativeHealthTimer()
	{
		for (const auto& [name, solarInfo]: global->spawnedSolars)
		{
			pub::SpaceObj::SetRelativeHealth(name, 1.0f);
		}
	}

	/** @ingroup SolarControl
	 * @brief Timer to clear the docking requests vector. This vector exists to stop the server from spamming docking requests when using SetTarget
	 */
	void ClearDockingRequestsTimer()
	{
		global->pendingDockingRequests.clear();
	}

	// Timers
	const std::vector<Timer> timers = {{RelativeHealthTimer, 5}, {ClearDockingRequestsTimer, 5}};

	void PlayerLaunch([[maybe_unused]] ShipId& shipId, ClientId& client)
	{
		if (global->pendingRedirects.find(client) != global->pendingRedirects.end())
		{
			Hk::Player::Beam(client, global->pendingRedirects[client]);
			PrintUserCmdText(client, L"Redirecting undock. Please launch again.");
		}
	}

	//! Base Enter hook
	void BaseEnter(const uint& baseId, ClientId& client)
	{
		if (global->pendingRedirects.find(client) != global->pendingRedirects.end())
		{
			global->pendingRedirects.erase(client);
		}
		else
		{
			global->pendingRedirects[client] = global->config->hashedBaseRedirects[baseId];
		}
	}

	void ClearClientInfo(ClientId& client)
	{
		global->pendingRedirects.erase(client);
	}

	// IPC
	SolarCommunicator::SolarCommunicator(const std::string& plug) : PluginCommunicator(plug)
	{
		this->CreateSolar = CreateUserDefinedSolar;
	}
} // namespace Plugins::SolarControl

using namespace Plugins::SolarControl;

DefaultDllMainSettings(LoadSettings);

REFL_AUTO(type(SolarArch), field(solarArch), field(loadout), field(iff), field(infocard), field(base), field(pilot));
REFL_AUTO(type(StartupSolar), field(name), field(system), field(position), field(rotation));
REFL_AUTO(type(Config), field(startupSolars), field(solarArches), field(baseRedirects));

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(SolarCommunicator::pluginName);
	pi->shortName("solar_control");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->timers(&timers);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &AdminCommandProcessing);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__SetTarget, &SetTarget, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);

	// Register IPC
	global->communicator = new SolarCommunicator(SolarCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}