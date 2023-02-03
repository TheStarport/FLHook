#include "hook.h"

#define ISERVER_LOG() \
	if (set_bDebug)   \
		AddLog(fLogDebug, __FUNCSIG__);
#define ISERVER_LOGARG_F(a) \
	if (set_bDebug)         \
		AddLog(fLogDebug, "     " #a ": %f", (float)a);
#define ISERVER_LOGARG_UI(a) \
	if (set_bDebug)          \
		AddLog(fLogDebug, "     " #a ": %u", (uint)a);
#define ISERVER_LOGARG_D(a) \
	if (set_bDebug)         \
		AddLog(fLogDebug, "     " #a ": %f", (double)a);
#define ISERVER_LOGARG_I(a) \
	if (set_bDebug)         \
		AddLog(fLogDebug, "     " #a ": %d", (int)a);
#define ISERVER_LOGARG_V(a) \
	if (set_bDebug)         \
		AddLog(fLogDebug, "     " #a ": %f %f %f", (float)a.x, (float)a.y, (float)a.z);

/**************************************************************************************************************
// flserver memory leak bugfix
**************************************************************************************************************/

int __cdecl FreeReputationVibeHook(int const& p1)
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

void __cdecl HkCb_Update_Time_AFTER(double dInterval)
{
	CALL_PLUGINS(PLUGIN_HkCb_Update_Time, (dInterval));
}

void __cdecl HkCb_Update_Time(double dInterval)
{
	CALL_PLUGINS(PLUGIN_HkCb_Update_Time, (dInterval));
	if (bPluginReturn)
		return;

	Timing::UpdateGlobalTime(dInterval);
	HkCb_Update_Time_AFTER(dInterval);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall HkCb_Elapse_Time_AFTER(float p1)
{
	CALL_PLUGINS(PLUGIN_HkCb_Elapse_Time, (p1));
}

uint iLastTicks = 0;

void __stdcall HkCb_Elapse_Time(float p1)
{
	CALL_PLUGINS(PLUGIN_HkCb_Elapse_Time, (p1));
	if (bPluginReturn)
		return;

	Server.ElapseTime(p1);
	HkCb_Elapse_Time_AFTER(p1);

	// low serverload missile jitter bugfix
	uint iCurLoad = GetTickCount() - iLastTicks;
	if (iCurLoad < 5)
	{
		uint iFakeLoad = 5 - iCurLoad;
		Sleep(iFakeLoad);
	}
	iLastTicks = GetTickCount();
}

/**************************************************************************************************************
**************************************************************************************************************/

int __cdecl HkCb_Dock_Call(
    unsigned int const& uShipID, unsigned int const& uSpaceID, int p3, enum DOCK_HOST_RESPONSE p4)
{
	//	p3 == -1, p4 -> 2 --> Dock Denied!
	//	p3 == -1, p4 -> 3 --> Dock in Use
	//	p3 != -1, p4 -> 4 --> Dock ok, proceed (p3 Dock Port?)
	//	p3 == -1, p4 -> 5 --> now DOCK!

	CALL_PLUGINS(PLUGIN_HkCb_Dock_Call, (uShipID, uSpaceID, p3, p4));
	if (bPluginReturn)
		return reinterpret_cast<int>(vPluginRet);

	try
	{
		return pub::SpaceObj::Dock(uShipID, uSpaceID, p3, p4);
	}
	catch (...)
	{
		AddLog(
		    "Exception in "
		    "SpaceObj::Dock(uShipID=%08x,uSpaceID=%08x,p3=%d,p4=%d)",
		    uShipID, uSpaceID, p3, p4);
	}
	return 0;
}

/**************************************************************************************************************
**************************************************************************************************************/

FARPROC fpOldLaunchPos;

bool __stdcall LaunchPosHook(uint iSpaceID, struct CEqObj& p1, Vector& p2, Matrix& p3, int iDock)
{
	CALL_PLUGINS(PLUGIN_LaunchPosHook, (iSpaceID, p1, p2, p3, iDock));
	if (bPluginReturn)
		return reinterpret_cast<bool>(vPluginRet);

	return p1.launch_pos(p2, p3, iDock);
}

__declspec(naked) void _HkCb_LaunchPos()
{
	__asm
	{ 
		push ecx // 4
		push [esp+8+8] // 8
		push [esp+12+4] // 12
		push [esp+16+0] // 16
		push ecx
		push [ecx+176]
		call LaunchPosHook	
		pop ecx
		ret 0x0C
	}
}
