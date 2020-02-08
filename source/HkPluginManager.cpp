#include "hook.h"
#include "CCmds.h"

bool g_bPlugin_nofunctioncall;

void* vPluginRet;

list<PLUGIN_HOOKDATA>* pPluginHooks;
list<PLUGIN_DATA> lstPlugins;

enum PLUGIN_MESSAGE;

void Plugin_Communication_CallBack(PLUGIN_MESSAGE msg, void* data)
{
	CALL_PLUGINS_NORET(PLUGIN_Plugin_Communication,,(PLUGIN_MESSAGE msg, void* data),(msg,data));
}

__declspec(dllexport) void Plugin_Communication(PLUGIN_MESSAGE msg, void* data)
{
	Plugin_Communication_CallBack(msg, data);
}

namespace PluginManager
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Init()
{
	// create array of callback-function plugin-data lists
	pPluginHooks = new list<PLUGIN_HOOKDATA>[(int)PLUGIN_CALLBACKS_AMOUNT];

	lstPlugins.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Destroy()
{
	delete[] pPluginHooks;

	lstPlugins.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR PausePlugin(const string &sShortName, bool bPause) 
{

	foreach(lstPlugins, PLUGIN_DATA, it) {
		if(it->sShortName == sShortName) {
			if(it->bMayPause == false)
				return HKE_PLUGIN_UNPAUSABLE;

			it->bPaused = bPause;

			for(int i=0; i<(int)PLUGIN_CALLBACKS_AMOUNT; i++) {
				foreach(pPluginHooks[i], PLUGIN_HOOKDATA, it2) {
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
			
			for(int i=0; i<(int)PLUGIN_CALLBACKS_AMOUNT; i++) {
				foreach(pPluginHooks[i], PLUGIN_HOOKDATA, it2) {
					if(it2->hDLL == it->hDLL) {
						pPluginHooks[i].erase(it2);
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

	for(int i=0; i<(int)PLUGIN_CALLBACKS_AMOUNT; i++) 
		pPluginHooks[i].clear();

	foreach(lstPlugins, PLUGIN_DATA, it)
	{
		if(it->bMayUnload)
			FreeLibrary(it->hDLL);
	}

	lstPlugins.clear();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadPlugin(const string &sFileName, CCmds* adminInterface, bool bStartup) 
{

	string sDLLName = sFileName;
	if(sDLLName.find(".dll") == string::npos)
		sDLLName.append(".dll");

	foreach(lstPlugins,PLUGIN_DATA,it) 
	{
		if(it->sDLL == sDLLName) 
			return adminInterface->Print(L"Plugin already loaded, skipping: (%s)\n", stows(it->sDLL).c_str());
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


	// plugins that may not unload are interpreted as crucial plugins that can also not be loaded after FLServer startup
	if(!plugin.bMayUnload && !bStartup) {
		adminInterface->Print(L"Error, could not load plugin (unloadable, need server restart to load): %s\n", stows(plugin.sDLL).c_str());
		throw "";
	}

	foreach(p_PI->lstHooks, PLUGIN_HOOKINFO, it)
	{
		PLUGIN_HOOKDATA hook;
		hook.sName = plugin.sShortName;
		hook.sPluginFunction = hook.sName + "-" + itos((int)it->eCallbackID);
		hook.bPaused = false;
		hook.hDLL = plugin.hDLL;
		hook.iPriority = it->iPriority;
		hook.pFunc = it->pFunc;
		hook.ePluginReturnCode = p_PI->ePluginReturnCode;
		if (!p_PI->ePluginReturnCode)
			throw "plugin return code pointer not defined";

		pPluginHooks[(int)it->eCallbackID].push_back(hook);
		pPluginHooks[(int)it->eCallbackID].sort(PLUGIN_SORTCRIT());
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
			LoadPlugin(finddata.cFileName, adminInterface, bStartup);

		} catch(char*) {};
		
	} while (FindNextFile(hfindplugins,&finddata));

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

}