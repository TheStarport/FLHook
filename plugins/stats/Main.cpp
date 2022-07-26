// Stats Plugin
// by Raikkonen

// Includes
#include "Main.h"

namespace Plugins::Stats
{
	std::unique_ptr<Global> global = std::make_unique<Global>();

	// Load configuration file
	void LoadSettings()
	{
		global->jsonFileName = Serializer::JsonToObject<FileName>();
		std::filesystem::create_directories(global->jsonFileName.FilePath);

		HkLoadStringDLLs();

		// Load in shiparch.ini to generate IDs based off the nickname and generate
		// ship names via ids_name
		std::string shiparchfile = "..\\data\\ships\\shiparch.ini";

		INI_Reader ini;
		if (ini.open(shiparchfile.c_str(), false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("Ship"))
				{
					int idsname = 0;
					while (ini.read_value())
					{
						if (ini.is_value("nickname"))
						{
							uint shiphash = CreateID(ini.get_value_string(0));
							global->Ships[shiphash] = HkGetWStringFromIDS(idsname);
						}
						if (ini.is_value("ids_name"))
						{
							idsname = ini.get_value_int(0);
						}
					}
				}
			}
			ini.close();
		}
	}

	// This removes double quotes from player names. This causes invalid json.
	std::string encode(std::string data)
	{
		std::string scEncoded;
		scEncoded.reserve(data.size());
		for (char pos : data)
		{
			if (pos == '\"')
				scEncoded.append("&quot;");
			else
				scEncoded.append(1, pos);
		}
		return scEncoded;
	}

	// Function to export load and player data to a json file
	void ExportJSON()
	{
		std::ofstream out(global->jsonFileName.FilePath);

		nlohmann::json jExport;
		jExport["serverload"] = g_iServerLoad;

		nlohmann::json jPlayers;
		const std::list<HKPLAYERINFO> lstPlayers = HkGetPlayers();

		for (auto& lstPlayer : lstPlayers)
		{
			nlohmann::json jPlayer;

			// Add name
			jPlayer["name"] = encode(wstos(lstPlayer.wscCharname));

			// Add rank
			int iRank;
			pub::Player::GetRank(lstPlayer.iClientID, iRank);
			jPlayer["rank"] = std::to_string(iRank);

			// Add group
			int groupID = Players.GetGroupID(lstPlayer.iClientID);
			jPlayer["group"] = groupID ? std::to_string(groupID) : "None";

			// Add ship
			Archetype::Ship* ship = Archetype::GetShip(Players[lstPlayer.iClientID].iShipArchetype);
			jPlayer["ship"] = (ship) ? wstos(global->Ships[ship->get_id()]) : "Unknown";

			// Add system
			uint iSystemID;
			pub::Player::GetSystem(lstPlayer.iClientID, iSystemID);
			const Universe::ISystem* iSys = Universe::get_system(iSystemID);
			jPlayer["system"] = wstos(HkGetWStringFromIDS(iSys->strid_name));

			jPlayers.push_back(jPlayer);
		}

		jExport["players"] = jPlayers;
		out << jExport;
		out.close();
	}

	// Hooks for updating stats
	void __stdcall DisConnect_AFTER(unsigned int iClientID, enum EFLConnection state) { ExportJSON(); }

	void __stdcall PlayerLaunch_AFTER(uint& iShip, uint& client) { ExportJSON(); }

	void __stdcall CharacterSelect_AFTER(std::string& szCharFilename, uint& iClientID) { ExportJSON(); }
} // namespace Plugins::Stats

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
using namespace Plugins::Stats;
REFL_AUTO(type(FileName), field(FilePath))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Stats");
	pi->shortName("stats");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect_AFTER, HookStep::After);
}