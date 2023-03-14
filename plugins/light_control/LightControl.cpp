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
 * - lights options [Page Number] - Shows a page of lights that can be swapped to.
 * - lights change <Light Point> <Item> - Swap a light.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "bases": ["li01_01_base"],
 *     "cost": 0,
 *     "introMessage1": "Light customization facilities are available here.",
 *     "introMessage2": "type /lights on your console to see options.",
 *     "lights": ["SmallWhite","LargeGreen"],
 *     "itemsPerPage": 24,
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
#include "LightControl.h"
#include "refl.hpp"

namespace Plugins::LightControl
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	auto RegexReplace(const jpWide::NumSub& m1, const void*, const void*)
	{
		return L" " + m1[0];
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		if (config.lights.empty())
		{
			for (const auto& lights = DataManager::c()->GetLights(); const auto& [id, light] : lights)
			{
				config.lights.emplace_back(stows(light.nickname));
			}

			// Save to populate it!
			Serializer::SaveToJson(config);
		}

		// Sort into alphabetical order
		std::ranges::sort(config.lights);

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
		if (!global->config->notifyAvailabilityOnEnter || std::ranges::find(global->config->baseIdHashes, baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		if (!global->config->introMessage1.empty())
			PrintUserCmdText(client, global->config->introMessage1);

		if (!global->config->introMessage2.empty())
			PrintUserCmdText(client, global->config->introMessage2);
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

		if (!global->config->baseIdHashes.empty() && std::ranges::find(global->config->baseIdHashes, baseId.value()) == global->config->baseIdHashes.end())
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

		std::vector<EquipDesc> lights;
		const st6::list<EquipDesc>& eq = Players[client].equipDescList.equip;
		for (const auto& i : eq)
		{
			if (std::ranges::find(global->config->lightsHashed, i.archId) == global->config->lightsHashed.end())
			{
				continue;
			}

			lights.emplace_back(i);
		}

		if (lights.empty())
		{
			PrintUserCmdText(client, L"Error: You have no valid hard points that can be changed. ");
			return;
		}

		std::ranges::sort(lights, [](const EquipDesc& a, const EquipDesc& b) { return a.get_hardpoint().value < b.get_hardpoint().value; });

		int itemNumber = 1;
		for (const auto& i : lights)
		{
			const auto& index = std::ranges::find(global->config->lightsHashed, i.archId);
			if (index == global->config->lightsHashed.end())
			{
				continue;
			}

			const auto& str = global->config->lights[std::distance(global->config->lightsHashed.begin(), index)];
			PrintUserCmdText(client,
			    std::format(
			        L"|    {}: {}", itemNumber, jpWide::MatchEvaluator(RegexReplace).setRegexObject(&global->regex).setSubject(str).setFindAll().nreplace()));
			itemNumber++;
		}
	}
	/** @ingroup LightControl
	 * @brief Shows the lights available to be used.
	 */
	void UserCmdListLights(ClientId& client, const std::wstring& param)
	{
		if (global->config->lights.empty())
		{
			PrintUserCmdText(client, L"Error: There are no available options. ");
			return;
		}

		uint pageNumber = 0;
		if (!GetParam(param, ' ', 1).empty())
		{
			pageNumber = ToUInt(GetParam(param, ' ', 1)) - 1;
		}
		else
		{
			pageNumber = 0;
		}

		const uint lightsSize = static_cast<int>(global->config->lights.size());

		// +1 the quotient to account for the last page being the remainder.
		const uint maxPages = global->config->lights.size() / global->config->itemsPerPage + 1;

		if (pageNumber >= maxPages)
		{
			PrintUserCmdText(client, std::format(L"Error, invalid page number, the valid page numbers are any integer between 1 and {}", maxPages));
			return;
		}

		PrintUserCmdText(client, std::format(L"Displaying {} items from page {} of {}", global->config->itemsPerPage, pageNumber + 1, maxPages));

		uint j = 0;
		for (uint i = pageNumber * global->config->itemsPerPage; (i < lightsSize && j < global->config->itemsPerPage); i++, j++)
		{
			PrintUserCmdText(
			    client, jpWide::MatchEvaluator(RegexReplace).setRegexObject(&global->regex).setSubject(global->config->lights[i]).setFindAll().nreplace());
		}
	}

	/** @ingroup LightControl
	 * @brief Change the item on the Slot Id to the specified item.
	 */
	void UserCmdChangeItem(ClientId& client, const std::wstring& param)
	{
		if (const auto cash = Hk::Player::GetCash(client); cash.has_value() && cash.value() < global->config->cost)
		{
			PrintUserCmdText(client, std::format(L"Error: Not enough credits, the cost is {}", global->config->cost));
			return;
		}

		std::vector<std::wstring> hardPointIds;

		const std::wstring inputParam = (GetParam(param, ' ', 1));
		if (inputParam != L"all")
		{
			for (const auto i : inputParam)
			{
				if (!(std::isdigit(i) || i == L'-'))
				{
					PrintUserCmdText(client,
					    L"Error: Please input an appropriate light point \n"
					    L"Example: 1-2-3 , 1 , or all.");
					return;
				}
			}
			hardPointIds = Split(inputParam, L'-');
			if (hardPointIds.empty())
				hardPointIds.emplace_back(inputParam);
		}
		const std::wstring selectedLight = ReplaceStr(ViewToWString(GetParamToEnd(param, ' ', 2)), L" ", L"");
		std::vector<EquipDesc> lights;
		st6::list<EquipDesc>& eq = Players[client].equipDescList.equip;

		for (const auto& i : eq)
		{
			if (std::ranges::find(global->config->lightsHashed, i.archId) == global->config->lightsHashed.end())
			{
				continue;
			}

			lights.emplace_back(i);
		}
		if (lights.empty())
		{
			PrintUserCmdText(client, L"Error: You have no valid hard points that can be changed. ");
			return;
		}

		std::ranges::sort(lights, [](const EquipDesc& a, const EquipDesc& b) { return a.get_hardpoint().value < b.get_hardpoint().value; });

		// if the user inputted all, filling of hardpoint ids will be skipped and thus empty, fill it with all possible hard point indexes.
		if (hardPointIds.empty())
		{
			for (uint i = 1; i < lights.size() + 1; i++)
			{
				hardPointIds.emplace_back(std::format(L"{}", i));
			}
		}

		if (Hk::Player::GetCash(client).value() <= global->config->cost * hardPointIds.size())
		{
			PrintUserCmdText(client,
			    std::format(L"This light change require a total {} cash, you only have {} cash.",
			        global->config->cost * hardPointIds.size(),
			        Hk::Player::GetCash(client).value()));
			return;
		}

		for (const auto& hardPointIdString : hardPointIds)
		{
			const auto hardPointId = ToUInt(hardPointIdString) - 1;
			if (hardPointId > lights.size())
			{
				PrintUserCmdText(client, L"Error: Invalid light point");
				return;
			}

			const auto lightId = CreateID(wstos(selectedLight).c_str());
			if (std::ranges::find(global->config->lightsHashed, lightId) == global->config->lightsHashed.end())
			{
				PrintUserCmdText(client, std::format(L"ERR: {} is not a valid option", selectedLight));
				return;
			}

			const auto& selectedLightEquipDesc = lights[hardPointId];

			const auto light = std::find_if(
			    eq.begin(), eq.end(), [&selectedLightEquipDesc](const EquipDesc& eq) { return eq.get_id() == selectedLightEquipDesc.get_id(); });

			light->archId = lightId;
			auto err = Hk::Player::SetEquip(client, eq);
			if (err.has_error())
			{
				PrintUserCmdText(client, L"ERR: " + Hk::Err::ErrGetText(err.error()));
				return;
			}
		}

		if (global->config->cost != 0)
		{
			const auto err = Hk::Player::RemoveCash(client, global->config->cost * hardPointIds.size());
			if (err.has_error())
			{
				PrintUserCmdText(client, L"ERR: " + Hk::Err::ErrGetText(err.error()));
				return;
			}
		}

		Hk::Player::SaveChar(client);
		PrintUserCmdText(client, L"Light(s) successfully changed, when you are finished with all your changes, log off for them to take effect. ");
	}

	/** @ingroup LightControl
	 * @brief Custom user command handler.
	 */
	void UserCommandHandler(ClientId& client, const std::wstring& param)
	{
		if (const auto subCommand = GetParam(param, ' ', 0); subCommand == L"change")
		{
			if (!IsInValidBase(client))
			{
				return;
			}

			UserCmdChangeItem(client, param);
		}
		else if (subCommand == L"show")
		{
			UserCmdShowSetup(client);
		}
		else if (subCommand == L"options")
		{
			UserCmdListLights(client, param);
		}
		else
		{
			PrintUserCmdText(client,
			    L"Usage: /lights show\n"
			    L"Usage: /lights options [page number]\n"
			    L"Usage: /lights change <Light Point> <Item>");
			if (global->config->cost > 0)
			{
				PrintUserCmdText(client, std::format(L"Each light changed will cost {} credits.", global->config->cost));
			}
			PrintUserCmdText(client, L"Please log off for light changes to take effect.");
		}
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/lights", L"", UserCommandHandler, L""),
	}};
} // namespace Plugins::LightControl

using namespace Plugins::LightControl;

REFL_AUTO(
    type(Config), field(lights), field(cost), field(bases), field(introMessage1), field(introMessage2), field(notifyAvailabilityOnEnter), field(itemsPerPage))
DefaultDllMainSettings(LoadSettings);

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("lightControl");
	pi->shortName("lightControl");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
