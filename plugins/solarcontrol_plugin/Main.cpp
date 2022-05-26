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
		IObjInspectImpl* inspect;
		if (GetShipInspect(iSpaceID, inspect, dunno))
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

	pub::AI::SetPersonalityParams HkMakePersonality()
	{
		pub::AI::SetPersonalityParams p;
		p.iStateGraph = pub::StateGraph::get_state_graph("STATION", pub::StateGraph::TYPE_STANDARD);
		p.bStateID = true;

		p.personality.GunUse.gun_fire_interval_time = 0.5f;
		p.personality.GunUse.gun_fire_interval_variance_percent = 0.5f;
		p.personality.GunUse.gun_fire_burst_interval_time = 5.0f;
		p.personality.GunUse.gun_fire_burst_interval_variance_percent = 0.50f;
		p.personality.GunUse.gun_fire_no_burst_interval_time = 2.0f;
		p.personality.GunUse.gun_fire_accuracy_cone_angle = 1.0f;
		p.personality.GunUse.gun_fire_accuracy_power = 1.1f;
		p.personality.GunUse.gun_range_threshold = 1.1f;
		p.personality.GunUse.gun_target_point_switch_time = 3.0f;
		p.personality.GunUse.fire_style = 0;
		p.personality.GunUse.auto_turret_interval_time = 1.8f;
		p.personality.GunUse.auto_turret_burst_interval_time = 2.0f;
		p.personality.GunUse.auto_turret_no_burst_interval_time = 0.0f;
		p.personality.GunUse.auto_turret_burst_interval_variance_percent = 0.2f;
		p.personality.GunUse.gun_range_threshold_variance_percent = 0.3f;
		p.personality.GunUse.gun_fire_accuracy_power_npc = 2.0f;

		p.personality.MissileUse.missile_launch_interval_time = 20.0f;
		p.personality.MissileUse.missile_launch_interval_variance_percent = 0.25f;
		p.personality.MissileUse.missile_launch_range = 1000.0f;
		p.personality.MissileUse.missile_launch_cone_angle = 20.0f;
		p.personality.MissileUse.missile_launch_allow_out_of_range = false;

		p.personality.FormationUse.force_attack_formation_active_time = 0.0f;
		p.personality.FormationUse.force_attack_formation_unactive_time = 0.0f;

		return p;
	}


	uint CreateSolar(std::wstring name, Vector pos, Matrix rot, uint iSystem, bool varyPos)
	{
		SOLAR_ARCHTYPE_STRUCT arch = global->mapSolarArchtypes[name];

		pub::SpaceObj::SolarInfo si {};
		memset(&si, 0, sizeof(si));
		si.iFlag = 4;

		// Prepare the settings for the space object
		si.iArchID = arch.Shiparch;
		si.iLoadoutID = arch.Loadout;
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
		std::string npcid = wstos(name).c_str() + std::to_string(global->spawnedSOLARs.size());
		strncpy_s(si.cNickName, sizeof(si.cNickName), npcid.c_str(), name.size() + global->spawnedSOLARs.size());

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
		si.iUnk8 = arch.Base;

		// Define the string used for the scanner name.
		// Because this is empty, the solar_name will be used instead.
		FmtStr scanner_name(0, 0);
		scanner_name.begin_mad_lib(0);
		scanner_name.end_mad_lib();

		// Define the string used for the solar name.
		FmtStr solar_name(0, 0);
		solar_name.begin_mad_lib(16163); // ids of "%s0 %s1"
		solar_name.append_string(arch.Infocard);
		solar_name.end_mad_lib();

		// Set Reputation
		pub::Reputation::Alloc(si.iRep, scanner_name, solar_name);
		pub::Reputation::SetAffiliation(si.iRep, MakeId("fc_lr_grp"));

		// Spawn the solar object
		uint iSpaceObj;
		HkCreateSolar(iSpaceObj, si);

		pub::AI::SetPersonalityParams pers = HkMakePersonality();
		pub::AI::SubmitState(iSpaceObj, &pers);

		// Set the visible health for the Space Object
		pub::SpaceObj::SetRelativeHealth(iSpaceObj, 1);

		global->spawnedSOLARs[iSpaceObj] = name;

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

		SOLAR_ARCHTYPE_STRUCT arch;

		std::map<std::wstring, SOLAR_ARCHTYPE_STRUCT>::iterator iter = global->mapSolarArchtypes.find(SolarType);
		if (iter != global->mapSolarArchtypes.end())
			arch = iter->second;
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

		Vector pos;
		Matrix rot;
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

		for (auto& [id, name] : global->spawnedSOLARs)
			pub::SpaceObj::SetRelativeHealth(id, 0.0f);

		global->spawnedSOLARs.clear();
		cmds->Print(L"OK\n");

		return;
	}

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (IS_CMD("solarcreate"))
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmd_SolarMake(cmds, cmds->ArgInt(1), cmds->ArgStr(2));
			return true;
		}
		else if (IS_CMD("solardestroy"))
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmd_SolarKill(cmds);
			return true;
		}
		return false;
	}

	// Settings
	void LoadSolarInfo()
	{
		global->startupSOLARs.clear();
		global->spawnedSOLARs.clear();

		// The path to the configuration file.
		char szCurDir[MAX_PATH];
		GetCurrentDirectory(sizeof(szCurDir), szCurDir);
		std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\solar.cfg";

		INI_Reader ini;
		if (ini.open(scPluginCfgFile.c_str(), false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("solars"))
				{
					SOLAR_ARCHTYPE_STRUCT solarstruct;
					while (ini.read_value())
					{
						if (ini.is_value("solar"))
						{
							std::string setsolarname = ini.get_value_string(0);
							std::wstring solarname = stows(setsolarname);
							solarstruct.Shiparch = CreateID(ini.get_value_string(1));
							std::string loadoutstring = ini.get_value_string(2);
							solarstruct.Loadout = CreateID(loadoutstring.c_str());

							// IFF calc
							pub::Reputation::GetReputationGroup(solarstruct.IFF, ini.get_value_string(3));

							solarstruct.Infocard = ini.get_value_int(4);
							solarstruct.Base = CreateID(ini.get_value_string(5));

							global->mapSolarArchtypes[solarname] = solarstruct;
						}
					}
				}
				else if (ini.is_header("startupsolars"))
				{
					while (ini.read_value())
					{
						if (ini.is_value("startupsolar"))
						{
							SOLAR n;
							n.name = stows(ini.get_value_string(0));
							n.pos.x = ini.get_value_int(1);
							n.pos.y = ini.get_value_int(2);
							n.pos.z = ini.get_value_int(3);
							n.rot.data[0][0] = ini.get_value_float(4);
							n.rot.data[0][1] = ini.get_value_float(5);
							n.rot.data[0][2] = ini.get_value_float(6);
							n.rot.data[1][0] = ini.get_value_float(7);
							n.rot.data[1][1] = ini.get_value_float(8);
							n.rot.data[1][2] = ini.get_value_float(9);
							n.rot.data[2][0] = ini.get_value_float(10);
							n.rot.data[2][1] = ini.get_value_float(11);
							n.rot.data[2][2] = ini.get_value_float(12);
							n.system = CreateID(ini.get_value_string(13));
							global->startupSOLARs[global->startupSOLARs.size()] = n;
						}
					}
				}
			}
			ini.close();
		}
	}

	void LoadSettings()
	{
		
	}

	// We have to spawn here since the Startup/LoadSettings hooks are too early
	void __stdcall Login(struct SLoginInfo const& li, uint& iClientID)
	{
		if (global->FirstRun)
		{
			LoadSolarInfo();

			// hook solar creation to fix fl-bug in MP where loadout is not sent
			char* pAddressCreateSolar = ((char*)GetModuleHandle("content.dll") + 0x1134D4);
			FARPROC fpHkCreateSolar = (FARPROC)HkCreateSolar;
			WriteProcMem(pAddressCreateSolar, &fpHkCreateSolar, 4);

			for (std::map<int, SOLAR>::iterator i = global->startupSOLARs.begin(); i != global->startupSOLARs.end(); ++i)
			{
				CreateSolar(i->second.name, i->second.pos, i->second.rot, i->second.system, false);
			}
			global->FirstRun = false;
		}
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHook Stuff
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::SolarControl;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
	{
		LoadSettings();
	}
	return true;
}


extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Solar Control");
	pi->shortName("solar_control");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
}


