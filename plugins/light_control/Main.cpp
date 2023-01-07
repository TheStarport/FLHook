/**
 * @date Feb, 2010
 * @author Cannon (Ported by Nen)
 * @defgroup LightControl Light Control
 * @brief
 * Adds functionality to change the lights on player ships.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - lights show - Shows the current equipped lights.
 * - lights options - Shows what lights you can equip.
 * - lights change <Light Point> <Item> - Swap a light.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "bases": ["li01_01_base"],
 *     "cost": 123,
 *     "introMessage1": "Light customization facilities are available here.",
 *     "introMessage2": "Type /lights on your console to see options.",
 *     "lights": ["SmallWhite","LargeGreen"],
 *     "notifyAvailabilityOnEnter": false
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
#include "refl.hpp"

namespace Plugins::LightControl
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	auto RegexReplace(const jpWide::NumSub& m1, void*, void*)
	{
		return L" " + m1[0];
	}
	
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		// Clean empty values? For some reason, we have a bunch of empty items in the JSON array. This will remove them.
		config.lights.erase(std::remove(config.lights.begin(), config.lights.end(), L""), config.lights.end());
		// Sort into alphabetical order
		std::sort(config.lights.begin(), config.lights.end());

		global->config = std::make_unique<Config>(config);

		for (const auto& base : config.bases)
		{
			uint baseIdHash = CreateID(base.c_str());
			global->config->baseIdHashes.emplace_back(baseIdHash);
		}

		for (const auto& light : config.lights)
		{
			uint lightIdHash = CreateID(wstos(light).c_str());
			global->config->lightsHashed.emplace_back(lightIdHash);
		}
	}

	/** @ingroup LightControl
	 * @brief Hook on BaseEnter. Shows availability messages if configured.
	 */
	void BaseEnter(const uint& baseId, ClientId& client)
	{
		if (!global->config->notifyAvailabilityOnEnter)
		{
			return;
		}

		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		if (!global->config->introMessage1.empty())
			PrintUserCmdText(client, L"%s", global->config->introMessage1.c_str());

		if (!global->config->introMessage2.empty())
			PrintUserCmdText(client, L"%s", global->config->introMessage2.c_str());
	}

	/** @ingroup LightControl
	 * @brief Returns a baseId if in a valid base.
	 */
	uint IsInValidBase(ClientId& client) 
	{
		const auto baseId = Hk::Player::GetCurrentBase(client);
		if (baseId.has_error()) 
		{ 
			const std::wstring errorString = Hk::Err::ErrGetText(baseId.error()); 
			PrintUserCmdText(client, L"ERR:" + errorString); 
			return 0; 
		}
		
		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId.value()) == global->config->baseIdHashes.end())
		{
			PrintUserCmdText(client, L"Light customization is not available at this facility.");
			return 0;
		}

		return baseId.value();
	}

	/** @ingroup LightControl
	 * @brief Show the setup of the player's ship.
	 */
	void UserCmdShowSetup(ClientId& client)
	{
		PrintUserCmdText(client, L"Current light setup:");

		const st6::list<EquipDesc> &eqLst = Players[client].equipDescList.equip;
		int itemNumber = 1;
		for (const auto& i : eqLst) 
		{
			const auto& index = std::find(global->config->lightsHashed.begin(), global->config->lightsHashed.end(), i.iArchId);
			if (index == global->config->lightsHashed.end()) 
			{
				continue;
			}

			const auto str = global->config->lights[std::distance(global->config->lightsHashed.begin(), index)];
			auto& me = jpWide::MatchEvaluator(RegexReplace).setRegexObject(&global->regex).setSubject(str).setFindAll();
			PrintUserCmdText(client, fmt::format(L"|    {}: {}", itemNumber++, me.nreplace().c_str()));
		}
	}

	/** @ingroup LightControl
	 * @brief Show the options available to the player.
	 */
	void UserCmdShowOptions(ClientId& client)
	{
		for (const auto& light : global->config->lights)
		{
			auto& me = jpWide::MatchEvaluator(RegexReplace).setRegexObject(&global->regex).setSubject(light).setFindAll();
			PrintUserCmdText(client, me.nreplace());
		}
	}

	/** @ingroup LightControl
	 * @brief Change the item on the Slot Id to the specified item.
	 */
	void UserCmdChangeItem(ClientId& client, const std::wstring& param)
	{
		if (const auto cash = Hk::Player::GetCash(client); cash.has_value() && cash.value() < global->config->cost)
		{
			PrintUserCmdText(client, L"Error: Not enough credits, the cost is %u", global->config->cost);
			return;
		}

		const int hardPointId = ToInt(GetParam(param, ' ', 1)) - 1;
		const std::wstring selectedLight = ReplaceStr(ViewToWString(GetParamToEnd(param, ' ', 2)), L" ", L"");

		std::vector<EquipDesc> lights;
		st6::list<EquipDesc>& eqLst = Players[client].equipDescList.equip;
		for (const auto& i : eqLst)
		{
			if (std::find(global->config->lightsHashed.begin(), global->config->lightsHashed.end(), i.iArchId) == global->config->lightsHashed.end())
			{
				continue;
			}

			lights.emplace_back(i);
		}

		if (hardPointId < 0 || hardPointId > lights.size())
		{
			PrintUserCmdText(client, L"Error: Invalid light point");
			return;
		}

		const auto lightId = CreateID(wstos(selectedLight).c_str());
		const auto& selectedLightEquipDesc = lights[hardPointId];

		const auto light = std::find_if(eqLst.begin(), eqLst.end(), [selectedLightEquipDesc](const EquipDesc& eq) { 
			return eq.get_id() == selectedLightEquipDesc.get_id();
		});

		light->iArchId = lightId;
		auto err = Hk::Player::SetEquip(client, eqLst);
		if (err.has_error())
		{
			PrintUserCmdText(client, L"ERR: " + Hk::Err::ErrGetText(err.error()));
			return;
		}

		err = Hk::Player::RemoveCash(client, global->config->cost);
		if (err.has_error())
		{
			PrintUserCmdText(client, L"ERR: " + Hk::Err::ErrGetText(err.error()));
			return;
		}

		Hk::Player::SaveChar(client);
		PrintUserCmdText(client, L"Light successfully changed, when you are finished with all your changes, log off for them to take effect. ");
	}

	/** @ingroup LightControl
	 * @brief Custom user command handler.
	 */
	void UserCommandHandler(ClientId& client, const std::wstring& param) 
	{
		if (!IsInValidBase(client))
			return;

		if (const auto subCommand = GetParam(param, ' ', 0); subCommand == L"change") 
		{
			UserCmdChangeItem(client, param);
		}
		else if (subCommand == L"show")
		{
			UserCmdShowSetup(client);
		}
		else if (subCommand == L"options") 
		{
			UserCmdShowOptions(client);
		}
		else 
		{
			PrintUserCmdText(client, L"Usage: /lights show");
			PrintUserCmdText(client, L"Usage: /lights options");
			PrintUserCmdText(client, L"Usage: /lights change <Light Point> <Item>");
			if (global->config->cost > 0) 
			{
				PrintUserCmdText(client, L"Each light changed will cost %u credits.", global->config->cost);
			}
			PrintUserCmdText(client, L"Please log off for light changes to take effect.");

		}
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/lights", L"",UserCommandHandler, L""),
	}};
} // namespace Plugins::LightControl

using namespace Plugins::LightControl;

REFL_AUTO(type(Config), field(lights), field(cost), field(bases), field(introMessage1), field(introMessage2), field(notifyAvailabilityOnEnter))
DefaultDllMainSettings(LoadSettings);

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("lightControl");
	pi->shortName("lightControl");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}