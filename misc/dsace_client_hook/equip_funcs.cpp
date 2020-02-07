#include "dllmain.h"
#include "equip_funcs.h"

#include "FLCoreDALib.h"

extern bool set_bTurretZoomDisabled;

// Animation
#define ADDR_SHIELD	((PBYTE)  0x62b2ba1)

__declspec(naked) IObjRW* GetPlayerShip()
{
  __asm mov	eax, 0x54baf0
  __asm call eax
  __asm ret
}

__declspec(naked) CShip* GetCShip()
{
  __asm mov	eax, 0x54baf0
  __asm call	eax
  __asm test	eax, eax
  __asm jz	noship
  __asm add	eax, 12
  __asm mov	eax, [eax+4]
  noship:
  __asm ret
}

static HANDLE hTurretZoomThread = 0;
static float fDefaultTurretZoom = 0.0f;
static float fTargetTurretZoom = 0.0f;
bool bTurretMode = false;

static CRITICAL_SECTION cs;

DWORD WINAPI ZoomTurretCam(LPVOID arg)
{
	float *cam = (float*)0x00678f60;
	while (cam[0x42] != fTargetTurretZoom && bTurretMode)
	{
		float diff = cam[0x42] - fTargetTurretZoom;
		cam[0x42] = fTargetTurretZoom + (diff/2);
		Sleep(25);
	}
	EnterCriticalSection(&cs);
	CloseHandle(hTurretZoomThread);
	hTurretZoomThread = 0;
	LeaveCriticalSection(&cs);
	return 0;
}

void __stdcall InitTurretCam()
{
	float *cam = (float*)0x00678f60;
	fDefaultTurretZoom = cam[0x42];
	fTargetTurretZoom = cam[0x42];

	// Reduce screen shake when hull is hit. Modify this based on the
	// ship mass.
	CShip* cship = GetCShip();
	if (cship)
	{
		float hull_hit_shake = 0.02f / (cship->shiparch()->fMass / 100.0f);
		WriteProcMem((char*)0x51DB5C, &hull_hit_shake, 4);

		float shield_hit_shake = 0.005f / (cship->shiparch()->fMass / 100.0f);
		WriteProcMem((char*)0x51DB89, &shield_hit_shake, 4);
	}
}

void __stdcall SetCamType(const char *type)
{
	if (strcmp("turret", type)==0)
	{
		bTurretMode = true;
	}
	else
	{
		bTurretMode = false;
	}
}

void SetTurretCamZoom(float dir)
{
	if (set_bTurretZoomDisabled)
		return;

	// float *v3 = (float*)0x0067dbf8; // win camera (i.e. hud)
	// float *v3 = (float*)0x00678ba4; // cockpit cam
	// float *cam = (float*)0x00678d48; // third-person cam
	// float *cam = (float*)0x006791a4; // death cam
	if (bTurretMode)
	{
		if (dir > 0)
			fTargetTurretZoom -= fTargetTurretZoom / 3.0f;
		else
			fTargetTurretZoom += fTargetTurretZoom / 3.0f;

		if (fTargetTurretZoom < fDefaultTurretZoom)
			fTargetTurretZoom = fDefaultTurretZoom;

		else if (fTargetTurretZoom > 5000.0f)
			fTargetTurretZoom = 5000.0f;

		EnterCriticalSection(&cs);
		if (!hTurretZoomThread)
			hTurretZoomThread = CreateThread(0, 4096, ZoomTurretCam, 0, 0, 0);
		LeaveCriticalSection(&cs);
	}
}

void SetLight(bool bOn)
{
	CShip* cship = GetCShip();
	if (!cship)
		return;

	CEquipTraverser tr;
	CEquip *equip = cship->equip_manager.Traverse(tr);
	while (equip)
	{
		if (CELightEquip::cast(equip))
		{
			EquipDesc ed;
			equip->GetEquipDesc(ed);
			string hp = ToLower(ed.szHardPoint.value);
			if (hp.find("dock")!=string::npos)
				equip->Activate(bOn);
		}
		equip = cship->equip_manager.Traverse(tr);
	}
}

void SetCloak(bool bOn)
{
	CShip* cship = GetCShip();
	if (!cship)
		return;

	CEquipTraverser tr;
	CEquip *equip = cship->equip_manager.Traverse(tr);
	while (equip)
	{
		if (CECloakingDevice::cast(equip))
		{
			equip->Activate(bOn);
			return;
		}
		equip = cship->equip_manager.Traverse(tr);
	}
}


enum { CLOSED, CLOSING, OPENED, OPENING };
struct ANIM_INFO
{
	HANDLE handle;
	int old_bay_anim;
	bool abortClose;
};

static map<CShip*,ANIM_INFO> mapAnims;

// Wait for the animation to finish, remove it and restore the original.
DWORD WINAPI AnimWaitClose( LPVOID vcship )
{
	CShip* cship = (CShip*)vcship;
	bool abortClose = false;
	// Wait for the animation to complete or be aborted.
	while (cship->iBayState != CLOSED && !abortClose)
	{
		EnterCriticalSection(&cs);
		abortClose = mapAnims[cship].abortClose;
		LeaveCriticalSection(&cs);
		Sleep( 100 );
	}

	// If the thread is aborted then don't close the animation because
	// the wings are opening again but rather just destroy the thread.
	if (abortClose)
	{
		EnterCriticalSection(&cs);
		HANDLE handle = mapAnims[cship].handle;
		mapAnims[cship].handle = 0;
		LeaveCriticalSection(&cs);
		CloseHandle(handle);
	}
	// Otherwise the animation has completed. Clean up and reset the
	// animation to the bay door animation.
	else
	{
      try
      {
		   AnimDB::Rem( cship->iBayAnim );
		   DALib::Anim->Stop(cship->iBayAnim);
		   DALib::Anim->Close(cship->iBayAnim);
      }
      catch (...) {}

		EnterCriticalSection(&cs);
		HANDLE handle = mapAnims[cship].handle;
		cship->iBayAnim = mapAnims[cship].old_bay_anim;
		mapAnims.erase(cship);
		LeaveCriticalSection(&cs);
		CloseHandle(handle);
	}
	return 0;
}

void SetWings(CShip *cship, bool bOpen)
{
	int state = cship->get_bay_state();
	try
	{
		if (state == CLOSED && bOpen)
		{
		   int anim = DALib::Anim->Open(cship->ship_arch->anim, cship->obj, "Sc_extend wing", 0, 0);
		   if (anim == -1)
			   return;

		 int old_bay_anim = cship->iBayAnim;
		   cship->iBayAnim = anim;

		   *ADDR_SHIELD = 0xEB;
		   cship->open_bay();
		   *ADDR_SHIELD = 0x74;

		   EnterCriticalSection(&cs);
		   mapAnims[cship].old_bay_anim = old_bay_anim;
		   mapAnims[cship].handle = 0;
		   LeaveCriticalSection(&cs);
		}
		else if (state == CLOSING && bOpen)
		{
		   EnterCriticalSection(&cs);
		   if (mapAnims[cship].handle)
			   mapAnims[cship].abortClose = true;
		   LeaveCriticalSection(&cs);

		   *ADDR_SHIELD = 0xEB;
		   cship->open_bay();
		   *ADDR_SHIELD = 0x74;
		}
		else if ((state == OPENING || state == OPENED) && !bOpen)
		{
		   cship->close_bay();

		   EnterCriticalSection(&cs);
		   mapAnims[cship].handle = CreateThread(0, 4096, AnimWaitClose, cship, 0, 0);
		   LeaveCriticalSection(&cs);
		}
	} catch (...) {}
}

PUINT const pSysSwitch = (PUINT)0x67977c;
PUINT const pShipLaunchCallBack = (PUINT)(0x62b2a80+1);
static UINT targetSystem = 0;

__declspec(naked) void ChangeSystemNaked()
{
  __asm push	0x62aff80
  __asm push	ecx

  *pSysSwitch = targetSystem;

  *pShipLaunchCallBack = 0xffffd4fb;

  __asm pop	ecx
 
  // Force the station type, since docking rings cause additional problems.
  __asm mov	dword ptr [ecx+0x28], 0x100
  __asm ret
}

void ForceChangeSystem(DWORD sys)
{
	// If not in space then emulate a ship launch key
	if (!GetCShip())
	{
		targetSystem = sys;
		*pShipLaunchCallBack = (DWORD)ChangeSystemNaked - (DWORD)pShipLaunchCallBack - 4;
		__asm
		{
			push	eax
			push	offset done
			sub	esp, 0x38
			push	ebx
			push	ebp
			push	esi
			push	edi
			mov	eax, 0x440f94
			jmp	eax
		}
	}
	else
	{
		targetSystem = 0;
		*pSysSwitch = sys;
	}
	done:
	return;
}


static FARPROC PatchCallAddr(char *hMod, DWORD dwInstallAddress, char *dwHookFunction)
{
	DWORD dwRelAddr;
	ReadProcMem(hMod + dwInstallAddress + 1, &dwRelAddr, 4);

	DWORD dwOffset = (DWORD)dwHookFunction - (DWORD)(hMod + dwInstallAddress + 5);
	WriteProcMem(hMod + dwInstallAddress + 1, &dwOffset, 4);

	return (FARPROC)(hMod + dwRelAddr + dwInstallAddress + 5);
}

__declspec(naked) void HkCb_AnimHook()
{
   __asm
   {
      mov     eax, [ecx+18h]
      test    eax, eax
      jz      bad
      fld     dword ptr [eax+1Ch]
      retn
bad:
      fld      ds:0x662920C
      retn
   }
}

void InitEquipFuncs()
{
	InitializeCriticalSection(&cs);

	DWORD dummy;
	VirtualProtect(ADDR_SHIELD, 1, PAGE_EXECUTE_READWRITE, &dummy );

	HMODULE hEngBase = GetModuleHandleA("engbase.dll");
	PatchCallAddr((char*)hEngBase, 0x48b4, (char*)HkCb_AnimHook);
}

