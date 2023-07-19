/**
 * @date Jan 2023
 * @author Raikkonen
 * @defgroup Stats Stats
 * @brief
 * The Stats plugin collects various statistics and exports them into a JSON for later view.
 *
 * @paragraph cmds Player Commands
 * None
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "FilePath": "EXPORTS",
 *     "StatsFile": "stats.json"
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

// Includes
#include "Stats.h"
#include <Tools/Serialization/Attributes.hpp>

namespace Plugins::Stats
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup Stats
	 * @brief Load configuration file
	 */
	void LoadSettings()
	{
		global->jsonFileName = Serializer::JsonToObject<FileName>();
		if (!std::filesystem::exists(global->jsonFileName.FilePath))
			std::filesystem::create_directories(global->jsonFileName.FilePath);

		Hk::Message::LoadStringDLLs();

		// Load in shiparch.ini to generate Ids based off the nickname and generate
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
							global->Ships[shiphash] = Hk::Message::GetWStringFromIdS(idsname);
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

	/** @ingroup Stats
	 * @brief Encodes the string as UTF-8 and removes double quotes which are invalid json
	 */
	std::string encode(const std::wstring& data)
	{
		if (data.empty())
		{
			return "";
		}

		const auto size = WideCharToMultiByte(CP_UTF8, 0, &data.at(0), (int)data.size(), nullptr, 0, nullptr, nullptr);
		if (size <= 0)
		{
			throw std::runtime_error("WideCharToMultiByte() failed: " + std::to_string(size));
		}

		std::string convertedString(size, 0);
		WideCharToMultiByte(CP_UTF8, 0, &data.at(0), (int)data.size(), &convertedString.at(0), size, nullptr, nullptr);

		std::string sanitizedString;
		sanitizedString.reserve(convertedString.size());

		for (char pos : convertedString)
		{
			if (pos == '\"')
				sanitizedString.append("&quot;");
			else
				sanitizedString.append(1, pos);
		}
		return sanitizedString;	 
	}

	/** @ingroup Stats
	 * @brief Function to export load and player data to a json file
	 */
	void ExportJSON()
	{
		std::ofstream out(global->jsonFileName.FilePath + "\\" + global->jsonFileName.StatsFile);

		nlohmann::json jExport;
		jExport["serverload"] = CoreGlobals::c()->serverLoadInMs;

		nlohmann::json jPlayers = nlohmann::json::array();

		for (const std::list<PlayerInfo> lstPlayers = Hk::Admin::GetPlayers(); auto& lstPlayer : lstPlayers)
		{
			nlohmann::json jPlayer;

			// Add name
			jPlayer["name"] = encode(lstPlayer.character);

			// Add rank
			int iRank = Hk::Player::GetRank(lstPlayer.client).value();
			jPlayer["rank"] = std::to_string(iRank);

			// Add group
			int groupId = Players.GetGroupID(lstPlayer.client);
			jPlayer["group"] = groupId ? std::to_string(groupId) : "None";

			// Add ship
			const Archetype::Ship* ship = Archetype::GetShip(Players[lstPlayer.client].shipArchetype);
			jPlayer["ship"] = ship ? wstos(global->Ships[ship->get_id()]) : "Unknown";

			// Add system
			SystemId iSystemId = Hk::Player::GetSystem(lstPlayer.client).value();
			const Universe::ISystem* iSys = Universe::get_system(iSystemId);
			jPlayer["system"] = wstos(Hk::Message::GetWStringFromIdS(iSys->strid_name));

			jPlayers.push_back(jPlayer);
		}

		jExport["players"] = jPlayers;
		out << jExport;
		out.close();
	}

	/** @ingroup Stats
	 * @brief Hook for updating stats
	 */
	void DisConnect_AFTER([[maybe_unused]] uint client, [[maybe_unused]] enum EFLConnection state)
	{
		ExportJSON();
	}

	/** @ingroup Stats
	 * @brief Hook for updating stats
	 */
	void PlayerLaunch_AFTER([[maybe_unused]] const uint& ship, [[maybe_unused]] ClientId& client)
	{
		ExportJSON();
	}

	/** @ingroup Stats
	 * @brief Hook for updating stats
	 */
	void CharacterSelect_AFTER([[maybe_unused]] const std::string& charFilename, [[maybe_unused]] ClientId& client)
	{
		ExportJSON();
	}
} // namespace Plugins::Stats

using namespace Plugins::Stats;
REFL_AUTO(type(FileName), field(FilePath, AttrNotEmptyNotWhiteSpace<std::string> {}), field(StatsFile, AttrNotEmptyNotWhiteSpace<std::string> {}))

DefaultDllMainSettings(LoadSettings);

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