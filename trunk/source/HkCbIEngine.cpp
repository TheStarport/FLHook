#include "hook.h"

#define ISERVER_LOG() if(set_bDebug) AddDebugLog(__FUNCSIG__);
#define ISERVER_LOGARG_F(a) if(set_bDebug) AddDebugLog("     " #a ": %f", (float)a);
#define ISERVER_LOGARG_UI(a) if(set_bDebug) AddDebugLog("     " #a ": %u", (uint)a);
#define ISERVER_LOGARG_D(a) if(set_bDebug) AddDebugLog("     " #a ": %f", (double)a);
#define ISERVER_LOGARG_I(a) if(set_bDebug) AddDebugLog("     " #a ": %d", (int)a);
#define ISERVER_LOGARG_V(a) if(set_bDebug) AddDebugLog("     " #a ": %f %f %f", (float)a.x, (float)a.y, (float)a.z);


/**************************************************************************************************************
// misc flserver engine function hooks
**************************************************************************************************************/

namespace HkIEngine
{

/**************************************************************************************************************
// ship create & destroy
**************************************************************************************************************/

FARPROC fpOldInitCShip;
FARPROC fpOldDestroyCShip;
FARPROC fpOldLoadRepCharFile;

void __stdcall CShip_init(CShip* ship)
{
	CALL_PLUGINS_V(PLUGIN_HkIEngine_CShip_init,__stdcall,(CShip*),(ship));
}


__declspec(naked) void _CShip_init()
{
	__asm
	{
		push ecx
		push [esp+8]
		call fpOldInitCShip
		call CShip_init
		ret 4
	}
}

void __stdcall CShip_destroy(CShip* ship)
{
	CALL_PLUGINS_V(PLUGIN_HkIEngine_CShip_destroy,__stdcall,(CShip*),(ship));
}

__declspec(naked) void _CShip_destroy()
{
	__asm
	{
		push ecx
		push ecx
		call CShip_destroy
		pop ecx
		jmp fpOldDestroyCShip
	}
}



/**************************************************************************************************************
// flserver memory leak bugfix
**************************************************************************************************************/

int __cdecl FreeReputationVibe(int const &p1)
{

	__asm 
	{
		mov eax, p1
		push eax
		mov eax, [hModServer]
		add eax, 0x65C20
		call eax
		add esp, 4
	}

	return Reputation::Vibe::Free(p1);

}

/**************************************************************************************************************
**************************************************************************************************************/

void __cdecl Update_Time(double dInterval)
{

	CALL_PLUGINS_V(PLUGIN_HkCb_Update_Time,,(double),(dInterval));

	Timing::UpdateGlobalTime(dInterval);
	
	CALL_PLUGINS_V(PLUGIN_HkCb_Update_Time_AFTER,,(double),(dInterval));
}

/**************************************************************************************************************
**************************************************************************************************************/


uint iLastTicks = 0;

void __stdcall Elapse_Time(float p1)
{

	CALL_PLUGINS_V(PLUGIN_HkCb_Elapse_Time,__stdcall,(float),(p1));

	Server.ElapseTime(p1);

	CALL_PLUGINS_V(PLUGIN_HkCb_Elapse_Time_AFTER,__stdcall,(float),(p1));

	// low serverload missile jitter bugfix
	uint iCurLoad = GetTickCount() - iLastTicks;
	if(iCurLoad < 5) {
		uint iFakeLoad = 5 - iCurLoad;
		Sleep(iFakeLoad);
	}
	iLastTicks = GetTickCount();

}

/**************************************************************************************************************
**************************************************************************************************************/

int __cdecl Dock_Call(unsigned int const &uShipID,unsigned int const &uSpaceID,int p3,enum DOCK_HOST_RESPONSE p4)
{
	
	//	p3 == -1, p4 -> 2 --> Dock Denied!
	//	p3 == -1, p4 -> 3 --> Dock in Use
	//	p3 != -1, p4 -> 4 --> Dock ok, proceed (p3 Dock Port?)
	//	p3 == -1, p4 -> 5 --> now DOCK!

	CALL_PLUGINS(PLUGIN_HkCb_Dock_Call,int,,(unsigned int const &,unsigned int const &,int,DOCK_HOST_RESPONSE),(uShipID,uSpaceID,p3,p4));

	TRY_HOOK {
		return pub::SpaceObj::Dock(uShipID,uSpaceID,p3,p4);
	} CATCH_HOOK({})

	CALL_PLUGINS(PLUGIN_HkCb_Dock_Call_AFTER, int, , (unsigned int const &, unsigned int const &, int, DOCK_HOST_RESPONSE), (uShipID, uSpaceID, p3, p4));

	return 0;
}


/**************************************************************************************************************
**************************************************************************************************************/

FARPROC fpOldLaunchPos;

bool __stdcall LaunchPos(uint iSpaceID, struct CEqObj &p1, Vector &p2, Matrix &p3, int iDock)
{
	
	CALL_PLUGINS(PLUGIN_LaunchPosHook,bool,__stdcall,(uint,CEqObj &,Vector &,Matrix &,int),(iSpaceID,p1,p2,p3,iDock));

	return p1.launch_pos(p2,p3,iDock);
	
}


__declspec(naked) void _LaunchPos()
{
	__asm
	{ 
		push ecx //4
		push [esp+8+8] //8
		push [esp+12+4] //12
		push [esp+16+0] //16
		push ecx
		push [ecx+176]
		call LaunchPos	
		pop ecx
		ret 0x0C
	}

}

/**************************************************************************************************************
**************************************************************************************************************/

struct LOAD_REP_DATA
{
	uint iRepID;
	float fAttitude;
};

struct REP_DATA_LIST
{
	uint iDunno;
	LOAD_REP_DATA* begin;
	LOAD_REP_DATA* end;
};

bool __stdcall HkLoadRepFromCharFile(REP_DATA_LIST* savedReps, LOAD_REP_DATA* repToSave)
{
	// check of the rep id is valid
	if(repToSave->iRepID == 0xFFFFFFFF)
		return false; // rep id not valid!

	LOAD_REP_DATA* repIt = savedReps->begin;

	while(repIt != savedReps->end)
	{
		if(repIt->iRepID == repToSave->iRepID)
			return false; // we already saved this rep!

		repIt++;
	}

	// everything seems fine, add
	return true;
}

__declspec(naked) void _HkLoadRepFromCharFile()
{
	__asm
	{ 
		push ecx // save ecx because thiscall
		push [esp+4+4+8] // rep data
		push ecx // rep data list
		call HkLoadRepFromCharFile
		pop ecx // recover ecx
		test al, al
		jz abort_lbl
		jmp [fpOldLoadRepCharFile]
abort_lbl:
		ret 0x0C
	}

}


/**************************************************************************************************************
**************************************************************************************************************/


}