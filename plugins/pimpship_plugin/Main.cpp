// LightControl plugin, adds functionality to changing the lights on player ships.
//
// Created by Canon
//
// Ported to 4.0 by Nen
//
#include "Main.h"
#include "refl.hpp"
#include <ctre-unicode.hpp>

namespace Plugins::LightControl
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();
	constexpr auto pattern = ctll::fixed_string("(?<!(?=^.{2}))[A-Z]");


	// TODO: For Lazrius, finish the regex expression
	/*
	constexpr auto AddSpaceToPascalCase(std::string str) noexcept {
    	const auto match = ctre::match<pattern>(std::string_view(str));
		if (!match)
			return str;
	}
	*/
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
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

	void BaseEnter(const uint& baseId, const uint& clientId)
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
			PrintUserCmdText(clientId, L"%s", global->config->introMessage1.c_str());

		if (!global->config->introMessage2.empty())
			PrintUserCmdText(clientId, L"%s", global->config->introMessage2.c_str());
	}

	uint IsInValidBase(const uint& clientId) 
	{
		uint baseId;
		if (HK_ERROR err; (err = HkGetCurrentBase(clientId, baseId)) != HKE_OK) 
		{ 
			std::wstring errorString = HkErrGetText(err); 
			PrintUserCmdText(clientId, L"ERR:" + errorString); 
			return 0; 
		}
		
		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			PrintUserCmdText(clientId, L"Light customization is not available at this facility.");
			return 0;
		}

		return baseId;
	}

	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// USER COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// Show the setup of the player's ship.
	void UserCmdShowSetup(const uint& clientId, const std::wstring_view& param)
	{
		PrintUserCmdText(clientId, L"Current light setup:");

		const st6::list<EquipDesc> &eqLst = Players[clientId].equipDescList.equip;
		int itemNumber = 1;
		for (const auto& i : eqLst) 
		{
			const auto& index = std::find(global->config->lightsHashed.begin(), global->config->lightsHashed.end(), i.iArchID);
			if (index == global->config->lightsHashed.end()) 
			{
				continue;
			}

			const auto str = global->config->lights[std::distance(global->config->lightsHashed.begin(), index)];

			
			PrintUserCmdText(clientId, L"|    %u: %s", itemNumber++, str.c_str());
		}
	}

	/// Change the item on the Slot ID to the specified item.
	void UserCmdChangeItem(const uint& clientId, const std::wstring_view& param)
	{
		int cash;
		HkFunc(HkGetCash, clientId, cash);
		if (cash < global->config->cost)
		{
			PrintUserCmdText(clientId, L"Error: Not enough credits, the cost is %u", global->config->cost);
			return;
		}

		const int hardPointId = ToInt(GetParam(param, ' ', 1)) -1;
		const std::wstring selectedLight = Trim(ViewToWString(GetParamToEnd(param, ' ', 2)));

		std::vector<EquipDesc> lights;
		st6::list<EquipDesc>& eqLst = Players[clientId].equipDescList.equip;
		for (const auto& i : eqLst)
		{
			if (std::find(global->config->lightsHashed.begin(), global->config->lightsHashed.end(), i.iArchID) == global->config->lightsHashed.end())
			{
				continue;
			}

			lights.emplace_back(i);
		}

		if (hardPointId < 0 || hardPointId > lights.size())
		{
			PrintUserCmdText(clientId, L"Error: Invalid light point");
			return;
		}

		const auto lightId = CreateID(wstos(selectedLight).c_str());
		const auto selectedLightEquipDesc = lights[hardPointId];

		const auto light = std::find_if(eqLst.begin(), eqLst.end(), [selectedLightEquipDesc](const EquipDesc& eq) { 
			return eq.get_id() == selectedLightEquipDesc.get_id();
		});

		light->iArchID = lightId;
		HkFunc(HkSetEquip, clientId, eqLst);

		HkFunc(HkAddCash, clientId, -global->config->cost);
		HkSaveChar(clientId);

		PrintUserCmdText(clientId, L"Light successfully changed, when you are finished with all your changes and for changes to take effect please use /lights update ");
	}


	// TODO: For Laz, turn this into a helper function
	//TODO: This function seems to cause the player to be kicked upon undocking regardless of light changing.
	void UserCmdUpdateLights(const uint& clientId, const std::wstring_view& param)
	{
		uint targetBase;

		HkFunc(HkGetCurrentBase,clientId, targetBase)

		Server.BaseEnter(targetBase, clientId);
		Server.BaseExit(targetBase, clientId);
		std::wstring wscCharFileName;
		HkGetCharFileName(clientId, wscCharFileName);
		wscCharFileName += L".fl";
		CHARACTER_ID cID;
		strcpy_s(cID.szCharFilename, wstos(wscCharFileName.substr(0, 14)).c_str());
		Server.CharacterSelect(cID, clientId);
	}


	void UserCommandHandler(const uint& clientId, const std::wstring_view& param) 
	{
		if (!IsInValidBase(clientId))
			return;

		const auto subCommand = GetParam(param, ' ', 0);
		if (subCommand == L"update")
		{
			UserCmdUpdateLights(clientId, param);
		}
		else if (subCommand == L"change") 
		{
			UserCmdChangeItem(clientId, param);
		}
		else if (subCommand == L"show")
		{
			UserCmdShowSetup(clientId, param);
		}
		else if (subCommand == L"options") 
		{
			PrintUserCmdText(clientId, L"Urmumlmao");
		}
		else 
		{
			PrintUserCmdText(clientId, L"Usage: /lights show");
			PrintUserCmdText(clientId, L"Usage: /lights options");
			PrintUserCmdText(clientId, L"Usage: /lights change <Light Point> <Item>");
			PrintUserCmdText(clientId, L"Usage: /lights update");
			if (global->config->cost > 0) 
			{
				PrintUserCmdText(clientId, L"Each light changed will cost %u credits.", global->config->cost);
			}
		}
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/lights", L"",UserCommandHandler, L""),
	}};
} // namespace Plugins::LightControl

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::LightControl;

REFL_AUTO(type(Config), field(lights), field(cost), field(bases), field(introMessage1), field(introMessage2), field(notifyAvailabilityOnEnter))
DefaultDllMainSettings(LoadSettings);

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("LightControl");
	pi->shortName("LightControl");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}