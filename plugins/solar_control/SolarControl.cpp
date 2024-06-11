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
 * - solarcreateformation [name] - Creates a specified premade formation of solars. The name for the formation is configured in the json file.
 * - solardestroy - Destroys all spawned solars.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *    "baseRedirects": {
 *      "li01_01_base": "li01_02_base"
 *  },
 *  "solarArchFormations": {
 *      "wplatform_form": {
 *          "components": [
 *              {
 *                  "relativePosition": [
 *                      0.0,
 *                      0.0,
 *                      0.0
 *                  ],
 *                  "rotation": [
 *                      0.0,
 *                      0.0,
 *                      0.0
 *                  ],
 *                  "solarArchName": "wplatform"
 *              },
 *              {
 *                  "relativePosition": [
 *                      0.0,
 *                      100.0,
 *                      0.0
 *                  ],
 *                  "rotation": [
 *                      0.0,
 *                      0.0,
 *                      0.0
 *                  ],
 *                  "solarArchName": "wplatform"
 *              }
 *          ]
 *      }
 *  },
 *  "solarArches": {
 *      "largestation1": {
 *          "base": "li01_01_base",
 *          "iff": "li_n_grp",
 *          "infocard": 197808,
 *          "loadout": "",
 *          "pilot": "pilot_solar_hardest",
 *          "solarArch": "largestation1"
 *      },
 *      "wplatform": {
 *          "base": "",
 *          "iff": "li_p_grp",
 *          "infocard": 197808,
 *          "loadout": "weapon_platform_1",
 *          "pilot": "pilot_solar_hardest",
 *          "solarArch": "wplatform"
 *      }
 *  },
 *  "startupSolars": [
 *      {
 *          "name": "largestation1",
 *          "position": [
 *              -30367.0,
 *              120.0,
 *              -25810.0
 *          ],
 *          "rotation": [
 *              0.0,
 *              0.0,
 *              0.0
 *          ],
 *          "system": "li01"
 *      }
 *  ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * SolarCommunicator: exposes CreateUserDefinedSolar method with parameters (const std::wstring& name, Vector position, const Matrix& rotation, SystemId
 * systemId, bool varyPosition, bool mission) and CreateUserDefinedSolarFormation method with parameters (SolarArchFormation& formation, const Vector& position,
 * uint system)
 */

#include "SolarControl.h"

#include <ranges>

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
			// clang-format off
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
			// clang-format on
			// Send packet to every client in the system
			PlayerData* playerData = nullptr;
			while (playerData = Players.traverse_active(playerData))
			{
				if (playerData->systemId == solarInfo.systemId)
				{
					GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(playerData->iOnlineId, reinterpret_cast<FLPACKET_CREATESOLAR&>(solarPacket));
				}
			}
		}
	}

	/** @ingroup SolarControl
	 * @brief Creates a solar from a solarInfo struct
	 */
	void CreateSolar(uint& spaceId, pub::SpaceObj::SolarInfo& solarInfo)
	{
		// Hack server.dll so it does not call create solar packet send
		char* serverHackAddress = reinterpret_cast<char*>(hModServer) + 0x2A62A;
		constexpr char serverHack[] = {'\xEB'};
		WriteProcMem(serverHackAddress, &serverHack, 1);

		// Create the Solar
		pub::SpaceObj::CreateSolar(spaceId, solarInfo);

		// Send solar creation packet
		SendSolarPacket(spaceId, solarInfo);

		// Undo the server.dll hack
		constexpr char serverUnHack[] = {'\x74'};
		WriteProcMem(serverHackAddress, &serverUnHack, 1);
	}

	/** @ingroup SolarControl
	 * @brief Returns a loaded personality
	 */
	pub::AI::SetPersonalityParams GetPersonality(const std::string& personalityString)
	{
		pub::AI::SetPersonalityParams p;
		p.iStateGraph = pub::StateGraph::get_state_graph("NOTHING", pub::StateGraph::TYPE_STANDARD);
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
	 * @brief Checks to ensure a solarArch is valid before attempting to spawn it. loadout, iff, base and pilot are all optional, but do require valid values to
	 * avoid a crash if populated
	 */
	bool CheckSolar(const std::wstring& solarArch)
	{
		auto arch = global->config->solarArches[solarArch];
		bool validity = true;

		// Check solar solarArch is valid
		if (!Archetype::GetSolar(CreateID(arch.solarArch.c_str())))
		{
			Console::ConErr(
			    std::format("The solarArch '{}' loaded for '{}' is invalid. Spawning this solar may cause a crash", arch.solarArch, wstos(solarArch)));
			validity = false;
		}

		// Check the loadout is valid
		EquipDescVector loadout;
		pub::GetLoadout(loadout, arch.loadoutId);

		if (!arch.loadout.empty() && loadout.equip.empty())
		{
			Console::ConErr(std::format("The loadout '{}' loaded for '{}' is invalid. Spawning this solar may cause a crash", arch.loadout, wstos(solarArch)));
			validity = false;
		}

		// Check if solar iff is valid
		uint npcIff;
		pub::Reputation::GetReputationGroup(npcIff, arch.iff.c_str());
		if (!arch.iff.empty() && npcIff == UINT_MAX)
		{
			Console::ConErr(std::format("The reputation '{}' loaded for '{}' is invalid. Spawning this solar may cause a crash", arch.iff, wstos(solarArch)));
			validity = false;
		}

		// Check solar base is valid
		if (!arch.base.empty() && !Universe::get_base(arch.baseId))
		{
			Console::ConWarn(std::format("The base '{}' loaded for '{}' is invalid. Docking with this solar may cause a crash", arch.base, wstos(solarArch)));
			validity = false;
		}

		// Check solar pilot is valid
		if (!arch.pilot.empty() && !Hk::Personalities::GetPersonality(arch.pilot).has_value())
		{
			Console::ConErr(std::format("The pilot '{}' loaded for '{}' is invalid. Spawning this solar may cause a crash", arch.pilot, wstos(solarArch)));
			validity = false;
		}
		return validity;
	}

	/** @ingroup SolarControl
	 * @brief Creates a solar defined in the solar json file
	 */
	uint CreateUserDefinedSolar(const std::wstring& name, Vector position, const Matrix& rotation, SystemId system, bool varyPosition, bool mission)
	{
		if (!CheckSolar(name))
		{
			Console::ConWarn(std::format("Unable to spawn '{}', invalid data was found in the solarArch", wstos(name)));
			return 0;
		}

		Console::ConDebug(std::format("Spawning solar '{}'", wstos(name)));
		SolarArch arch = global->config->solarArches[name];

		pub::SpaceObj::SolarInfo si {};
		memset(&si, 0, sizeof(si));
		si.iFlag = 4;

		// Prepare the settings for the space object
		si.iArchId = arch.solarArchId;
		if (!arch.loadout.empty())
		{
			si.iLoadoutId = arch.loadoutId;
		}
		si.iHitPointsLeft = 1000;
		si.systemId = system;
		si.mOrientation = rotation;
		si.Costume.head = CreateID("benchmark_male_head");
		si.Costume.body = CreateID("benchmark_male_body");
		si.Costume.lefthand = 0;
		si.Costume.righthand = 0;
		si.Costume.accessories = 0;
		si.iVoiceId = CreateID("atc_leg_m01");
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

		if (arch.baseId)
		{
			// Which base this links to
			si.baseId = arch.baseId;
		}

		// Mission base?
		if (mission)
		{
			si.mission = 1;
		}

		// Define the string used for the scanner name.ad.
		FmtStr scannerName(arch.infocard, nullptr);
		scannerName.begin_mad_lib(arch.infocard);
		scannerName.end_mad_lib();

		// Define the string used for the solar name.
		FmtStr solarName(arch.infocard, nullptr);
		solarName.begin_mad_lib(arch.infocard);
		solarName.end_mad_lib();

		// Set Reputation
		pub::Reputation::Alloc(si.iRep, scannerName, solarName);

		if (!arch.iff.empty())
		{
			uint iff;
			pub::Reputation::GetReputationGroup(iff, arch.iff.c_str());
			pub::Reputation::SetAffiliation(si.iRep, iff);
		}

		// Spawn the solar object
		uint spaceId;
		CreateSolar(spaceId, si);

		if (!arch.pilot.empty())
		{
			pub::AI::SetPersonalityParams personalityParams = GetPersonality(arch.pilot);
			pub::AI::SubmitState(spaceId, &personalityParams);
		}

		// Set the visible health for the Space Object
		pub::SpaceObj::SetRelativeHealth(spaceId, 1);

		global->spawnedSolars[spaceId] = si;

		return spaceId;
	}

	/** @ingroup SolarControl
	 * @brief Creates a premade group of solars defined in the solar json file
	 */
	std::vector<uint> CreateUserDefinedSolarFormation(const std::wstring& formation, const Vector& position, uint system)
	{
		std::vector<uint> formationSpaceIds;

		// TODO: Use find
		// auto group = solararchformation.find(formation)
		// returns an iterator

		auto group = global->config->solarArchFormations.find(formation);

		if (group == global->config->solarArchFormations.end())
		{
			Console::ConErr(std::format("Unable to find {} while attempting to spawn user defined formation", wstos(formation)));
			return {};
		}

		for (auto& component : group->second.components)
		{
			if (component.relativePosition.size() != 3 || component.rotation.size() != 3)
			{
				Console::ConErr(std::format("Invalid rotation or coordinate values provided for '{}', failed to create", component.solarArchName));
				return {};
			}

			auto solar = CreateUserDefinedSolar(stows(component.solarArchName),
			    Vector {
			        {position.x + component.relativePosition[0]}, {position.y + component.relativePosition[1]}, {position.z + component.relativePosition[2]}},
			    EulerMatrix(Vector {component.rotation[0], component.rotation[1], component.rotation[2]}),
			    system,
			    false,
			    false);
			formationSpaceIds.emplace_back(solar);
		}
		return formationSpaceIds;
	}

	/** @ingroup SolarControl
	 * @brief Admin command to create a user defined solar
	 */
	void AdminCommandSolarCreate(CCmds* commands, int amount, const std::wstring& solarType)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission\n");
			return;
		}

		if (amount == 0)
		{
			amount = 1;
		}

		if (const auto iter = global->config->solarArches.find(solarType); iter != global->config->solarArches.end())
		{
			SolarArch arch = iter->second;
		}
		else
		{
			commands->Print("ERR Wrong Solar name\n");
			return;
		}

		const uint client = Hk::Client::GetClientIdFromCharName(commands->GetAdminName()).value();
		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
		{
			return;
		}

		uint system;
		pub::Player::GetSystem(client, system);

		Vector pos {};
		Matrix rot {};
		pub::SpaceObj::GetLocation(ship, pos, rot);

		for (int i = 0; i < amount; i++)
		{
			CreateUserDefinedSolar(solarType, pos, rot, system, true, false);
		}
	}

	void AdminCommandSolarFormationCreate(CCmds* commands, const std::wstring& formationName)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission\n");
			return;
		}

		if (const auto iter = global->config->solarArchFormations.find(formationName); iter != global->config->solarArchFormations.end())
		{
			SolarArchFormation arch = iter->second;
		}

		else
		{
			commands->Print("ERR Wrong Solar formation name\n");
			return;
		}

		const uint client = Hk::Client::GetClientIdFromCharName(commands->GetAdminName()).value();
		uint ship;
		pub::Player::GetShip(client, ship);
		if (!ship)
		{
			return;
		}

		uint system;
		pub::Player::GetSystem(client, system);

		Vector pos {};
		Matrix rot {};
		pub::SpaceObj::GetLocation(ship, pos, rot);

		CreateUserDefinedSolarFormation(formationName, pos, system);
	}

	/** @ingroup SolarControl
	 * @brief Admin command to delete all spawned solars
	 */
	void AdminCommandSolarKill(CCmds* commands)
	{
		if (!(commands->rights & RIGHT_SUPERADMIN))
		{
			commands->Print("ERR No permission\n");
			return;
		}

		for (auto const& [id, name] : global->spawnedSolars)
		{
			pub::SpaceObj::SetRelativeHealth(id, 0.0f);
		}

		global->spawnedSolars.clear();
		commands->Print("OK\n");

		return;
	}

	/** @ingroup SolarControl
	 * @brief Processing for admin commands
	 */
	bool AdminCommandProcessing(CCmds* commands, const std::wstring& command)
	{
		if (command == L"solarcreate")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCommandSolarCreate(commands, commands->ArgInt(1), commands->ArgStr(2));
			return true;
		}
		if (command == L"solarformationcreate")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCommandSolarFormationCreate(commands, commands->ArgStr(1));
			return true;
		}
		else if (command == L"solardestroy")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCommandSolarKill(commands);
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
		std::byte* addressCreateSolar = (reinterpret_cast<std::byte*>(GetModuleHandle("content.dll")) + 0x1134D4);
		auto const createSolar = reinterpret_cast<FARPROC>(CreateSolar);
		WriteProcMem(addressCreateSolar, &createSolar, 4);

		auto config = Serializer::JsonToObject<Config>();
		global->spawnedSolars.clear();
		global->log = spdlog::basic_logger_mt<spdlog::async_factory>("solars", "logs/solar.log");

		for (auto& solar : config.solarArches | std::views::values)
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

		for (const auto& [baseFrom, baseTo] : config.baseRedirects)
		{
			config.hashedBaseRedirects[CreateID(baseFrom.c_str())] = CreateID(baseTo.c_str());
		}

		global->config = std::make_unique<Config>(config);
	}

	/** @ingroup SolarControl
	 * @brief We have to spawn here since the Startup/LoadSettings hooks are too early
	 */
	void Login([[maybe_unused]] struct SLoginInfo const& loginInfo, [[maybe_unused]] const uint& client)
	{
		if (global->firstRun)
		{
			for (const auto& [key, value] : global->config->solarArches)
			{
				CheckSolar(key);
			}

			for (const auto& solar : global->config->startupSolars)
			{
				if (!global->config->solarArches.contains(solar.name))
				{
					Console::ConWarn(
					    std::format("Attempted to load the startupSolar {}, but it was not defined in solarArches as a startupSolar", wstos(solar.name)));
					continue;
				}

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
		for (const auto& name : global->spawnedSolars | std::views::keys)
		{
			if (Universe::get_base(global->spawnedSolars[name].baseId))
			{
				pub::SpaceObj::SetRelativeHealth(name, 1.0f);
			}
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
		if (global->pendingRedirects.contains(client))
		{
			const auto beamResult = Hk::Player::Beam(client, global->pendingRedirects[client]);
			if (beamResult.has_error())
			{
				AddLog(LogType::Normal, LogLevel::Err, std::format("Error when beaming player: {}", wstos(Hk::Err::ErrGetText(beamResult.error()))));
			}
			else
			{
				PrintUserCmdText(client, L"Redirecting undock. Please launch again.");
			}
		}
	}

	//! Base Enter hook
	void BaseEnter(const uint& baseId, ClientId& client)
	{
		if (global->pendingRedirects.contains(client))
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
		this->CreateSolarFormation = CreateUserDefinedSolarFormation;
	}
} // namespace Plugins::SolarControl

using namespace Plugins::SolarControl;

DefaultDllMainSettings(LoadSettings);

REFL_AUTO(type(SolarArchFormation), field(components));
REFL_AUTO(type(SolarArchFormationComponent), field(solarArchName), field(relativePosition), field(rotation));
REFL_AUTO(type(SolarArch), field(solarArch), field(loadout), field(iff), field(infocard), field(base), field(pilot));
REFL_AUTO(type(StartupSolar), field(name), field(system), field(position), field(rotation));
REFL_AUTO(type(Config), field(startupSolars), field(solarArches), field(baseRedirects), field(solarArchFormations));

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pluginInfo)
{
	pluginInfo->name(SolarCommunicator::pluginName);
	pluginInfo->shortName("solar_control");
	pluginInfo->mayUnload(true);
	pluginInfo->returnCode(&global->returnCode);
	pluginInfo->timers(&timers);
	pluginInfo->versionMajor(PluginMajorVersion::VERSION_04);
	pluginInfo->versionMinor(PluginMinorVersion::VERSION_00);

	using enum HookStep;
	pluginInfo->emplaceHook(HookedCall::IServerImpl__Login, &Login);
	pluginInfo->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &AdminCommandProcessing);
	pluginInfo->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, After);
	pluginInfo->emplaceHook(HookedCall::IServerImpl__SetTarget, &SetTarget, After);
	pluginInfo->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pluginInfo->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch, After);
	pluginInfo->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, After);

	// Register IPC
	global->communicator = new SolarCommunicator(SolarCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}