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

HK_ERROR PausePlugin(const string &sShortName, bool bPause) 
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

HK_ERROR UnloadPlugin(const string &sShortName) 
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

void LoadPlugin(const string &sFileName, CCmds* adminInterface) 
{

	string sDLLName = sFileName;
	if(sDLLName.find(".dll") == string::npos)
		sDLLName.append(".dll");

	foreach(lstPlugins,PLUGIN_DATA,it) 
	{
		if(it->sDLL == sDLLName) 
			return adminInterface->Print(L"Plugin already loaded (%s)\n", stows(it->sDLL).c_str());
	}

	string sPathToDLL = "./flhook_plugins/";
	sPathToDLL += sDLLName;

	FILE* fp = fopen(sPathToDLL.c_str(), "r");
	if (!fp)
		return adminInterface->Print(L"Error, Plugin not found (%s)\n", stows(sDLLName).c_str());
	fclose(fp);

	PLUGIN_DATA plugin;
	plugin.bMayPause = false;
	plugin.bMayUnload = false;
	plugin.hDLL = NULL;
	plugin.sDLL = "";
	plugin.sName = "";
	plugin.sShortName = "";
	
	plugin.sDLL = sDLLName;

	plugin.hDLL = LoadLibrary(sPathToDLL.c_str());

	if(!plugin.hDLL) 
		return adminInterface->Print(L"Error, can't load plugin (%s)\n", stows(sDLLName).c_str());

	plugin.bPaused = false;

	FARPROC pPluginInfo = GetProcAddress(plugin.hDLL, "?Get_PluginInfo@@YAPAUPLUGIN_INFO@@XZ");

	if(!pPluginInfo)
	{
		adminInterface->Print(L"Error, could not read plugin info (Get_PluginInfo not exported?): %s\n", stows(sDLLName).c_str());
		FreeLibrary(plugin.hDLL);
		return;
	}

	PLUGIN_Get_PluginInfo Plugin_Info = (PLUGIN_Get_PluginInfo)pPluginInfo;
	PLUGIN_INFO* p_PI = Plugin_Info();
	if(!p_PI || !p_PI->sShortName.length() || !p_PI->sName.length())
	{
		adminInterface->Print(L"Error, invalid plugin info: %s\n", stows(sDLLName).c_str());
		FreeLibrary(plugin.hDLL);
		return;
	}

	plugin.bMayPause = p_PI->bMayPause;
	plugin.bMayUnload = p_PI->bMayUnload;
	plugin.sName = p_PI->sName;
	plugin.sShortName = p_PI->sShortName;
					
	FARPROC pPluginReturnCode = GetProcAddress(plugin.hDLL, "?Get_PluginReturnCode@@YA?AW4PLUGIN_RETURNCODE@@XZ");

	for ( std::map<string, int>::const_iterator iter = p_PI->mapHooks.begin(); iter != p_PI->mapHooks.end(); ++iter )
	{
		PLUGIN_HOOKDATA hook;
		hook.sName = plugin.sShortName;
		hook.sPluginFunction = hook.sName + "-" + iter->first;
		hook.bPaused = false;
		hook.hDLL = plugin.hDLL;
		hook.iPriority = iter->second;
		hook.pFunc = 0;
		hook.pPluginReturnCode = pPluginReturnCode;

		std::map<string, list<PLUGIN_HOOKDATA>*>::iterator iterPH;
		iterPH = mpPluginHooks.find(iter->first);
		if(iterPH != mpPluginHooks.end()) 
			iterPH->second->push_back(hook);
		else {
			mpPluginHooks[iter->first] = new list<PLUGIN_HOOKDATA>;
			mpPluginHooks[iter->first]->push_back(hook);
		}
		mpPluginHooks[iter->first]->sort(PLUGIN_SORTCRIT());
	}

	adminInterface->Print(L"Plugin loaded: %s (%s)\n", stows(plugin.sShortName).c_str(), stows(sDLLName).c_str());

	lstPlugins.push_back(plugin);

	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadPlugins(bool bStartup, CCmds* adminInterface)
{

// plugin loader

	WIN32_FIND_DATAA finddata;

	HANDLE hfindplugins = FindFirstFileA("./flhook_plugins/*.dll",&finddata);
	do 
	{
		if(hfindplugins == INVALID_HANDLE_VALUE)
			break;

		try
		{
			PLUGIN_DATA plugin;
			plugin.bMayPause = false;
			plugin.bMayUnload = false;
			plugin.hDLL = NULL;
			plugin.sDLL = "";
			plugin.sName = "";
			plugin.sShortName = "";

			string sPathToDLL = "./flhook_plugins/";
			sPathToDLL += finddata.cFileName;
			plugin.sDLL = finddata.cFileName;

			foreach(lstPlugins,PLUGIN_DATA,it) 
			{
				if(it->sDLL == plugin.sDLL) 
				{
					adminInterface->Print(L"Plugin already loaded, skipping: %s\n", stows(plugin.sDLL).c_str());
					throw "";
				}
			}
			plugin.hDLL = LoadLibrary(sPathToDLL.c_str());

			if(!plugin.hDLL) 
			{
					adminInterface->Print(L"Error, could not load plugin: %s\n", stows(plugin.sDLL).c_str());
					throw "";
			} 

			plugin.bPaused = false;

			FARPROC pPluginInfo = GetProcAddress(plugin.hDLL, "?Get_PluginInfo@@YAPAUPLUGIN_INFO@@XZ");

			if(!pPluginInfo)
			{
				adminInterface->Print(L"Error, could not read plugin info (Get_PluginInfo not exported?): %s\n", stows(plugin.sDLL).c_str());
				FreeLibrary(plugin.hDLL);
				throw "";
			}

			PLUGIN_Get_PluginInfo Plugin_Info = (PLUGIN_Get_PluginInfo)pPluginInfo;
			PLUGIN_INFO* p_PI = Plugin_Info();

			if(!p_PI || !p_PI->sShortName.length() || !p_PI->sName.length())
			{
				adminInterface->Print(L"Error, invalid plugin info: %s\n", stows(plugin.sDLL).c_str());
				FreeLibrary(plugin.hDLL);
				throw "";
			}

			plugin.bMayPause = p_PI->bMayPause;
			plugin.bMayUnload = p_PI->bMayUnload;
			plugin.sName = p_PI->sName;
			plugin.sShortName = p_PI->sShortName;

			/* 
			// shouldn't be needed cause the plugin wasn't loaded before...
			if(!plugin.bMayUnload && !bStartup) {
				adminInterface->Print(L"Error, could not load plugin (unloadable): %s\n", stows(plugin.sDLL).c_str());
				throw "";
			}
			*/
					
			FARPROC pPluginReturnCode = GetProcAddress(plugin.hDLL, "?Get_PluginReturnCode@@YA?AW4PLUGIN_RETURNCODE@@XZ");

			for ( std::map<string, int>::const_iterator iter = p_PI->mapHooks.begin(); iter != p_PI->mapHooks.end(); ++iter )
			{
				PLUGIN_HOOKDATA hook;
				hook.sName = plugin.sShortName;
				hook.sPluginFunction = hook.sName + "-" + iter->first;
				hook.bPaused = false;
				hook.hDLL = plugin.hDLL;
				hook.iPriority = iter->second;
				hook.pFunc = 0;
				hook.pPluginReturnCode = pPluginReturnCode;

				std::map<string, list<PLUGIN_HOOKDATA>*>::iterator iterPH;
				iterPH = mpPluginHooks.find(iter->first);
				if(iterPH != mpPluginHooks.end()) 
					iterPH->second->push_back(hook);
				else {
					mpPluginHooks[iter->first] = new list<PLUGIN_HOOKDATA>;
					mpPluginHooks[iter->first]->push_back(hook);
				}
				mpPluginHooks[iter->first]->sort(PLUGIN_SORTCRIT());
			}

			adminInterface->Print(L"Plugin loaded: %s (%s)\n", stows(plugin.sShortName).c_str(), stows(plugin.sDLL).c_str());

			lstPlugins.push_back(plugin);

		} catch(char*) {};
		
	} while (FindNextFile(hfindplugins,&finddata));

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

}