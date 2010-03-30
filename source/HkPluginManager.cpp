#include "hook.h"
#include "CCmds.h"

bool g_bPlugin_nofunctioncall;

std::map<string, list<PLUGIN_HOOKDATA>*> mpPluginHooks;
list<PLUGIN_DATA> lstPlugins;

enum PLUGIN_MESSAGE;

void Plugin_Communication_CallBack(PLUGIN_MESSAGE msg, void* data)
{
	CALL_PLUGINS(PLUGIN_Plugin_Communication,(msg,data));
}

__declspec(dllexport) void Plugin_Communication(PLUGIN_MESSAGE msg, void* data)
{
	Plugin_Communication_CallBack(msg, data);
}

namespace PluginManager
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR PausePlugin(string sShortName, bool bPause) 
{

	foreach(lstPlugins, PLUGIN_DATA, it) {
		if(it->sShortName == sShortName) {
			if(it->bMayPause == false)
				return HKE_PLUGIN_UNPAUSABLE;

			it->bPaused = bPause;

			std::map<string, list<PLUGIN_HOOKDATA>*>::iterator iter;
			for(iter = mpPluginHooks.begin(); iter != mpPluginHooks.end(); iter++) {
				foreach((*iter->second), PLUGIN_HOOKDATA, it2) {
					if(it2->hDLL == it->hDLL) 
						it2->bPaused = bPause;		
				}
			}
			return HKE_OK;
		}
	}

	return HKE_PLUGIN_NOT_FOUND;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR UnloadPlugin(string sShortName) 
{

	foreach(lstPlugins, PLUGIN_DATA, it) {
		if(it->sShortName == sShortName) {
			if(it->bMayUnload == false)
				return HKE_PLUGIN_UNLOADABLE;
			
			FreeLibrary(it->hDLL);
			
			std::map<string, list<PLUGIN_HOOKDATA>*>::iterator iter;
			for(iter = mpPluginHooks.begin(); iter != mpPluginHooks.end(); iter++) {
				foreach((*iter->second), PLUGIN_HOOKDATA, it2) {
					if(it2->hDLL == it->hDLL) {
						(*iter->second).erase(it2);
						break;
					}
				}
			}
			lstPlugins.erase(it);
			return HKE_OK;
		}
	}

	return HKE_PLUGIN_NOT_FOUND;

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UnloadPlugins()
{

	mpPluginHooks.clear();
	foreach(lstPlugins, PLUGIN_DATA, it)
		FreeLibrary(it->hDLL);

	lstPlugins.clear();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadPlugins(bool bStartup, CCmds* adminInterface)
{

// plugin loader

	WIN32_FIND_DATAA finddata;

	HANDLE hfindplugins = FindFirstFileA("./flhook_plugins/*.ini",&finddata);
	do {

		if(hfindplugins == INVALID_HANDLE_VALUE)
			break;

		INI_Reader ini;
		string sPluginFile = "./flhook_plugins/";
		sPluginFile += finddata.cFileName;
		ini.open(sPluginFile.c_str(), false);
		PLUGIN_DATA plugin;
		plugin.bMayPause = false;
		plugin.bMayUnload = false;
		plugin.hDLL = NULL;
		plugin.sDLL = "";
		plugin.sName = "";
		plugin.sShortName = "";

		try {

			while(ini.read_header()){
				while(ini.is_header("Settings") && ini.read_value()) {
					if(ini.is_value("dllname")) {
						plugin.sDLL = ini.get_value_string(0);
						string sPathToDLL = "./flhook_plugins/dlls/";
						sPathToDLL += plugin.sDLL;
						plugin.hDLL = LoadLibrary(sPathToDLL.c_str());
						if(!plugin.hDLL)
							break;
					}

					if(ini.is_value("shortname")) 
						plugin.sShortName = (string)ini.get_value_string(0);
					
					if(ini.is_value("name")) 
						plugin.sName = (string)ini.get_value_string(0);
					
					if(ini.is_value("maypause")) {
						if(!((string)ini.get_value_string(0)).find("yes") || !((string)ini.get_value_string(0)).find("true"))
							plugin.bMayPause = true;
						else
							plugin.bMayPause = false;
					}

					if(ini.is_value("mayunload")) {
						if(!((string)ini.get_value_string(0)).find("yes") || !((string)ini.get_value_string(0)).find("true"))
							plugin.bMayUnload = true;
						else
							plugin.bMayUnload = false;
					}				

				}

				bool bLoaded = false;
				if(!bStartup) {
					foreach(lstPlugins,PLUGIN_DATA,it) {
						if(it->sDLL == plugin.sDLL) {
							adminInterface->Print(L"Plugin already loaded, skipping: %s (%s)\n", stows(plugin.sDLL).c_str(), stows((string)finddata.cFileName).c_str());
							bLoaded = true;
							break;
						}
					}
					if(bLoaded)
						throw "";
				}

				if(!plugin.hDLL) {
					adminInterface->Print(L"Error, could not load plugin: %s (%s)\n", stows(plugin.sDLL).c_str(), stows((string)finddata.cFileName).c_str());
					throw "";
				} 
				if(!plugin.bMayUnload && !bStartup) {
					adminInterface->Print(L"Error, could not load plugin (unloadable): %s (%s)\n", stows(plugin.sDLL).c_str(), stows((string)finddata.cFileName).c_str());
					throw "";
				}

				plugin.bPaused = false;

				while(ini.is_header("Hooks") && ini.read_value()) {
					if(ini.is_value("hook")) {
						string sFunction = ini.get_value_string(0);
						PLUGIN_HOOKDATA hook;
						hook.sName = plugin.sShortName;
						hook.bPaused = false;
						hook.hDLL = plugin.hDLL;
						hook.iPriority = atoi(ini.get_value_string(1));

						std::map<string, list<PLUGIN_HOOKDATA>*>::iterator iter;
						iter = mpPluginHooks.find(sFunction);
						if(iter != mpPluginHooks.end()) 
							iter->second->push_back(hook);
						else {
							mpPluginHooks[sFunction] = new list<PLUGIN_HOOKDATA>;
							mpPluginHooks[sFunction]->push_back(hook);
						}
						mpPluginHooks[sFunction]->sort(PLUGIN_SORTCRIT());
					}
				}
			}

			adminInterface->Print(L"Plugin loaded: %s (%s)\n", stows(plugin.sShortName).c_str(), stows(plugin.sDLL).c_str());

			lstPlugins.push_back(plugin);

		} catch(char*) {};
		

	} while (FindNextFile(hfindplugins,&finddata));

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

}