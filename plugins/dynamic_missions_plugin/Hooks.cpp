// includes 
#include "../flhookplugin_sdk/headers/FLHook.h"
#include "../flhookplugin_sdk/headers/plugin.h"


#include <math.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


EXPORT PLUGIN_RETURNCODE returncode;

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode()
{
	return returncode;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

struct TRIGGER_ARG
{
	uint iTriggerHash;
	uint iTriggerHash2;
	uint iClientID;
};

struct ACTION_DEBUGMSG_DATA
{
	void* freeFunc;
	void* mission_struct;
	uint iTriggerHash;
	uint iDunno;
	char szMessage[255];
};

void __stdcall HkActTrigger(TRIGGER_ARG* trigger)
{
	wstring msg = L"Mission trigger activated with hash id: " + stows(itos(trigger->iTriggerHash)) + L"\n";
	ConPrint(msg);
	HkMsgU(msg);
}

char* pAddressTriggerAct;

__declspec(naked) void _HkActTrigger()
{
	__asm
	{
		push ecx
		push [esp+8]
		call HkActTrigger
		pop ecx
		// original func instructions
		push ebx
		push esi
		mov esi, [esp + 8 + 4]
		push edi
		mov eax, [pAddressTriggerAct]
		add eax, 7
		jmp eax
	}
}

int __stdcall HkActDebugMsg(ACTION_DEBUGMSG_DATA* action_dbgMsg)
{
	wstring msg = L"Mission trigger (" + stows(itos(action_dbgMsg->iTriggerHash)) + L") sent debug msg: " + stows(string(action_dbgMsg->szMessage)) + L"\n";
	ConPrint(msg);
	HkMsgU(msg);

	return 1;
}

__declspec(naked) void _HkActDebugMsg()
{
	__asm
	{
		push ecx
		call HkActDebugMsg
		mov eax, 1
		ret
	}
}

int __cdecl HkCreateSolar(uint &iSpaceID, pub::SpaceObj::SolarInfo &solarInfo)
{
	// hack server.dll so it does not call create solar packet send

    char* serverHackAddress = (char*)hModServer + 0x2A62A;
    char serverHack[] = {'\xEB'};
    WriteProcMem(serverHackAddress, &serverHack, 1);

	// create it
	int returnVal = pub::SpaceObj::CreateSolar(iSpaceID, solarInfo);

	uint dunno;
	IObjInspectImpl* inspect;
	if(GetShipInspect(iSpaceID, inspect, dunno))
	{
		CSolar* solar = (CSolar*)inspect->cobject();

		// for every player in the same system, send solar creation packet

		struct SOLAR_STRUCT
		{
			byte dunno[0x100];
		};

		SOLAR_STRUCT packetSolar;

		char* address1 = (char*)hModServer + 0x163F0;
		char* address2 = (char*)hModServer + 0x27950;

		// fill struct
		__asm
		{
			lea ecx, packetSolar
			mov eax, address1
			call eax
			push solar
			lea ecx, packetSolar
			push ecx
			mov eax, address2
			call eax
			add esp, 8
		}

		struct PlayerData *pPD = 0;
		while(pPD = Players.traverse_active(pPD))
		{
			if(pPD->iSystemID == solarInfo.iSystemID)
				GetClientInterface()->Send_FLPACKET_SERVER_CREATESOLAR(pPD->iOnlineID, (FLPACKET_CREATESOLAR&)packetSolar);
		}

	}

	 // undo the server.dll hack
    char serverUnHack[] = {'\x74'};	
    WriteProcMem(serverHackAddress, &serverUnHack, 1);

	return returnVal;
}

bool HkSinglePlayer()
{
	return true;
}

EXPORT void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;

/*	// patch cheat detection
	char* pAddress = ((char*)hModServer + 0x6CD44);
	char szJMP[] = { '\xEB' };
	WriteProcMem(pAddress, szJMP, 1);
	
	pAddress = ((char*)hModCommon + 0x13E678);
	float fJumpinDistance = 30000;
	WriteProcMem(pAddress, &fJumpinDistance, 4);*/


	// patch singleplayer check in Player pos calculation
	char* pAddress = ((char*)GetModuleHandle("content.dll") + 0xD3B0D);
	char szNOP[] = { '\x90', '\x90','\x90','\x90','\x90','\x90'};
	WriteProcMem(pAddress, szNOP, 6);

	// install hook for trigger events

	pAddressTriggerAct = ((char*)GetModuleHandle("content.dll") + 0x182C0);
	FARPROC fpTF = (FARPROC)_HkActTrigger;
	char szMovEDX[] = { '\xBA' };
	char szJmpEDX[] = { '\xFF', '\xE2' };

	WriteProcMem(pAddressTriggerAct, szMovEDX, 1);
	WriteProcMem(pAddressTriggerAct + 1, &fpTF, 4);
	WriteProcMem(pAddressTriggerAct + 5, szJmpEDX, 2);

	// hook debug msg
	char* pAddressActDebugMsg = ((char*)GetModuleHandle("content.dll") + 0x115BC4);
	FARPROC fpADbg = (FARPROC)_HkActDebugMsg;
	WriteProcMem(pAddressActDebugMsg, &fpADbg, 4);

	// hook solar creation to fix fl-bug in MP where loadout is not sent
	char* pAddressCreateSolar = ((char*)GetModuleHandle("content.dll") + 0x1134D4);
	FARPROC fpHkCreateSolar = (FARPROC)HkCreateSolar;
	WriteProcMem(pAddressCreateSolar, &fpHkCreateSolar, 4);

	/*
	// hook singleplayer check
	char* pAddressSP = ((char*)GetModuleHandle("content.dll") + 0x1136A0);
	FARPROC fpHkSP = (FARPROC)HkSinglePlayer;
	WriteProcMem(pAddressSP, &fpHkSP, 4);
	*/
}

namespace HkIEngine
{    

	EXPORT void __stdcall CShip_init(CShip* ship)
	{
		returncode = DEFAULT_RETURNCODE;

		

	}


	EXPORT void __stdcall CShip_destroy(CShip* ship)
	{
		returncode = DEFAULT_RETURNCODE;

		
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint iControllerID = 0;

HK_ERROR HkTest(uint iTest)
{
	
	if(iControllerID)
		return HKE_UNKNOWN_ERROR;

	char* pAddress = (char*)hModContent + 0x114388;
	pub::Controller::CreateParms params = {pAddress, 1};
	iControllerID = pub::Controller::Create("Content.dll", "Mission_01a", &params, (pub::Controller::PRIORITY)2);
	pub::Controller::_SendMessage(iControllerID, 0x1000, 0);
	
	return HKE_OK;
}

HK_ERROR HkTest2(uint iTest)
{
	if(!iControllerID)
		return HKE_UNKNOWN_ERROR;

	pub::Controller::Destroy(iControllerID);
	iControllerID = 0;

	return HKE_OK;
}

void CmdTest(CCmds* classptr, uint iTest)
{

	// right check
	if(!(classptr->rights & RIGHT_SUPERADMIN)) { classptr->Print(L"ERR No permission\n"); return;}

	if(((classptr->hkLastErr = HkTest(iTest)) == HKE_OK)) // hksuccess 
		classptr->Print(L"OK\n");
	else
		classptr->PrintError();
}

void CmdTest2(CCmds* classptr, uint iTest)
{

	// right check
	if(!(classptr->rights & RIGHT_SUPERADMIN)) { classptr->Print(L"ERR No permission\n"); return;}

	if(((classptr->hkLastErr = HkTest2(iTest)) == HKE_OK)) // hksuccess 
		classptr->Print(L"OK\n");
	else
		classptr->PrintError();
}

#define IS_CMD(a) !wscCmd.compare(L##a)

EXPORT bool ExecuteCommandString_Callback(CCmds* classptr, const wstring& wscCmd)
{
	returncode = NOFUNCTIONCALL;  // flhook needs to care about our return code

	if(IS_CMD("startmission")) {

		returncode = SKIPPLUGINS_NOFUNCTIONCALL; // do not let other plugins kick in since we now handle the command

		CmdTest(classptr, classptr->ArgInt(1));

		return true;
	}

	if(IS_CMD("endmission")) {

		returncode = SKIPPLUGINS_NOFUNCTIONCALL; // do not let other plugins kick in since we now handle the command

		CmdTest2(classptr, classptr->ArgInt(1));

		return true;
	}

    return false;
}

EXPORT void CmdHelp_Callback(CCmds* classptr)
{
	returncode = DEFAULT_RETURNCODE;

	classptr->Print(L"startmission\n");
	classptr->Print(L"endmission\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "Dynamic Missions Plugin by w0dk4";
	p_PI->sShortName = "dynmissions";
	p_PI->bMayPause = false;
	p_PI->bMayUnload = false;
	p_PI->ePluginReturnCode = &returncode;
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkIEngine::CShip_init, PLUGIN_HkIEngine_CShip_init, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkIEngine::CShip_destroy, PLUGIN_HkIEngine_CShip_destroy, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&ExecuteCommandString_Callback, PLUGIN_ExecuteCommandString_Callback, 0));
	p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&CmdHelp_Callback, PLUGIN_CmdHelp_Callback, 0));
	return p_PI;
}
