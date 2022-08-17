// Solar Control 
// By Raikkonen

// Includes
#include "Main.h"

namespace Plugins::SolarControl
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	float rand_FloatRange(float a, float b) { return ((b - a) * ((float)rand() / RAND_MAX)) + a; }

	int __cdecl HkCreateSolar(uint& iSpaceID, pub::SpaceObj::SolarInfo& solarInfo)
	{
		// Hack server.dll so it does not call create solar packet send
		char* serverHackAddress = (char*)hModServer + 0x2A62A;
		char serverHack[] = {'\xEB'};
		WriteProcMem(serverHackAddress, &serverHack, 1);

		// Create the Solar
		int returnVal = pub::SpaceObj::CreateSolar(iSpaceID, solarInfo);

		uint dunno;
		if (IObjInspectImpl* inspect; GetShipInspect(iSpaceID, inspect, dunno))
		{
			CSolar* solar = (CSolar*)inspect->cobject();

			struct SOLAR_STRUCT
			{
				std::byte dunno[0x100];
			};

			SOLAR_STRUCT packetSolar;

			char* address1 = (char*)hModServer + 0x163F0;
			char* address2 = (char*)hModServer + 0x27950;

			// fill struct
			__asm
			{
			lea ecx, packetSolar
			mov eax, address1
			call eax
			push solar
			lea ecx, packetSolar
			push ecx
			mov eax, address2
			call eax
			add esp, 8
			}

			// Send packet to every client in the system
			struct PlayerData* pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				if (pPD->iSystemID == solarInfo.iSystemID)
					GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(pPD->iOnlineID, (FLPACKET_CREATESOLAR&)packetSolar);
			}
		}

		// Undo the server.dll hack
		char serverUnHack[] = {'\x74'};
		WriteProcMem(serverHackAddress, &serverUnHack, 1);

		return returnVal;
	}

	/** @ingroup NPCControl
	 * @brief Function to return a Personality
	 */
	pub::AI::SetPersonalityParams HkMakePersonality(const std::string& personality)
	{
		pub::AI::SetPersonalityParams p;
		p.iStateGraph = pub::StateGraph::get_state_graph("STATION", pub::StateGraph::TYPE_STANDARD);
		p.bStateID = true;
		HK_ERROR error;
		p.personality = HkGetPersonality(personality, error);

		if (error != HKE_OK)
		{
			std::wstring errorMessage = stows(personality) + L" is not recognised as a pilot name.";
			Console::ConErr(errorMessage);
			AddLog(LogType::Normal, LogLevel::Critical, errorMessage);
		}

		return p;
	}

	uint CreateSolar(std::wstring name, Vector pos, Matrix rot, uint iSystem, bool varyPos)
	{
		SolarArch arch = global->config->solarArches[name];

		pub::SpaceObj::SolarInfo si {};
		memset(&si, 0, sizeof(si));
		si.iFlag = 4;

		// Prepare the settings for the space object
		si.iArchID = arch.solarArchId;
		si.iLoadoutID = arch.loadoutId;
		si.iHitPointsLeft = 1000;
		si.iSystemID = iSystem;
		si.mOrientation = rot;
		si.Costume.head = CreateID("benchmark_male_head");
		si.Costume.body = CreateID("benchmark_male_body");
		si.Costume.lefthand = 0;
		si.Costume.righthand = 0;
		si.Costume.accessories = 0;
		si.iVoiceID = CreateID("atc_leg_m01");
		si.iRep = MakeId("fc_lr_grp");
		std::string npcId = wstos(name) + std::to_string(global->spawnedSolars.size());
		strncpy_s(si.cNickName, sizeof(si.cNickName), npcId.c_str(), name.size() + global->spawnedSolars.size());

		// Do we need to vary the starting position slightly? Useful when spawning multiple objects
		si.vPos = pos;
		if (varyPos)
		{
			si.vPos.x = pos.x + rand_FloatRange(0, 1000);
			si.vPos.y = pos.y + rand_FloatRange(0, 1000);
			si.vPos.z = pos.z + rand_FloatRange(0, 2000);
		}
		else
		{
			si.vPos.x = pos.x;
			si.vPos.y = pos.y;
			si.vPos.z = pos.z;
		}

		// Which base this links to
		si.iUnk8 = arch.baseId;

		// Define the string used for the scanner name.
		// Because this is empty, the solar_name will be used instead.
		FmtStr scanner_name(0, 0);
		scanner_name.begin_mad_lib(0);
		scanner_name.end_mad_lib();

		// Define the string used for the solar name.
		FmtStr solar_name(0, 0);
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
		HkCreateSolar(iSpaceObj, si);

		pub::AI::SetPersonalityParams pers = HkMakePersonality(arch.pilot);
		pub::AI::SubmitState(iSpaceObj, &pers);

		// Set the visible health for the Space Object
		pub::SpaceObj::SetRelativeHealth(iSpaceObj, 1);

		global->spawnedSolars[iSpaceObj] = name;

		return iSpaceObj;
	}

	// Client command processing
	void AdminCmd_SolarMake(CCmds* cmds, int Amount, std::wstring SolarType)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		if (Amount == 0)
			Amount = 1;

		if (const auto iter = global->config->solarArches.find(SolarType); iter != global->config->solarArches.end())
			SolarArch arch = iter->second;
		else
		{
			cmds->Print(L"ERR Wrong Solar name\n");
			return;
		}

		uint iShip;
		pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()), iShip);
		if (!iShip)
			return;

		uint iSystem;
		pub::Player::GetSystem(HkGetClientIdFromCharname(cmds->GetAdminName()), iSystem);

		Vector pos{};
		Matrix rot{};
		pub::SpaceObj::GetLocation(iShip, pos, rot);

		for (int i = 0; i < Amount; i++)
			CreateSolar(SolarType, pos, rot, iSystem, true);

		return;
	}

	void AdminCmd_SolarKill(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		for (auto const& [id, name] : global->spawnedSolars)
			pub::SpaceObj::SetRelativeHealth(id, 0.0f);

		global->spawnedSolars.clear();
		cmds->Print(L"OK\n");

		return;
	}

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		global->returnCode = ReturnCode::SkipAll;

		if (wscCmd == L"solarcreate")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_SolarMake(cmds, cmds->ArgInt(1), cmds->ArgStr(2));
			return true;
		}
		else if (wscCmd == L"solardestroy")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_SolarKill(cmds);
			return true;
		}
		else
		{
			global->returnCode = ReturnCode::Default;
			return false;
		}
	}

	// Settings
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->spawnedSolars.clear();
		global->Log = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_solars", "flhook_logs/solar.log");

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

		global->config = std::make_unique<Config>(config);
	}

	// We have to spawn here since the Startup/LoadSettings hooks are too early
	void __stdcall Login(struct SLoginInfo const& li, uint& iClientID)
	{
		if (global->firstRun)
		{
			LoadSettings();

			// hook solar creation to fix fl-bug in MP where loadout is not sent
			char* pAddressCreateSolar = ((char*)GetModuleHandle("content.dll") + 0x1134D4);
			FARPROC fpHkCreateSolar = (FARPROC)HkCreateSolar;
			WriteProcMem(pAddressCreateSolar, &fpHkCreateSolar, 4);

			for (const auto& solar : global->config->startupSolars)
			{
				CreateSolar(solar.name, solar.pos, solar.rot, solar.systemId, false);
			}
			global->firstRun = false;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHook Stuff
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::SolarControl;

DefaultDllMainSettings(LoadSettings)

REFL_AUTO(type(SolarArch), field(solarArch), field(loadout), field(iff), field(infocard), field(base), field(pilot))
REFL_AUTO(type(StartupSolar), field(name), field(system), field(position), field(rotation))
REFL_AUTO(type(Config), field(startupSolars), field(solarArches))

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Solar Control");
	pi->shortName("solar_control");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}


