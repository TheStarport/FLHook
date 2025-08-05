#include "PCH.hpp"

#include "Wardrobe.hpp"
#include "API/FLHook/ClientList.hpp"

namespace Plugins
{
    WardrobePlugin::WardrobePlugin(const PluginInfo& info) : Plugin(info) {}

    concurrencpp::result<void> WardrobePlugin::UserCmdShowWardrobe(ClientId client, const std::wstring_view param)
    {
        if (StringUtils::ToLower(param) == L"head")
        {
            (void)client.Message(L"Heads:");
            std::wstring heads;
            for (const auto& [name, id] : config.heads)
            {
                heads += (StringUtils::stows(name) + L" | ");
            }
            (void)client.Message(heads);
        }
        else if (StringUtils::ToLower(param) == L"body")
        {
            (void)client.Message(L"Bodies:");
            std::wstring bodies;
            for (const auto& [name, id] : config.bodies)
            {
                bodies += (StringUtils::stows(name) + L" | ");
            }
            (void)client.Message(bodies);
        }

        co_return;
    }

    concurrencpp::result<void> WardrobePlugin::UserCmdChangeCostume(ClientId client, const std::wstring_view type, const std::wstring_view costume)
    {
        if (type.empty() || costume.empty())
        {
            (void)client.MessageErr(L"Invalid parameters");
            co_return;
        }

        if (StringUtils::ToLower(type) == L"head")
        {
            if (!config.heads.contains(StringUtils::wstos(costume)))
            {
                (void)client.MessageErr(L"Head not found. Use \"/warehouse show head\" to get available heads.");
                co_return;
            }
            client.GetData().playerData->baseCostume.head = CreateID(config.heads[StringUtils::wstos(costume)].c_str());
        }
        else if (StringUtils::ToLower(type) == L"body")
        {
            if (!config.bodies.contains(StringUtils::wstos(costume)))
            {
                (void)client.MessageErr(L"Body not found.Use \"/warehouse show body\" to get available bodies.");
                co_return;
            }
            client.GetData().playerData->baseCostume.body = CreateID(config.bodies[StringUtils::wstos(costume)].c_str());
        }
        else
        {
            (void)client.MessageErr(L"Invalid parameters");
            co_return;
        }

        // Saving the characters forces an anti-cheat checks and fixes
        // up a multitude of other problems.
        (void)client.SaveChar();
        (void)client.Kick(L"Updating character, please wait 10 seconds before reconnecting");

        co_return;
    }

    concurrencpp::result<void> WardrobePlugin::UserCmdHandle(ClientId client, const std::wstring_view command, const std::wstring_view param,
                                                             const std::wstring_view param2)
    {
        // Check character is in base
        if (!client.IsDocked())
        {
            (void)client.MessageErr(L"Not in base");
            co_return;
        }

        if (command == L"list")
        {
            UserCmdShowWardrobe(client, param);
        }
        else if (command == L"change")
        {
            UserCmdChangeCostume(client, param, param2);
        }
        else
        {
            (void)client.Message(L"Command usage:");
            (void)client.Message(L"/wardrobe list <head/body> - lists available bodies/heads");
            (void)client.Message(L"/wardrobe change <head/body> <name> - changes your head/body to the chosen model");
        }

        co_return;
    }

    bool WardrobePlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/wardrobe.json");

        return true;
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Wardrobe",
	    .shortName = L"wardrobe",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(WardrobePlugin);
