
#include "Wardrobe.hpp"
#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"

namespace Plugins
{
    WardrobePlugin::WardrobePlugin(const PluginInfo& info) : Plugin(info) {}

	void WardrobePlugin::UserCmdShowWardrobe(const std::wstring_view param)
	{
		if (StringUtils::ToLower(param) == L"head")
		{
			(void)userCmdClient.Message(L"Heads:");
			std::wstring heads;
			for (const auto& [name, id] : config.heads)
				heads += (StringUtils::stows(name) + L" | ");
			(void)userCmdClient.Message(heads);
		}
		else if (StringUtils::ToLower(param) == L"body")
		{
			(void)userCmdClient.Message(L"Bodies:");
			std::wstring bodies;
			for (const auto& [name, id] : config.bodies)
				bodies += (StringUtils::stows(name) + L" | ");
			(void)userCmdClient.Message(bodies);
		}
	}

	void WardrobePlugin::UserCmdChangeCostume(const std::wstring_view type, const std::wstring_view costume)
	{
		if (type.empty() || costume.empty())
		{
			(void)userCmdClient.Message(L"ERR Invalid parameters");
			return;
		}

		if (StringUtils::ToLower(type) == L"head")
		{
			if (!config.heads.contains(StringUtils::wstos(costume)))
			{
				(void)userCmdClient.Message(L"ERR Head not found. Use \"/warehouse show head\" to get available heads.");
				return;
			}
			userCmdClient.GetData().playerData->baseCostume.head = CreateID(config.heads[StringUtils::wstos(costume)].c_str());
		}
		else if (StringUtils::ToLower(type) == L"body")
		{
			if (!config.bodies.contains(StringUtils::wstos(costume)))
			{
				(void)userCmdClient.Message(L"ERR Body not found. Use \"/warehouse show body\" to get available bodies.");
				return;
			}
			userCmdClient.GetData().playerData->baseCostume.body = CreateID(config.bodies[StringUtils::wstos(costume)].c_str());
		}
		else
		{
			(void)userCmdClient.Message(L"ERR Invalid parameters");
			return;
		}

		// Saving the characters forces an anti-cheat checks and fixes
		// up a multitude of other problems.
        (void)userCmdClient.SaveChar();

        (void)userCmdClient.Kick(L"Updating character, please wait 10 seconds before reconnecting");
	}


	void WardrobePlugin::UserCmdHandle(const std::wstring_view command, const std::wstring_view param, const std::wstring_view param2)
	{
		// Check character is in base
		if (!userCmdClient.IsDocked())
		{
			(void)userCmdClient.Message(L"ERR Not in base");
			return;
		}

		if (command == L"list")
		{
			UserCmdShowWardrobe(param);
		}
		else if (command == L"change")
		{
			UserCmdChangeCostume(param, param2);
		}
		else
		{
			(void)userCmdClient.Message(L"Command usage:");
			(void)userCmdClient.Message(L"/wardrobe list <head/body> - lists available bodies/heads");
			(void)userCmdClient.Message(L"/wardrobe change <head/body> <name> - changes your head/body to the chosen model");
		}
	}

	void WardrobePlugin::OnLoadSettings()
	{
	    if (const auto conf = Json::Load<Config>("config/wardrobe.json"); !conf.has_value())
	    {
	        Json::Save(config, "config/wardrobe.json");
	    }
	    else
	    {
	        config = conf.value();
	    }
	}

} // namespace Plugins::Wardrobe

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Wardrobe", L"wardrobe", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(WardrobePlugin, Info);
